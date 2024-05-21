/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SynthGrannyAudioProcessorEditor::SynthGrannyAudioProcessorEditor (SynthGrannyAudioProcessor& p)                                                 //konstruktor editoru
    : AudioProcessorEditor (&p), myWaveThumbnail (p), myADSR(p), myGrainComponent(p), audioProcessor (p) 
{
    Image webcamButton = ImageCache::getFromMemory(BinaryData::buttonSetParametersHSL_PNG, BinaryData::buttonSetParametersHSL_PNGSize);
    Image webcamButtonDown = ImageCache::getFromMemory(BinaryData::buttonSetParametersHSLDown_PNG, BinaryData::buttonSetParametersHSLDown_PNGSize);

    Image granulisationButton = ImageCache::getFromMemory(BinaryData::buttonSetParameters_PNG, BinaryData::buttonSetParameters_PNGSize);
    Image granulistaionButtonDown = ImageCache::getFromMemory(BinaryData::buttonSetParametersDown_PNG, BinaryData::buttonSetParametersDown_PNGSize);

    Image degranulisationButton = ImageCache::getFromMemory(BinaryData::buttonDegranulize_PNG, BinaryData::buttonDegranulize_PNGSize);
    Image degranulisationButtonDown = ImageCache::getFromMemory(BinaryData::buttonDegranulizeDown_PNG, BinaryData::buttonDegranulizeDown_PNGSize);

    startTimerHz(30);

    myWebcamButton.setImages(true, true, true, webcamButton, 1.0f, {}, webcamButton, 1.0f, {}, webcamButtonDown, 1.0f, {});
    myWebcamButton.onClick = [&]() { audioProcessor.colourModifier();                                                                         //boolean, ktery rozhoduje, kdy se vykresli waveform je aktivovan
    repaint(); };                                                                                                                              //spusti se proces vykreslovani
    /*myWebcamButton.setColour(TextButton::buttonColourId, Colours::silver);
    myWebcamButton.setColour(TextButton::textColourOffId, Colours::black);*/

    myGranulisationButton.setImages(true, true, true, granulisationButton, 1.0f, {}, granulisationButton, 1.0f, {}, granulistaionButtonDown, 1.0f, {});
    myGranulisationButton.onClick = [&]() { audioProcessor.granulisation();
    repaint(); };
    /*myGranulisationButton.setColour(TextButton::buttonColourId, Colours::silver);
    myGranulisationButton.setColour(TextButton::textColourOffId, Colours::black);*/

    myDegranulisationButton.setImages(true, true, true, degranulisationButton, 1.0f, {}, degranulisationButton, 1.0f, {}, degranulisationButtonDown, 1.0f, {});
    myDegranulisationButton.onClick = [&]() { audioProcessor.degranulize();
    repaint(); };
    /*myDegranulisationButton.setColour(TextButton::buttonColourId, Colours::silver);
    myDegranulisationButton.setColour(TextButton::textColourOffId, Colours::black);*/

    addAndMakeVisible(myWebcamButton);
    addAndMakeVisible(myGranulisationButton);
    addAndMakeVisible(myDegranulisationButton);

    addAndMakeVisible(myWaveThumbnail);
    addAndMakeVisible(myGrainComponent);
    addAndMakeVisible(myADSR);

    setResizable(true, true);
    setResizeLimits(720, 480, 8640, 5760);
    getConstrainer()->setFixedAspectRatio(1.5);

    setSize(1024, 500);
}


SynthGrannyAudioProcessorEditor::~SynthGrannyAudioProcessorEditor()                                                                             //destruktor editoru
{
    stopTimer();
}

//==============================================================================
void SynthGrannyAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setGradientFill(ColourGradient(Colours::grey.darker().darker().darker(), 0.0f, getHeight(), Colours::grey.darker(), getWidth(), 0, false));        //nastaveni barvy pozadi
    g.fillRect(0, 0, getWidth(), getHeight());
    g.setColour(Colours::white);                                                                                                                //barva cary je nastavena na bilou

    g.setFont(15.0f);                                                                                                                           //nastaveni velikosti fontu
}

void SynthGrannyAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    myWebcamButton.setBoundsRelative(0.15f, 0.53f, 0.08f, 0.08f);
    myGranulisationButton.setBoundsRelative(0.05f, 0.53f, 0.08f, 0.08f);
    myDegranulisationButton.setBoundsRelative(0.25f, 0.53f, 0.08f, 0.08f);

    myWaveThumbnail.setBoundsRelative(0.05f, 0.05f, 0.9f, 0.45f);
    myADSR.setBoundsRelative(0.5f, 0.65f, 0.5f, 0.25f);
    myGrainComponent.setBoundsRelative(0.0f, 0.65f, 0.5f, 0.25f);
}

bool SynthGrannyAudioProcessorEditor::isInterestedInFileDrag(const StringArray& files)                                                          //funkce, ktera overi, jestli zvoleny soubor je ve spravnem formatu
{
    for (auto file : files)
    {
        if (file.contains(".wav") || file.contains(".WAV") || file.contains(".aiff") || file.contains(".AIFF") || file.contains(".flac") || file.contains(".FLAC") || file.contains(".wma") || file.contains(".WMA") || file.contains(".ogg") || file.contains(".OGG") || file.contains(".mp3") || file.contains(".MP3"))
        {
            return true;
        }
    }
    return false;
}

void SynthGrannyAudioProcessorEditor::filesDropped(const StringArray& files, int x, int y)                                                      //co se stane po umisteni souboru do vkladaciho pole
{
    for (auto file : files)
    {
        if (isInterestedInFileDrag(file))
        {
            auto myFile = std::make_unique<File>(file);
            myWaveThumbnail.myFileName = myFile->getFileName();

            audioProcessor.loadFileViaDragNDrop(file);
        }
    }
    repaint();
}

void SynthGrannyAudioProcessorEditor::timerCallback()
{
    repaint();
}