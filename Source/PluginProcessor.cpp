/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void FirFilter::prepare(int numChannels)
{
    channelRegister.clear();
    channelRegister.resize(numChannels, 0.f);
}

void FirFilter::process(AudioBuffer<float>& buffer)
{
    for (auto channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        for (auto sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            const auto currentSample = buffer.getReadPointer(channel)[sample];
            buffer.getWritePointer(channel)[sample] = a0 * currentSample + a1 * channelRegister[channel];
            channelRegister[channel] = currentSample;
        }
    }
}

void FirFilter::setCoefficients(float coeffA0, float coeffA1)
{
    a0 = coeffA0;
    a1 = coeffA1;
}


void IirFilter::prepare(int numChannels)
{
    channelRegister.clear();
    channelRegister.resize(numChannels, 0.f);
}

void IirFilter::process(AudioBuffer<float> &buffer)
{
    for (auto channel = 0; channel < buffer.getNumChannels(); channel++)
    {
        for (auto sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            const auto currentSample = buffer.getReadPointer(channel)[sample];
            auto v = currentSample - b1 * channelRegister[channel];
            if (isinf(v) || isnan(v))
                v = 10.f;
            auto output = a0 * v + a1 * channelRegister[channel];
            channelRegister[channel] = v;
            buffer.getWritePointer(channel)[sample] = output;
        }
    }
}

void IirFilter::setCoefficients(float coeffA0, float coeffA1, float coeffB1)
{
    a0 = coeffA0;
    a1 = coeffA1;
    b1 = coeffB1;
}

//==============================================================================
FilterBasicsAudioProcessor::FilterBasicsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), valueTreeState(*this, nullptr, "FilterBasics", createParameterLayout()),
        fft(fftOrder),
        window(fft.getSize(), dsp::WindowingFunction<float>::hann)
#endif
{
    valueTreeState.addParameterListener("firCoeffA0", this);
    valueTreeState.addParameterListener("firCoeffA1", this);

    valueTreeState.addParameterListener("iirCoeffA0", this);
    valueTreeState.addParameterListener("iirCoeffA1", this);
    valueTreeState.addParameterListener("iirCoeffB1", this);
}

FilterBasicsAudioProcessor::~FilterBasicsAudioProcessor()
{
    valueTreeState.removeParameterListener("firCoeffA0", this);
    valueTreeState.removeParameterListener("firCoeffA1", this);

    valueTreeState.removeParameterListener("iirCoeffB1", this);
    valueTreeState.removeParameterListener("iirCoeffA1", this);
    valueTreeState.removeParameterListener("iirCoeffA0", this);
}

//==============================================================================
const juce::String FilterBasicsAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FilterBasicsAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FilterBasicsAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FilterBasicsAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FilterBasicsAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FilterBasicsAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FilterBasicsAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FilterBasicsAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FilterBasicsAudioProcessor::getProgramName (int index)
{
    return {};
}

void FilterBasicsAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FilterBasicsAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    {
        const auto a0 = valueTreeState.getRawParameterValue("firCoeffA0")->load();
        const auto a1 = valueTreeState.getRawParameterValue("firCoeffA1")->load();
        firFilter.prepare(getTotalNumInputChannels());
        firFilter.setCoefficients(a0, a1);

        firAnalysisFilter.prepare(1);
        firAnalysisFilter.setCoefficients(a0, a1);
    }

    {
        const auto a0 = valueTreeState.getRawParameterValue("iirCoeffA0")->load();
        const auto a1 = valueTreeState.getRawParameterValue("iirCoeffA1")->load();
        const auto b1 = valueTreeState.getRawParameterValue("iirCoeffB1")->load();
        iirFilter.prepare(getTotalNumInputChannels());
        iirFilter.setCoefficients(a0, a1, b1);

        iirAnalysisFilter.prepare(1);
        iirAnalysisFilter.setCoefficients(a0, a1, b1);
    }
    

    impulse.setSize(1, fft.getSize(), false);
    impulseResponse.resize(fft.getSize(), {0.f, 0.f});
    impulseFft.resize(fft.getSize(), {0.f, 0.f});
}

void FilterBasicsAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FilterBasicsAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FilterBasicsAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    //firFilter.process(buffer);
    //iirFilter.process(buffer);
}

//==============================================================================
bool FilterBasicsAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FilterBasicsAudioProcessor::createEditor()
{
    return new FilterBasicsAudioProcessorEditor (*this);
}

