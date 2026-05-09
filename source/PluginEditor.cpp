#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

//==============================================================================
// Custom LookAndFeel implementation
void CustomLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle,
                                        juce::Slider& slider)
{
    // Load the knob image from BinaryData
    static juce::Image knobImage = juce::ImageCache::getFromMemory(
        BinaryData::knob_png, BinaryData::knob_pngSize);
    
    if (knobImage.isValid())
    {
        // Calculate the rotation angle based on slider position
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        
        // Use a square area for the knob to maintain aspect ratio
        auto knobSize = juce::jmin(width, height);
        auto destX = x + (width - knobSize) / 2.0f;
        auto destY = y + (height - knobSize) / 2.0f;
        
        // Create transform: scale to fit the square area, rotate around center, then position
        juce::AffineTransform transform = juce::AffineTransform::scale((float)knobSize / knobImage.getWidth(), 
                                                                      (float)knobSize / knobImage.getHeight())
            .rotated(angle, knobSize / 2.0f, knobSize / 2.0f)
            .translated(destX, destY);
        
        // Draw the transformed knob image
        g.drawImageTransformed(knobImage, transform);
    }
    else
    {
        // Fallback to default drawing if image fails to load
        juce::LookAndFeel_V4::drawRotarySlider(g, x, y, width, height, sliderPos, rotaryStartAngle, rotaryEndAngle, slider);
    }
}

//==============================================================================
PreampComponent::PreampComponent(AudioPluginAudioProcessorEditor& editor)
    : editorRef(editor)
{
    // Load custom font from BinaryData
    static juce::Font customFont = juce::Font(juce::Typeface::createSystemTypefaceFor(
        BinaryData::SairaRegular_ttf, BinaryData::SairaRegular_ttfSize));
    
    // Setup title label
    preampTitleLabel.setFont(customFont.withHeight(36.0f));
    preampTitleLabel.setText("PREAMP", juce::dontSendNotification);
    preampTitleLabel.setJustificationType(juce::Justification::centred);
    preampTitleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(preampTitleLabel);
    
    auto& apvts = editorRef.getAPVTS();

    // Setup presence slider
    presenceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    presenceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    presenceSlider.setLookAndFeel(&editorRef.getCustomLookAndFeel());
    addAndMakeVisible(presenceSlider);

    presenceLabel.setFont(customFont.withHeight(24.0f));
    presenceLabel.setText("Presence", juce::dontSendNotification);
    presenceLabel.setJustificationType(juce::Justification::centred);
    presenceLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(presenceLabel);

    presenceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "PreampPresence", presenceSlider);

    // Setup bass slider
    bassSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    bassSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    bassSlider.setLookAndFeel(&editorRef.getCustomLookAndFeel());
    addAndMakeVisible(bassSlider);

    bassLabel.setFont(customFont.withHeight(24.0f));
    bassLabel.setText("Bass", juce::dontSendNotification);
    bassLabel.setJustificationType(juce::Justification::centred);
    bassLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(bassLabel);

    bassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "PreampBass", bassSlider);

    // Setup mid slider
    midSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    midSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    midSlider.setLookAndFeel(&editorRef.getCustomLookAndFeel());
    addAndMakeVisible(midSlider);

    midLabel.setFont(customFont.withHeight(24.0f));
    midLabel.setText("Mid", juce::dontSendNotification);
    midLabel.setJustificationType(juce::Justification::centred);
    midLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(midLabel);

    midAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "PreampMid", midSlider);

    // Setup treble slider
    trebleSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    trebleSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    trebleSlider.setLookAndFeel(&editorRef.getCustomLookAndFeel());
    addAndMakeVisible(trebleSlider);

    trebleLabel.setFont(customFont.withHeight(24.0f));
    trebleLabel.setText("Treble", juce::dontSendNotification);
    trebleLabel.setJustificationType(juce::Justification::centred);
    trebleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(trebleLabel);

    trebleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "PreampTreble", trebleSlider);

    // Setup gain slider
    gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    gainSlider.setLookAndFeel(&editorRef.getCustomLookAndFeel());
    addAndMakeVisible(gainSlider);

    gainLabel.setFont(customFont.withHeight(24.0f));
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(gainLabel);

    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "PreampGain", gainSlider);

    // Setup volume slider
    volumeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    volumeSlider.setLookAndFeel(&editorRef.getCustomLookAndFeel());
    addAndMakeVisible(volumeSlider);

    volumeLabel.setFont(customFont.withHeight(24.0f));
    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.setJustificationType(juce::Justification::centred);
    volumeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(volumeLabel);

    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "PreampVolume", volumeSlider);
}

void PreampComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Try to load preamp_bg.jpg from BinaryData first (most portable)
    static juce::Image preampBg = juce::ImageCache::getFromMemory(
        BinaryData::preamp_bg_jpg, BinaryData::preamp_bg_jpgSize);

    // Fallback to file system if BinaryData not available
    if (!preampBg.isValid())
    {
        preampBg = juce::ImageFileFormat::loadFrom(
            juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                .getParentDirectory().getParentDirectory().getParentDirectory()
                .getChildFile("UI/Colours/preamp_bg.jpg"));
    }

    if (preampBg.isValid())
    {
        g.drawImage(preampBg, bounds);
    }
    else
    {
        // Fallback to a gradient background if image fails to load
        juce::ColourGradient gradient(juce::Colours::darkgrey.darker(0.8f), 0, 0,
                                     juce::Colours::black, 0, bounds.getHeight(), false);
        g.setGradientFill(gradient);
        g.fillAll();
    }

    // Draw border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(bounds.reduced(1.0f), 2.0f);
}

void PreampComponent::resized()
{
    auto area = getLocalBounds().reduced(8);
    auto titleArea = area.removeFromTop(30);
    preampTitleLabel.setBounds(titleArea);
    
    // Remove some space for the controls
    area.removeFromTop(25);
    
    // Calculate knob size
    auto knobSize = juce::jlimit(60, 80, static_cast<int>(getHeight() * 0.15f));
    auto spacing = (area.getWidth() - knobSize * 6) / 7;

    // Center the knob group horizontally
    int totalKnobWidth = knobSize * 6 + spacing * 5; // 5 gaps between 6 knobs
    int startX = (area.getWidth() - totalKnobWidth) / 2;
    area.removeFromLeft(startX);

    // Position all knobs in a single row
    auto presenceArea = area.removeFromLeft(knobSize);
    area.removeFromLeft(spacing);
    auto bassArea = area.removeFromLeft(knobSize);
    area.removeFromLeft(spacing);
    auto midArea = area.removeFromLeft(knobSize);
    area.removeFromLeft(spacing);
    auto trebleArea = area.removeFromLeft(knobSize);
    area.removeFromLeft(spacing);
    auto gainArea = area.removeFromLeft(knobSize);
    area.removeFromLeft(spacing);
    auto volumeArea = area.removeFromLeft(knobSize);

    // Position knobs at the top of their areas
    auto presenceKnobArea = presenceArea.removeFromTop(knobSize);
    presenceSlider.setBounds(presenceKnobArea.withSizeKeepingCentre(knobSize, knobSize));
    presenceLabel.setBounds(presenceArea.removeFromTop(20));

    auto bassKnobArea = bassArea.removeFromTop(knobSize);
    bassSlider.setBounds(bassKnobArea.withSizeKeepingCentre(knobSize, knobSize));
    bassLabel.setBounds(bassArea.removeFromTop(20));

    auto midKnobArea = midArea.removeFromTop(knobSize);
    midSlider.setBounds(midKnobArea.withSizeKeepingCentre(knobSize, knobSize));
    midLabel.setBounds(midArea.removeFromTop(20));

    auto trebleKnobArea = trebleArea.removeFromTop(knobSize);
    trebleSlider.setBounds(trebleKnobArea.withSizeKeepingCentre(knobSize, knobSize));
    trebleLabel.setBounds(trebleArea.removeFromTop(20));

    auto gainKnobArea = gainArea.removeFromTop(knobSize);
    gainSlider.setBounds(gainKnobArea.withSizeKeepingCentre(knobSize, knobSize));
    gainLabel.setBounds(gainArea.removeFromTop(20));

    auto volumeKnobArea = volumeArea.removeFromTop(knobSize);
    volumeSlider.setBounds(volumeKnobArea.withSizeKeepingCentre(knobSize, knobSize));
    volumeLabel.setBounds(volumeArea.removeFromTop(20));
}

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    std::vector<int> order;
    auto& apvts = processorRef.getAPVTS();
    for (int i = 0; i < 6; ++i)
    {
        auto* param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("Order" + juce::String(i + 1)));
        order.push_back(param->getIndex());
    }
    effectsList = std::make_unique<EffectsList>(*this, order);
    addAndMakeVisible(*effectsList);
    
    // Create and add preamp component
    preampComponent = std::make_unique<PreampComponent>(*this);
    addAndMakeVisible(*preampComponent);

    setSize(1545, 605);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll(juce::Colour::fromString("FF303030"));
    /*
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
    */
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    
    // Reserve space for preamp at the top (150 pixels height)
    auto preampArea = bounds.removeFromTop(175);
    preampComponent->setBounds(preampArea);
    
    // Add 10px buffer between preamp and effects
    bounds.removeFromTop(10);
    
    // Effects list takes the remaining space
    effectsList->setBounds(bounds);
}

