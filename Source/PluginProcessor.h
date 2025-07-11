#pragma once

#include <JuceHeader.h>
#include "PitchDetector.h"
#include "MelodyGenerator.h"

class CounterTuneIOAudioProcessor : public juce::AudioProcessor
{
public:
    CounterTuneIOAudioProcessor();
    ~CounterTuneIOAudioProcessor() override;
    // Standard AudioProcessor overrides __________________________________________________________________________________________________
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Method to play the audio file
    void playTestFile();

    // public crepe getters
    bool isPitchDetectorReady() const { return pitchDetectorReady.load(); }
    float getCurrentFrequency() const;
    float getCurrentConfidence() const;

    // public generator getters
    bool isGeneratorReady() const { return generatorReady.load(); }

    // melody access
    const std::vector<int>& getCapturedMelody() const { return capturedMelody; };
    const std::vector<int>& getGeneratedMelody() const { return generatedMelody; };

//    std::vector<int> getCapturedMelodySnapshot() const;

private:


    // Audio playback for testing _________________________________________________________________________________________________________
    std::unique_ptr<juce::AudioFormatManager> formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::AudioTransportSource> transportSource;
    std::unique_ptr<juce::ResamplingAudioSource> resamplingSource;
    void initializeAudioPlayback();


    // timing
    bool active = false;
    float bpm = 140.0f;
    double captureCounter = 0.0;
    double samplesPerSymbol = 0.0;
//    double totalSamples = 0.0;
//    double nextSymbolTime = 0.0;
    void updateSamplesPerSymbol() { if (active) samplesPerSymbol = 60.0 / bpm * getSampleRate() / 4.0 * 8.0 / 8.0; }
    std::atomic<bool> awaitingResponse{ false };
    bool shouldResetCapturedMelody = false;
    bool shouldResetTimekeeping = true;
    bool inputNoteActive = false;
    int testInt = 0;



    // Pitch detection ____________________________________________________________________________________________________________________
    std::unique_ptr<PitchDetector> pitchDetector;
    class PitchDetectionThread : public juce::Thread {
    public:
        PitchDetectionThread(PitchDetector& detector)
            : juce::Thread("Pitch Detection Thread"), pitchDetector(detector) {}
        void run() override;
        void processAudio(const juce::AudioBuffer<float>& buffer);
    private:
        PitchDetector& pitchDetector;
        juce::AudioBuffer<float> accumulatedBuffer;  // Changed name
        juce::CriticalSection bufferLock;
        int writePosition = 0;  // Track where to write next
    };
    std::unique_ptr<PitchDetectionThread> pitchThread;
    std::atomic<bool> pitchDetectorReady{ false };

    // Melody capture _____________________________________________________________________________________________________________________
    std::vector<int> capturedMelody
    {
        -2, -2, -2, -2,
        -2, -2, -2, -2,
        -2, -2, -2, -2,
        -2, -2, -2, -2,
        -2, -2, -2, -2,
        -2, -2, -2, -2,
        -2, -2, -2, -2,
        -2, -2, -2, -2
    };
    int frequencyToMidiNote(float frequency) const;
    juce::CriticalSection capturedMelodyLock;
    int capturePosition = 0;


    // Melody generation __________________________________________________________________________________________________________________
    std::vector<int> generatedMelody
    {
        -2, -2, -2, -2,
        -2, -2, -2, -2,
        -2, -2, -2, -2,
        -2, -2, -2, -2,
        -2, -2, -2, -2,
        -2, -2, -2, -2,
        -2, -2, -2, -2,
        -2, -2, -2, -2
    };
    std::unique_ptr<MelodyGenerator> melodyGenerator;


    //class MelodyGenerationThread : public juce::Thread {
    // ...
    //};

    std::atomic<bool> generatorReady{ false };











    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CounterTuneIOAudioProcessor)
};