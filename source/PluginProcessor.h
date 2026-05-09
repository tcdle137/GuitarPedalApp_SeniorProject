#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

struct EffectProcessor
{
    virtual ~EffectProcessor() = default;
    virtual void prepare(const juce::dsp::ProcessSpec& spec) {}
    virtual void reset() {}
    virtual void process(juce::dsp::AudioBlock<float>& block, juce::AudioProcessorValueTreeState& apvts) = 0;
};

struct OverdriveProcessor : EffectProcessor
{
    juce::dsp::IIR::Filter<float> toneFilter; // Low-pass filter for tone control
    double sampleRate = 48000.0;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        toneFilter.prepare(spec);
        sampleRate = spec.sampleRate;
        
        // Initialize tone filter as low-pass at 6kHz (neutral position)
        toneFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            sampleRate, 6000.0f);
    }
    
    void process(juce::dsp::AudioBlock<float>& block, juce::AudioProcessorValueTreeState& apvts) override
    {
        auto* driveParam = apvts.getRawParameterValue("OverdriveDrive");
        auto* toneParam = apvts.getRawParameterValue("OverdriveTone");
        auto* levelParam = apvts.getRawParameterValue("OverdriveLevel");
        
        float drive = driveParam ? driveParam->load() : 2.0f;
        float tone = toneParam ? toneParam->load() : 0.5f;
        float level = levelParam ? levelParam->load() : 1.0f;
        
        // Apply tone filter (low-pass) to shape high frequencies before overdrive
        // Tone parameter: 0.0 = dark (more low-pass filtering), 1.0 = bright (less filtering)
        float toneFreq = juce::jmap(tone, 0.0f, 1.0f, 800.0f, 5000.0f);
        toneFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            sampleRate, toneFreq);
        
        // Apply tone filter
        juce::dsp::ProcessContextReplacing<float> toneContext(block);
        toneFilter.process(toneContext);
        
        // Apply overdrive after tone filtering
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
        {
            auto* data = block.getChannelPointer(ch);
            for (size_t i = 0; i < block.getNumSamples(); ++i)
            {
                // Apply drive
                float driven = data[i] * drive;
                
                // Smoother asymmetric clipping for warm tube-like distortion
                float saturated;
                if (driven > 0.0f)
                {
                    // Positive half-wave: gentle soft clipping
                    saturated = (driven / (1.0f + driven * 0.3f)) * 1.2f;
                    saturated = std::tanh(saturated * 0.8f); // Reduced saturation
                }
                else
                {
                    // Negative half-wave: slightly softer clipping
                    saturated = (driven / (1.0f - driven * 0.2f)) * 0.9f;
                    saturated = std::tanh(saturated * 0.7f);
                }
                
                // Add subtle harmonics for warmth (reduced amount)
                float harmonic = saturated * saturated * 0.05f; // Less 2nd harmonic
                saturated += harmonic;
                
                // Gentle limiting to smooth out peaks
                saturated = std::tanh(saturated * 0.9f);
                
                // Apply level
                data[i] = saturated * level;
            }
        }
    }
};

struct DistortionProcessor : EffectProcessor
{
    juce::dsp::IIR::Filter<float> toneFilter; // Low-pass filter for tone control
    double sampleRate = 48000.0;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        toneFilter.prepare(spec);
        sampleRate = spec.sampleRate;
        
        // Initialize tone filter as low-pass at 5kHz (neutral position)
        toneFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            sampleRate, 5000.0f);
    }
    
    void process(juce::dsp::AudioBlock<float>& block, juce::AudioProcessorValueTreeState& apvts) override
    {
        auto* driveParam = apvts.getRawParameterValue("DistortionDrive");
        auto* toneParam = apvts.getRawParameterValue("DistortionTone");
        auto* levelParam = apvts.getRawParameterValue("DistortionLevel");
        
        float drive = driveParam ? driveParam->load() : 2.0f;
        float tone = toneParam ? toneParam->load() : 0.5f;
        float level = levelParam ? levelParam->load() : 0.5f;
        
        // Apply tone filter (low-pass) to shape high frequencies before distortion
        // Tone parameter: 0.0 = dark (more low-pass filtering), 1.0 = bright (less filtering)
        float toneFreq = juce::jmap(tone, 0.0f, 1.0f, 500.0f, 5000.0f);
        toneFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            sampleRate, toneFreq);
        
        // Apply tone filter
        juce::dsp::ProcessContextReplacing<float> toneContext(block);
        toneFilter.process(toneContext);
        
        // Apply distortion after tone filtering
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
        {
            auto* data = block.getChannelPointer(ch);
            for (size_t i = 0; i < block.getNumSamples(); ++i)
            {
                // Apply drive (hard clipping distortion)
                float driven = data[i] * drive;
                float distorted = juce::jlimit(-1.0f, 1.0f, driven);
                
                // Apply level
                data[i] = distorted * level;
            }
        }
    }
};

