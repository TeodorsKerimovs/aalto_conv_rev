#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <algorithm>

static const std::vector<mrta::ParameterInfo> Parameters
{

    //{ Param::ID::Enabled,  Param::Name::Enabled,  Param::Ranges::EnabledOff, Param::Ranges::EnabledOn, true },
    { Param::ID::WetDry,   Param::Name::WetDry,   Param::Units::Pct, 0.f, Param::Ranges::WetDryMin,   Param::Ranges::WetDryMax,   Param::Ranges::WetDryInc,   Param::Ranges::WetDrySkw },
    { Param::ID::ModType,  Param::Name::ModType,  Param::Ranges::ModLabels, 0 },
    { Param::ID::Depth,    Param::Name::Depth,    Param::Units::Ms,  0.f,  Param::Ranges::DepthMin,    Param::Ranges::DepthMax,    Param::Ranges::DepthInc,    Param::Ranges::DepthSkw },
    { Param::ID::Rate,     Param::Name::Rate,     Param::Units::Hz,  0.f, Param::Ranges::RateMin,     Param::Ranges::RateMax,     Param::Ranges::RateInc,     Param::Ranges::RateSkw },
    { Param::ID::Band0Type, Param::Name::Band0Type, Param::Ranges::Types, mrta::ParametricEqualizer::HighShelf },
    { Param::ID::Band0Freq, Param::Name::Band0Freq, Param::Units::Freq, 100.f, Param::Ranges::FreqMin, Param::Ranges::FreqMax, Param::Ranges::FreqInc, Param::Ranges::FreqSkw },
    { Param::ID::Band0Reso, Param::Name::Band0Reso, "", 0.71f, Param::Ranges::ResoMin, Param::Ranges::ResoMax, Param::Ranges::ResoInc, Param::Ranges::ResoSkw },
    { Param::ID::Band0Gain, Param::Name::Band0Gain, Param::Units::Gain, 0.f, Param::Ranges::GainMin, Param::Ranges::GainMax, Param::Ranges::GainInc, Param::Ranges::GainSkw },
};

FlangerAudioProcessor::FlangerAudioProcessor() :
    parameterManager(*this, ProjectInfo::projectName, Parameters),
    flanger(20.f, 2),
    enableRamp(0.05f),
    dryRamp(0.05f),
    eq(1),
    wetRamp(0.05f),
    wetDryPotVal{ 0.f }
{

    parameterManager.registerParameterCallback(Param::ID::Band0Type,
        [this](float val, bool /*force*/)
        {
            eq.setBandType(0, static_cast<mrta::ParametricEqualizer::FilterType>(std::round(val)));
        });

    parameterManager.registerParameterCallback(Param::ID::Band0Freq,
        [this](float val, bool /*force*/)
        {
            eq.setBandFrequency(0, val);
        });

    parameterManager.registerParameterCallback(Param::ID::Band0Reso,
        [this](float val, bool /*force*/)
        {
            eq.setBandResonance(0, val);
        });

    parameterManager.registerParameterCallback(Param::ID::Band0Gain,
        [this](float val, bool /*force*/)
        {
            eq.setBandGain(0, val);
        });

    parameterManager.registerParameterCallback(Param::ID::WetDry,
        [this](float newValue, bool force)
        {
            wetDryPotVal = std::fmin(std::fmax(newValue, 0.f), 1.f);
            // TODO do the math so that the target value at 0.5 is 0.707 for both.
            // find reference constant energy panning
            wetRamp.setTarget(wetDryPotVal, force);
            dryRamp.setTarget(1.f - wetDryPotVal, force);
        });


    parameterManager.registerParameterCallback(Param::ID::Band0Enabled,
    [this](float newValue, bool force)
    {
        enableRamp.setTarget(std::fmin(std::fmax(newValue, 0.f), 1.f), force);
    });

    parameterManager.registerParameterCallback(Param::ID::Depth,
    [this](float newValue, bool /*force*/)
    {
        // This is for Ring Mod = OUR REVERB
        ringMod.setModDepth(newValue); // Convert from % to [0; 1]
    });

    parameterManager.registerParameterCallback(Param::ID::Rate,
    [this] (float newValue, bool /*force*/)
    {
        // This is for Ring Mod = OUR REVERB
        ringMod.setModRate(newValue);
    });

    parameterManager.registerParameterCallback(Param::ID::ModType,
    [this](float newValue, bool /*force*/)
    {
        // mrta::Flanger::ModulationType modType = static_cast<mrta::Flanger::ModulationType>(std::round(newValue));
        // flanger.setModulationType(std::min(std::max(modType, mrta::Flanger::Sin), mrta::Flanger::Saw));
        
        // This is for Ring Mod = OUR REVERB
        mrta::RingMod::ModType modType = static_cast<mrta::RingMod::ModType>(std::round(newValue));
        ringMod.setModType(modType);
    });





}

