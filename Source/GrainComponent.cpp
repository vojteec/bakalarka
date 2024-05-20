/*
  ==============================================================================

    GrainComponent.cpp
    Created: 10 Apr 2024 11:07:24pm
    Author:  vojte

  ==============================================================================
*/

#include "GrainComponent.h"
#include <JuceHeader.h>

//==============================================================================
GrainComponent::GrainComponent(SynthGrannyAudioProcessor& p) : audioProcessor(p)
{
    //Grain length
    myGrainLengthSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    myGrainLengthSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    myGrainLengthSlider.setColour(Slider::ColourIds::thumbColourId, Colours::lightgreen);
    myGrainLengthSlider.setColour(Slider::ColourIds::textBoxTextColourId, Colours::white);
    addAndMakeVisible(myGrainLengthSlider);

    myGrainLengthLabel.setFont(18.0f);
    myGrainLengthLabel.setText(CharPointer_UTF8("D\xc3\xa9lka grainu (ms)"), NotificationType::dontSendNotification);
    myGrainLengthLabel.setColour(Label::textColourId, Colours::white);
    myGrainLengthLabel.setJustificationType(Justification::centredTop);
    myGrainLengthLabel.attachToComponent(&myGrainLengthSlider, false);

    myGrainLengthAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValTrSt(), "GRAIN LENGTH", myGrainLengthSlider);

    //Grain attack
    myGrainAttackSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    myGrainAttackSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    myGrainAttackSlider.setColour(Slider::ColourIds::thumbColourId, Colours::lightgreen.darker());
    addAndMakeVisible(myGrainAttackSlider);

    myGrainAttackLabel.setFont(18.0f);
    myGrainAttackLabel.setText("Attack grainu (%)", NotificationType::dontSendNotification);
    myGrainAttackLabel.setJustificationType(Justification::centredTop);
    myGrainAttackLabel.attachToComponent(&myGrainAttackSlider, false);

    myGrainAttackAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValTrSt(), "GRAIN ATTACK", myGrainAttackSlider);

    //Grain decay
    myGrainDecaySlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    myGrainDecaySlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    myGrainDecaySlider.setColour(Slider::ColourIds::thumbColourId, Colours::lightgreen.darker().darker());
    addAndMakeVisible(myGrainDecaySlider);

    myGrainDecayLabel.setFont(18.0f);
    myGrainDecayLabel.setText("Decay grainu (%)", NotificationType::dontSendNotification);
    myGrainDecayLabel.setJustificationType(Justification::centredTop);
    myGrainDecayLabel.attachToComponent(&myGrainDecaySlider, false);

    myGrainDecayAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValTrSt(), "GRAIN DECAY", myGrainDecaySlider);

    //Grain overlap
    myGrainOverlapSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    myGrainOverlapSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    myGrainOverlapSlider.setColour(Slider::ColourIds::thumbColourId, Colours::lightgreen.darker().darker().darker());
    addAndMakeVisible(myGrainOverlapSlider);

    myGrainOverlapLabel.setFont(18.0f);
    myGrainOverlapLabel.setText(CharPointer_UTF8("P\xc5\x99\ekryt\xc3\xad (%)"), NotificationType::dontSendNotification);
    myGrainOverlapLabel.setJustificationType(Justification::centredTop);
    myGrainOverlapLabel.attachToComponent(&myGrainOverlapSlider, false);

    myGrainOverlapAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValTrSt(), "GRAIN OVERLAP", myGrainOverlapSlider);

    //Grain balance
    myGrainBalanceSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    myGrainBalanceSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
    myGrainBalanceSlider.setColour(Slider::ColourIds::thumbColourId, Colours::lightgreen.darker().darker().darker().darker());
    addAndMakeVisible(myGrainBalanceSlider);

    myGrainBalanceLabel.setFont(18.0f);
    myGrainBalanceLabel.setText(CharPointer_UTF8("Panorama (%)"), NotificationType::dontSendNotification);
    myGrainBalanceLabel.setJustificationType(Justification::centredTop);
    myGrainBalanceLabel.attachToComponent(&myGrainBalanceSlider, false);

    myGrainBalanceAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValTrSt(), "GRAIN BALANCE", myGrainBalanceSlider);
}

GrainComponent::~GrainComponent()
{
}

void GrainComponent::paint(juce::Graphics& g)
{
    g.fillAll(Colours::grey.darker());
}

void GrainComponent::resized()
{
    const auto startX = 0.0f;
    const auto startY = 0.2f;
    const auto compWidth = 0.2f;
    const auto compHeight = 0.75f;

    myGrainLengthSlider.setBoundsRelative(startX, startY, compWidth, compHeight);
    myGrainAttackSlider.setBoundsRelative(startX + compWidth, startY, compWidth, compHeight);
    myGrainDecaySlider.setBoundsRelative(startX + 2 * compWidth, startY, compWidth, compHeight);
    myGrainOverlapSlider.setBoundsRelative(startX + 3 * compWidth, startY, compWidth, compHeight);
    myGrainBalanceSlider.setBoundsRelative(startX + 4 * compWidth, startY, compWidth, compHeight);
}
