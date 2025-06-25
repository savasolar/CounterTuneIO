// MelodyGenerator.h
#pragma once
#include <JuceHeader.h>
#include <onnxruntime_cxx_api.h>
#include <vector>
#include <random>

class MelodyGenerator {
public:
	MelodyGenerator();
	~MelodyGenerator();

	bool initialize(const void* modelData, size_t modelDataLength);

	// generate melody from input vector<int>
	std::vector<int> generateMelody(std::vector<int>& events, float temperature = 0.8f, int steps = 32);

	// get last error message if initialization fails
	std::string getLastError() const { return lastError; }

	bool isInitialized() const { return session != nullptr; }

private:
	// onnx runtime env & session
	Ort::Env env;
	std::unique_ptr<Ort::Session> session;
	Ort::AllocatorWithDefaultOptions allocator;
	Ort::MemoryInfo memoryInfo;

	// random number generator for sampling (? why do I need this ?)
	std::mt19937 generator;

	// error tracking
	std::string lastError;

	// helper functions
	std::vector<float> eventsToOnehot(const std::vector<int>& events);
	std::vector<float> createBatchInput(const std::vector<float>& onehot, int batchSize = 128);

	std::string eventsToString(const std::vector<int>& events); // for debugging
};