struct ReverbProcessor : EffectProcessor
{
    juce::dsp::Reverb reverb;
    juce::dsp::DelayLine<float> preDelay { 48000 * 2 };
    juce::dsp::IIR::Filter<float> toneFilter; // Low-pass filter for tone control
    double sampleRate = 48000.0;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        reverb.prepare(spec);
        preDelay.prepare(spec);
        toneFilter.prepare(spec);
        sampleRate = spec.sampleRate;
        
        // Initialize tone filter as low-pass at 10kHz (neutral position)
        toneFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            sampleRate, 10000.0f);
    }

    void process(juce::dsp::AudioBlock<float>& block, juce::AudioProcessorValueTreeState& apvts) override
    {
        auto* preDelayParam = apvts.getRawParameterValue("ReverbPreDelay");
        auto* decayParam = apvts.getRawParameterValue("ReverbDecay");
        auto* sizeParam = apvts.getRawParameterValue("ReverbSize");
        auto* toneParam = apvts.getRawParameterValue("ReverbTone");
        auto* mixParam = apvts.getRawParameterValue("ReverbMix");
        
        float preDelayTime = preDelayParam ? preDelayParam->load() : 0.03f;
        float decay = decayParam ? decayParam->load() : 1.5f;
        float size = sizeParam ? sizeParam->load() : 0.5f;
        float tone = toneParam ? toneParam->load() : 0.5f;
        float mix = mixParam ? mixParam->load() : 0.3f;
        
        // Set pre-delay
        preDelay.setDelay(preDelayTime * static_cast<float>(sampleRate));
        
        // Apply pre-delay
        juce::dsp::ProcessContextReplacing<float> preDelayContext(block);
        preDelay.process(preDelayContext);
        
        // Configure reverb parameters
        auto params = reverb.getParameters();
        params.roomSize = size;
        params.damping = juce::jlimit(0.0f, 1.0f, 1.0f - (decay / 5.0f)); // Map decay (0.1-5.0) to damping (0.0-1.0)
        params.wetLevel = mix;
        params.dryLevel = 1.0f - mix;
        params.width = 1.0f;
        reverb.setParameters(params);

        // Apply reverb
        juce::dsp::ProcessContextReplacing<float> reverbContext(block);
        reverb.process(reverbContext);
        
        // Apply tone filter (low-pass) to shape reverb high frequencies
        // Tone parameter: 0.0 = dark (more low-pass filtering), 1.0 = bright (less filtering)
        float toneFreq = juce::jmap(tone, 0.0f, 1.0f, 1000.0f, 10000.0f);
        toneFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            sampleRate, toneFreq);
        juce::dsp::ProcessContextReplacing<float> toneContext(block);
        toneFilter.process(toneContext);
    }
};

struct DelayProcessor : EffectProcessor
{
    std::vector<std::vector<float>> delayBuffer;
    std::vector<int> writePos;
    int maxDelaySamples = 48000 * 2; // 2 seconds max delay at 48kHz
    double sampleRate = 48000.0;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;
        maxDelaySamples = static_cast<int>(sampleRate * 2.0); // 2 seconds max delay
        
        // Initialize delay buffers for each channel
        delayBuffer.resize(spec.numChannels);
        writePos.resize(spec.numChannels);
        
