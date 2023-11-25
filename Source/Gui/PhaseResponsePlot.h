#pragma once

#include <JuceHeader.h>
#include "Plot.h"

class PhaseResponsePlot : public Plot
{
public:
	PhaseResponsePlot(std::function<std::vector<float>()> callback);
	void refresh() override;
	void setDegreeRange(float minDegreeValue, float maxDegreeValue);
protected:
	void drawXAxisGuides(Graphics& g) override;
	void drawYAxisGuides(Graphics& g) override;
	String getPopupTextForNormalizedScopePosition(float x, float y) override;

	float maxAngle{ 180.f };
    float minAngle{ -180.f };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseResponsePlot)
};