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
    /*auto happy_grannyImage = ImageCache::getFromMemory(BinaryData::happy_granny_png, BinaryData::happy_granny_pngSize);

    if (! happy_grannyImage.isNull())
        myImageComponent.setImage(happy_grannyImage, RectanglePlacement::stretchToFit);
    else
        jassert(! happy_grannyImage.isNull());
    */
    //addAndMakeVisible(myImageComponent);

    startTimerHz(30);                                       //nastaveni velikosti okna

    myWebcamButton.onClick = [&]() { audioProcessor.colourModifier();                                                                         //boolean, ktery rozhoduje, kdy se vykresli waveform je aktivovan
    repaint(); };                                                                                                                              //spusti se proces vykreslovani
    myWebcamButton.setColour(TextButton::buttonColourId, Colours::silver);
    myWebcamButton.setColour(TextButton::textColourOffId, Colours::black);
    addAndMakeVisible(myWebcamButton);

    addAndMakeVisible(myWaveThumbnail);
    addAndMakeVisible(myGrainComponent);
    addAndMakeVisible(myADSR);

    setSize(1080, 720);
}


SynthGrannyAudioProcessorEditor::~SynthGrannyAudioProcessorEditor()                                                                             //destruktor editoru
{
    stopTimer();
}

//==============================================================================
void SynthGrannyAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(Colours::grey.darker());                                                                                                                  //nastaveni barvy pozadi
    g.setColour(Colours::white);                                                                                                                //barva cary je nastavena na bilou

    g.setFont(15.0f);                                                                                                                           //nastaveni velikosti fontu
}

void SynthGrannyAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    myWebcamButton.setBoundsRelative(0.01f, 0.01f, 0.08f, 0.025f);
    myWaveThumbnail.setBoundsRelative(0.1f, 0.05f, 0.8f, 0.45f);
    myADSR.setBoundsRelative(0.5f, 0.65f, 0.4f, 0.3f);
    myGrainComponent.setBoundsRelative(0.1f, 0.65f, 0.3f, 0.3f);
    myImageComponent.setBoundsRelative(0.0f, 0.0f, 0.2f, 0.2f);
}

bool SynthGrannyAudioProcessorEditor::isInterestedInFileDrag(const StringArray& files)                                                          //funkce, ktera overi, jestli zvoleny soubor je ve spravnem formatu
{
    for (auto file : files)
    {
        if (file.contains(".wav") || file.contains(".aiff") || file.contains(".flac") || file.contains(".wma") || file.contains(".ogg"))
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