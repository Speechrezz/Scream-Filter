/*
  ==============================================================================

    ScreamFilter.cpp
    Created: 24 Jan 2025 6:28:34pm
    Author:  Mark

  ==============================================================================
*/

#include "ScreamFilter.h"

namespace xynth
{

void SingleChannelScreamFilter::prepare(const juce::dsp::ProcessSpec& spec) noexcept
{
    lowpass.prepare(spec);
    highpass.prepare(spec);

    sampleRate = spec.sampleRate;
    prevSample = 0.f;
}

float SingleChannelScreamFilter::processSample(float sample) noexcept
{
    // Feedforward
    sample = std::tanh(sample + prevSample);
    sample = lowpass.processSample(sample);
    const float output = sample;

    // Feedback
    sample = highpass.processSample(sample);
    sample = std::tanh(sample * feedbackGain);
    prevSample = sample;

    return output;
}

void SingleChannelScreamFilter::setParameters(float lowpassFreq, float highpassFreq, float newFeedbackGain) noexcept
{
    jassert(sampleRate > 0.0);
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    *lowpass.coefficients = *Coefficients::makeLowPass(sampleRate, lowpassFreq);
    *highpass.coefficients = *Coefficients::makeHighPass(sampleRate, highpassFreq);
    feedbackGain = newFeedbackGain;
}

void ScreamFilter::prepare(const juce::dsp::ProcessSpec& spec) noexcept
{
    processors.resize(spec.numChannels);
    for (auto& processor : processors)
        processor.prepare(spec);
}

void ScreamFilter::process(juce::dsp::ProcessContextReplacing<float>& context) noexcept
{
    const auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();

    for (int channel = 0; channel < inputBlock.getNumChannels(); ++channel)
    {
        const auto* inputPointer = inputBlock.getChannelPointer(channel);
        auto* outputPointer = outputBlock.getChannelPointer(channel);
        auto& processor = processors[channel];

        for (int i = 0; i < inputBlock.getNumSamples(); ++i)
        {
            outputPointer[i] = processor.processSample(inputPointer[i]);
        }
    }
}

void ScreamFilter::setParameters(float lowpass, float highpass, float feedbackGain) noexcept
{
    for (auto& processor : processors)
        processor.setParameters(lowpass, highpass, feedbackGain);
}

} // namespace xynth