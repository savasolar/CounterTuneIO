#include "PluginProcessor.h"
#include "PluginEditor.h"

CounterTuneIOAudioProcessorEditor::CounterTuneIOAudioProcessorEditor (CounterTuneIOAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (800, 600);
    startTimer(50);
}

CounterTuneIOAudioProcessorEditor::~CounterTuneIOAudioProcessorEditor()
{
    stopTimer();
}

std::string CounterTuneIOAudioProcessorEditor::vectorToString(const std::vector<int>& vec)
{
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (i > 0) oss << ", "; // Add comma and space between elements
        oss << vec[i];
    }
    oss << "]";
    return oss.str();
}

void CounterTuneIOAudioProcessorEditor::timerCallback() {


    pitchStatusLabel.setText(audioProcessor.isPitchDetectorReady() ? "STATUS: READY" : "STATUS: LOADING...", juce::dontSendNotification);
    frequencyLabel.setText("FREQ: " + juce::String(audioProcessor.getCurrentFrequency(), 2) + " Hz", juce::dontSendNotification);
    confidenceLabel.setText("CONFIDENCE: " + juce::String(audioProcessor.getCurrentConfidence(), 3), juce::dontSendNotification);

    // to do: set melodyStatusLabel text

    // to do: set inputMelodyLabel text
    inputMelodyLabel.setText("INPUT: " + vectorToString(audioProcessor.getCapturedMelody()), juce::dontSendNotification);

    // to do: set generatedMelodyLabel text
}

void CounterTuneIOAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB(0, 0, 0));
    g.setColour (juce::Colours::white);
    g.fillRect(0, 75, 800, 1);
    g.fillRect(0, 250, 800, 1);
    g.fillRect(0, 425, 800, 1);
    g.fillRect(200, 75, 1, 525);
}

void CounterTuneIOAudioProcessorEditor::resized()
{
    pitchDetectionLabel.setText("PITCH DETECTION", juce::dontSendNotification);
    pitchDetectionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    pitchDetectionLabel.setJustificationType(juce::Justification::centredLeft);
    pitchDetectionLabel.setBounds(12, 75, 188, 175);
    addAndMakeVisible(pitchDetectionLabel);

    pitchStatusLabel.setText("STATUS: LOADING...", juce::dontSendNotification);
    pitchStatusLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    pitchStatusLabel.setJustificationType(juce::Justification::centredLeft);
    pitchStatusLabel.setBounds(212, 75, 588, 58);
    addAndMakeVisible(pitchStatusLabel);

    frequencyLabel.setText("FREQ: ", juce::dontSendNotification);
    frequencyLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    frequencyLabel.setJustificationType(juce::Justification::centredLeft);
    frequencyLabel.setBounds(212, 133, 588, 58);
    addAndMakeVisible(frequencyLabel);

    confidenceLabel.setText("CONFIDENCE: ", juce::dontSendNotification);
    confidenceLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    confidenceLabel.setJustificationType(juce::Justification::centredLeft);
    confidenceLabel.setBounds(212, 191, 588, 59);
    addAndMakeVisible(confidenceLabel);

    melodyGenerationLabel.setText("MELODY GENERATION", juce::dontSendNotification);
    melodyGenerationLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    melodyGenerationLabel.setJustificationType(juce::Justification::centredLeft);
    melodyGenerationLabel.setBounds(12, 250, 188, 175);
    addAndMakeVisible(melodyGenerationLabel);

    melodyStatusLabel.setText("STATUS: LOADING...", juce::dontSendNotification);
    melodyStatusLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    melodyStatusLabel.setJustificationType(juce::Justification::centredLeft);
    melodyStatusLabel.setBounds(212, 250, 588, 58);
    addAndMakeVisible(melodyStatusLabel);

    inputMelodyLabel.setText("INPUT: [-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1]", juce::dontSendNotification);
    inputMelodyLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    inputMelodyLabel.setJustificationType(juce::Justification::centredLeft);
    inputMelodyLabel.setBounds(212, 308, 588, 58);
    addAndMakeVisible(inputMelodyLabel);

    generatedMelodyLabel.setText("OUTPUT: [-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1]", juce::dontSendNotification);
    generatedMelodyLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    generatedMelodyLabel.setJustificationType(juce::Justification::centredLeft);
    generatedMelodyLabel.setBounds(212, 366, 588, 59);
    addAndMakeVisible(generatedMelodyLabel);

    sampleCollectionLabel.setText("SAMPLE COLLECTION", juce::dontSendNotification);
    sampleCollectionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    sampleCollectionLabel.setJustificationType(juce::Justification::centredLeft);
    sampleCollectionLabel.setBounds(12, 425, 188, 175);
    addAndMakeVisible(sampleCollectionLabel);

    playFileButton.setButtonText("PLAY TEST NOTE");
    playFileButton.setBounds(12, 12, 150, 50);
    addAndMakeVisible(playFileButton);
    playFileButton.onClick = [this]
    {
        DBG("playFileButton clicked");
        audioProcessor.playTestFile();
    };

    sampleButtonA.setButtonText("A");
    sampleButtonA.setBounds(212, 488, 50, 50);
    addAndMakeVisible(sampleButtonA);
    sampleButtonA.onClick = [this]
    {
        DBG("sampleButtonA clicked");
    };

    sampleButtonB.setButtonText("B");
    sampleButtonB.setBounds(274, 488, 50, 50);
    addAndMakeVisible(sampleButtonB);
    sampleButtonB.onClick = [this]
    {
        DBG("sampleButtonB clicked");
    };

    sampleButtonC.setButtonText("C");
    sampleButtonC.setBounds(336, 488, 50, 50);
    addAndMakeVisible(sampleButtonC);
    sampleButtonC.onClick = [this]
    {
        DBG("sampleButtonC clicked");
    };

    sampleButtonD.setButtonText("D");
    sampleButtonD.setBounds(398, 488, 50, 50);
    addAndMakeVisible(sampleButtonD);
    sampleButtonD.onClick = [this]
    {
        DBG("sampleButtonD clicked");
    };

    sampleButtonE.setButtonText("E");
    sampleButtonE.setBounds(460, 488, 50, 50);
    addAndMakeVisible(sampleButtonE);
    sampleButtonE.onClick = [this]
    {
        DBG("sampleButtonE clicked");
    };
}
