#pragma once

#include <JuceHeader.h>
#include "Plot.h"

class PhaseResponsePlot : public Plot
{
public:
	PhaseResponsePlot(std::function<std::vector<float>()> callback);
	void refresh() override;
protected:
	void drawXAxisGuides(Graphics& g) override;
	void drawYAxisGuides(Graphics& g) override;
	String getPopupTextForNormalizedScopePosition(float x, float y) override;
};