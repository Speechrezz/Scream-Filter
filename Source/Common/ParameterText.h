/*
  ==============================================================================

    ParameterText.h
    Created: 24 Jan 2025 7:31:22pm
    Author:  Mark

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

namespace xynth
{

juce::String frequencyAsText(float value, int maxLength)
{
    if (value >= 1000.f)
    {
        value = value / 1000.f;
        return juce::String(value, maxLength) + " kHz";
    }

    return juce::String(value, std::max(maxLength - 1, 0)) + " Hz";
}

float textToValue(const juce::String& text)
{
    float value = text.getFloatValue();
    if (text.containsChar('k') || text.containsChar('K'))
        value *= 1000.f;

    return value;
}


} // namespace xynth