#pragma once

#include <JuceHeader.h>
#include "Flanger.h"
#include "RingMod.h"
#include "ParametricEqualizer.h"

namespace Param
{
    namespace ID
    {
        static const juce::String Band0Enabled{ "band0_enabled" };
        static const juce::String Band0Type{ "band0_type" };
        static const juce::String Band0Freq{ "band0_freq" };
        static const juce::String Band0Reso{ "band0_reso" };
        static const juce::String Band0Gain{ "band0_gain" };

        //static const juce::String Enabled { "enabled" };
        static const juce::String Offset { "offset" };
        static const juce::String Depth { "depth" };
        static const juce::String Feedback { "feedback" };
        static const juce::String Rate { "rate" };
        static const juce::String ModType { "mod_type" };
        static const juce::String WetDry{ "wet_dry" };
    }

    namespace Name
    {

        static const juce::String Band0Enabled{ "B0 Enabled" };
        static const juce::String Band0Type{ "B0 Type" };
        static const juce::String Band0Freq{ "B0 Frequency" };
        static const juce::String Band0Reso{ "B0 Resonance" };
        static const juce::String Band0Gain{ "B0 Gain" };
        //static const juce::String Enabled { "Enabled" };
        static const juce::String Offset { "Offset" };
        static const juce::String Depth { "Depth" };
        static const juce::String Feedback { "Feedback" };
        static const juce::String Rate { "Rate" };
        static const juce::String ModType { "Mod. Type" };
        static const juce::String WetDry{ "Dry/Wet" };
    }

    namespace Ranges
    {
        static const float FreqMin{ 20.f };
        static const float FreqMax{ 20000.f };
        static const float FreqInc{ 1.f };
        static const float FreqSkw{ 0.3f };

        static const float ResoMin{ 0.1f };
        static const float ResoMax{ 10.f };
        static const float ResoInc{ 0.01f };
        static const float ResoSkw{ 0.5f };

        static const float GainMin{ -24.f };
        static const float GainMax{ 24.f };
        static const float GainInc{ 0.1f };
        static const float GainSkw{ 1.f };

        static const juce::StringArray Types{ "Flat", "High Pass", "Low Shelf", "Peak", "Low Pass", "High Shelf" };

        static const juce::String EnabledOn{ "On" };
        static const juce::String EnabledOff{ "Off" };

        static constexpr float OffsetMin { 1.f };
        static constexpr float OffsetMax { 10.f };
        static constexpr float OffsetInc { 0.1f };
        static constexpr float OffsetSkw { 0.5f };

        static constexpr float DepthMin { 0.f };
        static constexpr float DepthMax { 1.f };
        static constexpr float DepthInc { 0.01f };
        static constexpr float DepthSkw { 0.7f };

        static constexpr float FeedbackMin { -100.f };
        static constexpr float FeedbackMax { 100.f };
        static constexpr float FeedbackInc { 0.1f };
        static constexpr float FeedbackSkw { 1.f };

        static constexpr float RateMin { 0.0f };
        static constexpr float RateMax { 20.f };
        static constexpr float RateInc { 0.2f };
        static constexpr float RateSkw { 0.5f };

        static constexpr float WetDryMin{ 0.0f };
        static constexpr float WetDryMax{ 1.0f };
        static constexpr float WetDryInc{ 0.01f };
        static constexpr float WetDrySkw{ 1.0f };

        static const juce::StringArray ModLabels { "RectSin", "Sin"};

        //static const juce::String EnabledOff { "Off" };
        //static const juce::String EnabledOn { "On" };
    }     

    namespace Units
    {
        static const juce::String Freq{ "Hz" };
        static const juce::String Gain{ "dB" };

        static const juce::String Ms { "ms" };
        static const juce::String Hz { "Hz" };
        static const juce::String Pct { "%" };
    }

}

class FlangerAudioProcessor : public juce::AudioProcessor
{
public:
    FlangerAudioProcessor();
    ~FlangerAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void releaseResources() override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    mrta::ParameterManager& getParameterManager() { return parameterManager; }

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    //==============================================================================
    


    static const unsigned int MaxDelaySizeSamples { 1 << 12 };
    static const unsigned int MaxChannels { 2 };
    static const unsigned int MaxProcessBlockSamples{ 32 };
   
    // TODO
    juce::File root, savedFile;
    juce::dsp::Convolution irLoader;

    juce::File rootDD, savedFileDD;
    juce::dsp::Convolution irLoaderDD;

private:
    mrta::ParametricEqualizer eq;
    juce::dsp::ProcessSpec specs;
    juce::AudioBuffer<float> fxBuffer;
    
    juce::AudioBuffer<float> convBuffer;
    juce::AudioBuffer<float> convBufferDD;
    juce::AudioBuffer<float> dryBuffer;

    juce::SmoothedValue<float> SmoothWetDry;
    mrta::Ramp<float> enableRamp;

    float wetDryPotVal;
    mrta::Ramp<float> dryRamp;
    mrta::Ramp<float> wetRamp;
    mrta::ParameterManager parameterManager;
    mrta::Flanger flanger;
    mrta::RingMod ringMod;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlangerAudioProcessor)
};