//==============================================================================

EffectComponent::EffectComponent(EffectsList& list, const juce::String& n, int idx)
    : name(n), index(idx), listRef(list)
{
    // Load custom font from BinaryData
    static juce::Font customFont = juce::Font(juce::Typeface::createSystemTypefaceFor(
        BinaryData::SairaRegular_ttf, BinaryData::SairaRegular_ttfSize));
    
    // Setup bypass button
    //bypassButton.setFont(customFont.withHeight(24.0f));
    bypassButton.setButtonText("Bypass");
    bypassButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(bypassButton);

    titleLabel.setFont(customFont.withHeight(32.0f));
    titleLabel.setText(name, juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    //titleLabel.setColour(juce::Label::backgroundColourId, juce::Colours::white);
    titleLabel.setInterceptsMouseClicks(true, false);
    titleLabel.setOpaque(false);
    titleLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(titleLabel);
    titleLabel.addMouseListener(this, false);

    auto& apvts = listRef.editorRef.getAPVTS();

    // Setup bypass attachment
    juce::String bypassParamID = name + "Bypass";
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, bypassParamID, bypassButton);

    auto addParam = [&](juce::Slider& slider, juce::Label& label, const juce::String& paramID, const juce::String& labelText,
                        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
        slider.setLookAndFeel(&listRef.editorRef.getCustomLookAndFeel()); // Apply custom LookAndFeel
        addAndMakeVisible(slider);
        
        label.setFont(customFont.withHeight(24.0f));
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(label);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID, slider);
        ++numParams;
    };

    if (name == "Overdrive")
    {
        addParam(param1Slider, param1Label, "OverdriveDrive", "Drive", param1Attachment);
        addParam(param2Slider, param2Label, "OverdriveTone", "Tone", param2Attachment);
        addParam(param3Slider, param3Label, "OverdriveLevel", "Level", param3Attachment);
    }
    else if (name == "Distortion")
    {
        addParam(param1Slider, param1Label, "DistortionDrive", "Drive", param1Attachment);
        addParam(param2Slider, param2Label, "DistortionTone", "Tone", param2Attachment);
        addParam(param3Slider, param3Label, "DistortionLevel", "Level", param3Attachment);
    }
    else if (name == "Reverb")
    {
        addParam(param1Slider, param1Label, "ReverbPreDelay", "Pre-Delay", param1Attachment);
        addParam(param2Slider, param2Label, "ReverbDecay", "Decay", param2Attachment);
        addParam(param3Slider, param3Label, "ReverbSize", "Size", param3Attachment);
        addParam(param4Slider, param4Label, "ReverbTone", "Tone", param4Attachment);
        addParam(param5Slider, param5Label, "ReverbMix", "Mix", param5Attachment);
    }
    else if (name == "Delay")
    {
        addParam(param1Slider, param1Label, "DelayTime", "Time", param1Attachment);
        addParam(param2Slider, param2Label, "DelayFeedback", "Feedback", param2Attachment);
        addParam(param3Slider, param3Label, "DelayMix", "Mix", param3Attachment);
    }
    else if (name == "Chorus")
    {
        addParam(param1Slider, param1Label, "ChorusTime", "Time", param1Attachment);
        addParam(param2Slider, param2Label, "ChorusDepth", "Depth", param2Attachment);
        addParam(param3Slider, param3Label, "ChorusMix", "Mix", param3Attachment);
    }
    /*
    else if (name == "EQ")
    {
        addParam(param1Slider, param1Label, "EQLowGain", "Low", param1Attachment);
        addParam(param2Slider, param2Label, "EQMidGain", "Mid", param2Attachment);
        addParam(param3Slider, param3Label, "EQHighGain", "High", param3Attachment);
    }
    */
    else if (name == "Compressor")
    {
        addParam(param1Slider, param1Label, "CompressorThreshold", "Threshold", param1Attachment);
        addParam(param2Slider, param2Label, "CompressorRatio", "Ratio", param2Attachment);
        addParam(param3Slider, param3Label, "CompressorAttack", "Attack", param3Attachment);
        addParam(param4Slider, param4Label, "CompressorRelease", "Release", param4Attachment);
        addParam(param5Slider, param5Label, "CompressorMakeupGain", "Makeup", param5Attachment);
    }
    /*
    else if (name == "Tuner")
    {
        addParam(param1Slider, param1Label, "TunerReferenceFreq", "Ref Freq", param1Attachment);
        
        // Setup note display label
        noteDisplayLabel.setFont(customFont.withHeight(48.0f));
        noteDisplayLabel.setText("---", juce::dontSendNotification);
        noteDisplayLabel.setJustificationType(juce::Justification::centred);
        noteDisplayLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(noteDisplayLabel);
        
        // Setup frequency display label
        frequencyDisplayLabel.setFont(customFont.withHeight(24.0f));
        frequencyDisplayLabel.setText("0.0 Hz", juce::dontSendNotification);
        frequencyDisplayLabel.setJustificationType(juce::Justification::centred);
        frequencyDisplayLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        addAndMakeVisible(frequencyDisplayLabel);
        
        // Setup tuning status label
        tuningStatusLabel.setFont(customFont.withHeight(18.0f));
        tuningStatusLabel.setText("---", juce::dontSendNotification);
        tuningStatusLabel.setJustificationType(juce::Justification::centred);
        tuningStatusLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);
        addAndMakeVisible(tuningStatusLabel);
        
        // Start timer to update display
        startTimerHz(60); // Update 60 times per second for smoother display
    }
    */
}

