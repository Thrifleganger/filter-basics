/*
  ==============================================================================

    IirFilterComponent.h
    Created: 27 Nov 2022 4:21:03pm
    Author:  Akash Murthy

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "FrequencyResponsePlot.h"
#include "PhaseResponsePlot.h"
#include "LookAndFeel.h"

class IirFilterComponent : public Component,
                           public Timer,
                           public AudioProcessorValueTreeState::Listener,
                           public ComboBox::Listener
{
public:
    IirFilterComponent(FilterBasicsAudioProcessor& processor);
    void resized() override;
    void timerCallback() override;
    void paint(Graphics& g) override;
    void parameterChanged(const String& parameterID, float newValue) override;
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;
private:
    FilterBasicsAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    Slider coeffA0Slider, coeffA1Slider, coeffB1Slider;
    Label coeffA0Label, coeffA1Label, coeffB1Label;
    std::vector<std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    ComboBox modeSelection;

    Image iirFilterImage;
    Rectangle<int> imageBounds;
    FrequencyResponsePlot magnitudeResponsePlot;
    PhaseResponsePlot phaseResponsePlot;
    bool shouldRefreshGraphs{ true };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IirFilterComponent)
};

