
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Gui/FirFilterComponent.h"
#include "Gui/IirFilterComponent.h"

class CustomTabbedComponent : public TabbedComponent
{
public:
    CustomTabbedComponent(FilterBasicsAudioProcessor& p);
    void currentTabChanged(int newCurrentTabIndex, const String& newCurrentTabName) override;
private:
    FilterBasicsAudioProcessor& processor;
};

class FilterBasicsAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FilterBasicsAudioProcessorEditor (FilterBasicsAudioProcessor&);
    ~FilterBasicsAudioProcessorEditor() override;

    void paint (Graphics&) override;
    void resized() override;

private:
    FilterBasicsAudioProcessor& audioProcessor;
    FirFilterComponent firFilterComponent;
    IirFilterComponent iirFilterComponent;
    CustomTabbedComponent tab;
    CustomLookAndFeel lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterBasicsAudioProcessorEditor)
};
