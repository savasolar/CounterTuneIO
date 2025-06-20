#include "PitchDetector.h"

PitchDetector::PitchDetector()
    : env(ORT_LOGGING_LEVEL_WARNING, "PitchDetector"),
    memoryInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)) {}

PitchDetector::~PitchDetector() {}

bool PitchDetector::initialize(const void* modelData, size_t modelDataLength) {
    try {
        Ort::SessionOptions sessionOptions;
        sessionOptions.SetIntraOpNumThreads(1);
        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);

        // Load the model directly from BinaryData
        session = std::make_unique<Ort::Session>(env, modelData, modelDataLength, sessionOptions);

        // Verify input shape (example: [1, 1024] for a frame of 1024 samples)
        auto inputInfo = session->GetInputTypeInfo(0);
        auto inputTensorInfo = inputInfo.GetTensorTypeAndShapeInfo();
        auto inputShape = inputTensorInfo.GetShape();
        DBG("Input shape: [" + juce::String(inputShape[0]) + ", " + juce::String(inputShape[1]) + "]");
        if (inputShape.size() != 2 || (inputShape[0] != 1 && inputShape[0] != -1) || inputShape[1] != 1024) {
            DBG("Invalid input shape for CREPE model");
            return false;
        }

        // Verify output shape (example: [1, num_bins] for pitch probabilities)
        auto outputInfo = session->GetOutputTypeInfo(0);
        auto outputTensorInfo = outputInfo.GetTensorTypeAndShapeInfo();
        auto outputShape = outputTensorInfo.GetShape();
        DBG("Output shape: [" + juce::String(outputShape[0]) + ", " + juce::String(outputShape[1]) + "]");
        // Adjust validation based on actual model output

        DBG("CREPE model initialized successfully");
        return true;
    }
    catch (const Ort::Exception& e) {
        DBG("ONNX Runtime error: " + juce::String(e.what()));
        return false;
    }
}

void PitchDetector::processBuffer(const juce::AudioBuffer<float>& buffer) {
    if (buffer.getNumChannels() < 1 || !session) return;

    // Use first channel (mono assumption; adjust for stereo if needed)
    const float* channelData = buffer.getReadPointer(0);
    int numSamples = buffer.getNumSamples();
    internalBuffer.insert(internalBuffer.end(), channelData, channelData + numSamples);

    // Process frames when enough samples are available
    while (internalBuffer.size() >= frameSize) {
        std::vector<float> frame(internalBuffer.begin(), internalBuffer.begin() + frameSize);

        // Prepare input tensor
        std::vector<int64_t> inputShape = { 1, static_cast<int64_t>(frameSize) };
        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo, frame.data(), frame.size(), inputShape.data(), inputShape.size());

        // Get input/output names
        Ort::AllocatorWithDefaultOptions allocator;
        auto inputName = session->GetInputNameAllocated(0, allocator);
        auto outputName = session->GetOutputNameAllocated(0, allocator);
        const char* inputNames[] = { inputName.get() };
        const char* outputNames[] = { outputName.get() };

        // Run inference
        std::vector<Ort::Value> outputTensors = session->Run(
            Ort::RunOptions{ nullptr }, inputNames, &inputTensor, 1, outputNames, 1);

        if (!outputTensors.empty()) {
            float* outputData = outputTensors[0].GetTensorMutableData<float>();
            int numBins = outputTensors[0].GetTensorTypeAndShapeInfo().GetShape()[1];

            // Find pitch with highest probability
            auto maxIt = std::max_element(outputData, outputData + numBins);
            int maxIndex = std::distance(outputData, maxIt);
            currentConfidence = *maxIt;
            currentFrequency = mapIndexToFrequency(maxIndex);

            // Log the prediction details
            DBG("Max index: " + juce::String(maxIndex) + ", Confidence: " + juce::String(currentConfidence) + ", Frequency: " + juce::String(currentFrequency) + " Hz");
        }

        // Remove processed samples
        internalBuffer.erase(internalBuffer.begin(), internalBuffer.begin() + frameSize);
    }
}


float PitchDetector::getCurrentFrequency() const { return currentFrequency; }
float PitchDetector::getCurrentConfidence() const { return currentConfidence; }

//float PitchDetector::mapIndexToFrequency(int index) const {
//    // Placeholder: Map bin index to frequency based on CREPE model specifics
//    // Example: CREPE typically uses 360 bins over 6 octaves (C1 to C7)
//    // Adjust this based on the actual model’s output mapping
//    float cents = index * 20.0f; // Assuming 20-cent steps
//    float refFreq = 10.0f; // Reference frequency (e.g., 10 Hz)
//    return refFreq * powf(2.0f, cents / 1200.0f);
//}

float PitchDetector::mapIndexToFrequency(int index) const {
    const float f_min = 32.7f; // C1
    const float cents_per_bin = 20.0f; // CREPE uses 20 cents per bin
    return f_min * powf(2.0f, (index * cents_per_bin) / 1200.0f);
}