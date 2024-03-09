/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginEditor.cpp"
#include <algorithm>
#include <random>
#include <vector>

//==============================================================================
SynthGrannyAudioProcessor::SynthGrannyAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), myValTrSt (*this, nullptr, "PARAMETERS", createParams())
#endif
{
    myFormatManager.registerBasicFormats();
    myValTrSt.state.addListener(this);              //sleduje zmeny ve ValueTree

    for (int i = 0; i < myNumVoices; i++)
    {
        myGrannySynth.addVoice(new SamplerVoice());
    }
}

SynthGrannyAudioProcessor::~SynthGrannyAudioProcessor()
{
    myFormatReader = nullptr;
}

//==============================================================================
const juce::String SynthGrannyAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SynthGrannyAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SynthGrannyAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SynthGrannyAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SynthGrannyAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SynthGrannyAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SynthGrannyAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SynthGrannyAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SynthGrannyAudioProcessor::getProgramName (int index)
{
    return {};
}

void SynthGrannyAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SynthGrannyAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    myGrannySynth.setCurrentPlaybackSampleRate(sampleRate);
    updateADSR();
}

void SynthGrannyAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SynthGrannyAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SynthGrannyAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (myShouldUpdate)
    {
        updateADSR();
    }

    MidiMessage m;
    MidiBuffer::Iterator it( midiMessages );
    int sample;
    static double midiNoteInHertz;
    while (it.getNextEvent(m, sample))
    {
        if (m.isNoteOn())
        {
            midiNoteInHertz = m.getMidiNoteInHertz(m.getNoteNumber(), 440.0);
            myIsNotePlayed = true;
        }
        else if (m.isNoteOff())
        {
            myIsNotePlayed = false;
        }
    }

    float pitchedSamples = buffer.getNumSamples() * (midiNoteInHertz / 523.25113);
    mySampleCount = myIsNotePlayed ? mySampleCount += pitchedSamples : 0;

    myGrannySynth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    for (int channel = 0; channel < totalNumInputChannels; ++channel) //pùvodnì totalNumInputChannels
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...

    }
}

//==============================================================================
bool SynthGrannyAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SynthGrannyAudioProcessor::createEditor()
{
    return new SynthGrannyAudioProcessorEditor (*this);
}

//==============================================================================
void SynthGrannyAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SynthGrannyAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void SynthGrannyAudioProcessor::loadFileViaDragNDrop(const String& path)
{
    myGrannySynth.clearSounds();

    auto file = File(path);
    myFormatReader = myFormatManager.createReaderFor(file);

    readWaveform();

    BigInteger range;
    range.setRange(0, 128, true);

    /*auto samplerSound = */ myGrannySynth.addSound(new SamplerSound("Sample", *myFormatReader, range, 72, 0.1, 0.1, 300.0));
    
    //granulizeAndShuffle(samplerSound);
    updateADSR();
}

/////////////////////////////////////////////////////////////////////////////////////////////////       TATO METODA SI ŽÁDÁ VELMI DETAILNÍ ROZBOR A ÚPRAVY
/*void SynthGrannyAudioProcessor::granulizeAndShuffle(SamplerSound* samplerSound)
{
    AudioBuffer<float>& soundBuffer = *samplerSound->getAudioData();
    const int grainSize = 1024;
    const int numChannels = soundBuffer.getNumChannels();
    const int numGrains = soundBuffer.getNumSamples() / grainSize;

    // Seznam indexù pro granule
    Array<int> grainIndices;
    grainIndices.resize(numGrains);
    for (int i = 0; i < numGrains; ++i)
        grainIndices.set(i, i * grainSize);


    // Náhodnì seøadí indexy granulí pro každý kanál
    std::shuffle(grainIndices.begin(), grainIndices.end(), std::default_random_engine(std::random_device()()));


    // Pøeskupení vzorkù podle náhodnì seøazených indexù
    for (int channel = 0; channel < numChannels; ++channel)
    {
        AudioBuffer<float> tempBuffer(1, soundBuffer.getNumSamples());  // Doèasný buffer pro jeden kanál

        for (int i = 0; i < numGrains; ++i)
        {
            int index = grainIndices[i] * grainSize;

            // Pøidání vzorkù granulí do doèasného bufferu
            tempBuffer.copyFrom(0, i * grainSize, soundBuffer, channel, index, grainSize);
        }

        // Kopírování doèasného bufferu zpìt do pùvodního bufferu
        soundBuffer.copyFrom(channel, 0, tempBuffer, 0, 0, soundBuffer.getNumSamples());
    }

    myGrannySynth.addSound(samplerSound);
}*/

void SynthGrannyAudioProcessor::loadFileViaButton()
{
    myGrannySynth.clearSounds();

    FileChooser chooser{ "Vyberte soubor", {}, "*.wav;*.flac;*.aiff;*.wma;*.ogg"};
    if (chooser.browseForFileToOpen())
    {
        auto file = chooser.getResult();

        //NOT WORKING    NOT WORKING    NOT WORKING    NOT WORKING    NOT WORKING (yet)
        /*
        auto myFile = std::make_unique<File>(file);
        myWaveThumbnail.myFileName = myFile->getFileName();
        */

    myFormatReader = myFormatManager.createReaderFor(file);
    }
    else
    {
        return;
    }

    readWaveform();

    BigInteger range;
    range.setRange(0, 128, true);

    myGrannySynth.addSound(new SamplerSound("Sample", *myFormatReader, range, 72, 0.1, 0.1, 300.0));

    updateADSR();
}

void SynthGrannyAudioProcessor::readWaveform()
{
    auto sampleLength = static_cast<int>(myFormatReader->lengthInSamples);

    myWave.setSize(1, sampleLength);
    myFormatReader->read(&myWave, 0, sampleLength, 0, true, true);
}

void SynthGrannyAudioProcessor::updateADSR()
{
    myADSRParams.attack = myValTrSt.getRawParameterValue("ATTACK")->load();
    myADSRParams.decay = myValTrSt.getRawParameterValue("DECAY")->load();
    myADSRParams.sustain = myValTrSt.getRawParameterValue("SUSTAIN")->load();
    myADSRParams.release = myValTrSt.getRawParameterValue("RELEASE")->load();

    for (int i = 0; i < myGrannySynth.getNumSounds(); ++i)
    {
        if (auto sound = dynamic_cast<SamplerSound*>(myGrannySynth.getSound(i).get()))
        {
            sound->setEnvelopeParameters(myADSRParams);
        }
    }
}

AudioProcessorValueTreeState::ParameterLayout SynthGrannyAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> parameters;

    parameters.push_back(std::make_unique<AudioParameterFloat>("ATTACK", "Attack", 0.0f, 10.0f, 1.0f));
    parameters.push_back(std::make_unique<AudioParameterFloat>("DECAY", "Decay", 0.0f, 10.0f, 2.0f));
    parameters.push_back(std::make_unique<AudioParameterFloat>("SUSTAIN", "Sustain", 0.0f, 1.0f, 1.0f));
    parameters.push_back(std::make_unique<AudioParameterFloat>("RELEASE", "Release", 0.0f, 10.0f, 2.0f));


    return{ parameters.begin(), parameters.end() };
}

void SynthGrannyAudioProcessor::valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property)
{
    myShouldUpdate = true;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynthGrannyAudioProcessor();
}
