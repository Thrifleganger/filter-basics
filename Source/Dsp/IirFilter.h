#pragma once

#include <JuceHeader.h>

class IirFilter
{
public:
    enum class Mode
    {
        Simple,
        DirectForm1,
        DirectForm2
    };
    void prepare(int numChannels);
    void process(AudioBuffer<float>& buffer);
    void setMode(Mode m);
    void setCoefficients(float a0, float a1, float b1);
private:
    std::vector<float> channelRegister, channelRegisterExtra;
    float a0{ 0.5f }, a1{ 0.5f }, b1{ 0.5f };
    Mode mode;
};