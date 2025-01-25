/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void FullSlider::attach(juce::AudioProcessorValueTreeState& treeState, juce::StringRef ParamID) noexcept
{
    auto& parameter = *treeState.getParameter(ParamID);
    attachment = std::make_unique<juce::SliderParameterAttachment>(parameter, slider);
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);

    label.attachToComponent(&slider, false);
    label.setText(parameter.name, juce::NotificationType::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
}


ScreamFilterAudioProcessorEditor::ScreamFilterAudioProcessorEditor (ScreamFilterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);

    lowpassSlider.attach(p.treeState, "lowpass");
    highpassSlider.attach(p.treeState, "highpass");
    feedbackGainSlider.attach(p.treeState, "feedbackGain");

    addAndMakeVisible(lowpassSlider.slider);
    addAndMakeVisible(highpassSlider.slider);
    addAndMakeVisible(feedbackGainSlider.slider);
}

ScreamFilterAudioProcessorEditor::~ScreamFilterAudioProcessorEditor()
{
}

void ScreamFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText("Scream Filter", getLocalBounds().reduced(10), juce::Justification::centredTop, 1);
}

void ScreamFilterAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().withTrimmedTop(20).withSizeKeepingCentre(340, 100);

    lowpassSlider.slider.setBounds(bounds.removeFromLeft(100));
    bounds.removeFromLeft(20);
    highpassSlider.slider.setBounds(bounds.removeFromLeft(100));
    bounds.removeFromLeft(20);
    feedbackGainSlider.slider.setBounds(bounds.removeFromLeft(100));
}