        for (size_t ch = 0; ch < spec.numChannels; ++ch)
        {
            delayBuffer[ch].resize(maxDelaySamples, 0.0f);
            writePos[ch] = 0;
        }
    }

    void reset() override
    {
        for (auto& channel : delayBuffer)
        {
            std::fill(channel.begin(), channel.end(), 0.0f);
        }
        std::fill(writePos.begin(), writePos.end(), 0);
    }

    void setSampleRate(float newSampleRate)
    {
        sampleRate = newSampleRate;
        maxDelaySamples = static_cast<int>(sampleRate * 2.0); // 2 seconds max delay
    }

    void process(juce::dsp::AudioBlock<float>& block, juce::AudioProcessorValueTreeState& apvts) override
    {
        auto* timeParam = apvts.getRawParameterValue("DelayTime");
        auto* feedbackParam = apvts.getRawParameterValue("DelayFeedback");
        auto* mixParam = apvts.getRawParameterValue("DelayMix");
        
        float time = timeParam ? timeParam->load() : 0.3f;
        float feedback = juce::jlimit(0.0f, 0.95f, feedbackParam ? feedbackParam->load() : 0.3f);
        float mix = mixParam ? mixParam->load() : 0.3f;
        
        int delaySamples = static_cast<int>(time * static_cast<float>(sampleRate));
        delaySamples = juce::jlimit(0, maxDelaySamples - 1, delaySamples);
        
        auto numChannels = block.getNumChannels();
        auto numSamples = block.getNumSamples();
        
        // Store original dry signal
        juce::AudioBuffer<float> dryBuffer(static_cast<int>(numChannels), static_cast<int>(numSamples));
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* channelData = block.getChannelPointer(ch);
            dryBuffer.copyFrom(static_cast<int>(ch), 0, channelData, static_cast<int>(numSamples));
        }
        
        // Process digital delay with proper feedback
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* channelData = block.getChannelPointer(ch);
            auto& channelBuffer = delayBuffer[ch];
            int& writeIndex = writePos[ch];
            
            for (size_t i = 0; i < numSamples; ++i)
            {
                // Calculate read position for delayed signal
                int readIndex = writeIndex - delaySamples;
                if (readIndex < 0)
                    readIndex += maxDelaySamples;
                
                // Get delayed sample from buffer
                float delayedSample = channelBuffer[readIndex];
                
                // Calculate output with feedback
                float inputSample = channelData[i];
                float outputSample = inputSample + delayedSample;
                
                // Apply feedback and store in delay buffer
                channelBuffer[writeIndex] = outputSample * feedback;
                
                // Update channel data with processed signal
                channelData[i] = outputSample;
                
                // Advance write position
                writeIndex = (writeIndex + 1) % maxDelaySamples;
            }
        }
        
        // Mix wet and dry signals
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* channelData = block.getChannelPointer(ch);
            auto* dryData = dryBuffer.getReadPointer(static_cast<int>(ch));
            
            for (size_t i = 0; i < numSamples; ++i)
            {
                channelData[i] = dryData[i] * (1.0f - mix) + channelData[i] * mix;
            }
        }
    }
};

struct ChorusProcessor : EffectProcessor
{
    juce::dsp::Chorus<float> chorus;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        chorus.prepare(spec);
    }

    void setDepth(float depth)
    {
        chorus.setDepth(depth);
    }

    void process(juce::dsp::AudioBlock<float>& block, juce::AudioProcessorValueTreeState& apvts) override
    {
        auto* timeParam = apvts.getRawParameterValue("ChorusTime");
        auto* depthParam = apvts.getRawParameterValue("ChorusDepth");
        auto* mixParam = apvts.getRawParameterValue("ChorusMix");
        
        float time = timeParam ? timeParam->load() : 1.0f;
        float depth = depthParam ? depthParam->load() : 0.5f;
        float mix = mixParam ? mixParam->load() : 0.3f;
        
        // Store original dry signal for mixing
        auto numChannels = block.getNumChannels();
        auto numSamples = block.getNumSamples();
        
        juce::AudioBuffer<float> dryBuffer(static_cast<int>(numChannels), static_cast<int>(numSamples));
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* channelData = block.getChannelPointer(ch);
            dryBuffer.copyFrom(static_cast<int>(ch), 0, channelData, static_cast<int>(numSamples));
        }
        
        // Set chorus parameters
        chorus.setRate(time);
        chorus.setDepth(depth);
        
        // Apply chorus effect
        juce::dsp::ProcessContextReplacing<float> context(block);
        chorus.process(context);
        
        // Mix wet and dry signals
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* channelData = block.getChannelPointer(ch);
            auto* dryData = dryBuffer.getReadPointer(static_cast<int>(ch));
            for (size_t i = 0; i < numSamples; ++i)
            {
                channelData[i] = dryData[i] * (1.0f - mix) + channelData[i] * mix;
            }
        }
    }
};

