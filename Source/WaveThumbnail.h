/*
  ==============================================================================

    WaveThumbnail.h
    Created: 23 Oct 2023 5:01:54pm
    Author:  vojte

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <format>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class WaveThumbnail  : public juce::Component
{
public:
    WaveThumbnail (SynthGrannyAudioProcessor& p);
    ~WaveThumbnail() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    bool myShouldBePainting{ false };

    String myFileName{ " " };
    //String mySat = audioProcessor.averageSaturationString;
    //String myLit = audioProcessor.averageLightnessString;

private:
    ImageButton myLoadButton/*{ CharPointer_UTF8("Na\xc4\x8dt\xc4\x9bte soubor do sampleru bez granulizace") }*/;

    std::vector<float> myAudioPoints;

    SynthGrannyAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveThumbnail)
};
