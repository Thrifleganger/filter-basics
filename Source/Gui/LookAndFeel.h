/*
  ==============================================================================

    LookAndFeel.h
    Created: 20 Oct 2022 11:33:40am
    Author:  Akash

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct Theme
{
	static const Colour mainBackground;
	static const Colour darkBackground;
	static const Colour plotBackground;
	static const Colour axis;
	static const Colour curve1;
	static const Colour curve2;
};

class CustomLookAndFeel : public LookAndFeel_V4
{
public:
	CustomLookAndFeel()
	{
	}

	void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override
	{
		const auto b = slider.getLocalBounds().reduced(1).toFloat();

		g.setColour(Theme::curve2);
		g.drawEllipse(b, 2.f);

		Path dot;
		dot.addEllipse(Rectangle<float>{4, 4}.withCentre(b.getCentre().translated(0, -b.proportionOfHeight(0.35f))));
		const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
		g.fillPath(dot, AffineTransform::rotation(angle, b.getCentreX(), b.getCentreY()));
	}
};
