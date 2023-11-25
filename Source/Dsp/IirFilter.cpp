#include "IirFilter.h"

void IirFilter::prepare(int numChannels)
{
	channelRegister.clear();
	channelRegister.resize(numChannels, 0.f);

	channelRegisterExtra.clear();
	channelRegisterExtra.resize(numChannels, 0.f);
}

void IirFilter::process(AudioBuffer<float>& buffer)
{
	for (auto channel = 0; channel < buffer.getNumChannels(); channel++)
	{
		for (auto sample = 0; sample < buffer.getNumSamples(); sample++)
		{
			const auto input = buffer.getReadPointer(channel)[sample];
			auto output = input;
			if (mode == Mode::Simple)
			{
				output = a0 * input -
					b1 * channelRegister[channel];
				// Filter has blown up, deal with it.
				if (isinf(output) || isnan(output)) 
					output = 10.f;
				channelRegister[channel] = output;
			}
			else if (mode == Mode::DirectForm1)
			{
				output = a0 * input +
					a1 * channelRegisterExtra[channel] -
					b1 * channelRegister[channel];
				// Filter has blown up, deal with it.
				if (isinf(output) || isnan(output))
					output = 10.f;
				channelRegister[channel] = output;
				channelRegisterExtra[channel] = input;
			}
			else if (mode == Mode::DirectForm2)
			{
				auto v = input - b1 * channelRegister[channel];
				// Filter has blown up, deal with it.
				if (isinf(v) || isnan(v))
					v = 10.f;
				output = a0 * v + a1 * channelRegister[channel];
				channelRegister[channel] = v;
			}
			buffer.getWritePointer(channel)[sample] = output;
		}
	}
}

void IirFilter::setCoefficients(float coeffA0, float coeffA1, float coeffB1)
{
	a0 = coeffA0;
	a1 = coeffA1;
	b1 = coeffB1;
}

void IirFilter::setMode(Mode m)
{
	mode = m;
}