void EffectComponent::resized()
{
    juce::Rectangle<int> area = getLocalBounds().reduced(8);
    juce::Rectangle<int> titleArea = area.removeFromTop(30);
    titleLabel.setBounds(titleArea);

    // Add buffer space between title and parameters
    area.removeFromTop(10); // 10 pixels buffer

    // Reserve space for bypass button at bottom
    auto buttonArea = area.removeFromBottom(35);
    bypassButton.setBounds(buttonArea.reduced(5));

    if (numParams == 0)
        return;

    auto controlArea = area.reduced(6);
    auto labelHeight = 20;
    // Calculate knob size - 1.5x bigger for better visibility
    auto knobSize = juce::jlimit(75, 100, static_cast<int>(getHeight() * 0.18f));
    auto sliderHeight = knobSize;

    /*
    if (numParams == 1)
    {
        // Special case for Tuner: layout with note display
        if (name == "Tuner")
        {
            // Note display area (large text) - now at the top
            auto noteArea = controlArea.removeFromTop(80);
            noteDisplayLabel.setBounds(noteArea);
            
            // Space between note and frequency
            controlArea.removeFromTop(10);
            
            // Frequency display area
            auto freqArea = controlArea.removeFromTop(30);
            frequencyDisplayLabel.setBounds(freqArea);
            
            // Space between frequency and tuning status
            controlArea.removeFromTop(10);
            
            // Tuning status display area
            auto tuningStatusArea = controlArea.removeFromTop(25);
            tuningStatusLabel.setBounds(tuningStatusArea);
            
            // Space for reference frequency knob
            controlArea.removeFromTop(20);
            
            // Reserve space for reference frequency knob at the bottom
            auto refFreqArea = controlArea.removeFromTop(100);
            auto knobArea = refFreqArea.removeFromTop(knobSize);
            auto centeredKnobArea = knobArea.withSizeKeepingCentre(knobSize, knobSize);
            param1Slider.setBounds(centeredKnobArea);
            param1Label.setBounds(refFreqArea);
        }
        else
        {
            auto paramArea = controlArea.reduced(6);
            // Use smaller area for knob to reduce gap
            auto knobArea = paramArea.removeFromTop(knobSize + 4); // Only 4px buffer
            auto centeredKnobArea = knobArea.withSizeKeepingCentre(knobSize, knobSize);
            param1Slider.setBounds(centeredKnobArea);
            param1Label.setBounds(paramArea);
        }
    }
    else */if (numParams == 2)
    {
        auto leftArea = controlArea.removeFromLeft(controlArea.getWidth() / 2).reduced(6);
        auto rightArea = controlArea.reduced(6);
        
        // Center knobs in their respective areas with reduced gap
        auto leftKnobArea = leftArea.removeFromTop(knobSize + 4); // Only 4px buffer
        auto rightKnobArea = rightArea.removeFromTop(knobSize + 4); // Only 4px buffer
        param1Slider.setBounds(leftKnobArea.withSizeKeepingCentre(knobSize, knobSize));
        param2Slider.setBounds(rightKnobArea.withSizeKeepingCentre(knobSize, knobSize));
        param1Label.setBounds(leftArea);
        param2Label.setBounds(rightArea);
    }
    else if (numParams == 3)
    {
        // Special case for EQ: arrange all 3 knobs in a single row, centered
        if (name == "EQ")
        {
            // Calculate total width needed for 3 knobs with spacing
            int totalKnobWidth = knobSize * 3 + 8 * 2; // 3 knobs + 8px spacing between each
            int startX = (controlArea.getWidth() - totalKnobWidth) / 2;

            // Calculate total height needed for knob + label
            int totalHeight = knobSize + 25; // knob + buffer for label
            int startY = (controlArea.getHeight() - totalHeight) / 2;
            
            // Remove left margin to center the knob group
            controlArea.removeFromLeft(startX);
            // Remove top space to center vertically
            controlArea.removeFromTop(startY);

            // Create 3 equal columns for the knobs
            int sliceWidth = totalKnobWidth / 3;
            auto leftArea = controlArea.removeFromLeft(sliceWidth).reduced(2);
            auto middleArea = controlArea.removeFromLeft(sliceWidth).reduced(2);
            auto rightArea = controlArea.reduced(2);
            
            // Center knobs in their respective areas
            auto leftKnobArea = leftArea.removeFromTop(knobSize + 15); // Small buffer for label
            auto middleKnobArea = middleArea.removeFromTop(knobSize + 15); // Small buffer for label
            auto rightKnobArea = rightArea.removeFromTop(knobSize + 15); // Small buffer for label
            
            param1Slider.setBounds(leftKnobArea.withSizeKeepingCentre(knobSize, knobSize));
            param2Slider.setBounds(middleKnobArea.withSizeKeepingCentre(knobSize, knobSize));
            param3Slider.setBounds(rightKnobArea.withSizeKeepingCentre(knobSize, knobSize));
            
            param1Label.setBounds(leftArea.withHeight(20));
            param2Label.setBounds(middleArea.withHeight(20));
            param3Label.setBounds(rightArea.withHeight(20));
        }
        else
        {
            // Default 3-parameter layout: 2 knobs on top, 1 on bottom
            auto topRow = controlArea.removeFromTop(controlArea.getHeight() / 2 + 10).reduced(6);
            auto bottomRow = controlArea.reduced(6);
            
            // Top row: 2 knobs
            int topSliceWidth = topRow.getWidth() / 2;
            auto topArea1 = topRow.removeFromLeft(topSliceWidth).reduced(6);
            auto topArea2 = topRow.reduced(6);
            
            // Bottom row: 1 knob (centered)
            auto bottomArea = bottomRow.reduced(6);

            // Top row sliders and labels - center knobs with reduced gap
            auto topKnobArea1 = topArea1.removeFromTop(knobSize + 4); // Only 4px buffer
            auto topKnobArea2 = topArea2.removeFromTop(knobSize + 4); // Only 4px buffer
            param1Slider.setBounds(topKnobArea1.withSizeKeepingCentre(knobSize, knobSize));
            param2Slider.setBounds(topKnobArea2.withSizeKeepingCentre(knobSize, knobSize));
            param1Label.setBounds(topArea1);
            param2Label.setBounds(topArea2);
            
            // Bottom row slider and label (centered) with reduced gap
            auto bottomKnobArea = bottomArea.removeFromTop(knobSize + 4); // Only 4px buffer
            auto centeredBottomArea = bottomKnobArea.withSizeKeepingCentre(knobSize, knobSize);
            param3Slider.setBounds(centeredBottomArea);
            param3Label.setBounds(bottomArea);
        }
    }
    else if (numParams == 5)
    {
        // Create two rows: 3 knobs on top, 2 on bottom
        auto topRow = controlArea.removeFromTop(controlArea.getHeight() / 2 + 10).reduced(4);
        auto bottomRow = controlArea.reduced(4);
        
        // Top row: 3 knobs
        int topSliceWidth = topRow.getWidth() / 3;
        auto topArea1 = topRow.removeFromLeft(topSliceWidth).reduced(4);
        auto topArea2 = topRow.removeFromLeft(topSliceWidth).reduced(4);
        auto topArea3 = topRow.reduced(4);
        
        // Bottom row: 2 knobs
        int bottomSliceWidth = bottomRow.getWidth() / 2;
        auto bottomArea1 = bottomRow.removeFromLeft(bottomSliceWidth).reduced(4);
        auto bottomArea2 = bottomRow.reduced(4);

        // Top row sliders and labels - center knobs with reduced gap
        auto topKnobArea1 = topArea1.removeFromTop(knobSize + 4); // Only 4px buffer
        auto topKnobArea2 = topArea2.removeFromTop(knobSize + 4); // Only 4px buffer
        auto topKnobArea3 = topArea3.removeFromTop(knobSize + 4); // Only 4px buffer
        param1Slider.setBounds(topKnobArea1.withSizeKeepingCentre(knobSize, knobSize));
        param2Slider.setBounds(topKnobArea2.withSizeKeepingCentre(knobSize, knobSize));
        param3Slider.setBounds(topKnobArea3.withSizeKeepingCentre(knobSize, knobSize));
        param1Label.setBounds(topArea1);
        param2Label.setBounds(topArea2);
        param3Label.setBounds(topArea3);
        
        // Bottom row sliders and labels - center knobs with reduced gap
        auto bottomKnobArea1 = bottomArea1.removeFromTop(knobSize + 4); // Only 4px buffer
        auto bottomKnobArea2 = bottomArea2.removeFromTop(knobSize + 4); // Only 4px buffer
        param4Slider.setBounds(bottomKnobArea1.withSizeKeepingCentre(knobSize, knobSize));
        param5Slider.setBounds(bottomKnobArea2.withSizeKeepingCentre(knobSize, knobSize));
        param4Label.setBounds(bottomArea1);
        param5Label.setBounds(bottomArea2);
    }
}

void EffectComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Draw background image for Overdrive effect
    if (name == "Overdrive")
    {
        // Try to load from BinaryData first (most portable)
        static juce::Image overdriveBg = juce::ImageCache::getFromMemory(
            BinaryData::overdrive_bg_jpg, BinaryData::overdrive_bg_jpgSize);
        
        // Fallback to file system if BinaryData not available
        if (!overdriveBg.isValid())
        {
            overdriveBg = juce::ImageFileFormat::loadFrom(
                juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                    .getParentDirectory().getParentDirectory().getParentDirectory()
                    .getChildFile("UI_Colours/overdrive_bg.jpg"));
        }
        
        if (overdriveBg.isValid())
        {
            g.drawImage(overdriveBg, bounds);
        }
        else
        {
            // Fallback to solid color if image fails to load
            g.fillAll(juce::Colours::black);
        }
    }
    else if (name == "Distortion")
    {
        // Try to load from BinaryData first (most portable)
        static juce::Image distortionBg = juce::ImageCache::getFromMemory(
            BinaryData::distortion_bg_jpg, BinaryData::distortion_bg_jpgSize);
        
        // Fallback to file system if BinaryData not available
        if (!distortionBg.isValid())
        {
            distortionBg = juce::ImageFileFormat::loadFrom(
                juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                    .getParentDirectory().getParentDirectory().getParentDirectory()
                    .getChildFile("UI_Colours/distortion_bg.jpg"));
        }
        
        if (distortionBg.isValid())
        {
            g.drawImage(distortionBg, bounds);
        }
        else
        {
            // Fallback to solid color if image fails to load
            g.fillAll(juce::Colours::black);
        }
    }
    else if (name == "Reverb")
    {
        // Try to load from BinaryData first (most portable)
        static juce::Image reverbBg = juce::ImageCache::getFromMemory(
            BinaryData::reverb_bg_jpg, BinaryData::reverb_bg_jpgSize);
        
        // Fallback to file system if BinaryData not available
        if (!reverbBg.isValid())
        {
            reverbBg = juce::ImageFileFormat::loadFrom(
                juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                    .getParentDirectory().getParentDirectory().getParentDirectory()
                    .getChildFile("UI_Colours/reverb_bg.jpg"));
        }
        
        if (reverbBg.isValid())
        {
            g.drawImage(reverbBg, bounds);
        }
        else
        {
            // Fallback to solid color if image fails to load
            g.fillAll(juce::Colours::black);
        }
    }
    else if (name == "Delay")
    {
        // Try to load from BinaryData first (most portable)
        static juce::Image delayBg = juce::ImageCache::getFromMemory(
            BinaryData::delay_bg_jpg, BinaryData::delay_bg_jpgSize);
        
        // Fallback to file system if BinaryData not available
        if (!delayBg.isValid())
        {
            delayBg = juce::ImageFileFormat::loadFrom(
                juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                    .getParentDirectory().getParentDirectory().getParentDirectory()
                    .getChildFile("UI_Colours/delay_bg.jpg"));
        }
        
        if (delayBg.isValid())
        {
            g.drawImage(delayBg, bounds);
        }
        else
        {
            // Fallback to solid color if image fails to load
            g.fillAll(juce::Colours::black);
        }
    }
    else if (name == "Chorus")
    {
        // Try to load from BinaryData first (most portable)
        static juce::Image chorusBg = juce::ImageCache::getFromMemory(
            BinaryData::chorus_bg_jpg, BinaryData::chorus_bg_jpgSize);
        
        // Fallback to file system if BinaryData not available
        if (!chorusBg.isValid())
        {
            chorusBg = juce::ImageFileFormat::loadFrom(
                juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                    .getParentDirectory().getParentDirectory().getParentDirectory()
                    .getChildFile("UI_Colours/chorus_bg.jpg"));
        }
        
        if (chorusBg.isValid())
        {
            g.drawImage(chorusBg, bounds);
        }
        else
        {
            // Fallback to solid color if image fails to load
            g.fillAll(juce::Colours::black);
        }
    }
    /*
    else if (name == "EQ")
    {
        // Try to load from BinaryData first (most portable)
        static juce::Image eqBg = juce::ImageCache::getFromMemory(
            BinaryData::eq_bg_jpg, BinaryData::eq_bg_jpgSize);
        
        // Fallback to file system if BinaryData not available
        if (!eqBg.isValid())
        {
            eqBg = juce::ImageFileFormat::loadFrom(
                juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                    .getParentDirectory().getParentDirectory().getParentDirectory()
                    .getChildFile("UI_Colours/eq_bg.jpg"));
        }
        
        if (eqBg.isValid())
        {
            g.drawImage(eqBg, bounds);
        }
        else
        {
            // Fallback to solid color if image fails to load
            g.fillAll(juce::Colours::black);
        }
    }
    */
    else if (name == "Compressor")
    {
        // Try to load from BinaryData first (most portable)
        static juce::Image compressorBg = juce::ImageCache::getFromMemory(
            BinaryData::compressor_bg_jpg, BinaryData::compressor_bg_jpgSize);
        
        // Fallback to file system if BinaryData not available
        if (!compressorBg.isValid())
        {
            compressorBg = juce::ImageFileFormat::loadFrom(
                juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                    .getParentDirectory().getParentDirectory().getParentDirectory()
                    .getChildFile("UI_Colours/compressor_bg.jpg"));
        }
        
        if (compressorBg.isValid())
        {
            g.drawImage(compressorBg, bounds);
        }
        else
        {
            // Fallback to solid color if image fails to load
            g.fillAll(juce::Colours::black);
        }
    }
    /*
    else if (name == "Tuner")
    {
        // Try to load from BinaryData first (most portable)
        static juce::Image tunerBg = juce::ImageCache::getFromMemory(
            BinaryData::tuner_bg_jpg, BinaryData::tuner_bg_jpgSize);
        
        // Fallback to file system if BinaryData not available
        if (!tunerBg.isValid())
        {
            tunerBg = juce::ImageFileFormat::loadFrom(
                juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                    .getParentDirectory().getParentDirectory().getParentDirectory()
                    .getChildFile("UI_Colours/tuner_bg.jpg"));
        }
        
        if (tunerBg.isValid())
        {
            g.drawImage(tunerBg, bounds);
        }
        else
        {
            // Fallback to solid color if image fails to load
            g.fillAll(juce::Colours::black);
        }
    }
    */
    else
    {
        g.fillAll(juce::Colours::black);
    }
    
    //g.setColour(juce::Colours::grey);
    //g.fillRoundedRectangle(bounds, 10.0f);
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(bounds.reduced(1.0f), 2.0f);
}

