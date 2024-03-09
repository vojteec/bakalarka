/*
  ==============================================================================

    ADSRComponent.h
    Created: 24 Oct 2023 1:00:10am
    Author:  vojte

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class ADSRComponent  : public juce::Component
{
public:
    ADSRComponent(SynthGrannyAudioProcessor& p);
    ~ADSRComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    Slider myAttackSlider, myDecaySlider, mySustainSlider, myReleaseSlider;
    Label myAttackLabel, myDecayLabel, mySustainLabel, myReleaseLabel;

    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> myAttackAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> myDecayAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> mySustainAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> myReleaseAttachment;

    SynthGrannyAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADSRComponent)
};