struct EQProcessor : EffectProcessor
{
    juce::dsp::ProcessorChain<
        juce::dsp::IIR::Filter<float>,  // Low pass filter (bass control)
        juce::dsp::IIR::Filter<float>,  // Mid peak filter  
        juce::dsp::IIR::Filter<float>   // High pass filter (treble control)
    > eqChain;
    
    double sampleRate = 48000.0;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        eqChain.prepare(spec);
        sampleRate = spec.sampleRate;
        
        // Set up initial filter coefficients
        auto& lowFilter = eqChain.get<0>();
        auto& midFilter = eqChain.get<1>();
        auto& highFilter = eqChain.get<2>();
        
        // Low pass filter at 400 Hz (bass control)
        lowFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            sampleRate, 400.0f);
        
        // Mid peak at 800 Hz with Q=1.0
        midFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate, 800.0f, 1.0f, 1.0f);
        
        // High pass filter at 1200 Hz (treble control)
        highFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(
            sampleRate, 1200.0f);
    }

    void process(juce::dsp::AudioBlock<float>& block, juce::AudioProcessorValueTreeState& apvts) override
    {
        auto* lowParam = apvts.getRawParameterValue("EQLowGain");
        auto* midParam = apvts.getRawParameterValue("EQMidGain");
        auto* highParam = apvts.getRawParameterValue("EQHighGain");
        
        float lowGain = lowParam ? lowParam->load() : 1.0f;
        float midGain = midParam ? midParam->load() : 1.0f;
        float highGain = highParam ? highParam->load() : 1.0f;
        
        // Update filter coefficients with new gain values
        auto& lowFilter = eqChain.get<0>();
        auto& midFilter = eqChain.get<1>();
        auto& highFilter = eqChain.get<2>();
        
        // Update low pass filter frequency based on gain (more gain = lower cutoff = more bass)
        float lowFreq = juce::jmap(lowGain, 0.1f, 2.0f, 800.0f, 100.0f);
        auto lowCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            sampleRate, lowFreq);
        lowFilter.coefficients = lowCoeffs;
        
        // Update mid peak filter
        auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate, 800.0f, 1.0f, midGain);
        midFilter.coefficients = midCoeffs;
        
        // Update high pass filter frequency based on gain (more gain = higher cutoff = more treble)
        float highFreq = juce::jmap(highGain, 0.1f, 2.0f, 800.0f, 3000.0f);
        auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(
            sampleRate, highFreq);
        highFilter.coefficients = highCoeffs;
        
        // Process audio through the EQ chain
        juce::dsp::ProcessContextReplacing<float> context(block);
        eqChain.process(context);
    }
};

struct PreampProcessor : EffectProcessor
{
    juce::dsp::ProcessorChain<
        juce::dsp::IIR::Filter<float>,  // Low pass filter (bass control)
        juce::dsp::IIR::Filter<float>,  // Mid peak filter
        juce::dsp::IIR::Filter<float>,  // High pass filter (treble control)
        juce::dsp::IIR::Filter<float>   // Presence high pass filter
    > preampEQ;

    double sampleRate = 48000.0;

    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;
        preampEQ.prepare(spec);

        // Set up initial filter coefficients
        auto& bassFilter = preampEQ.get<0>();
        auto& midFilter = preampEQ.get<1>();
        auto& trebleFilter = preampEQ.get<2>();
        auto& presenceFilter = preampEQ.get<3>();

