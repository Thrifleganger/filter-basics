#pragma once

#include <JuceHeader.h>
#include "Plot.h"

class FrequencyResponsePlot : public Plot
{
public:
    FrequencyResponsePlot(std::function<std::vector<float>()> callback);
    void refresh() override;
    void setGainRange(float minDbValue, float maxDbValue);
protected:
    void drawXAxisGuides(Graphics& g) override;
    void drawYAxisGuides(Graphics& g) override;
    String getPopupTextForNormalizedScopePosition(float x, float y) override;

    float maxDb{ 10.f };
    float minDb{ -90.f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyResponsePlot)
};