//==============================================================================
void FilterBasicsAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FilterBasicsAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

AudioProcessorValueTreeState::ParameterLayout FilterBasicsAudioProcessor::createParameterLayout()
{
    return AudioProcessorValueTreeState::ParameterLayout{
        std::make_unique<AudioParameterFloat>(ParameterID{"firCoeffA0", 1}, "FIR Coefficient A0", NormalisableRange<float>{-2.f, 2.f, 0.01f}, 0.5f),
        std::make_unique<AudioParameterFloat>(ParameterID{"firCoeffA1", 1}, "FIR Coefficient A1", NormalisableRange<float>{-2.f, 2.f, 0.01f}, 0.5f),
        std::make_unique<AudioParameterFloat>(ParameterID{"iirCoeffA0", 1}, "IIR Coefficient A0", NormalisableRange<float>{-2.f, 2.f, 0.01f}, 0.5f),
        std::make_unique<AudioParameterFloat>(ParameterID{"iirCoeffA1", 1}, "IIR Coefficient A1", NormalisableRange<float>{-2.f, 2.f, 0.01f}, 0.5f),
        std::make_unique<AudioParameterFloat>(ParameterID{"iirCoeffB1", 1}, "IIR Coefficient B1", NormalisableRange<float>{-2.f, 2.f, 0.01f}, 0.5f)
    };
}

void FilterBasicsAudioProcessor::calculateImpulseResponse(Topology topology)
{
    // Create impulse signal
    impulse.setSample(0, 0, 1.f);
    for (int sample = 1; sample < fft.getSize(); sample++)
        impulse.setSample(0, sample, 0.f);

    // Filter impulse signal
    if (topology == Topology::FIR)
        firAnalysisFilter.process(impulse);
    else
        iirAnalysisFilter.process(impulse);

    for (int sample = 0; sample < impulse.getNumSamples(); sample++)
    {
        impulseResponse[sample] = dsp::Complex<float>{ impulse.getReadPointer(0)[sample], 0.f };
    }
    fft.perform(impulseResponse.data(), impulseFft.data(), false);
}

std::vector<float> FilterBasicsAudioProcessor::getMagnitudeResponse()
{
    std::vector<float> magnitudeArray;
    magnitudeArray.resize(impulseFft.size() / 2, 0.f);
    std::transform(impulseFft.begin(), impulseFft.begin() + impulseFft.size() / 2, magnitudeArray.begin(), 
        [](const dsp::Complex<float>& complex) { return std::abs(complex); });
    return magnitudeArray;
}

std::vector<float> FilterBasicsAudioProcessor::getPhaseResponse()
{
    std::vector<float> phaseArray;
    phaseArray.resize(impulseFft.size() / 2, 0.f);
    std::transform(impulseFft.begin(), impulseFft.begin() + impulseFft.size() / 2, phaseArray.begin(),
        [](const dsp::Complex<float>& complex) { return radiansToDegrees(std::arg(complex)); });
    return phaseArray;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FilterBasicsAudioProcessor();
}

void FilterBasicsAudioProcessor::parameterChanged(const String& parameterID, float newValue)
{
    if (parameterID.equalsIgnoreCase("firCoeffA0"))
    {
        firFilter.setCoefficients(newValue, valueTreeState.getRawParameterValue("firCoeffA1")->load());
        firAnalysisFilter.setCoefficients(newValue, valueTreeState.getRawParameterValue("firCoeffA1")->load());
    }
    if (parameterID.equalsIgnoreCase("firCoeffA1"))
    {
        firFilter.setCoefficients(valueTreeState.getRawParameterValue("firCoeffA0")->load(), newValue);
        firAnalysisFilter.setCoefficients(valueTreeState.getRawParameterValue("firCoeffA0")->load(), newValue);
    }
    {
        const auto a0 = valueTreeState.getRawParameterValue("iirCoeffA0")->load();
        const auto a1 = valueTreeState.getRawParameterValue("iirCoeffA1")->load();
        const auto b1 = valueTreeState.getRawParameterValue("iirCoeffB1")->load();
        if (parameterID.containsIgnoreCase({"iir"}))
        {
            iirFilter.setCoefficients(a0, a1, b1);
            iirAnalysisFilter.setCoefficients(a0, a1, b1);
        }
    }

}
