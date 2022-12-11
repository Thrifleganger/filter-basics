/*
  ==============================================================================

    IirFilterComponent.cpp
    Created: 27 Nov 2022 4:21:03pm
    Author:  Akash Murthy

  ==============================================================================
*/

#include "IirFilterComponent.h"

IirFilterComponent::IirFilterComponent(FilterBasicsAudioProcessor& processor) :
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
    configureSlider(coeffA0Slider, "iirCoeffA0", coeffA0Label);
    configureSlider(coeffA1Slider, "iirCoeffA1", coeffA1Label);
    configureSlider(coeffB1Slider, "iirCoeffB1", coeffB1Label);

    addAndMakeVisible(magnitudeResponsePlot);
    addAndMakeVisible(phaseResponsePlot);

    iirFilterImage = ImageCache::getFromMemory(BinaryData::FirFilter_png, BinaryData::FirFilter_pngSize);

    startTimerHz(24);
}

void IirFilterComponent::resized()
{
    auto bounds = getLocalBounds();
    imageBounds = bounds.removeFromTop(proportionOfHeight(0.3f)).withTrimmedTop(proportionOfHeight(0.05f)).reduced(20, 0);
    const auto transformedImageBounds = iirFilterImage.getBounds()
        .transformedBy(AffineTransform::scale((float)imageBounds.getHeight() / (float)iirFilterImage.getHeight()));
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
    coeffB1Slider.setBounds(Rectangle<int>{
        imageBounds.proportionOfWidth(0.747),
        imageBounds.proportionOfHeight(0.71f),
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
    coeffB1Label.setBounds(Rectangle<int>{
        imageBounds.proportionOfWidth(0.67f),
        imageBounds.proportionOfHeight(0.862f),
        imageBounds.proportionOfWidth(0.09f),
        imageBounds.proportionOfHeight(0.096)
    }.translated(imageBounds.getX(), imageBounds.getY()));

    magnitudeResponsePlot.setBounds(bounds.removeFromTop(bounds.proportionOfHeight(0.5f)));
    phaseResponsePlot.setBounds(bounds);

    coeffA0Label.setFont(Font{ (float)proportionOfHeight(0.018f) });
    coeffA1Label.setFont(Font{ (float)proportionOfHeight(0.018f) });
    coeffB1Label.setFont(Font{ (float)proportionOfHeight(0.018f) });
}

void IirFilterComponent::timerCallback()
{
    if (shouldRefreshGraphs)
    {
        processor.calculateImpulseResponse(Topology::IIR);
        magnitudeResponsePlot.refresh();
        phaseResponsePlot.refresh();
        shouldRefreshGraphs = false;
    }
}

void IirFilterComponent::paint(Graphics& g)
{
    g.setGradientFill(ColourGradient{ Theme::mainBackground.brighter(), getLocalBounds().getCentre().toFloat(), Theme::mainBackground.darker(), {}, true });
    g.fillAll();

    g.drawImage(iirFilterImage, imageBounds.toFloat(), RectanglePlacement::centred);
}

void IirFilterComponent::parameterChanged(const String& parameterID, float newValue)
{
    if (parameterID.containsIgnoreCase("iir"))
        shouldRefreshGraphs = true;
}
