/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct FullSlider
{
    void attach(juce::AudioProcessorValueTreeState& treeState, juce::StringRef ParamID) noexcept;

    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::SliderParameterAttachment> attachment;
};

class ScreamFilterAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ScreamFilterAudioProcessorEditor (ScreamFilterAudioProcessor&);
    ~ScreamFilterAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ScreamFilterAudioProcessor& audioProcessor;

    FullSlider lowpassSlider, highpassSlider, feedbackGainSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScreamFilterAudioProcessorEditor)
};