        // Low-frequency shelving filter at 200 Hz (bass control)
        bassFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            sampleRate, 200.0f, 0.7f, 1.0f);

        // Mid peak at 800 Hz with Q=1.0
        midFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate, 800.0f, 1.0f, 1.0f);

        // High-frequency shelving filter at 3000 Hz (treble control)
        trebleFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate, 3000.0f, 0.7f, 1.0f);

        // Presence high-frequency shelving filter at 5000 Hz
        presenceFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate, 5000.0f, 0.7f, 1.0f);
    }

    void process(juce::dsp::AudioBlock<float>& block, juce::AudioProcessorValueTreeState& apvts) override
    {
        auto* presenceParam = apvts.getRawParameterValue("PreampPresence");
        auto* bassParam = apvts.getRawParameterValue("PreampBass");
        auto* midParam = apvts.getRawParameterValue("PreampMid");
        auto* trebleParam = apvts.getRawParameterValue("PreampTreble");
        auto* gainParam = apvts.getRawParameterValue("PreampGain");
        auto* volumeParam = apvts.getRawParameterValue("PreampVolume");

        float presence = presenceParam ? presenceParam->load() : 1.0f;
        float bass = bassParam ? bassParam->load() : 1.0f;
        float mid = midParam ? midParam->load() : 1.0f;
        float treble = trebleParam ? trebleParam->load() : 1.0f;
        float gain = gainParam ? gainParam->load() : 1.0f;
        float volume = volumeParam ? volumeParam->load() : 1.0f;

        // Update filter coefficients with new gain values
        auto& bassFilter = preampEQ.get<0>();
        auto& midFilter = preampEQ.get<1>();
        auto& trebleFilter = preampEQ.get<2>();
        auto& presenceFilter = preampEQ.get<3>();

        // Update low-frequency shelving filter for bass control
        auto bassCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            sampleRate, 200.0f, 0.7f, bass);
        bassFilter.coefficients = bassCoeffs;

        // Update mid peak filter
        auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            sampleRate, 800.0f, 1.0f, mid);
        midFilter.coefficients = midCoeffs;

        // Update high-frequency shelving filter for treble control
        auto trebleCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate, 3000.0f, 0.7f, treble);
        trebleFilter.coefficients = trebleCoeffs;

        // Update presence high-frequency shelving filter
        auto presenceCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
            sampleRate, 5000.0f, 0.7f, presence);
        presenceFilter.coefficients = presenceCoeffs;

        // Apply EQ
        juce::dsp::ProcessContextReplacing<float> eqContext(block);
        preampEQ.process(eqContext);

        // Apply gain and volume
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
        {
            auto* data = block.getChannelPointer(ch);
            for (size_t i = 0; i < block.getNumSamples(); ++i)
            {
                // Apply gain boost/overdrive
                float driven = data[i] * gain;

                // Apply soft saturation for tube-like warmth
                float saturated = static_cast<float>(std::tanh(driven * 0.7f));

                // Apply volume
                data[i] = saturated * volume;
            }
        }
    }
};

struct CompressorProcessor : EffectProcessor
{
    juce::dsp::Compressor<float> compressor;
    double sampleRate = 48000.0;
    
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        compressor.prepare(spec);
        sampleRate = spec.sampleRate;
    }

    void process(juce::dsp::AudioBlock<float>& block, juce::AudioProcessorValueTreeState& apvts) override
    {
        auto* thresholdParam = apvts.getRawParameterValue("CompressorThreshold");
        auto* ratioParam = apvts.getRawParameterValue("CompressorRatio");
        auto* attackParam = apvts.getRawParameterValue("CompressorAttack");
        auto* releaseParam = apvts.getRawParameterValue("CompressorRelease");
        auto* makeupGainParam = apvts.getRawParameterValue("CompressorMakeupGain");
        
        float threshold = thresholdParam ? thresholdParam->load() : -10.0f;
        float ratio = ratioParam ? ratioParam->load() : 4.0f;
        float attack = attackParam ? attackParam->load() : 3.0f;
        float release = releaseParam ? releaseParam->load() : 100.0f;
        float makeupGain = makeupGainParam ? makeupGainParam->load() : 0.0f;
        
        // Set compressor parameters
        compressor.setThreshold(threshold);
        compressor.setRatio(ratio);
        compressor.setAttack(attack);
        compressor.setRelease(release);
        
        // Apply compression
        juce::dsp::ProcessContextReplacing<float> context(block);
        compressor.process(context);
        
        // Apply makeup gain
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
        {
            auto* data = block.getChannelPointer(ch);
            auto gain = juce::Decibels::decibelsToGain(makeupGain);
            for (size_t i = 0; i < block.getNumSamples(); ++i)
            {
                data[i] *= gain;
            }
        }
    }
};

struct TunerProcessor : EffectProcessor
{
    double sampleRate = 48000.0;
    float currentFrequency = 0.0f;
    float currentNote = 0.0f;
    bool isProcessing = false;
    
    // Frequency smoothing to prevent stuck readings
    float smoothedFrequency = 0.0f;
    const float smoothingFactor = 0.7f; // 0.7 = moderate smoothing
    
    // FFT parameters
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int bufferSize = fftSize * 2;
    
    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;
    
