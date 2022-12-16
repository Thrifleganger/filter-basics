#pragma once

#include <JuceHeader.h>
#include "Plot.h"

class FrequencyResponsePlot : public Plot
{
public:
    FrequencyResponsePlot(std::function<std::vector<float>()> callback);
    void refresh() override;
protected:
    void drawXAxisGuides(Graphics& g) override;
    void drawYAxisGuides(Graphics& g) override;
    String getPopupTextForNormalizedScopePosition(float x, float y) override;

    static constexpr float maxDb{ 10.f };
    static constexpr float minDb{ -90.f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyResponsePlot)
};