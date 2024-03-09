/*
  ==============================================================================

  This class was taken from the Tutorial "Looping audio using the AudioSampleBuffer class (advanced)"
  at https://www.juce.com/doc/tutorial_looping_audio_sample_buffer_advanced

  ==============================================================================
*/

#include <JuceHeader.h>

#ifndef REFERENCECOUNTEDBUFFER_H_INCLUDED
#define REFERENCECOUNTEDBUFFER_H_INCLUDED

class ReferenceCountedBuffer : public juce::ReferenceCountedObject
{
public:
    typedef juce::ReferenceCountedObjectPtr<ReferenceCountedBuffer> Ptr; // [1]

    ReferenceCountedBuffer(const juce::String& nameToUse,
        int numChannels,
        int numSamples) : name(nameToUse),
        buffer(numChannels, numSamples) // [2]
    {
        DBG(
            String("Buffer named '") + name +
            "' constructed. numChannels = " + String(numChannels) +
            ", numSamples = " + String(numSamples));
    }

    ~ReferenceCountedBuffer()
    {
        DBG(String("Buffer named '") + name + "' destroyed");
    }

    juce::AudioSampleBuffer* getAudioSampleBuffer() // [3]
    {
        return &buffer;
    }

private:
    juce::String name;
    juce::AudioSampleBuffer buffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReferenceCountedBuffer)
};

#endif  // REFERENCECOUNTEDBUFFER_H_INCLUDED