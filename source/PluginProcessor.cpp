#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Initialize preset names
    for (int i = 0; i < NUM_PRESETS; ++i)
        presetNames[i] = "Preset " + juce::String(i + 1);
    
    effectModules.reserve(6);
    effectModules.emplace_back(std::make_unique<OverdriveProcessor>());
    effectModules.emplace_back(std::make_unique<DistortionProcessor>());
    effectModules.emplace_back(std::make_unique<ReverbProcessor>());
    effectModules.emplace_back(std::make_unique<DelayProcessor>());
    effectModules.emplace_back(std::make_unique<ChorusProcessor>());
    effectModules.emplace_back(std::make_unique<CompressorProcessor>());
    
    preampProcessor = std::make_unique<PreampProcessor>();
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    // Return the maximum tail length from effects (e.g., reverb decay)
    // For simplicity, estimate based on reverb (can be longer)
    return 2.0; // 2 seconds for reverb tail
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return NUM_PRESETS;
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return currentPreset;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    if (index >= 0 && index < NUM_PRESETS)
    {
        currentPreset = index;
        loadPreset(index);
    }
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    if (index >= 0 && index < NUM_PRESETS)
        return presetNames[index];
    return "Preset " + juce::String(index + 1);
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    if (index >= 0 && index < NUM_PRESETS)
    {
        presetNames[index] = newName;
        
        // Rename the preset file
        juce::File oldFile(getPresetFilePath(index));
        juce::File newFile(getPresetFilePath(index));
        
        if (oldFile.existsAsFile())
        {
            auto state = juce::XmlDocument::parse(oldFile);
            if (state != nullptr)
            {
                newFile.replaceWithText(state->toString());
                oldFile.deleteFile();
            }
        }
    }
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    // Log buffer size for debugging latency
    DBG("Sample Rate: " << sampleRate << ", Buffer Size: " << samplesPerBlock << ", Latency: " << (samplesPerBlock / sampleRate * 1000.0) << " ms");

    // Prepare modular effect processors
    juce::dsp::ProcessSpec spec { static_cast<float>(sampleRate), static_cast<juce::uint32>(samplesPerBlock), 2 };
    for (auto& effect : effectModules)
        effect->prepare(spec);
    
    // Prepare preamp processor
    preampProcessor->prepare(spec);

    if (auto* delayProcessor = dynamic_cast<DelayProcessor*>(effectModules[3].get()))
        delayProcessor->setSampleRate(static_cast<float>(sampleRate));

    if (auto* chorusProcessor = dynamic_cast<ChorusProcessor*>(effectModules[4].get()))
        chorusProcessor->setDepth(0.1f);

    if (auto* reverbProcessor = dynamic_cast<ReverbProcessor*>(effectModules[2].get()))
    {
        auto params = reverbProcessor->reverb.getParameters();
        params.roomSize = 0.5f;
        params.damping = 0.5f;
        params.wetLevel = 0.5f;
        params.dryLevel = 0.5f;
        params.width = 1.0f;
        reverbProcessor->reverb.setParameters(params);
    }

    // Set up EQ (simple filters)
    /*
    eqLow.setCoefficients(juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 200.0f));
    eqMid.setCoefficients(juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 1000.0f, 1.0f, 1.0f));
    eqHigh.setCoefficients(juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 5000.0f));
    */
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get current order
    std::vector<int> currentOrder;
    for (int i = 0; i < 6; ++i)
    {
        auto* param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("Order" + juce::String(i + 1)));
        currentOrder.push_back(param->getIndex());
    }

    // Create DSP context
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    // Process preamp first (always processes before any effects)
    preampProcessor->process(block, apvts);

    // Apply effects in order
    // Tuner and EQ are partially integrated, but abandoned due to redundency and scope of the project
    juce::StringArray effectNames = {"Overdrive", "Distortion", "Reverb", "Delay", "Chorus", "Compressor"};
    for (int effectIdx : currentOrder)
    {
        if (effectIdx >= 0 && effectIdx < static_cast<int>(effectModules.size()))
        {
            // Check if effect is bypassed
            auto* bypassParam = apvts.getRawParameterValue(effectNames[effectIdx] + "Bypass");
            bool isBypassed = bypassParam ? bypassParam->load() > 0.5f : false;

            if (!isBypassed)
                effectModules[effectIdx]->process(block, apvts);
        }
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save the current state of all parameters
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore the saved state of all parameters
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

void AudioPluginAudioProcessor::saveCurrentProgramAsPreset(int presetNumber)
{
    if (presetNumber < 0 || presetNumber >= NUM_PRESETS)
        return;
        
    // Save current state to preset file
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    
    juce::File presetFile(getPresetFilePath(presetNumber));
    if (presetFile.create())
    {
        presetFile.replaceWithText(xml->toString());
        presetNames[presetNumber] = "Preset " + juce::String(presetNumber + 1);
    }
}

void AudioPluginAudioProcessor::loadPreset(int presetNumber)
{
    if (presetNumber < 0 || presetNumber >= NUM_PRESETS)
        return;
        
    // Load from preset file
    juce::File presetFile(getPresetFilePath(presetNumber));
    if (presetFile.existsAsFile())
    {
        std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(presetFile));
        
        if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
            currentPreset = presetNumber;
        }
    }
}

