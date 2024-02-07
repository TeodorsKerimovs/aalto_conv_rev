#include "PluginProcessor.h"
#include "PluginEditor.h"

FlangerAudioProcessorEditor::FlangerAudioProcessorEditor(FlangerAudioProcessor& p) :
    AudioProcessorEditor(&p), audioProcessor(p),
    genericParameterEditor(audioProcessor.getParameterManager())
{
    unsigned int numParams { static_cast<unsigned int>(audioProcessor.getParameterManager().getParameters().size()) };
    unsigned int paramHeight { static_cast<unsigned int>(genericParameterEditor.parameterWidgetHeight) };

    addAndMakeVisible(genericParameterEditor);

    addAndMakeVisible(loadBtn);
    loadBtn.setButtonText("Load IR");
    loadBtn.onClick = [this]()
        {
            fileChooser = std::make_unique<juce::FileChooser>("Choose File",
                audioProcessor.root, "*");

            const auto fileChooserFlags = juce::FileBrowserComponent::openMode |
                                          juce::FileBrowserComponent::canSelectFiles |
                                          juce::FileBrowserComponent::canSelectDirectories;

            fileChooser->launchAsync(fileChooserFlags, [this](const juce::FileChooser& chooser)
                {
                    juce::File result(chooser.getResult());

                    if (result.getFileExtension() == ".wav" || result.getFileExtension() == ".mp3")
                    {
                        audioProcessor.savedFile = result;
                        audioProcessor.irLoader.reset();
                        audioProcessor.root = result.getParentDirectory().getFullPathName();
                        audioProcessor.irLoader.loadImpulseResponse(result,
                            juce::dsp::Convolution::Stereo::yes,
                            juce::dsp::Convolution::Trim::yes, 0);
                    }
                });
        };

    addAndMakeVisible(loadBtnDD);
    loadBtnDD.setButtonText("Load 2nd IR");
    loadBtnDD.onClick = [this]()
        {
            fileChooserDD = std::make_unique<juce::FileChooser>("Choose File",
                audioProcessor.rootDD, "*");

            const auto fileChooserDDFlags = juce::FileBrowserComponent::openMode |
                                            juce::FileBrowserComponent::canSelectFiles |
                                            juce::FileBrowserComponent::canSelectDirectories;

            fileChooserDD->launchAsync(fileChooserDDFlags, [this](const juce::FileChooser& chooser)
                {
                    juce::File result(chooser.getResult());

                    if (result.getFileExtension() == ".wav" || result.getFileExtension() == ".mp3")
                    {
                        audioProcessor.savedFileDD = result;
                        audioProcessor.irLoaderDD.reset();
                        audioProcessor.rootDD = result.getParentDirectory().getFullPathName();
                        audioProcessor.irLoaderDD.loadImpulseResponse(result,
                            juce::dsp::Convolution::Stereo::yes,
                            juce::dsp::Convolution::Trim::yes, 0);
                    }
                });
        };
    setSize(300, numParams * paramHeight);
}

FlangerAudioProcessorEditor::~FlangerAudioProcessorEditor()
{
}

void FlangerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void FlangerAudioProcessorEditor::resized()
{
    const auto btnX = getWidth() * 0.25;  // * JUCE_LIVE_CONSTANT(0.25);
    const auto btnY = getWidth() * 0.22; //* JUCE_LIVE_CONSTANT(0.5);
    const auto btnWidth = getWidth() * 0.25;// * JUCE_LIVE_CONSTANT(0.1);
    const auto btnHeight = btnWidth * 0.25;
    loadBtn.setBounds(btnX, btnY, btnWidth, btnHeight);


    const auto btnXDD = getWidth() * 0.65;  // * JUCE_LIVE_CONSTANT(0.25);
    const auto btnYDD = getWidth() * 0.22; //* JUCE_LIVE_CONSTANT(0.5);
    const auto btnWidthDD = getWidth() * 0.25;// * JUCE_LIVE_CONSTANT(0.1);
    const auto btnHeightDD = btnWidth * 0.25;
    loadBtnDD.setBounds(btnXDD, btnYDD, btnWidthDD, btnHeightDD);


    genericParameterEditor.setBounds(getLocalBounds());
}
