#pragma once

#include <JuceHeader.h>

class FirFilter
{
public:
    void prepare(int numChannels);
    void process(AudioBuffer<float>& buffer);
    void setCoefficients(float a0, float a1);
private:
    std::vector<float> channelRegister;
    float a0{ 0.5f }, a1{ 0.5f };
};