#include "FirFilter.h"

void FirFilter::prepare(int numChannels)
{
	channelRegister.clear();
	channelRegister.resize(numChannels, 0.f);
}

void FirFilter::process(AudioBuffer<float>& buffer)
{
	for (auto channel = 0; channel < buffer.getNumChannels(); channel++)
	{
		for (auto sample = 0; sample < buffer.getNumSamples(); sample++)
		{
			const auto currentSample = buffer.getReadPointer(channel)[sample];
			// Implementing the difference equation y(n) = a0.x(n) + a1.x(n-1)
			buffer.getWritePointer(channel)[sample] = a0 * currentSample + a1 * channelRegister[channel];
			channelRegister[channel] = currentSample;
		}
	}
}

void FirFilter::setCoefficients(float coeffA0, float coeffA1)
{
	a0 = coeffA0;
	a1 = coeffA1;
}

