#pragma once

#include <JuceHeader.h>

class FirFilter
{
public:
    void prepare(int numChannels);
    void process(AudioBuffer<float>& buffer);
    void setCoefficients(float a0, float a1);
private:
    std::vector<float> channelRegister;
    float a0{ 0.5f }, a1{ 0.5f };
};

class IirFilter
{
public:
    void prepare(int numChannels);
    void process(AudioBuffer<float>& buffer);
    void setCoefficients(float a0, float a1, float b1);
private:
    std::vector<float> channelRegister;
    float a0{ 0.5f }, a1{ 0.5f }, b1{ 0.5f };
};

enum class Topology {
    FIR, IIR
};

class FilterBasicsAudioProcessor  : public juce::AudioProcessor, public AudioProcessorValueTreeState::Listener
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    FilterBasicsAudioProcessor();
    ~FilterBasicsAudioProcessor() override;
    void parameterChanged(const String& parameterID, float newValue) override;
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    AudioProcessorValueTreeState& getValueTreeState() { return valueTreeState; };

    void calculateImpulseResponse(Topology topology);
    std::vector<float> getMagnitudeResponse();
    std::vector<float> getPhaseResponse();

private:
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    AudioProcessorValueTreeState valueTreeState;
    AudioBuffer<float> impulse;
    std::vector<dsp::Complex<float>> impulseResponse, impulseFft;
    FirFilter firFilter, firAnalysisFilter;
    IirFilter iirFilter, iirAnalysisFilter;
    dsp::FFT fft;
    dsp::WindowingFunction<float> window;
    float sampleRate;

    static constexpr int fftOrder{ 11 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterBasicsAudioProcessor)
};