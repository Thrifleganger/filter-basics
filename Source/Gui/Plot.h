#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"
#include <algorithm>

class Plot : public Component
{
public:
    Plot(const String& title, const Colour& primaryColour, std::function<std::vector<float>()> callback);
    void paint(Graphics& g) override;
    void resized() override;
    void setSampleRate(float sr);

    void mouseDown(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override;

    virtual void refresh() = 0;
protected:
    virtual void drawXAxisGuides(Graphics& g) = 0;
    virtual void drawYAxisGuides(Graphics& g) = 0;
    virtual String getPopupTextForNormalizedScopePosition(float x, float y) = 0;

    std::function<std::vector<float>()> callback;
    Rectangle<int> scopeBounds;
    String title;
    Colour primaryColour;
    Path path;
    bool isMouseDownOnScope{ false };
    Point<int> mousePosition;
    float sampleRate{ 44100 };
};
