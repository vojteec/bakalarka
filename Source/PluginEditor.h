/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "WaveThumbnail.h"
#include "ADSRComponent.h"

//==============================================================================
/**
*/
class SynthGrannyAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                         public FileDragAndDropTarget,
                                         public Timer
{
public:
    SynthGrannyAudioProcessorEditor (SynthGrannyAudioProcessor&);
    ~SynthGrannyAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    bool isInterestedInFileDrag(const StringArray& files) override;
    void filesDropped(const StringArray& files, int x, int y) override;

    void timerCallback() override;

private:
    WaveThumbnail myWaveThumbnail;
    ADSRComponent myADSR;
    ImageComponent myImageComponent;

    SynthGrannyAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthGrannyAudioProcessorEditor)
};
