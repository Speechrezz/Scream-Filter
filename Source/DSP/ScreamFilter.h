/*
  ==============================================================================

    ScreamFilter.h
    Created: 24 Jan 2025 6:28:34pm
    Author:  Mark

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace xynth
{

class SingleChannelScreamFilter
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) noexcept;
    float processSample(float sample) noexcept;

    void setParameters(float lowpassFreq, float highpassFreq, float feedbackGain) noexcept;

private:
    juce::dsp::IIR::Filter<float> lowpass, highpass;
    double sampleRate = 0.0;
    float prevSample = 0.f;
    float feedbackGain = 1.f;

};

class ScreamFilter
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec) noexcept;
    void process(juce::dsp::ProcessContextReplacing<float>& context) noexcept;

    void setParameters(float lowpass, float highpass, float feedbackGain) noexcept;

private:
    std::vector<SingleChannelScreamFilter> processors;

};

} // namespace xynth