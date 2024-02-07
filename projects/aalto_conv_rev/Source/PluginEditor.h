#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class FlangerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FlangerAudioProcessorEditor (FlangerAudioProcessor&);
    ~FlangerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    FlangerAudioProcessor& audioProcessor;
    juce::TextButton loadBtn;
    std::unique_ptr<juce::FileChooser> fileChooser;

    juce::TextButton loadBtnDD;
    std::unique_ptr<juce::FileChooser> fileChooserDD;

    mrta::GenericParameterEditor genericParameterEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlangerAudioProcessorEditor)
};
