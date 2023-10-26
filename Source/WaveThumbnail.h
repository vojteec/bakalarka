/*
  ==============================================================================

    WaveThumbnail.h
    Created: 23 Oct 2023 5:01:54pm
    Author:  vojte

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
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

private:
    TextButton myLoadButton{ CharPointer_UTF8("Najd\xc4\x9bte soubor") };

    std::vector<float> myAudioPoints;

    SynthGrannyAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveThumbnail)
};
