/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

FilterBasicsAudioProcessorEditor::FilterBasicsAudioProcessorEditor (FilterBasicsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), firFilterComponent(p), iirFilterComponent(p)
{
    tab.addTab("FIR Filter", Theme::darkBackground.darker(), &firFilterComponent, false);
    tab.addTab("IIR Filter", Theme::darkBackground.darker(), &iirFilterComponent, false);
    addAndMakeVisible(tab);

    setLookAndFeel(&lookAndFeel);

    setResizable(true, true);
    setSize (580, 780);
}

FilterBasicsAudioProcessorEditor::~FilterBasicsAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void FilterBasicsAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Theme::darkBackground);
}

void FilterBasicsAudioProcessorEditor::resized()
{
    tab.setBounds(getLocalBounds());
    
}