juce::String AudioPluginAudioProcessor::getPresetFilePath(int presetNumber)
{
    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
           .getChildFile("LeFXGuitarPedal")
           .getChildFile("Presets")
           .getChildFile("Preset" + juce::String(presetNumber + 1).paddedLeft('0', 2) + ".xml")
           .getFullPathName();
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    juce::StringArray choices = {"Overdrive", "Distortion", "Reverb", "Delay", "Chorus", "Compressor"};
    
    // Set default order: Tuner first, then rest in original order
    juce::StringArray defaultOrder = {"Overdrive", "Distortion", "Reverb", "Delay", "Chorus", "Compressor"};
    
    for (int i = 0; i < 6; ++i)
    {
        // Map default order to parameter indices
        juce::String effectName = defaultOrder[i];
        int choiceIndex = choices.indexOf(effectName);
        layout.add(std::make_unique<juce::AudioParameterChoice>("Order" + juce::String(i + 1), "Order " + juce::String(i + 1), choices, choiceIndex));
    }

    // Effect on/off parameters
    layout.add(std::make_unique<juce::AudioParameterBool>("OverdriveBypass", "Overdrive Bypass", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("DistortionBypass", "Distortion Bypass", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("ReverbBypass", "Reverb Bypass", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("DelayBypass", "Delay Bypass", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("ChorusBypass", "Chorus Bypass", false));
    //layout.add(std::make_unique<juce::AudioParameterBool>("EQBypass", "EQ Bypass", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("CompressorBypass", "Compressor Bypass", false));
    //layout.add(std::make_unique<juce::AudioParameterBool>("TunerBypass", "Tuner Bypass", false));

    // Effect parameters
    //Overdrive
    layout.add(std::make_unique<juce::AudioParameterFloat>("OverdriveDrive", "Overdrive Drive", 0.01f, 10.0f, 2.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("OverdriveTone", "Overdrive Tone", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("OverdriveLevel", "Overdrive Level", 0.01f, 1.0f, 0.5f));
    //Distortion
    layout.add(std::make_unique<juce::AudioParameterFloat>("DistortionDrive", "Distortion Drive", 0.01f, 10.0f, 2.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DistortionTone", "Distortion Tone", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DistortionLevel", "Distortion Level", 0.01f, 1.0f, 0.5f));
    //Reverb
    layout.add(std::make_unique<juce::AudioParameterFloat>("ReverbPreDelay", "Reverb Pre-Delay", 0.0f, 0.1f, 0.03f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ReverbDecay", "Reverb Decay", 0.1f, 5.0f, 1.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ReverbSize", "Reverb Size", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ReverbTone", "Reverb Tone", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ReverbMix", "Reverb Mix", 0.0f, 1.0f, 0.3f));
    //Delay
    layout.add(std::make_unique<juce::AudioParameterFloat>("DelayTime", "Delay Time", 0.01f, 2.0f, 0.3f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DelayFeedback", "Delay Feedback", 0.0f, 0.95f, 0.3f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DelayMix", "Delay Mix", 0.0f, 1.0f, 0.3f));
    //Chorus
    layout.add(std::make_unique<juce::AudioParameterFloat>("ChorusTime", "Chorus Time", 0.1f, 5.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ChorusDepth", "Chorus Depth", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ChorusMix", "Chorus Mix", 0.0f, 1.0f, 0.3f));
    //EQ
    //layout.add(std::make_unique<juce::AudioParameterFloat>("EQLowGain", "EQ Low Gain", 0.1f, 2.0f, 1.0f));
    //layout.add(std::make_unique<juce::AudioParameterFloat>("EQMidGain", "EQ Mid Gain", 0.1f, 2.0f, 1.0f));
    //layout.add(std::make_unique<juce::AudioParameterFloat>("EQHighGain", "EQ High Gain", 0.1f, 2.0f, 1.0f));
    //Compressor
    layout.add(std::make_unique<juce::AudioParameterFloat>("CompressorThreshold", "Compressor Threshold", -60.0f, 0.0f, -10.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("CompressorRatio", "Compressor Ratio", 1.0f, 20.0f, 4.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("CompressorAttack", "Compressor Attack", 0.1f, 100.0f, 3.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("CompressorRelease", "Compressor Release", 10.0f, 1000.0f, 100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("CompressorMakeupGain", "Compressor Makeup Gain", 0.0f, 20.0f, 0.0f));
    //Tuner
    //layout.add(std::make_unique<juce::AudioParameterFloat>("TunerReferenceFreq", "Tuner Reference Freq", 430.0f, 450.0f, 440.0f));

    // Preamp parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>("PreampPresence", "Preamp Presence", 0.1f, 2.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("PreampBass", "Preamp Bass", 0.1f, 2.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("PreampMid", "Preamp Mid", 0.1f, 2.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("PreampTreble", "Preamp Treble", 0.1f, 2.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("PreampGain", "Preamp Gain", 0.5f, 5.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("PreampVolume", "Preamp Volume", 0.01f, 2.0f, 0.1f));

    return layout;
}