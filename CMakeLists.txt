cmake_minimum_required(VERSION 3.15)

project(CounterTuneIO VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

# JUCE path
set(JUCE_PATH "C:/Program Files/JUCE")

# Include JUCE
add_subdirectory(${JUCE_PATH} JUCE)

# Conditional formats
set(PLUGIN_FORMATS VST3)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
    add_compile_options(
        $<$<CONFIG:Debug>:/MTd>
        $<$<CONFIG:Release>:/MT>
    )
endif()

# Plugin setup
juce_add_plugin(CounterTuneIO
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE GenD
    FORMATS ${PLUGIN_FORMATS}
    PRODUCT_NAME "CounterTuneIO"
    COMPANY_NAME "54v450l4r"
    BUNDLE_ID "com.54v450l4r.CounterTuneIO"
    PLUGIN_IS_A_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    IS_MIDI_EFFECT FALSE
    IS_SYNTH FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    MIDI_INPUT_NAME "MIDI Input"
    MIDI_OUTPUT_NAME "MIDI Output"
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_COPY_DIR_DISABLE_AUTO_COPY TRUE
)

target_compile_definitions(CounterTuneIO
    PRIVATE 
        JUCE_DISABLE_VST2_BUILDING=1
        JUCE_PLUGINHOST_VST3=1
    	JUCE_PLUGINHOST_VST=0
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
)

# Source files
target_sources(CounterTuneIO PRIVATE
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PitchDetector.cpp
    Source/PitchDetector.h
    Source/MelodyGenerator.cpp
    Source/MelodyGenerator.h
)

# Binary data
juce_add_binary_data(BinaryResources SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/test_note_71.wav
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/crepe_small.onnx
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/melody_model.onnx
)

# Set onnx runtime path
set(ONNXRUNTIME_DIR "C:/repos/onnxruntime-static-debug")
# set(ONNXRUNTIME_DIR "C:/repos/onnxruntime-static")

# include onnx runtime headers
target_include_directories(CounterTuneIO PRIVATE ${ONNXRUNTIME_DIR}/include)

# Link ONNX Runtime library with full path
target_link_libraries(CounterTuneIO PRIVATE "${ONNXRUNTIME_DIR}/lib/onnxruntime.lib")

# Link everything
target_link_libraries(CounterTuneIO PRIVATE 
    BinaryResources
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_osc
)

juce_generate_juce_header(CounterTuneIO)