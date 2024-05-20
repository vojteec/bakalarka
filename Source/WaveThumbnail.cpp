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

    auto textBounds = getLocalBounds().reduced(10, 10);
    auto colourParametersTextBounds = getLocalBounds().reduced(10, 35);
    
    auto waveform = audioProcessor.getWave();

    myLoadButton.onClick = [&]() { audioProcessor.loadFileViaButton();                                                                         //boolean, ktery rozhoduje, kdy se vykresli waveform je aktivovan
    repaint(); };                                                                                                                              //spusti se proces vykreslovani
    g.setFont(15.0f);
    myLoadButton.setColour(TextButton::buttonColourId, Colours::silver);
    myLoadButton.setColour(TextButton::textColourOffId, Colours::black);
    addAndMakeVisible(myLoadButton);

    if (waveform.getNumSamples() > 0)
    {
        Path p;
        myAudioPoints.clear();
        g.setColour(Colours::green);                                                                                                              //stavajici body k vykresleni jsou vymazany

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

        g.strokePath(p, PathStrokeType(1.5));                                                                                                    //tloustka cary
        g.setColour(Colours::lightgreen);
        g.fillPath(p);

        g.setFont(15.0f);

        g.drawFittedText("Soubor: " + myFileName, textBounds, Justification::topRight, 1);
        String delkaGrainu = CharPointer_UTF8("D\xc3\xa9lka grainu: ");
        String adObalka = CharPointer_UTF8("AD ob\xc3\xa1lka: ");
        String prekryti = CharPointer_UTF8("P\xc5\x99\ekryt\xc3\xad: ");

        g.fillRect(Rectangle(5, getHeight() - 55, getWidth() - 10, 1));
        g.drawFittedText(CharPointer_UTF8("|   |   |   |   |   |   |   |   |   |         S   v   \xc4\x9b   t   e   l   n   \xc3\xa9       p   a   r   a   m   e   t   r   y         |   |   |   |   |   |   |   |   |   |"), colourParametersTextBounds, Justification::centredBottom, 1);
        g.fillRect(Rectangle(5, getHeight() - 30, getWidth() - 10, 1));
        g.drawFittedText(delkaGrainu + String(audioProcessor.averageHueRanged, 2) + " ms", textBounds, Justification::bottomLeft, 1);
        g.drawFittedText(adObalka + String(audioProcessor.averageSaturationRanged, 2) + " %", textBounds, Justification::centredBottom, 1);
        g.drawFittedText(prekryti + String(audioProcessor.averageLightnessRanged, 2) + " %", textBounds, Justification::bottomRight, 1);

        auto playheadPosition = jmap<int>(audioProcessor.getSampleCount(), 0, audioProcessor.getWave().getNumSamples(), 0, getWidth());
        if (playheadPosition > audioProcessor.getWave().getNumSamples())
        {
            playheadPosition = 240;
        }
        g.drawLine(playheadPosition, 0, playheadPosition, getHeight(), 2.0f);

        /*g.setColour(Colours::grey.darker().withAlpha(0.5f));
        g.fillRect(0, 0, playheadPosition, getHeight());*/
    }
    else
    {
        g.setColour(Colours::lightgreen);
        g.setFont(40.0f);
        g.drawFittedText(CharPointer_UTF8("Na\xc4\x8dt\xc4\x9bte soubor metodou drag&drop"), textBounds, Justification::centred, 1);
    }
}

void WaveThumbnail::resized()
{
    myLoadButton.setBoundsRelative(0.01f, 0.02f, 0.2f, 0.1f);
}