    std::array<float, bufferSize> fftBuffer {};
    std::array<float, fftSize> magnitudeSpectrum {};
    int bufferWritePos = 0;
    
    // Noise gate parameters (hardcoded)
    float gateThreshold = 0.01f;  // Threshold for noise gate (-40dB)
    float gateReleaseTime = 0.1f;  // Release time in seconds
    float gateAttackTime = 0.001f; // Attack time in seconds
    float gateEnvelope = 0.0f;     // Current envelope level
    float gateSampleRate = 48000.0f;
    bool gateIsOpen = false;
    
    TunerProcessor() : fft(fftOrder), window(fftSize, juce::dsp::WindowingFunction<float>::hann)
    {
        std::fill(fftBuffer.begin(), fftBuffer.end(), 0.0f);
        std::fill(magnitudeSpectrum.begin(), magnitudeSpectrum.end(), 0.0f);
    }
    
    void prepare(const juce::dsp::ProcessSpec& spec) override
    {
        sampleRate = spec.sampleRate;
        gateSampleRate = static_cast<float>(spec.sampleRate);
        bufferWritePos = 0;
        gateEnvelope = 0.0f;
        gateIsOpen = false;
        std::fill(fftBuffer.begin(), fftBuffer.end(), 0.0f);
        std::fill(magnitudeSpectrum.begin(), magnitudeSpectrum.end(), 0.0f);
    }
    
    bool processNoiseGate(float sample)
    {
        // Calculate envelope using rectification and smoothing
        float absSample = std::abs(sample);
        
        // Attack and release coefficients
        float attackCoeff = std::exp(-1.0f / (gateAttackTime * gateSampleRate));
        float releaseCoeff = std::exp(-1.0f / (gateReleaseTime * gateSampleRate));
        
        // Update envelope
        if (absSample > gateEnvelope)
            gateEnvelope = absSample + (gateEnvelope - absSample) * attackCoeff;
        else
            gateEnvelope = absSample + (gateEnvelope - absSample) * releaseCoeff;
        
        // Determine gate state
        gateIsOpen = gateEnvelope > gateThreshold;
        
        return gateIsOpen;
    }

    void process(juce::dsp::AudioBlock<float>& block, juce::AudioProcessorValueTreeState& apvts) override
    {
        auto* bypassParam = apvts.getRawParameterValue("TunerBypass");
        bool isBypassed = bypassParam ? bypassParam->load() > 0.5f : false;
        
        if (isBypassed)
            return;
            
        isProcessing = true;
        
        // Process audio for tuning with noise gate
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
        {
            auto* channelData = block.getChannelPointer(ch);
            
            for (size_t i = 0; i < block.getNumSamples(); ++i)
            {
                // Process noise gate for this sample
                bool gateOpen = processNoiseGate(channelData[i]);
                
                // Only add samples to FFT buffer if noise gate is open
                if (gateOpen && bufferWritePos < bufferSize)
                {
                    fftBuffer[bufferWritePos] = channelData[i];
                    bufferWritePos++;
                }
                
                // When buffer is full, perform FFT
                if (bufferWritePos >= bufferSize)
                {
                    performFFT(apvts);
                    bufferWritePos = 0;
                }
            }
        }
        
        // If gate is closed, don't clear display immediately (let it decay naturally)
        // This prevents flickering when signal is borderline
        
        // Silence the audio output when tuner is active (not bypassed)
        // This prevents tuner audio from reaching subsequent effects
        for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
        {
            auto* channelData = block.getChannelPointer(ch);
            std::fill(channelData, channelData + block.getNumSamples(), 0.0f);
        }
    }
    
    void performFFT(juce::AudioProcessorValueTreeState& apvts)
    {
        // Apply windowing
        window.multiplyWithWindowingTable(fftBuffer.data(), fftSize);
        
        // Perform FFT
        fft.performFrequencyOnlyForwardTransform(fftBuffer.data());
        
        // Copy to magnitude spectrum
        std::copy(fftBuffer.begin(), fftBuffer.begin() + fftSize, magnitudeSpectrum.begin());
        
        // Find fundamental frequency
        findFundamentalFrequency(apvts);
    }
    
