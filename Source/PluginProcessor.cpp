/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FilterBasicsAudioProcessor::FilterBasicsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		  .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		  .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	  ), valueTreeState(*this, nullptr, "FilterBasics", createParameterLayout()),
	  fft(fftOrder)
#endif
{
	valueTreeState.addParameterListener(Id::FIR::a0, this);
	valueTreeState.addParameterListener(Id::FIR::a1, this);

	valueTreeState.addParameterListener(Id::IIR::a0, this);
	valueTreeState.addParameterListener(Id::IIR::a1, this);
	valueTreeState.addParameterListener(Id::IIR::b1, this);

	valueTreeState.addParameterListener(Id::tab, this);
}

FilterBasicsAudioProcessor::~FilterBasicsAudioProcessor()
{
	valueTreeState.removeParameterListener(Id::FIR::a0, this);
	valueTreeState.removeParameterListener(Id::FIR::a1, this);

	valueTreeState.removeParameterListener(Id::IIR::a0, this);
	valueTreeState.removeParameterListener(Id::IIR::a1, this);
	valueTreeState.removeParameterListener(Id::IIR::b1, this);

	valueTreeState.removeParameterListener(Id::tab, this);
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
	return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int FilterBasicsAudioProcessor::getCurrentProgram()
{
	return 0;
}

void FilterBasicsAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String FilterBasicsAudioProcessor::getProgramName(int index)
{
	return {};
}

void FilterBasicsAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void FilterBasicsAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	{
		const auto a0 = valueTreeState.getRawParameterValue(Id::FIR::a0)->load();
		const auto a1 = valueTreeState.getRawParameterValue(Id::FIR::a1)->load();
		firFilter.prepare(getTotalNumInputChannels());
		firFilter.setCoefficients(a0, a1);

		firAnalysisFilter.prepare(1);
		firAnalysisFilter.setCoefficients(a0, a1);
	}
	{
		const auto a0 = valueTreeState.getRawParameterValue(Id::IIR::a0)->load();
		const auto a1 = valueTreeState.getRawParameterValue(Id::IIR::a1)->load();
		const auto b1 = valueTreeState.getRawParameterValue(Id::IIR::b1)->load();
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
bool FilterBasicsAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
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

void FilterBasicsAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;

	if (currentProcessor.load() == 0)
		firFilter.process(buffer);
	else 
		iirFilter.process(buffer);
}

//==============================================================================
bool FilterBasicsAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FilterBasicsAudioProcessor::createEditor()
{
	return new FilterBasicsAudioProcessorEditor(*this);
}

//==============================================================================
void FilterBasicsAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	std::unique_ptr<XmlElement> xml(valueTreeState.copyState().createXml());
	copyXmlToBinary(*xml, destData);
}

void FilterBasicsAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	std::unique_ptr<XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
	if (xml && xml->hasTagName(valueTreeState.state.getType()))
	{
		valueTreeState.replaceState(ValueTree::fromXml(*xml));
	}
}

AudioProcessorValueTreeState::ParameterLayout FilterBasicsAudioProcessor::createParameterLayout()
{
	return AudioProcessorValueTreeState::ParameterLayout{
		std::make_unique<AudioParameterFloat>(ParameterID{Id::FIR::a0, 1}, "FIR Coefficient A0",
											  NormalisableRange<float>{-2.f, 2.f, 0.01f}, 0.5f),
		std::make_unique<AudioParameterFloat>(ParameterID{Id::FIR::a1, 1}, "FIR Coefficient A1",
											  NormalisableRange<float>{-2.f, 2.f, 0.01f}, 0.5f),
		std::make_unique<AudioParameterFloat>(ParameterID{Id::IIR::a0, 1}, "IIR Coefficient A0",
											  NormalisableRange<float>{-2.f, 2.f, 0.01f}, 0.5f),
		std::make_unique<AudioParameterFloat>(ParameterID{Id::IIR::a1, 1}, "IIR Coefficient A1",
											  NormalisableRange<float>{-2.f, 2.f, 0.01f}, 0.5f),
		std::make_unique<AudioParameterFloat>(ParameterID{Id::IIR::b1, 1}, "IIR Coefficient B1",
											  NormalisableRange<float>{-2.f, 2.f, 0.01f}, 0.5f),
		std::make_unique<AudioParameterInt>(ParameterID{Id::tab, 1}, "Processor Tab", 0, 1, 0),
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
		impulseResponse[sample] = dsp::Complex<float>{impulse.getReadPointer(0)[sample], 0.f};
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

void FilterBasicsAudioProcessor::setIirFilterMode(IirFilter::Mode mode)
{
	iirFilter.setMode(mode);
	iirAnalysisFilter.setMode(mode);
}

int FilterBasicsAudioProcessor::getCurrentProcessorIndex() const
{
	return currentProcessor.load();
}

void FilterBasicsAudioProcessor::setCurrentProcessorIndex(int processorIndex)
{
	if (auto* parameter = valueTreeState.getParameter(Id::tab))
	{
		parameter->beginChangeGesture();
		parameter->setValueNotifyingHost(processorIndex);
		parameter->endChangeGesture();
	}
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new FilterBasicsAudioProcessor();
}

void FilterBasicsAudioProcessor::parameterChanged(const String& parameterID, float newValue)
{
	if (parameterID.containsIgnoreCase("fir"))
	{
		const auto a0 = valueTreeState.getRawParameterValue(Id::FIR::a0)->load();
		const auto a1 = valueTreeState.getRawParameterValue(Id::FIR::a1)->load();

		firFilter.setCoefficients(a0, a1);
		firAnalysisFilter.setCoefficients(a0, a1);
	}
	else if (parameterID.containsIgnoreCase("iir"))
	{
		const auto a0 = valueTreeState.getRawParameterValue(Id::IIR::a0)->load();
		const auto a1 = valueTreeState.getRawParameterValue(Id::IIR::a1)->load();
		const auto b1 = valueTreeState.getRawParameterValue(Id::IIR::b1)->load();
		
		iirFilter.setCoefficients(a0, a1, b1);
		iirAnalysisFilter.setCoefficients(a0, a1, b1);
	}
	if (parameterID.equalsIgnoreCase(Id::tab))
	{
		currentProcessor.store(static_cast<int>(newValue));
	}
}
