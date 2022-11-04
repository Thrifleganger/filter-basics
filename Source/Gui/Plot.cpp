/*
  ==============================================================================

    Plot.cpp
    Created: 25 Sep 2022 8:30:04pm
    Author:  Akash

  ==============================================================================
*/

#include "Plot.h"

Plot::Plot(const String& title, const Colour& primaryColour, std::function<std::vector<float>()> callback) :
    callback(callback), title(title), primaryColour(primaryColour)
{
}

void Plot::paint(Graphics& g)
{
    DropShadow shadow;
    shadow.radius = 10.f;
    shadow.drawForRectangle(g, scopeBounds);

    g.setColour(Theme::plotBackground);
    g.fillRoundedRectangle(scopeBounds.toFloat(), 5.f);

    g.setColour(primaryColour);
    g.setFont(Font{ 15.f }.withStyle(Font::bold));
    g.drawFittedText(title, scopeBounds.getX(), scopeBounds.getY() - 30, 300, 30, Justification::centredLeft, 1);
    g.setFont(Font{ 15.f });

    g.setOrigin(scopeBounds.getX(), scopeBounds.getY());
    drawXAxisGuides(g);
    drawYAxisGuides(g);
    g.setOrigin(-scopeBounds.getX(), -scopeBounds.getY());

    g.setColour(primaryColour);
    g.strokePath(path, PathStrokeType{ 3.f });

    g.setColour(Theme::axis);
    g.drawRoundedRectangle(scopeBounds.toFloat(), 5.f, 1.f);

    if (isMouseDownOnScope)
    {
        Point<float> pointOnPath;
        path.getNearestPoint(mousePosition.toFloat(), pointOnPath);
        g.setColour(primaryColour);
        g.fillEllipse(pointOnPath.getX() - 4, pointOnPath.getY() - 4, 8, 8);

        const auto normalizedPoint = pointOnPath.translated(-scopeBounds.getX(), -scopeBounds.getY())
            .transformedBy(AffineTransform::scale(1.f / scopeBounds.getWidth(), 1.f / scopeBounds.getHeight()));
        const auto text = getPopupTextForNormalizedScopePosition(normalizedPoint.getX(), normalizedPoint.getY());

        const auto font =  Font{ 14.f };
        const auto boxBounds = Rectangle<float>{ pointOnPath.getX() - 1, pointOnPath.getY() - 50, 1, 1 }
            .withSizeKeepingCentre(font.getStringWidthFloat(text) + 20, 40)
            .constrainedWithin(getLocalBounds().toFloat());

        g.setColour(Theme::darkBackground.darker());
        g.fillRoundedRectangle(boxBounds, 5.f);
        g.setFont(font);
        g.setColour(Colours::white);
        g.drawFittedText(text, boxBounds.toNearestInt(), Justification::centred, 1);
    }
}

void Plot::resized()
{
    scopeBounds = getLocalBounds().reduced(proportionOfWidth(0.05f))
        .translated(proportionOfWidth(0.02f), 0);
    refresh();
}

void Plot::setSampleRate(float sr)
{
    sampleRate = sr;
    refresh();
}

void Plot::mouseDown(const MouseEvent& event)
{
    if (scopeBounds.contains(event.getPosition()))
    {
        isMouseDownOnScope = true;
        mousePosition = event.getPosition();
        repaint();
    }
    Component::mouseDown(event);
}

void Plot::mouseDrag(const MouseEvent& event)
{
    if (isMouseDownOnScope)
    {
        mousePosition = event.getPosition();
        repaint();
    }
    Component::mouseDrag(event);
}

void Plot::mouseUp(const MouseEvent& event)
{
    isMouseDownOnScope = false;
    repaint();
    Component::mouseUp(event);
}
