/*
  ==============================================================================

    GrainComponent.h
    Created: 10 Apr 2024 11:07:24pm
    Author:  vojte

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class GrainComponent : public juce::Component
{
public:
    GrainComponent(SynthGrannyAudioProcessor& p);
    ~GrainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    Slider myGrainLengthSlider, myGrainAttackSlider, myGrainDecaySlider, myGrainOverlapSlider;
    Label myGrainLengthLabel, myGrainAttackLabel, myGrainDecayLabel, myGrainOverlapLabel;

    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> myGrainLengthAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> myGrainAttackAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> myGrainDecayAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> myGrainOverlapAttachment;

    SynthGrannyAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrainComponent)
};