#pragma once

#include <JuceHeader.h>
#include "PitchDetector.h"

class CounterTuneIOAudioProcessor  : public juce::AudioProcessor
{
public:
    CounterTuneIOAudioProcessor();
    ~CounterTuneIOAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;


    // Method to play the audio file
    void playTestFile();


    float getCurrentFrequency() const;
    float getCurrentConfidence() const;


private:


    // Audio playback
    std::unique_ptr<juce::AudioFormatManager> formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::AudioTransportSource> transportSource;
    std::unique_ptr<juce::ResamplingAudioSource> resamplingSource;



    std::unique_ptr<PitchDetector> pitchDetector;



    void initializeAudioPlayback();



    // <new>

    //class PitchDetectionThread : public juce::Thread {
    //public:
    //    PitchDetectionThread(PitchDetector& detector)
    //        : juce::Thread("Pitch Detection Thread"), pitchDetector(detector) {}
    //    void run() override;
    //    void processAudio(const juce::AudioBuffer<float>& buffer);
    //private:
    //    PitchDetector& pitchDetector;
    //    juce::AudioBuffer<float> bufferToProcess;
    //    juce::CriticalSection bufferLock;
    //};

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

    // </new>

    // <new>
    std::unique_ptr<PitchDetectionThread> pitchThread;
    // </new>
    


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CounterTuneIOAudioProcessor)
};
