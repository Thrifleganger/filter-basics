#include "PhaseResponsePlot.h"

PhaseResponsePlot::PhaseResponsePlot(std::function<std::vector<float>()> callback) :
	Plot("Phase Response", Theme::curve2, callback)
{
}

void PhaseResponsePlot::refresh()
{
    if (!callback)
        return;
    if (getWidth() == 0)
        return;

    const auto data = callback();
    path.clear();

    for (int i = 0; i < data.size(); i++)
    {
        const auto x = jmap((float)i, 0.f, (float)data.size(), 0.f, 1.f);
        auto value = data[i];
        if (isnan(value) || isinf(value))
            value = maxAngle;
        const auto y = jmap(value, minAngle, maxAngle, 0.f, 1.f);
        if (i == 0)
            path.startNewSubPath(x, y);
        else 
            path.lineTo(x, y);
    }
    const auto transform = AffineTransform::scale(scopeBounds.getWidth(), scopeBounds.getHeight())
        .translated(scopeBounds.getX(), scopeBounds.getY());
    path.applyTransform(transform);

    repaint();
}

void PhaseResponsePlot::setDegreeRange(float minDegreeValue, float maxDegreeValue)
{
    maxAngle = maxDegreeValue;
    minAngle = minDegreeValue;
}

void PhaseResponsePlot::drawXAxisGuides(Graphics& g)
{
    g.setColour(Theme::axis.darker());
    g.setOpacity(0.5f);
    std::vector<float> xMainGuides;
    auto freq{ 2000.f };
    do
    {
        xMainGuides.push_back(freq);
        freq += 2000.f;
    } while (freq < sampleRate * 0.5f);

    for (const auto point : xMainGuides)
    {
        const auto x = point / (sampleRate * 0.5f) * scopeBounds.getWidth();
        g.drawLine(x, 0, x, scopeBounds.getHeight(), 1.f);

        g.setFont(12.f);
        g.drawFittedText(point < 1000.f ? String{ point } + "Hz" : String{ point / 1000.f } + "kHz",
            x - 20, scopeBounds.getHeight() + 10, 40, 20, Justification::centred, 1);
    }
}

void PhaseResponsePlot::drawYAxisGuides(Graphics& g)
{
    g.setColour(Theme::axis.darker());
    g.setOpacity(0.5f);

    std::vector<int> yAxisGuides;
    for (int i = (int)minAngle; i <= (int)maxAngle;)
    {
        yAxisGuides.push_back(i);
        i += 60;
    }

    for (const auto point : yAxisGuides)
    {
        const auto y = scopeBounds.getHeight() - jmap(point, (int)minAngle, (int)maxAngle, 0, scopeBounds.getHeight());
        g.drawLine(0, y, scopeBounds.getWidth(), y, 1.f);

        g.setFont(12.f);
        g.drawFittedText(String{ point } + String{ CharPointer_UTF8("\xc2\xb0") },
            -40, y - 10, 40, 20, Justification::centred, 1);
    }
}

String PhaseResponsePlot::getPopupTextForNormalizedScopePosition(float x, float y)
{
    return "Freq: " + String{ jmap(x, 0.f, sampleRate * 0.5f), 2 } + "Hz | "
        + "Phase: " + String{ jmap(y, minAngle, maxAngle), 2 } + String{ CharPointer_UTF8("\xc2\xb0") };
}
