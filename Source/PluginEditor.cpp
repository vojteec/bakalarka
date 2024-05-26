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

    Image continuousParamsButton = ImageCache::getFromMemory(BinaryData::buttonContinuousParams_png, BinaryData::buttonContinuousParams_pngSize);
    Image continuousParamsButtonDown = ImageCache::getFromMemory(BinaryData::buttonContinuousParamsDown_png, BinaryData::buttonContinuousParamsDown_pngSize);

    Image oneShotParamsButton = ImageCache::getFromMemory(BinaryData::buttonOneShotParams_png, BinaryData::buttonOneShotParams_pngSize);
    Image oneShotParamsButtonDown = ImageCache::getFromMemory(BinaryData::buttonOneShotParamsDown_png, BinaryData::buttonOneShotParamsDown_pngSize);

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
    myDegranulisationButton.onClick = [&]() { audioProcessor.myParameterContinuous = false;
    audioProcessor.degranulize();
    repaint(); };
    /*myDegranulisationButton.setColour(TextButton::buttonColourId, Colours::silver);
    myDegranulisationButton.setColour(TextButton::textColourOffId, Colours::black);*/

    myContinuousParamsButton.setImages(true, true, true, continuousParamsButton, 1.0f, {}, continuousParamsButton, 1.0f, {}, continuousParamsButtonDown, 1.0f, {});
    myContinuousParamsButton.onClick = [&]() { audioProcessor.myParameterContinuous = true;
    repaint(); };

    myOneShotParamsButton.setImages(true, true, true, oneShotParamsButton, 1.0f, {}, oneShotParamsButton, 1.0f, {}, oneShotParamsButtonDown, 1.0f, {});
    myOneShotParamsButton.onClick = [&]() { audioProcessor.myParameterContinuous = false;
    repaint(); };

    float fontSize = 16.0f;

    myRootNoteSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    myRootNoteSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    myRootNoteSlider.setColour(Slider::ColourIds::thumbColourId, Colours::lightgreen);
    myRootNoteSlider.setColour(Slider::ColourIds::textBoxTextColourId, Colours::white);
    addAndMakeVisible(myRootNoteSlider);

    myRootNoteLabel.setFont(fontSize);
    myRootNoteLabel.setText(CharPointer_UTF8("MIDI Root nota"), NotificationType::dontSendNotification);
    myRootNoteLabel.setColour(Label::textColourId, Colours::white);
    myRootNoteLabel.setJustificationType(Justification::centredTop);
    myRootNoteLabel.attachToComponent(&myRootNoteSlider, false);

    myRootNoteAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValTrSt(), "ROOT NOTE", myRootNoteSlider);

    myLowerLimitSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    myLowerLimitSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    myLowerLimitSlider.setColour(Slider::ColourIds::thumbColourId, Colours::lightgreen);
    myLowerLimitSlider.setColour(Slider::ColourIds::textBoxTextColourId, Colours::white);
    addAndMakeVisible(myLowerLimitSlider);

    myLowerLimitLabel.setFont(fontSize);
    myLowerLimitLabel.setText(CharPointer_UTF8("Doln\xc3\xad hranice z\xc3\xb3ny"), NotificationType::dontSendNotification);
    myLowerLimitLabel.setColour(Label::textColourId, Colours::white);
    myLowerLimitLabel.setJustificationType(Justification::centredTop);
    myLowerLimitLabel.attachToComponent(&myLowerLimitSlider, false);

    myLowerLimitAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValTrSt(), "ZONE LOWER LIMIT", myLowerLimitSlider);

    myUpperLimitSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    myUpperLimitSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    myUpperLimitSlider.setColour(Slider::ColourIds::thumbColourId, Colours::lightgreen);
    myUpperLimitSlider.setColour(Slider::ColourIds::textBoxTextColourId, Colours::white);
    addAndMakeVisible(myUpperLimitSlider);

    myUpperLimitLabel.setFont(fontSize);
    myUpperLimitLabel.setText(CharPointer_UTF8("Horn\xc3\xad hranice z\xc3\xb3ny"), NotificationType::dontSendNotification);
    myUpperLimitLabel.setColour(Label::textColourId, Colours::white);
    myUpperLimitLabel.setJustificationType(Justification::centredTop);
    myUpperLimitLabel.attachToComponent(&myUpperLimitSlider, false);

    myUpperLimitAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValTrSt(), "ZONE UPPER LIMIT", myUpperLimitSlider);

    myVolumeSlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
    myVolumeSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    myVolumeSlider.setColour(Slider::ColourIds::thumbColourId, Colours::lightgreen);
    myVolumeSlider.setColour(Slider::ColourIds::textBoxTextColourId, Colours::white);
    addAndMakeVisible(myVolumeSlider);

    myVolumeLabel.setFont(fontSize);
    myVolumeLabel.setText(CharPointer_UTF8("Zes\xc3\xadlen\xc3\xad"), NotificationType::dontSendNotification);
    myVolumeLabel.setColour(Label::textColourId, Colours::white);
    myVolumeLabel.setJustificationType(Justification::centredTop);
    myVolumeLabel.attachToComponent(&myVolumeSlider, false);

    myVolumeAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValTrSt(), "VOLUME", myVolumeSlider);

    addAndMakeVisible(myWebcamButton);
    addAndMakeVisible(myGranulisationButton);
    addAndMakeVisible(myDegranulisationButton);
    addAndMakeVisible(myContinuousParamsButton);
    addAndMakeVisible(myOneShotParamsButton);

    addAndMakeVisible(myRootNoteSlider);
    addAndMakeVisible(myLowerLimitSlider);
    addAndMakeVisible(myUpperLimitSlider);
    addAndMakeVisible(myVolumeSlider);

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
    g.setGradientFill(ColourGradient(Colours::grey.darker().darker().darker(), 0.0f, getHeight(), Colours::grey.darker(), 0, 0, false));        //nastaveni barvy pozadi
    g.fillRect(0, 0, getWidth(), getHeight());
    g.setColour(Colours::white);                                                                                                                //barva cary je nastavena na bilou

    g.setFont(15.0f);                                                                                                                           //nastaveni velikosti fontu
}

void SynthGrannyAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    myGranulisationButton.setBoundsRelative(0.05f, 0.51f, 0.08f, 0.08f);
    myWebcamButton.setBoundsRelative(0.15f, 0.51f, 0.08f, 0.08f);
    myDegranulisationButton.setBoundsRelative(0.25f, 0.51f, 0.08f, 0.08f);
    myContinuousParamsButton.setBoundsRelative(0.05f, 0.61f, 0.08f, 0.08f);
    myOneShotParamsButton.setBoundsRelative(0.15f, 0.61f, 0.08f, 0.08f);

    myRootNoteSlider.setBoundsRelative(0.35f, 0.55f, 0.1f, 0.1875f);
    myLowerLimitSlider.setBoundsRelative(0.45f, 0.55f, 0.1f, 0.1875f);
    myUpperLimitSlider.setBoundsRelative(0.55f, 0.55f, 0.1f, 0.1875f);
    myVolumeSlider.setBoundsRelative(0.75f, 0.55f, 0.2f, 0.1875f);

    myWaveThumbnail.setBoundsRelative(0.05f, 0.05f, 0.9f, 0.45f);
    myADSR.setBoundsRelative(0.5f, 0.75f, 0.5f, 0.25f);
    myGrainComponent.setBoundsRelative(0.0f, 0.75f, 0.5f, 0.25f);
}

bool SynthGrannyAudioProcessorEditor::isInterestedInFileDrag(const StringArray& files)                                                          //funkce, ktera overi, jestli zvoleny soubor je ve spravnem formatu
{
    for (auto file : files)
    {
        if (file.contains(".wav") || file.contains(".WAV") || file.contains(".aiff") || file.contains(".AIFF") || file.contains(".aif") || file.contains(".AIF") || file.contains(".flac") || file.contains(".FLAC") || file.contains(".wma") || file.contains(".WMA") || file.contains(".ogg") || file.contains(".OGG") || file.contains(".mp3") || file.contains(".MP3"))
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