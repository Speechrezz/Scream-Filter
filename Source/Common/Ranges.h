/*
  ==============================================================================

    Ranges.h
    Created: 24 Jan 2025 7:01:19pm
    Author:  Mark

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace xynth
{

juce::NormalisableRange<float> createFrequencyRange(const float minFreq, const float maxFreq)
{
    return juce::NormalisableRange<float>(minFreq, maxFreq,
        // ---convertFrom0To1Func---
        [](float rangeStart, float rangeEnd, float valueToRemap)
        {
            return juce::mapToLog10(valueToRemap, rangeStart, rangeEnd);
        },
        // ---convertTo0To1Func---
        [](float rangeStart, float rangeEnd, float valueToRemap)
        {
            return juce::mapFromLog10(valueToRemap, rangeStart, rangeEnd);
        }, {});
}

juce::NormalisableRange<float> createRange(const float minVal, const float maxVal, const float midVal)
{
    juce::NormalisableRange<float> range(minVal, maxVal);
    range.setSkewForCentre(midVal);
    return range;
}

} // namespace xynth