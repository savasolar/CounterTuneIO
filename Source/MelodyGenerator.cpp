// MelodyGenerator.cpp
#include "MelodyGenerator.h"
#include <sstream>
#include <numeric>

MelodyGenerator::MelodyGenerator()
    : env(ORT_LOGGING_LEVEL_WARNING, "MelodyGenerator"),
    memoryInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)),
    generator(std::random_device{}()) {
}

MelodyGenerator::~MelodyGenerator() {

}

bool MelodyGenerator::initialize(const void* modelData, size_t modelDataLength) {
    try {
        // create session options
        Ort::SessionOptions sessionOptions;
        sessionOptions.SetIntraOpNumThreads(1);
        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);

        // create session from model data
        session = std::make_unique<Ort::Session>(env, modelData, modelDataLength, sessionOptions);

        // verify input shape
        auto inputCount = session->GetInputCount();
        if (inputCount != 1) {
            lastError = "Expected 1 input, got " + std::to_string(inputCount);
            DBG(lastError);
            return false;
        }

        auto inputInfo = session->GetInputTypeInfo(0);
        auto inputTensorInfo = inputInfo.GetTensorTypeAndShapeInfo();
        auto inputShape = inputTensorInfo.GetShape();

        if (inputShape.size() != 3 || inputShape[0] != 128 || inputShape[1] != 32 || inputShape[2] != 130) {
            lastError = "Invalid input shape";
            DBG(lastError);
            return false;
        }

        DBG("Model initialized successfully");
        return true;
    }
    catch (const Ort::Exception& e) {
        lastError = "ONNX Runtime error: " + std::string(e.what());
        DBG(lastError);
        return false;
    }
    catch (const std::exception& e) {
        lastError = "Initialization error: " + std::string(e.what());
        DBG(lastError);
        return false;
    }
}

std::vector<float> MelodyGenerator::eventsToOnehot(const std::vector<int>& events) {
    const int seqLength = 32;
    const int numClasses = 130;
    std::vector<float> onehot(seqLength * numClasses, 0.0f);

    for (int i = 0; i < seqLength && i < events.size(); ++i) {
        int event = events[i];
        int index = (event == -1) ? 0 : (event == -2) ? 1 : event + 2;
        if (index >= 0 && index < numClasses) {
            onehot[i * numClasses + index] = 1.0f;
        }
    }

    // Debug: Check a few entries
    std::string onehotDbg = "First few onehot values: ";
    for (int i = 0; i < std::min(5, seqLength); ++i) {
        int idx = i * numClasses;
        for (int j = 0; j < numClasses; ++j) {
            if (onehot[idx + j] == 1.0f) {
                onehotDbg += std::to_string(j) + " ";
                break;
            }
        }
    }
    DBG(onehotDbg);
    return onehot;
}

std::vector<float> MelodyGenerator::createBatchInput(const std::vector<float>& onehot, int batchSize) {
    const int seqLength = 32;
    const int numClasses = 130;

    const size_t totalSize = batchSize * seqLength * numClasses;

    // preallocate w corr size
    std::vector<float> batchInput(totalSize, 0.0f);

    // copy the onehot vector for each batch
    for (int b = 0; b < batchSize; ++b) {
        const size_t offset = b * seqLength * numClasses;
        std::copy(onehot.begin(), onehot.end(), batchInput.begin() + offset);
    }

    DBG("Batch input created with size: " + std::to_string(batchInput.size()));
    return batchInput;
}

std::string MelodyGenerator::eventsToString(const std::vector<int>& events) {
    std::string s;
    for (int e : events) s += std::to_string(e) + " ";
    return s;
}

