/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
using namespace juce;

//==============================================================================
/**
*/
class SynthGrannyAudioProcessor : public juce::AudioProcessor,
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

    void loadFileViaButton();
    void loadFileViaDragNDrop(const String& path);
    void readWaveform(AudioBuffer<float> granulizedBuffer);

    int getNumSounds() { return myGrannySynth.getNumSounds(); }
    AudioBuffer<float>& getWave() { return myWave; }

    void updateADSR();

    ADSR::Parameters& getADSRParams() { return myADSRParams; }
    AudioProcessorValueTreeState& getValTrSt() { return myValTrSt; }

    std::atomic<bool>& isNotePlayed() { return myIsNotePlayed; }
    std::atomic<int>& getSampleCount() { return mySampleCount; }

private:
    Synthesiser myGrannySynth;
    const int myNumVoices{ 16 };
    AudioBuffer<float> myWave;

    ADSR::Parameters myADSRParams;

    AudioFormatManager myFormatManager;
    AudioFormatReader* myFormatReader{ nullptr };

    AudioProcessorValueTreeState myValTrSt;
    AudioProcessorValueTreeState::ParameterLayout createParams();
    void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property) override;

    std::atomic<bool> myShouldUpdate{ false };
    std::atomic<bool> myIsNotePlayed{ false };
    std::atomic<int> mySampleCount{ 0 };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthGrannyAudioProcessor)
};
