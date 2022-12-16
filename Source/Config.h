#pragma once

#include <JuceHeader.h>

namespace Id
{
#define Declare(name, value) static const inline String name (#value);

	namespace FIR
	{
		Declare(a0, firCoeffA0)
		Declare(a1, firCoeffA1)
	};

	namespace IIR
	{
		Declare(a0, iirCoeffA0)
		Declare(a1, iirCoeffA1)
		Declare(b1, iirCoeffB1)
	}

	Declare(tab, processingTab);
}
