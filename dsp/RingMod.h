#pragma once

#include "Ramp.h"

namespace mrta
{

class RingMod
{
public:
    enum ModType : unsigned int
    {
        RectSin,
        Sin
    };

    RingMod();
    ~RingMod() {}

    // No copy and move
    RingMod(const RingMod&) = delete;
    RingMod(RingMod&&) = delete;
    const RingMod& operator=(const RingMod&) = delete;
    const RingMod& operator=(RingMod&&) = delete;

    void prepare(double sampleRate);

    void process(float* const* output_1, const float* const* input_1, float* const* output_2, const float* const* input_2, unsigned int numChannels, unsigned int numSamples);

    // Set the modulation rate in Hz
    void setModRate(float modRateHz);

    // Set modulaion depth in linear gain [0; 1]
    void setModDepth(float modDepthLin);

    // Set modulation type
    void setModType(ModType type);

private:
    double sampleRate { 48000.0 };

    mrta::Ramp<float> depthRamp;
    mrta::Ramp<float> dryRamp;
    float modDepth { 0.f };
    float modRate { 0.f };

    // Phase state of the oscillators
    float phaseState[2] { 0.f, 0.f };
    float phaseInc { 0.f };

    ModType modType { Sin };
};

}
