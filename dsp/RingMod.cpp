#include "RingMod.h"

#include <algorithm>

// Windows does not have Pi constants
#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

namespace mrta
{

RingMod::RingMod()
{
}

void RingMod::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;

    depthRamp.prepare(sampleRate, true, modDepth);
    dryRamp.prepare(sampleRate, true, 1.f - modDepth);

    phaseState[0] = 0.f;
    phaseState[1] = static_cast<float>(M_PI / 2.0); // quadature osc between L and R channels
    phaseInc = static_cast<float>(2.0 * M_PI / sampleRate) * modRate;
}

void RingMod::process(float* const* output_1, const float* const* input_1, float* const* output_2, const float* const* input_2, unsigned int numChannels, unsigned int numSamples)
{
    numChannels = std::min(numChannels, 2u);
    for (unsigned int n = 0; n < numSamples; ++n)
    {
        // Process LFO acording to mod type
        float lfo[2] { 0.f, 0.f };
        switch (modType)
        {

        case RectSin:
            lfo[0] = std::fabs(std::sin(phaseState[0]));
            lfo[1] = std::fabs(std::sin(phaseState[1]));
            break;

        case Sin:
            lfo[0] = 0.5f + 0.5f * std::sin(phaseState[0]);
            lfo[1] = 0.5f + 0.5f * std::sin(phaseState[1]);
            break;

        }

        // Increment and wrap phase states
        phaseState[0] = std::fmod(phaseState[0] + phaseInc, static_cast<float>(2 * M_PI));
        phaseState[1] = std::fmod(phaseState[1] + phaseInc, static_cast<float>(2 * M_PI));

        // Apply modulation depth gain ramp
        depthRamp.applyGain(lfo, numChannels);
        dryRamp.applySum(lfo, numChannels);

        // Do amplitude modulation
        for (unsigned int ch = 0; ch < numChannels; ++ch) 
        {
            output_1[ch][n] = lfo[0] * input_1[ch][n];
            output_2[ch][n] = lfo[1] * input_2[ch][n];
        }
            

    }
}

void RingMod::setModRate(float newModRate)
{
    modRate = std::fmax(newModRate, 0.f);
    phaseInc = static_cast<float>(2.0 * M_PI / sampleRate) * modRate;
}

void RingMod::setModDepth(float newModDepth)
{
    modDepth = std::fmin(std::fmax(newModDepth, 0.f), 1.f);
    depthRamp.setTarget(modDepth);
    dryRamp.setTarget(1.f - modDepth);
}

void RingMod::setModType(ModType type)
{
    modType = type;
}


}
