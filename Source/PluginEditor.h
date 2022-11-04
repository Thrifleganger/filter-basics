
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Gui/FirFilterComponent.h"

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
    TabbedComponent tab{TabbedButtonBar::Orientation::TabsAtTop};
    CustomLookAndFeel lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterBasicsAudioProcessorEditor)
};