void EffectComponent::timerCallback()
{
    /*
    if (name == "Tuner")
    {
        auto* tuner = listRef.editorRef.getProcessor().getTunerProcessor();
        if (tuner && tuner->getIsProcessing())
        {
            noteDisplayLabel.setText(tuner->getNoteName(), juce::dontSendNotification);
            frequencyDisplayLabel.setText(juce::String(tuner->getFrequency(), 1) + " Hz", juce::dontSendNotification);
            
            // Update tuning status with color coding
            float cents = tuner->getCentsDeviation(listRef.editorRef.getAPVTS());
            auto status = tuner->getTuningStatus(listRef.editorRef.getAPVTS());
            
            switch (status)
            {
                case TunerProcessor::TuningStatus::InTune:
                    tuningStatusLabel.setText("IN TUNE", juce::dontSendNotification);
                    tuningStatusLabel.setColour(juce::Label::textColourId, juce::Colours::green);
                    break;
                case TunerProcessor::TuningStatus::Sharp:
                    tuningStatusLabel.setText("SHARP " + juce::String(cents, 0) + "¢", juce::dontSendNotification);
                    tuningStatusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
                    break;
                case TunerProcessor::TuningStatus::Flat:
                    tuningStatusLabel.setText("FLAT " + juce::String(std::abs(cents), 0) + "¢", juce::dontSendNotification);
                    tuningStatusLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
                    break;
            }
        }
        else
        {
            noteDisplayLabel.setText("---", juce::dontSendNotification);
            frequencyDisplayLabel.setText("0.0 Hz", juce::dontSendNotification);
            tuningStatusLabel.setText("---", juce::dontSendNotification);
            tuningStatusLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);
        }
    }
    */
}

