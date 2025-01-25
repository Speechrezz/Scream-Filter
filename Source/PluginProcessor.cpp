/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "Common/Ranges.h"
#include "Common/ParameterText.h"

//==============================================================================
ScreamFilterAudioProcessor::ScreamFilterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    treeState(*this, nullptr, "PARAMETER", createParameterLayout())
#endif
{
    lowpassParam = treeState.getRawParameterValue("lowpass");
    highpassParam = treeState.getRawParameterValue("highpass");
    feedbackGainParam = treeState.getRawParameterValue("feedbackGain");
}

ScreamFilterAudioProcessor::~ScreamFilterAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ScreamFilterAudioProcessor::createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    const auto filterRange = xynth::createFrequencyRange(20.f, 20000.f);
    auto frequencyAttributes = AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float val, int) { return xynth::frequencyAsText(val, 2); })
        .withValueFromStringFunction(xynth::textToValue);
    auto gainAttributes = AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float val, int) { return juce::String(val, 2) + " dB"; })
        .withValueFromStringFunction(xynth::textToValue);

    // --Parameters--
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "lowpass",  1 }, "Lowpass", filterRange, 20.f, frequencyAttributes));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "highpass",  1 }, "Highpass", filterRange, 20.f, frequencyAttributes));
    layout.add(std::make_unique<AudioParameterFloat>(ParameterID{ "feedbackGain",  1 }, "Feedback Gain", xynth::createRange(-18.f, 6.f, -2.f), 0.f, gainAttributes));

    return layout;
}

//==============================================================================
const juce::String ScreamFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ScreamFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ScreamFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ScreamFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ScreamFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ScreamFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ScreamFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ScreamFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ScreamFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void ScreamFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ScreamFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    using uint32 = juce::uint32;

    const auto channels = juce::jmax(getTotalNumInputChannels(), getTotalNumOutputChannels());
    juce::dsp::ProcessSpec spec{ sampleRate, (uint32)samplesPerBlock, (uint32)channels };
    DBG("prepareToPlay() - sampleRate: " << sampleRate << ", maxBlockSize: " << (int)spec.maximumBlockSize << ", numChannels: " << (int)spec.numChannels);

    screamFilter.prepare(spec);
}

void ScreamFilterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ScreamFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
#endif

void ScreamFilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> audioBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> context(audioBlock);

    const float lowpass  = lowpassParam ->load(std::memory_order_relaxed);
    const float highpass = highpassParam->load(std::memory_order_relaxed);
    const float feedbackGain = feedbackGainParam->load(std::memory_order_relaxed);

    screamFilter.setParameters(lowpass, highpass, juce::Decibels::decibelsToGain(feedbackGain));
    screamFilter.process(context);
}

//==============================================================================
bool ScreamFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ScreamFilterAudioProcessor::createEditor()
{
    return new ScreamFilterAudioProcessorEditor (*this);
}

//==============================================================================
void ScreamFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    DBG("Save state...");
    auto xmlState = std::make_unique<juce::XmlElement>(JucePlugin_Name);
    xmlState->setAttribute("version", JucePlugin_VersionString);

    auto xmlTreeState = treeState.copyState().createXml();
    xmlState->addChildElement(xmlTreeState.release());

    DBG("\nXML:\n" << xmlState->toString());
    copyXmlToBinary(*xmlState, destData);
}

void ScreamFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    DBG("Load state...");
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (!xmlState)
        return;

    DBG("\nXML:\n" << xmlState->toString());
    if (auto* xmlTreeState = xmlState->getChildByName(treeState.state.getType()))
        treeState.replaceState(juce::ValueTree::fromXml(*xmlTreeState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ScreamFilterAudioProcessor();
}
