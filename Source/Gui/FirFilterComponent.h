/*
  ==============================================================================

    FirFilterComponent.h
    Created: 20 Oct 2022 11:33:30am
    Author:  Akash

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "FrequencyResponsePlot.h"
#include "PhaseResponsePlot.h"
#include "LookAndFeel.h"

class FirFilterComponent : public Component, public Timer, public AudioProcessorValueTreeState::Listener
{
public:
    FirFilterComponent(FilterBasicsAudioProcessor& processor);
    void resized() override;
    void timerCallback() override;
    void paint(Graphics& g) override;
    void parameterChanged(const String& parameterID, float newValue) override;
private:
    FilterBasicsAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    Slider coeffA0Slider, coeffA1Slider;
    Label coeffA0Label, coeffA1Label;
    std::vector<std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;

    Image firFilterImage;
    Rectangle<int> imageBounds;
    FrequencyResponsePlot magnitudeResponsePlot;
    PhaseResponsePlot phaseResponsePlot;
    bool shouldRefreshGraphs{ true };
};