    void findFundamentalFrequency(juce::AudioProcessorValueTreeState& apvts)
    {
        float maxMagnitude = 0.0f;
        
        // Search for peak in frequency spectrum with improved algorithm
        // Ignore very low frequencies and focus on guitar range (80Hz - 2000Hz)
        int minBin = juce::jlimit(10, static_cast<int>(80.0f * fftSize / sampleRate), fftSize / 2);
        int maxBin = juce::jlimit(minBin, static_cast<int>(2000.0f * fftSize / sampleRate), fftSize / 2);
        
        for (int bin = minBin; bin <= maxBin; ++bin)
        {
            if (magnitudeSpectrum[bin] > maxMagnitude)
            {
                maxMagnitude = magnitudeSpectrum[bin];
                maxBin = bin;
            }
        }
        
        // Additional validation: ensure detected frequency is reasonable for guitar
        float detectedFrequency = maxBin * (static_cast<float>(sampleRate) / fftSize);
        if (detectedFrequency < 80.0f || detectedFrequency > 2000.0f)
        {
            DBG("Tuner: Frequency out of guitar range, ignoring");
            return; // Skip this update
        }
        
        // Convert bin to frequency
        float binWidth = static_cast<float>(sampleRate) / fftSize;
        detectedFrequency = maxBin * binWidth;
        
        // Apply frequency smoothing to prevent stuck readings
        smoothedFrequency = smoothedFrequency * smoothingFactor + detectedFrequency * (1.0f - smoothingFactor);
        currentFrequency = smoothedFrequency;
        
        // Debug output
        DBG("Tuner: maxBin=" << maxBin << ", rawFreq=" << detectedFrequency << ", smoothFreq=" << smoothedFrequency << ", maxMag=" << maxMagnitude);
        
        // Get user-adjustable reference frequency from UI
        auto* refFreqParam = apvts.getRawParameterValue("TunerReferenceFreq");
        float referenceFreq = refFreqParam ? refFreqParam->load() : 440.0f;
        
        // Convert frequency to note number (MIDI note format) using user's reference frequency
        currentNote = 69.0f + 12.0f * std::log2f(currentFrequency / referenceFreq);
    }
    
    float getFrequency() const { return currentFrequency; }
    float getNote() const { return currentNote; }
    bool getIsProcessing() const { return isProcessing; }
    
    float getCentsDeviation(juce::AudioProcessorValueTreeState& apvts) const
    {
        if (currentFrequency <= 0.0f || currentNote < 0.0f || currentNote > 127.0f)
            return 0.0f;
        
        // Get user-adjustable reference frequency from UI
        auto* refFreqParam = apvts.getRawParameterValue("TunerReferenceFreq");
        float referenceFreq = refFreqParam ? refFreqParam->load() : 440.0f;
        
        // Calculate the frequency of the nearest semitone using user's reference frequency
        float nearestNote = std::round(currentNote);
        float nearestFreq = referenceFreq * std::pow(2.0f, (nearestNote - 69.0f) / 12.0f);
        
        // Calculate cents deviation (1200 cents = 1 octave)
        if (nearestFreq > 0.0f)
            return 1200.0f * std::log2f(currentFrequency / nearestFreq);
        
        return 0.0f;
    }
    
    enum TuningStatus { Flat, InTune, Sharp };
    TuningStatus getTuningStatus(juce::AudioProcessorValueTreeState& apvts) const
    {
        float cents = getCentsDeviation(apvts);
        if (cents < -5.0f) return Flat;
        else if (cents > 5.0f) return Sharp;
        else return InTune;
    }
    
    juce::String getNoteName() const
    {
        if (currentNote < 0.0f || currentNote > 127.0f)
            return "---";
        
        static const juce::String noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        int noteIndex = static_cast<int>(std::round(currentNote)) % 12;
        int octave = static_cast<int>(std::round(currentNote) / 12) - 1;
        
        return noteNames[noteIndex] + juce::String(octave);
    }
};

//=====================================================================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    
    // Preset management using JUCE program system
    void saveCurrentProgramAsPreset(int presetNumber);
    void loadPreset(int presetNumber);
    juce::String getPresetFilePath(int presetNumber);
    static const int NUM_PRESETS = 10;
    std::array<juce::String, NUM_PRESETS> presetNames;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    //TunerProcessor* getTunerProcessor() { return dynamic_cast<TunerProcessor*>(effectModules[7].get()); }

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;
    std::vector<std::unique_ptr<EffectProcessor>> effectModules;
    std::unique_ptr<PreampProcessor> preampProcessor;
    
    // Preset management
    int currentPreset = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};