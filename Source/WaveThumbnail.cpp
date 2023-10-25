/*
  ==============================================================================

    WaveThumbnail.cpp
    Created: 23 Oct 2023 5:01:54pm
    Author:  vojte

  ==============================================================================
*/

#include <JuceHeader.h>
#include "WaveThumbnail.h"

//==============================================================================
WaveThumbnail::WaveThumbnail(SynthGrannyAudioProcessor& p) : audioProcessor (p)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

WaveThumbnail::~WaveThumbnail()
{
}

void WaveThumbnail::paint (juce::Graphics& g)
{

    g.fillAll(Colours::black);
    g.setColour(Colours::lightgreen);

    auto textBounds = getLocalBounds().reduced(10, 10);
    
    auto waveform = audioProcessor.getWave();

    if (waveform.getNumSamples() > 0)
    {
        Path p;
        myAudioPoints.clear();                                                                                                                  //stavajici body k vykresleni jsou vymazany

        auto wave = audioProcessor.getWave();                                                                                                   //vlna z procesoru je ziskana a pripravena k ulozeni
        auto ratio = wave.getNumSamples() / getWidth();                                                                                         //vytvoreni pomeru, ve kterem ma byt vlna horizontalne zkreslena
        auto buffer = wave.getReadPointer(0);                                                                                                   //vytvoreni bufferu pro ulozeni vlny

        for (int sample = 0; sample < wave.getNumSamples(); sample += ratio)
        {
            myAudioPoints.push_back(buffer[sample]);                                                                                            //po jednotlivych vzorcich se do bufferu uklada vlna a pripravi osa x
        }

        p.startNewSubPath(0, getHeight() / 2);                                                                                                  //vytvoreni nove cesty, ktera zacina vlevo uprostred

        for (int sample = 0; sample < myAudioPoints.size(); ++sample)
        {
            auto point = jmap<float>(myAudioPoints[sample], -1.0f, 1.0f, getHeight(), 0);                                                               //vykresleni hodnot na ose y
            p.lineTo(sample, point);                                                                                                            //spojeni bodu carou
        }

        g.strokePath(p, PathStrokeType(2));                                                                                                    //tloustka cary

        g.setFont(15.0f);

        g.drawFittedText("Název souboru: "+myFileName, textBounds, Justification::topRight, 1);

        auto playheadPosition = jmap<int>(audioProcessor.getSampleCount(), 0, audioProcessor.getWave().getNumSamples(), 0, getWidth());
        
        g.setColour(Colours::white);
        g.drawLine(playheadPosition, 0, playheadPosition, getHeight(), 2.0f);
    }
    else
    {
        g.setFont(60.0f);
        g.drawFittedText("Naètìte soubor metodou drag&drop", textBounds, Justification::centred, 1);
    }
}

void WaveThumbnail::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
