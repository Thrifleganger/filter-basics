/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

CustomTabbedComponent::CustomTabbedComponent(FilterBasicsAudioProcessor& p) :
    TabbedComponent(TabbedButtonBar::Orientation::TabsAtTop),
    processor(p)
{}

void CustomTabbedComponent::currentTabChanged(int newCurrentTabIndex, const String& newCurrentTabName)
{
    // TabbedComponent internally calls currentTabChanged when adding the very first tab. Which is a dick move.
    if (getNumTabs() < 2)
        return;
    processor.setCurrentProcessorIndex(newCurrentTabIndex);
}

FilterBasicsAudioProcessorEditor::FilterBasicsAudioProcessorEditor (FilterBasicsAudioProcessor& p) :
    AudioProcessorEditor (&p),
    audioProcessor (p),
    firFilterComponent(p),
    iirFilterComponent(p),
    tab(p)
{
    tab.addTab("FIR Filter", Theme::darkBackground.darker(), &firFilterComponent, false);
    tab.addTab("IIR Filter", Theme::darkBackground.darker(), &iirFilterComponent, false);
    addAndMakeVisible(tab);
    tab.setCurrentTabIndex(audioProcessor.getCurrentProcessorIndex(), false);

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