void EffectComponent::mouseDown(const juce::MouseEvent& event)
{
    if (event.originalComponent == &titleLabel && event.mods.isLeftButtonDown())
    {
        int idx = listRef.getIndexOf(this);
        if (idx >= 0)
            listRef.dragContainer.startDragging(juce::String(idx), this);
    }
}

EffectsList::EffectsList(AudioPluginAudioProcessorEditor& editor, const std::vector<int>& ord)
    : editorRef(editor), order(ord)
{
    // Create container for effects
    effectsContainer = std::make_unique<juce::Component>();
    
    // Create viewport
    viewport = std::make_unique<juce::Viewport>();
    viewport->setScrollBarsShown(false, true); // Show horizontal scrollbar only (false = horizontal, true = vertical)
    viewport->setViewedComponent(effectsContainer.get(), false);
    addAndMakeVisible(*viewport);
    
    juce::StringArray effectNames = {"Overdrive", "Distortion", "Reverb", "Delay", "Chorus", "Compressor"};
    for (int idx : order)
    {
        effects.push_back(std::make_unique<EffectComponent>(*this, effectNames[idx], idx));
        effectsContainer->addAndMakeVisible(*effects.back());
    }
}

void EffectsList::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromString("FF303030"));
    // Drop line visualization removed since viewport handles scrolling differently
    // Drag and drop indicators will be handled by the viewport content area
}

