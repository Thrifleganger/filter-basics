/*
  ==============================================================================

    FirFilterComponent.cpp
    Created: 20 Oct 2022 11:33:30am
    Author:  Akash

  ==============================================================================
*/

#include "FirFilterComponent.h"

FirFilterComponent::FirFilterComponent(FilterBasicsAudioProcessor& processor) :
    processor(processor),
    valueTreeState(processor.getValueTreeState()),
    magnitudeResponsePlot([&] { return processor.getMagnitudeResponse(); }),
    phaseResponsePlot([&] { return processor.getPhaseResponse(); })
{
    const auto configureSlider = [&](Slider& slider, const String& id, Label& label)
    {
        slider.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        sliderAttachments.push_back(std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(valueTreeState, id, slider));
        slider.setTextBoxStyle(Slider::NoTextBox, false, 50, 25);
        slider.onValueChange = [&] {label.setText(String{ slider.getValue(), 2 }, dontSendNotification); };

        label.setColour(Label::textColourId, Theme::curve2);
        label.setText(String{ slider.getValue(), 2 }, dontSendNotification);
        label.setEditable(false, true);
        label.onEditorShow = [&]
        {
            if (auto* editor = label.getCurrentTextEditor())
                editor->setJustification(Justification::centred);
        };
        label.onTextChange = [&] { slider.setValue(label.getText().getDoubleValue()); };
        label.setJustificationType(Justification::centred);
        
        addAndMakeVisible(slider);
        addAndMakeVisible(label);
        valueTreeState.addParameterListener(id, this);
    };
    configureSlider(coeffA0Slider, "firCoeffA0", coeffA0Label);
    configureSlider(coeffA1Slider, "firCoeffA1", coeffA1Label);

    addAndMakeVisible(magnitudeResponsePlot);
    addAndMakeVisible(phaseResponsePlot);

    firFilterImage = ImageCache::getFromMemory(BinaryData::FirFilter_png, BinaryData::FirFilter_pngSize);

    startTimerHz(24);
}

void FirFilterComponent::resized()
{
    auto bounds = getLocalBounds();
    imageBounds = bounds.removeFromTop(proportionOfHeight(0.3f)).withTrimmedTop(proportionOfHeight(0.05f)).reduced(20, 0);
    const auto transformedImageBounds = firFilterImage.getBounds()
        .transformedBy(AffineTransform::scale((float)imageBounds.getHeight() / (float)firFilterImage.getHeight()));
    imageBounds = Rectangle<int>{transformedImageBounds.getWidth(), transformedImageBounds.getHeight()}.withCentre(imageBounds.getCentre());

    coeffA0Slider.setBounds(Rectangle<int>{
        imageBounds.proportionOfWidth(0.547),
            -imageBounds.proportionOfHeight(0.1f),
            imageBounds.proportionOfWidth(0.09f),
            imageBounds.proportionOfWidth(0.09f),
    }.translated(imageBounds.getX(), imageBounds.getY()));
    coeffA1Slider.setBounds(Rectangle<int>{
        imageBounds.proportionOfWidth(0.547),
            imageBounds.proportionOfHeight(0.51f),
            imageBounds.proportionOfWidth(0.09f),
            imageBounds.proportionOfWidth(0.09f),
    }.translated(imageBounds.getX(), imageBounds.getY()));


    coeffA0Label.setBounds(Rectangle<int>{
        imageBounds.proportionOfWidth(0.47f),
        imageBounds.proportionOfHeight(0.158f),
        imageBounds.proportionOfWidth(0.09f),
        imageBounds.proportionOfHeight(0.096)
    }.translated(imageBounds.getX(), imageBounds.getY()));
    coeffA1Label.setBounds(Rectangle<int>{
        imageBounds.proportionOfWidth(0.47f),
        imageBounds.proportionOfHeight(0.762f),
        imageBounds.proportionOfWidth(0.09f),
        imageBounds.proportionOfHeight(0.096)
    }.translated(imageBounds.getX(), imageBounds.getY()));

    magnitudeResponsePlot.setBounds(bounds.removeFromTop(bounds.proportionOfHeight(0.5f)));
    phaseResponsePlot.setBounds(bounds);

    coeffA0Label.setFont(Font{ (float)proportionOfHeight(0.018f) });
    coeffA1Label.setFont(Font{ (float)proportionOfHeight(0.018f) });
}

void FirFilterComponent::timerCallback()
{
    if (shouldRefreshGraphs)
    {
        processor.calculateImpulseResponse(Topology::FIR);
        magnitudeResponsePlot.refresh();
        phaseResponsePlot.refresh();
        shouldRefreshGraphs = false;
    }
}

void FirFilterComponent::paint(Graphics& g)
{
    g.setGradientFill(ColourGradient{ Theme::mainBackground.brighter(), getLocalBounds().getCentre().toFloat(), Theme::mainBackground.darker(), {}, true });
    g.fillAll();

    g.drawImage(firFilterImage, imageBounds.toFloat(), RectanglePlacement::centred);
}

void FirFilterComponent::parameterChanged(const String& parameterID, float newValue)
{
    if (parameterID.equalsIgnoreCase("firCoeffA0") || parameterID.equalsIgnoreCase("firCoeffA1"))
        shouldRefreshGraphs = true;
}
