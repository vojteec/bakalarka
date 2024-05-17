/*
  ==============================================================================

    ADSRComponent.cpp
    Created: 24 Oct 2023 1:00:10am
    Author:  vojte

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ADSRComponent.h"

//==============================================================================
ADSRComponent::ADSRComponent(SynthGrannyAudioProcessor& p) : audioProcessor (p)
{
    //Attack
    myAttackSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    myAttackSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
    myAttackSlider.setColour(Slider::ColourIds::thumbColourId, Colours::lightgreen);
    myAttackSlider.setColour(Slider::ColourIds::textBoxTextColourId, Colours::white);
    addAndMakeVisible(myAttackSlider);

    myAttackLabel.setFont(10.0f);
    myAttackLabel.setText("Attack (s)", NotificationType::dontSendNotification);
    myAttackLabel.setColour(Label::textColourId, Colours::white);
    myAttackLabel.setJustificationType(Justification::centredTop);
    myAttackLabel.attachToComponent(&myAttackSlider, false);

    myAttackAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValTrSt(), "ATTACK", myAttackSlider);

    //Decay
    myDecaySlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    myDecaySlider.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
    myDecaySlider.setColour(Slider::ColourIds::thumbColourId, Colours::lightgreen.darker());
    addAndMakeVisible(myDecaySlider);

    myDecayLabel.setFont(10.0f);
    myDecayLabel.setText("Decay (s)", NotificationType::dontSendNotification);
    myDecayLabel.setJustificationType(Justification::centredTop);
    myDecayLabel.attachToComponent(&myDecaySlider, false);

    myDecayAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValTrSt(), "DECAY", myDecaySlider);

    //Sustain
    mySustainSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    mySustainSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
    mySustainSlider.setColour(Slider::ColourIds::thumbColourId, Colours::lightgreen.darker().darker());
    addAndMakeVisible(mySustainSlider);

    mySustainLabel.setFont(10.0f);
    mySustainLabel.setText("Sustain (%)", NotificationType::dontSendNotification);
    mySustainLabel.setJustificationType(Justification::centredTop);
    mySustainLabel.attachToComponent(&mySustainSlider, false);

    mySustainAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValTrSt(), "SUSTAIN", mySustainSlider);

    //Release
    myReleaseSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    myReleaseSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 40, 20);
    myReleaseSlider.setColour(Slider::ColourIds::thumbColourId, Colours::lightgreen.darker().darker().darker());
    addAndMakeVisible(myReleaseSlider);

    myReleaseLabel.setFont(10.0f);
    myReleaseLabel.setText("Release (s)", NotificationType::dontSendNotification);
    myReleaseLabel.setJustificationType(Justification::centredTop);
    myReleaseLabel.attachToComponent(&myReleaseSlider, false);

    myReleaseAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getValTrSt(), "RELEASE", myReleaseSlider);

}

ADSRComponent::~ADSRComponent()
{
}

void ADSRComponent::paint (juce::Graphics& g)
{
    g.fillAll(Colours::grey.darker());
}

void ADSRComponent::resized()
{
    const auto startX = 0.2f;
    const auto startY = 0.2f;
    const auto compWidth = 0.2f;
    const auto compHeight = 0.75f;

    myAttackSlider.setBoundsRelative(startX, startY, compWidth, compHeight);
    myDecaySlider.setBoundsRelative(startX + compWidth, startY, compWidth, compHeight);
    mySustainSlider.setBoundsRelative(startX + 2 * compWidth, startY, compWidth, compHeight);
    myReleaseSlider.setBoundsRelative(startX + 3 * compWidth, startY, compWidth, compHeight);

}
