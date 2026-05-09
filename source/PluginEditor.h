#pragma once

#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

// Forward declarations
class AudioPluginAudioProcessorEditor;
class EffectsList;

//==============================================================================
// Custom LookAndFeel for image-based knobs
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle,
                         juce::Slider& slider) override;
};

//==============================================================================
class EffectComponent : public juce::Component, public juce::Timer
{
public:
    EffectComponent(class EffectsList& list, const juce::String& name, int index);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void timerCallback() override;

    juce::String name;
    int index;
    class EffectsList& listRef;

    juce::ToggleButton bypassButton;
    juce::Label titleLabel;
    juce::Slider param1Slider;
    juce::Label param1Label;
    juce::Slider param2Slider;
    juce::Label param2Label;
    juce::Slider param3Slider;
    juce::Label param3Label;
    juce::Slider param4Slider;
    juce::Label param4Label;
    juce::Slider param5Slider;
    juce::Label param5Label;

    // Tuner-specific components
    //juce::Label noteDisplayLabel;
    //juce::Label frequencyDisplayLabel;
    //juce::Label tuningStatusLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> param1Attachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> param2Attachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> param3Attachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> param4Attachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> param5Attachment;
    int numParams = 0;
};

class PreampComponent : public juce::Component
{
public:
    PreampComponent(AudioPluginAudioProcessorEditor& editor);
    void paint(juce::Graphics& g) override;
    void resized() override;

    juce::Label preampTitleLabel;
    juce::Slider presenceSlider;
    juce::Label presenceLabel;
    juce::Slider bassSlider;
    juce::Label bassLabel;
    juce::Slider midSlider;
    juce::Label midLabel;
    juce::Slider trebleSlider;
    juce::Label trebleLabel;
    juce::Slider gainSlider;
    juce::Label gainLabel;
    juce::Slider volumeSlider;
    juce::Label volumeLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> presenceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trebleAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;

private:
    AudioPluginAudioProcessorEditor& editorRef;
};

class EffectsList : public juce::Component, public juce::DragAndDropTarget
{
public:
    EffectsList(AudioPluginAudioProcessorEditor& editor, const std::vector<int>& order);
    ~EffectsList() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragMove(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;

    void reorderEffects(int fromIndex, int toIndex);
    int getIndexOf(EffectComponent* comp);

    AudioPluginAudioProcessorEditor& editorRef;
    juce::DragAndDropContainer dragContainer;
    std::vector<std::unique_ptr<EffectComponent>> effects;
    std::vector<int> order;
    int dropIndex = -1;
    std::unique_ptr<juce::Viewport> viewport;
    std::unique_ptr<juce::Component> effectsContainer;
};

class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return processorRef.getAPVTS(); }
    CustomLookAndFeel& getCustomLookAndFeel() { return customLookAndFeel; }
    AudioPluginAudioProcessor& getProcessor() { return processorRef; }

protected:
    CustomLookAndFeel customLookAndFeel;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;
    std::unique_ptr<PreampComponent> preampComponent;
    std::unique_ptr<EffectsList> effectsList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};