void EffectsList::resized()
{
    // Set viewport to fill the entire bounds
    if (viewport)
        viewport->setBounds(getLocalBounds());
    
    int effectHeight = 400; // Static height for each pedal effect

    // Calculate number of effects
    int numEffects = static_cast<int>(effects.size());
    if (numEffects == 0) return;

    // Calculate total width and positions with variable widths
    int totalWidth = 0;
    std::vector<int> widths;
    for (int i = 0; i < numEffects; ++i)
    {
        int width = 275; // Default width
        const juce::String& name = effects[i]->name;
        if (name == "Overdrive" || name == "Distortion" || name == "Delay" || name == "Chorus")
            width = 225;
        // Reverb and EQ remain 275
        widths.push_back(width);
        totalWidth += width;
    }

    // Calculate spacing to evenly distribute pedals with minimum 10px gap
    int numGaps = numEffects - 1;
    int gap = (numGaps > 0) ? std::max(10, 15) : 0; // Reduced gap to 15px

    // Recalculate total width with gaps
    totalWidth += numGaps * gap;
    
    // Set the container size to accommodate all effects
    if (effectsContainer)
        effectsContainer->setBounds(0, 0, totalWidth, effectHeight);

    int currentX = 0; // Start from left edge of container
    for (int i = 0; i < numEffects; ++i)
    {
        effects[i]->setBounds(currentX, 0, widths[i], effectHeight);
        currentX += widths[i] + gap;
    }
}

bool EffectsList::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails&)
{
    return true;
}

void EffectsList::itemDragEnter(const juce::DragAndDropTarget::SourceDetails&)
{
    dropIndex = 0;
    repaint();
}

void EffectsList::itemDragMove(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    int gap = 10;
    int totalGaps = gap * (static_cast<int>(effects.size()) - 1);
    int availableWidth = getWidth() - totalGaps;
    int effectWidth = availableWidth / static_cast<int>(effects.size());
    int effectPlusGap = effectWidth + gap;
    
    // Calculate which effect position the cursor is closest to
    int position = 0;
    for (int i = 0; i < static_cast<int>(effects.size()); ++i)
    {
        int effectCenter = i * effectPlusGap + effectWidth / 2;
        if (dragSourceDetails.localPosition.x < effectCenter)
        {
            position = i;
            break;
        }
        position = i + 1;
    }
    dropIndex = juce::jlimit(0, 5, position);
    repaint();
}

void EffectsList::itemDragExit(const juce::DragAndDropTarget::SourceDetails&)
{
    dropIndex = -1;
    repaint();
}

void EffectsList::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails)
{
    int draggedIdx = dragSourceDetails.description.toString().getIntValue();
    if (dropIndex >= 0 && dropIndex < 6 && draggedIdx != dropIndex)
        reorderEffects(draggedIdx, dropIndex);
    dropIndex = -1;
    repaint();
}

void EffectsList::reorderEffects(int fromIndex, int toIndex)
{
    int effectIdx = effects[fromIndex]->index;
    auto item = std::move(effects[fromIndex]);
    effects.erase(effects.begin() + fromIndex);
    effects.insert(effects.begin() + toIndex, std::move(item));

    order.erase(order.begin() + fromIndex);
    order.insert(order.begin() + toIndex, effectIdx);

    auto& apvts = editorRef.getAPVTS();
    for (int i = 0; i < 6; ++i)
    {
        auto* param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("Order" + juce::String(i + 1)));
        param->setValueNotifyingHost(static_cast<float>(order[i]) / 5.0f);
    }
    resized();
}

int EffectsList::getIndexOf(EffectComponent* comp)
{
    for (int i = 0; i < static_cast<int>(effects.size()); ++i)
        if (effects[i].get() == comp)
            return i;
    return -1;
}