FlangerAudioProcessor::~FlangerAudioProcessor()
{
}

void FlangerAudioProcessor::prepareToPlay(double newSampleRate, int samplesPerBlock)
{
    const unsigned int numChannels { static_cast<unsigned int>(std::max(getMainBusNumInputChannels(), getMainBusNumOutputChannels())) };

    flanger.prepare(newSampleRate, 20.f, numChannels);
    wetRamp.prepare(newSampleRate);
    dryRamp.prepare(newSampleRate);
    ringMod.prepare(newSampleRate);
    parameterManager.updateParameters(true);
    eq.prepare(newSampleRate, numChannels);
    fxBuffer.setSize(static_cast<int>(numChannels), samplesPerBlock);
    fxBuffer.clear();
    convBuffer.setSize(static_cast<int>(numChannels), samplesPerBlock);
    convBuffer.clear();
    convBufferDD.setSize(static_cast<int>(numChannels), samplesPerBlock);
    convBufferDD.clear();

    specs.maximumBlockSize = samplesPerBlock;
    specs.numChannels = numChannels;
    specs.sampleRate = newSampleRate;
    irLoader.prepare(specs);
    irLoaderDD.prepare(specs);
}

void FlangerAudioProcessor::releaseResources()
{
    eq.clear();
    //ringMod.clear(); // TODO
    flanger.clear();
}

void FlangerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;
    parameterManager.updateParameters();

    const unsigned int numChannels{ static_cast<unsigned int>(buffer.getNumChannels()) };
    const unsigned int numSamples{ static_cast<unsigned int>(buffer.getNumSamples()) };

    // Read both buffers for convolution
    for (int ch = 0; ch < static_cast<int>(numChannels); ++ch) {
        convBuffer.copyFrom(ch, 0, buffer, ch, 0, static_cast<int>(numSamples));
        convBufferDD.copyFrom(ch, 0, buffer, ch, 0, static_cast<int>(numSamples));
    }

    

    // Convolution 1
    juce::dsp::AudioBlock<float> firstConvBuff{ convBuffer };
    juce::dsp::ProcessContextReplacing<float>context(firstConvBuff);
    irLoader.process(context);

    // Convolution 2
    juce::dsp::AudioBlock<float> secondConvBuff{ convBufferDD };
    juce::dsp::ProcessContextReplacing<float>contextDD(secondConvBuff);
    irLoaderDD.process(contextDD);

    ringMod.process(convBuffer.getArrayOfWritePointers(), convBuffer.getArrayOfReadPointers(), convBufferDD.getArrayOfWritePointers(), convBufferDD.getArrayOfReadPointers(), numChannels, numSamples);

    // Sum both processed convolution buffers
    for (int ch = 0; ch < static_cast<int>(numChannels); ++ch)
        convBuffer.addFrom(ch, 0, convBufferDD, ch, 0, static_cast<int>(numSamples));

    // Just in place apply dryRamp
    dryRamp.applyGain(buffer.getArrayOfWritePointers(), numChannels, numSamples);

    // Just apply wetRamp to convBuffer
    wetRamp.applyGain(convBuffer.getArrayOfWritePointers(), numChannels, numSamples);
    eq.process(convBuffer.getArrayOfWritePointers(), convBuffer.getArrayOfReadPointers(), convBuffer.getNumChannels(), convBuffer.getNumSamples());

    // This can be just the addition now, since the original input buffer can be zeroed when dryRamp target is 0
    for (int ch = 0; ch < static_cast<int>(numChannels); ++ch)
        buffer.addFrom(ch, 0, convBuffer, ch, 0, static_cast<int>(numSamples));

}


void FlangerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    parameterManager.getStateInformation(destData);
}

void FlangerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    parameterManager.setStateInformation(data, sizeInBytes);
}

//==============================================================================
bool FlangerAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* FlangerAudioProcessor::createEditor() { return new FlangerAudioProcessorEditor(*this); }
const juce::String FlangerAudioProcessor::getName() const { return JucePlugin_Name; }
bool FlangerAudioProcessor::acceptsMidi() const { return false; }
bool FlangerAudioProcessor::producesMidi() const { return false; }
bool FlangerAudioProcessor::isMidiEffect() const { return false; }
double FlangerAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int FlangerAudioProcessor::getNumPrograms() { return 1; }
int FlangerAudioProcessor::getCurrentProgram() { return 0; }
void FlangerAudioProcessor::setCurrentProgram(int) { }
const juce::String FlangerAudioProcessor::getProgramName (int) { return {}; }
void FlangerAudioProcessor::changeProgramName (int, const juce::String&) { }
//==============================================================================

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FlangerAudioProcessor();
}
