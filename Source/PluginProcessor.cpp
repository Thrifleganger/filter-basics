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
            buffer.getWritePointer(channel)[sample] = coeffA0 * currentSample + coeffA1 * channelRegister[channel];
            channelRegister[channel] = currentSample;
        }
    }
}

void FirFilter::setCoefficients(float a0, float a1)
{
    coeffA0 = a0;
    coeffA1 = a1;
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
}

FilterBasicsAudioProcessor::~FilterBasicsAudioProcessor()
{
    valueTreeState.removeParameterListener("firCoeffA0", this);
    valueTreeState.removeParameterListener("firCoeffA1", this);
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
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    firFilter.process(buffer);
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
        std::make_unique<AudioParameterFloat>(ParameterID{"iirCoeffA1", 1}, "IIR Coefficient A1", NormalisableRange<float>{-2.f, 2.f, 0.01f}, 0.5f)
    };
}

void FilterBasicsAudioProcessor::calculateFirImpulseResponse()
{
    // Create impulse signal
    impulse.setSample(0, 0, 1.f);
    for (int sample = 1; sample < fft.getSize(); sample++)
        impulse.setSample(0, sample, 0.f);

    // Filter impulse signal 
    firAnalysisFilter.process(impulse);
    for (int sample = 0; sample < impulse.getNumSamples(); sample++)
    {
        impulseResponse[sample] = dsp::Complex<float>{ impulse.getReadPointer(0)[sample], 0.f };
    }
    fft.perform(impulseResponse.data(), impulseFft.data(), false);
}

std::vector<float> FilterBasicsAudioProcessor::getFirMagnitudeResponse()
{
    std::vector<float> magnitudeArray;
    magnitudeArray.resize(impulseFft.size() / 2, 0.f);
    std::transform(impulseFft.begin(), impulseFft.begin() + impulseFft.size() / 2, magnitudeArray.begin(), 
        [](const dsp::Complex<float>& complex) { return std::abs(complex); });
    return magnitudeArray;
}

std::vector<float> FilterBasicsAudioProcessor::getFirPhaseResponse()
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
        
}