std::vector<int> MelodyGenerator::generateMelody(std::vector<int>& events, float temperature, int steps)
{

    try {

        if (!session) {
            DBG("Error: Model not initialized");
            return std::vector<int>();
        }
        
        // step 1: convert text to events... calready accounted for
        if (events.size() < 32) {
            events.resize(32, -2);  // Use resize instead of insert
        }
        else if (events.size() > 32) {
            events.resize(32);
        }
        DBG("Padded events: " + eventsToString(events));


        // step 2: convert to one-hot
        std::vector<float> onehot = eventsToOnehot(events);

        // step 3: create batch input with explicit size
        const size_t batchSize = 128;
        const size_t seqLength = 32;
        const size_t numClasses = 130;
        std::vector<float> batchInput = createBatchInput(onehot, batchSize);

        // verify batch input size
        if (batchInput.size() != batchSize * seqLength * numClasses) {
            DBG("Error: Invalid batch input size");
            return std::vector<int>();
        }

        // step 4: prepare input tensor
        std::vector<int64_t> inputShape = { batchSize, seqLength, numClasses };
        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo,
            batchInput.data(),
            batchInput.size(),
            inputShape.data(),
            inputShape.size()
        );

        // step 5: run the model
        auto inputName = session->GetInputNameAllocated(0, allocator);
        auto outputName = session->GetOutputNameAllocated(0, allocator);

        const char* inputNames[] = { inputName.get() };
        const char* outputNames[] = { outputName.get() };

        std::vector<Ort::Value> outputTensors = session->Run(
            Ort::RunOptions{ nullptr },
            inputNames,
            &inputTensor,
            1,
            outputNames,
            1
        );

        // step 6: process output
        if (outputTensors.empty()) {
            DBG("Error: No output tensors");
            return std::vector<int>();
        }

        float* outputData = outputTensors[0].GetTensorMutableData<float>();
        std::vector<float> outputProbs(seqLength * numClasses);
        std::copy(outputData, outputData + seqLength * numClasses, outputProbs.begin());

        // step 7:  generate events
        std::vector<int> generatedEvents;
        generatedEvents.reserve(steps);


        for (int t = 0; t < steps; ++t) {
            std::vector<float> stepProbs(numClasses);
            std::copy(outputProbs.begin() + t * numClasses,
                outputProbs.begin() + (t + 1) * numClasses,
                stepProbs.begin());

            // Normalize probabilities
            float sumProbs = std::accumulate(stepProbs.begin(), stepProbs.end(), 0.0f);
            if (sumProbs > 0) {
                for (float& p : stepProbs) {
                    p /= sumProbs;
                }
            }

            // Apply temperature scaling
            if (temperature != 0.8f) {
                std::vector<float> logits(numClasses);
                for (size_t c = 0; c < numClasses; ++c) {
                    logits[c] = logf(std::max(stepProbs[c], 1e-7f));
                }

                float scale = temperature / 0.8f;
                for (float& logP : logits) {
                    logP /= scale;
                }

                std::vector<float> expLogits(numClasses);
                float sumExp = 0.0f;
                for (size_t c = 0; c < numClasses; ++c) {
                    expLogits[c] = expf(logits[c]);
                    sumExp += expLogits[c];
                }

                if (sumExp > 0) {
                    for (size_t c = 0; c < numClasses; ++c) {
                        stepProbs[c] = expLogits[c] / sumExp;
                    }
                }
            }

            // Sample next event
            std::discrete_distribution<int> dist(stepProbs.begin(), stepProbs.end());
            int idx = dist(generator);
            int event = (idx == 0) ? -1 : (idx == 1) ? -2 : idx - 2;
            generatedEvents.push_back(event);

            if (t < 3) {
                DBG("Step " + std::to_string(t) + ": event " + std::to_string(event) +
                    " (idx " + std::to_string(idx) + ")");
            }
        }



        return generatedEvents;

    }
    catch (const Ort::Exception& e) {
        DBG("ONNX Runtime error: " + std::string(e.what()));
        return std::vector<int>();
    }
    catch (const std::exception& e) {
        DBG("Generation error: " + std::string(e.what()));
        return std::vector<int>();
    }

    
    
    //    return std::vector<int>();
}