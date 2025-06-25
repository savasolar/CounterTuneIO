#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CounterTuneIOAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    CounterTuneIOAudioProcessorEditor (CounterTuneIOAudioProcessor&);
    ~CounterTuneIOAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    static std::string vectorToString(const std::vector<int>& vec);

private:
    void timerCallback() override;

    juce::Label pitchDetectionLabel;
    juce::Label pitchStatusLabel;
    juce::Label frequencyLabel;
    juce::Label confidenceLabel;
    juce::Label melodyGenerationLabel;
    juce::Label melodyStatusLabel;
    juce::Label inputMelodyLabel;
    juce::Label generatedMelodyLabel;
    juce::Label sampleCollectionLabel;

    juce::TextButton playFileButton;
    juce::TextButton sampleButtonA;
    juce::TextButton sampleButtonB;
    juce::TextButton sampleButtonC;
    juce::TextButton sampleButtonD;
    juce::TextButton sampleButtonE;

    CounterTuneIOAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CounterTuneIOAudioProcessorEditor)
};
