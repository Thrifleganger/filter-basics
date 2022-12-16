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
    configureSlider(coeffA0Slider, Id::IIR::a0, coeffA0Label);
    configureSlider(coeffA1Slider, Id::IIR::a1, coeffA1Label);
    configureSlider(coeffB1Slider, Id::IIR::b1, coeffB1Label);

    modeSelection.addItemList({ "Simple","Direct Form 1", "Direct Form 2" }, 1);
    modeSelection.addListener(this);
    modeSelection.setSelectedItemIndex(0, sendNotificationAsync);
    addAndMakeVisible(modeSelection);

    addAndMakeVisible(magnitudeResponsePlot);
    addAndMakeVisible(phaseResponsePlot);

    startTimerHz(24);
}

void IirFilterComponent::resized()
{
    auto bounds = getLocalBounds();
    imageBounds = bounds.removeFromTop(proportionOfHeight(0.3f)).withTrimmedTop(proportionOfHeight(0.05f)).reduced(20, 0);
    const auto transformedImageBounds = iirFilterImage.getBounds()
        .transformedBy(AffineTransform::scale((float)imageBounds.getHeight() / (float)iirFilterImage.getHeight()));
    imageBounds = Rectangle<int>{transformedImageBounds.getWidth(), transformedImageBounds.getHeight()}.withCentre(imageBounds.getCentre());

    Rectangle<int> a0SliderBounds, a1SliderBounds, b1SliderBounds, b0SliderBounds;
    Rectangle<int> a0LabelBounds, a1LabelBounds, b1LabelBounds, b0LabelBounds;

    const auto getRect = [&](float x, float y, float w, float h)
    {
        return Rectangle<int>{
            imageBounds.proportionOfWidth(x),
            imageBounds.proportionOfHeight(y),
            imageBounds.proportionOfWidth(w),
            imageBounds.proportionOfWidth(h),
        }.translated(imageBounds.getX(), imageBounds.getY());
    };

    switch (static_cast<IirFilter::Mode>(modeSelection.getSelectedItemIndex())) {
    case IirFilter::Mode::Simple: 
        a0SliderBounds = getRect(0.27f, -0.091f, 0.09f, 0.09f); 
        a1SliderBounds = Rectangle<int>{};
        b1SliderBounds = getRect(0.578f, 0.505f, 0.09f, 0.09f);

        a0LabelBounds = getRect(0.209f, 0.122f, 0.09f, 0.09f);
        a1LabelBounds = Rectangle<int>{};
        b1LabelBounds = getRect(0.65f, 0.705f, 0.09f, 0.09f);
        break;
    case IirFilter::Mode::DirectForm1: 
        a0SliderBounds = getRect(0.263f, -0.078f, 0.09f, 0.09f);
        a1SliderBounds = getRect(0.263f, 0.521f, 0.09f, 0.09f);
        b1SliderBounds = getRect(0.578f, 0.521f, 0.09f, 0.09f);
        b0SliderBounds = getRect(0.874f, 0.521f, 0.09f, 0.09f);

        a0LabelBounds = getRect(0.211f, 0.114f, 0.09f, 0.09f);
        a1LabelBounds = getRect(0.211f, 0.714f, 0.09f, 0.09f);
        b1LabelBounds = getRect(0.65f, 0.714f, 0.09f, 0.09f);
        b0LabelBounds = getRect(0.823f, 0.713f, 0.09f, 0.09f);
        break;
    case IirFilter::Mode::DirectForm2:
    default:
        a0SliderBounds = getRect(0.674f, -0.078f, 0.09f, 0.09f);
        a1SliderBounds = getRect(0.674f, 0.521f, 0.09f, 0.09f);
        b1SliderBounds = getRect(0.182f, 0.521f, 0.09f, 0.09f);

        a0LabelBounds = getRect(0.623f, 0.114f, 0.09f, 0.09f);
        a1LabelBounds = getRect(0.623f, 0.713f, 0.09f, 0.09f);
        b1LabelBounds = getRect(0.239f, 0.713f, 0.09f, 0.09f);
        break;
    }

    coeffA0Slider.setBounds(a0SliderBounds);
    coeffA1Slider.setBounds(a1SliderBounds);
    coeffB1Slider.setBounds(b1SliderBounds);

    coeffA0Label.setBounds(a0LabelBounds);
    coeffA1Label.setBounds(a1LabelBounds);
    coeffB1Label.setBounds(b1LabelBounds);

    magnitudeResponsePlot.setBounds(bounds.removeFromTop(bounds.proportionOfHeight(0.5f)));
    phaseResponsePlot.setBounds(bounds);

    coeffA0Label.setFont(Font{ (float)proportionOfHeight(0.018f) });
    coeffA1Label.setFont(Font{ (float)proportionOfHeight(0.018f) });
    coeffB1Label.setFont(Font{ (float)proportionOfHeight(0.018f) });

    modeSelection.setBounds(Rectangle<int>(120, 25).withPosition(proportionOfHeight(0.02f), proportionOfWidth(0.02f)));
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

void IirFilterComponent::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &modeSelection)
    {
        const auto mode = static_cast<IirFilter::Mode>(modeSelection.getSelectedItemIndex());
        processor.setIirFilterMode(mode);
        switch (mode) {
        case IirFilter::Mode::Simple: 
            iirFilterImage = ImageCache::getFromMemory(BinaryData::IirFilterSimple_png, BinaryData::IirFilterSimple_pngSize);
            break;
        case IirFilter::Mode::DirectForm1: 
            iirFilterImage = ImageCache::getFromMemory(BinaryData::IirFilterDirectForm1_png, BinaryData::IirFilterDirectForm1_pngSize);
            break;
        case IirFilter::Mode::DirectForm2:
        default:
            iirFilterImage = ImageCache::getFromMemory(BinaryData::IirFilterDirectForm2_png, BinaryData::IirFilterDirectForm2_pngSize);
            break;
        }
        shouldRefreshGraphs = true;
        repaint();
        resized();
    }
}
