/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
//#include <juce_video/juce_video.h>
//#include <juce_video/capture/juce_CameraDevice.h>
using namespace juce;

//==============================================================================
/**
*/
class SynthGrannyAudioProcessor : public AudioProcessor,
                                  public ValueTree::Listener
#if JucePlugin_Enable_ARA
    , public juce::AudioProcessorARAExtension
#endif
{
public:
    //==============================================================================
    SynthGrannyAudioProcessor();
    ~SynthGrannyAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void colourModifier();

    void loadFileViaButton();
    void loadFileViaDragNDrop(const String& path);
    void granulisation();
    void granulisationByColour(float averageHue, float averageSaturation, float averageLightness);
    void readWaveform(AudioBuffer<float> granulizedBuffer);
    void degranulize();

    int getNumSounds() { return myGrannySynth.getNumSounds(); }
    AudioBuffer<float>& getWave() { return myWave; }

    void updateADSR();
    //void updateGrain();

    ADSR::Parameters& getADSRParams() { return myADSRParams; }
    //GrainComponent::Parameters& getGrainParams() { return myGrainParams; } //????????????????
    AudioProcessorValueTreeState& getValTrSt() { return myValTrSt; }

    std::atomic<bool>& isNotePlayed() { return myIsNotePlayed; }
    std::atomic<int>& getSampleCount() { return mySampleCount; }

    float averageHueRanged = 50.0f;
    float averageSaturationRanged = 50.0f;
    float averageLightnessRanged = 25.0f;

    //String averageHueString = String(averageHueRanged);
    //String averageSaturationString = String(averageSaturationRanged);
    //String averageLightnessString = std::to_string(averageLightnessRanged);

private:
    Synthesiser myGrannySynth;
    const int myNumVoices{ 16 };
    AudioBuffer<float> myWave;

    AudioBuffer<float> originalBuffer;

    ADSR::Parameters myADSRParams;
    //Grain::Parameters myGrainParams; //????????????????

    CameraDevice* myCameraDevice = CameraDevice::openDevice(0);

    AudioFormatManager myFormatManager;
    AudioFormatReader* myFormatReader{ nullptr };

    AudioProcessorValueTreeState myValTrSt;                         
    AudioProcessorValueTreeState::ParameterLayout createParams(); 

    void averagePixel(const Image& img);

    void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property) override;

    std::atomic<bool> myShouldUpdate{ false };
    std::atomic<bool> myIsNotePlayed{ false };
    std::atomic<int> mySampleCount{ 0 };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthGrannyAudioProcessor)
};
