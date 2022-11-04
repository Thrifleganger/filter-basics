#include "FrequencyResponsePlot.h"

FrequencyResponsePlot::FrequencyResponsePlot(std::function<std::vector<float>()> callback) : Plot("Frequency Response", Theme::curve1, callback)
{
}


void FrequencyResponsePlot::refresh()
{
    if (!callback)
        return;
    if (getWidth() == 0)
        return;

    const auto data = callback();
    path.clear();
    path.startNewSubPath(0, 0);

    const float logDataSize = log10(data.size());
    for (int i = 0; i < data.size(); i++)
    {
        const auto x = jmap(jlimit(0.f, logDataSize, log10((float)i)), 0.f, logDataSize, 0.f, 1.f);
        const auto y = jmap(Decibels::gainToDecibels(data[i]), minDb, maxDb, 1.f, 0.f);
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

void FrequencyResponsePlot::drawXAxisGuides(Graphics& g)
{
    g.setColour(Theme::axis.darker());
    g.setOpacity(0.5f);
    std::vector<float> xAuxGuides, xMainGuides;
    auto freq{ 10.f };
    xMainGuides.push_back(freq);
    do
    {
        for (int i = 2; i < 10; i++)
        {
            if (freq * i > sampleRate * 0.5f)
                continue;
            xAuxGuides.push_back(freq * i);
        }
        freq *= 10.f;
        xMainGuides.push_back(freq);
    } while (freq < sampleRate * 0.5f);

    const auto logNyquist = log10(sampleRate * 0.5f);

    for (const auto point : xAuxGuides)
    {
        const auto x = log10(point) / logNyquist * scopeBounds.getWidth();
        g.drawLine(x, 0, x, scopeBounds.getHeight(), 0.5f);
    }
    for (const auto point : xMainGuides)
    {
        const auto x = log10(point) / logNyquist * scopeBounds.getWidth();
        g.drawLine(x, 0, x, scopeBounds.getHeight(), 1.f);

        g.setFont(12.f);
        g.drawFittedText(point < 1000.f ? String{ point } + "Hz" : String{ point / 1000.f } + "kHz",
            x - 20, scopeBounds.getHeight() + 10, 40, 20, Justification::centred, 1);
    }
}

void FrequencyResponsePlot::drawYAxisGuides(Graphics& g)
{
    g.setColour(Theme::axis.darker());
    g.setOpacity(0.5f);

    std::vector<int> yAxisGuides;
    for (int i = minDb; i <= maxDb;)
    {
        yAxisGuides.push_back(i);
        i += 10;
    }

    for (const auto point : yAxisGuides)
    {
        const auto y = scopeBounds.getHeight() - jmap(point, (int)minDb, (int)maxDb, 0, scopeBounds.getHeight());
        g.drawLine(0, y, scopeBounds.getWidth(), y, 1.f);

        g.setFont(12.f);
        g.drawFittedText(String{ point } + "dB",
            -40, y - 10, 40, 20, Justification::centred, 1);
    }
}

String FrequencyResponsePlot::getPopupTextForNormalizedScopePosition(float x, float y)
{
    return "Freq: " + String{ powf(10, x * log10(sampleRate * 0.5f)) , 2 } + "Hz | "
        + "Amp: " + String{ jmap(y, maxDb, minDb), 2 } + "dB";
}
