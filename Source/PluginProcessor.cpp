#include "PluginProcessor.h"
#include "PluginEditor.h"

CounterTuneIOAudioProcessor::CounterTuneIOAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    pitchDetector(std::make_unique<PitchDetector>())
#endif
{


    if (!pitchDetector->initialize(BinaryData::crepe_small_onnx, BinaryData::crepe_small_onnxSize)) {
        DBG("Failed to initialize CREPE model");
    }



    initializeAudioPlayback();


}

CounterTuneIOAudioProcessor::~CounterTuneIOAudioProcessor()
{
}



const juce::String CounterTuneIOAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CounterTuneIOAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CounterTuneIOAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CounterTuneIOAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CounterTuneIOAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CounterTuneIOAudioProcessor::getNumPrograms()
{
    return 1;
}

int CounterTuneIOAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CounterTuneIOAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CounterTuneIOAudioProcessor::getProgramName (int index)
{
    return {};
}

void CounterTuneIOAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void CounterTuneIOAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    pitchDetector->setInputSampleRate(sampleRate);

    if (transportSource != nullptr)
        transportSource->prepareToPlay(samplesPerBlock, sampleRate);

    if (resamplingSource != nullptr)
        resamplingSource->prepareToPlay(samplesPerBlock, sampleRate);
}

void CounterTuneIOAudioProcessor::releaseResources()
{
    if (transportSource != nullptr)
        transportSource->releaseResources();

    if (resamplingSource != nullptr)
        resamplingSource->releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CounterTuneIOAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else

    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CounterTuneIOAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // If the audio’s playing, mix it in
    if (transportSource->isPlaying() && resamplingSource != nullptr)
    {
        juce::AudioBuffer<float> testBuffer(2, buffer.getNumSamples());
        juce::AudioSourceChannelInfo channelInfo(testBuffer);
        resamplingSource->getNextAudioBlock(channelInfo);

        int numChannels = std::min(buffer.getNumChannels(), 2);
        for (int channel = 0; channel < numChannels; ++channel)
        {
            buffer.addFrom(channel, 0, testBuffer, channel, 0, buffer.getNumSamples());
        }
    }

    // Add your other processing here if you want


    // analyze input audio for pitch detection
    if (pitchDetector) {
        pitchDetector->processBuffer(buffer);
    }
}

bool CounterTuneIOAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* CounterTuneIOAudioProcessor::createEditor()
{
    return new CounterTuneIOAudioProcessorEditor (*this);
}








void CounterTuneIOAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{

}

void CounterTuneIOAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{

}



void CounterTuneIOAudioProcessor::playTestFile()
{
    DBG("Playing the audio file");

    // Stop any current playback
    transportSource->stop();
    transportSource->setSource(nullptr);
    readerSource.reset();

    // Load from binary data
    if (BinaryData::test_note_71_wav != nullptr && BinaryData::test_note_71_wavSize > 0)
    {
        // Create a memory stream from the binary data
        std::unique_ptr<juce::InputStream> inputStream(new juce::MemoryInputStream(BinaryData::test_note_71_wav, BinaryData::test_note_71_wavSize, false));
        juce::AudioFormatReader* reader = formatManager->createReaderFor(std::move(inputStream));

        if (reader != nullptr)
        {
            // Log the sample rate of the test audio file
            DBG("Test note sample rate: " + juce::String(reader->sampleRate));

            // Set up the reader source and transport
            readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
            transportSource->setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
            transportSource->setPosition(0); // Start from the beginning
            transportSource->start(); // Play
            DBG("Transport started");
        }
        else
        {
            DBG("Couldn’t create reader from binary data");
        }
    }
    else
    {
        DBG("Binary data not found");
    }
}


void CounterTuneIOAudioProcessor::initializeAudioPlayback()
{
    formatManager = std::make_unique<juce::AudioFormatManager>();
    formatManager->registerBasicFormats();

    transportSource = std::make_unique<juce::AudioTransportSource>();
    resamplingSource = std::make_unique<juce::ResamplingAudioSource>(transportSource.get(), false, 2); // Stereo output
}




float CounterTuneIOAudioProcessor::getCurrentFrequency() const {
    return pitchDetector ? pitchDetector->getCurrentFrequency() : 0.0f;
}

float CounterTuneIOAudioProcessor::getCurrentConfidence() const {
    return pitchDetector ? pitchDetector->getCurrentConfidence() : 0.0f;
}






juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CounterTuneIOAudioProcessor();
}
