#pragma once
#include <JuceHeader.h>
#include <onnxruntime_cxx_api.h>
#include <vector>

class PitchDetector {

public:
    PitchDetector();
    ~PitchDetector();

    // Initialize the ONNX Runtime session with model data
    bool initialize(const void* modelData, size_t modelDataLength);

    // Process audio buffer to detect pitch
    void processBuffer(const juce::AudioBuffer<float>& buffer);

    // Getters for pitch results
    float getCurrentFrequency() const;
    float getCurrentConfidence() const;

    // <new>
    void setInputSampleRate(double rate) { inputSampleRate = rate; }
    // </new>

private:
    Ort::Env env;
    std::unique_ptr<Ort::Session> session;
    Ort::MemoryInfo memoryInfo;
    std::vector<float> internalBuffer; // Accumulate audio samples

    // resampling 44.1 kHz to 16 kHz
    std::vector<float> resampledBuffer;
    double inputSampleRate = 0.0;
    double targetSampleRate = 16000.0; // Crepe's expected rate
    double currentPosition = 0.0; // position in input buffer for resampling

    float currentFrequency = 0.0f;
    float currentConfidence = 0.0f;
    size_t frameSize = 1024; // Adjust based on model input requirements

    // Helper to map model output to frequency
    float mapIndexToFrequency(int index) const;


};

