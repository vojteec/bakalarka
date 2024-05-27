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
#include <iostream>
#include <fstream>

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

    //DEFINICE CHOVANI PRI ZMENE PARAMETRU POMOCI OVLADACICH PRVKU

    if (myShouldUpdate) //myShouldUpdate je true, pokud se zmeni nektery parametr z ovladacich prvku na panelu nastroje
    {
        updateADSR(); //odeslani hodnot ADSR do juce::Sampler, ktery je dale zpracuje
        if (originalBuffer.getNumSamples() > 0 && myParameterContinuous) //pokud bylo do originalniho bufferu neco nacteno a zaroven bylo vybrano kontinualni snimani parametru
        {
            granulisation(); //proved novou granulizaci
        }
        myShouldUpdate = false; //a necykluj se
    }
    
    //NASTAVENI POHYBU UKAZATELE POZICE V SAMPLU

    MidiMessage message;
    MidiBuffer::Iterator iterator( midiMessages );
    int sample;
    static double midiNoteInHertz;
    while (iterator.getNextEvent(message, sample))
    {
        if (message.isNoteOn())
        {
            midiNoteInHertz = message.getMidiNoteInHertz(message.getNoteNumber(), 440.0);
            myIsNotePlayed = true;
        }
        else if (message.isNoteOff())
        {
            myIsNotePlayed = false;
        }
    }

    int rootNote = myValTrSt.getRawParameterValue("ROOT NOTE")->load();
    float rootNoteFrequency = 440.0f * std::powf(2.0f, ((rootNote - 69.0f) / 12.0f)); //prizpusobeni rychlosti pohybu podle MIDI Root Note
    float pitchedSamples = buffer.getNumSamples() * (midiNoteInHertz / rootNoteFrequency);
    mySampleCount = myIsNotePlayed ? mySampleCount += pitchedSamples : 0;

    myGrannySynth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    //APLIKACE PANORAMY

    float balanceFromValTrSt = myValTrSt.getRawParameterValue("GRAIN BALANCE")->load();
    float balance = balanceFromValTrSt / 100;

    if (buffer.getNumChannels() < 2) //korekce pro mono stopy pro vytvoreni 2-channel mono
    {
        buffer.setSize(2, buffer.getNumSamples(), 1);
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            buffer.setSample(1, sample, buffer.getSample(0, sample));
        }
    }

    if (buffer.getNumChannels() == 2) //pokud je input stereo, aplikuj balance
    {
        if (balance < 0)
        {
            buffer.applyGain(1, 0, buffer.getNumSamples(), 1.0f + balance);
        }
        if (balance > 0)
        {
            buffer.applyGain(0, 0, buffer.getNumSamples(), 1.0f - balance);
        }
    }

    float gain = myValTrSt.getRawParameterValue("VOLUME")->load();

    buffer.applyGain(gain);
    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
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

void SynthGrannyAudioProcessor::averagePixel(const Image& img)
{
    std::vector<Colour> pixelColours;
    std::vector<float> pixelHue;
    std::vector<float> pixelSaturation;
    std::vector<float> pixelLightness;

    //prochazime pixel po pixelu cely snimek a ukladame Colour
    for (int row = 0; row < img.getHeight(); row++)
    {
        for (int pixel = 0; pixel < img.getWidth(); pixel++)
        {
            pixelColours.push_back(img.getPixelAt(pixel, row));
        }
    }

    //prochazime jednotlive Colours a tridime parametry do vektoru
    for (int pixel = 0; pixel < pixelColours.size(); pixel++)
    {
        pixelHue.push_back(pixelColours[pixel].getHue());
        pixelSaturation.push_back(pixelColours[pixel].getSaturationHSL());
        pixelLightness.push_back(pixelColours[pixel].getLightness());
    }

    //hledame prumernou hodnotu Hue, Saturation a Lightness
    float averageHue = std::accumulate(pixelHue.begin(), pixelHue.end(), 0.0f) / pixelHue.size();
    float averageSaturation = std::accumulate(pixelSaturation.begin(), pixelSaturation.end(), 0.0f) / pixelHue.size();
    float averageLightness = std::accumulate(pixelLightness.begin(), pixelLightness.end(), 0.0f) / pixelLightness.size();

    //uprava rozsahu parametru pro pouziti na parametry granulizace
    this->averageHueRanged = 100 * std::abs(std::cos(365 * averageHue * (3.141592653589793f / 180.0f)));
    this->averageSaturationRanged = 100 * averageSaturation;
    this->averageLightnessRanged = 50 * averageLightness;

    granulisationByColour(averageHueRanged, averageSaturationRanged, averageLightnessRanged);
}

void SynthGrannyAudioProcessor::colourModifier()
{
    myCameraDevice->takeStillPicture([this](const juce::Image& image) {
        this->averagePixel(image);
        });
}

void SynthGrannyAudioProcessor::loadFileViaDragNDrop(const String& path)
{
    auto file = File(path);
    myFormatReader = myFormatManager.createReaderFor(file);

    ////////////////////////////////////////////////////////
    originalBuffer.setSize(myFormatReader->numChannels, myFormatReader->lengthInSamples); //technicky je v soucasnem stavu pouziti originalBufferu zcela zbytecne, ale budouci prace by mohla zahrnovat vice bufferu obsahujicich ruzne samply, tak jako tomu muze byt treba v Ribs
    myFormatReader->read(&originalBuffer, 0, myFormatReader->lengthInSamples, 0, true, true);

    if (originalBuffer.getNumSamples() > 0)
    {
        granulisation();
    }
    else
    {
        return; //toto melo zabranit nahrani souboru s neplatnou priponou (napriklad pokud se soubor bude jmenovat Sample.wav.jpg), ale funguje to tak nejak humpolacky
    }
}

void SynthGrannyAudioProcessor::granulisation()
{
    myGrannySynth.clearSounds(); //smaz zvuky, ktere jiz v sampleru jsou

    if (originalBuffer.getNumSamples() == 0)
    {
        return; //prazdny buffer nebudeme granulizovat
    }

    //PREPOCET DELKY GRAINU Z ms NA VZORKY

    float grainLengthInMs = myValTrSt.getRawParameterValue("GRAIN LENGTH")->load();
    int grainSize = static_cast<int>((grainLengthInMs / 1000) * myFormatReader->sampleRate);

    //PREPOCET VELIKOSTI PREKRYTI Z PROCENT NA VZORKY

    float overlapInPercent = myValTrSt.getRawParameterValue("GRAIN OVERLAP")->load();
    int overlapInSamples = static_cast<int>((grainSize / 100) * overlapInPercent);

    //POKUD BY BYLA DELKA GRAINU VETSI NEZ DELKA SAMPLU

    if (grainSize > originalBuffer.getNumSamples())
    {
        grainSize = originalBuffer.getNumSamples();
    }

    //PRO ZKRACENI KODU SI DEFINUJEME POCET KANALU A POCET GRAINU

    int numChannels = originalBuffer.getNumChannels();
    int numGrains = static_cast<int>(originalBuffer.getNumSamples() / grainSize) + 1;

    AudioBuffer<float> myGrain;
    AudioBuffer<float> tempBuffer;
    std::vector<int> tempIdxs;

    //NAPLNENI VEKTORU INDEXU GRAINU

    for (int i = 0; i < numGrains; i++)
    {
        tempIdxs.push_back(i);
    }

    //NASTAVENI VELIKOSTI BUFFERU

    myGrain.setSize(numChannels, grainSize);
    tempBuffer.setSize(numChannels, numGrains * grainSize);

    //PREPOCET PARAMETRU GRAIN AD OBALKY Z PROCENT NA VZORKY

    float attackInPercent = myValTrSt.getRawParameterValue("GRAIN ATTACK")->load();
    float decayInPercent = myValTrSt.getRawParameterValue("GRAIN DECAY")->load();
    int attackInSamples = static_cast<int>(grainSize * (attackInPercent / 100));
    int decayInSamples = static_cast<int>(grainSize * (decayInPercent / 100));

    int decayStartIdx = grainSize - decayInSamples;

    //BEREME VZORKY, TVORIME GRAINY A PRESKLADAVAME

    int grainIdx = 0;
    for (int sample = 0; sample < (numGrains - 1) * grainSize; sample++)
    {

        //PLNENI GRAINU VZORKY

        for (int channel = 0; channel < numChannels; channel++)
        {
            myGrain.setSample(channel, grainIdx, originalBuffer.getSample(channel, sample));
        }
        if (grainIdx == grainSize - 1) //pokud je grain plny
        {
            //VYBER NAHODNY INDEX A ODSTRAN HO Z VEKTORU

            int removeIdx = rand() % size(tempIdxs);
            int idx = tempIdxs[removeIdx];
            tempIdxs.erase(tempIdxs.begin() + removeIdx);
            for (int channel = 0; channel < numChannels; channel++)
            {
                //APLIKUJ GRAIN AD OBALKU

                myGrain.applyGainRamp(0, attackInSamples, 0.0f, 1.0f);
                myGrain.applyGainRamp(decayStartIdx, decayInSamples, 1.0f, 0.0f);

                //ULOZ DO PRECHODNEHO BUFFERU

                for (int i = 0; i < grainSize; i++)
                {
                    tempBuffer.setSample(channel, idx * grainSize + i, myGrain.getSample(channel, i));
                }
            }
            myGrain.clear();
            grainIdx = 0;
        }
        else //pokud grain neni plny
        {
            grainIdx++;
        }
    }

    //PRAVDEPODOBNE NAM ZBYLY NEJAKE VZORKY, KTERE JIZ NEVYTVORI CELY GRAIN, POJDME SI JE TAKY VZIT

    int lastGrainStartIdx = tempIdxs[0] * grainSize;

    for (int channel = 0; channel < numChannels; channel++)
    {
        for (int i = 0; i < grainIdx - 1; i++) //doplneni zbylych vzorku
        {
            tempBuffer.setSample(channel, lastGrainStartIdx + i, myGrain.getSample(channel, i));
        }
        int numMovedSamples = tempBuffer.getNumSamples() - lastGrainStartIdx - grainIdx;
    }

    //Z PRECHODNEHO BUFFERU BUDEME KOPIROVAT DO GRANULIZOVANEHO BUFFERU S PREKRYTIM

    AudioBuffer<float> granulizedBuffer;
    granulizedBuffer.setSize(tempBuffer.getNumChannels(), numGrains * grainSize - (numGrains - 1) * overlapInSamples);
    
    //URCEME SI VZORKY, KTERE PREKRYTIM NEJSOU ZASAZENY

    int notOverlappedSamples = grainSize - 2 * overlapInSamples;

    for (int channel = 0; channel < granulizedBuffer.getNumChannels(); channel++)
    {
        int granulizedBufferSampleIdx = 0;

        for (int grainNumber = 0; grainNumber < numGrains; grainNumber++) //projdi postupne vsechny grainy
        {
            for (int sampleInGrainIdx = 0; sampleInGrainIdx < grainSize; sampleInGrainIdx++) //projdi vsechny vzorky v grainu
            {
                if (grainNumber == 0) //prvni grain (nechceme prekryti na zacatku)
                {
                    if (sampleInGrainIdx < grainSize - overlapInSamples) //vsechny grainy do prekryti pouze prepis tak jak jsou
                    {
                        granulizedBuffer.setSample(channel, granulizedBufferSampleIdx, tempBuffer.getSample(channel, grainNumber * grainSize + sampleInGrainIdx));
                        granulizedBufferSampleIdx++;
                        continue;
                    }

                    //VZORKY KONCE GRAINU SE PRUMERUJI SE VZORKY ZACATKU DALSIHO GRAINU
                    
                    float average = (tempBuffer.getSample(channel, grainNumber * grainSize + sampleInGrainIdx) + tempBuffer.getSample(channel, (grainNumber + 1) * grainSize + sampleInGrainIdx - overlapInSamples - notOverlappedSamples)) / 2;
                    //float addition = tempBuffer.getSample(channel, grainNumber * grainSize + sampleInGrainIdx) + tempBuffer.getSample(channel, (grainNumber + 1) * grainSize + sampleInGrainIdx - overlapInSamples - notOverlappedSamples);
                    granulizedBuffer.setSample(channel, granulizedBufferSampleIdx, average);
                    granulizedBufferSampleIdx++;
                    continue;
                }
                if (grainNumber == numGrains - 1) //posledni grain (nechceme prekryti na konci)
                {
                    if (sampleInGrainIdx < overlapInSamples) //prekryti ze zacatku preskakujeme, protoze uz bylo vyreseno v predposlednim grainu
                    {
                        continue;
                    }
                    if (sampleInGrainIdx < grainSize) //grainy, ktere jsou mimo prekryti, zapisujeme tak jak jsou
                    {
                        granulizedBuffer.setSample(channel, granulizedBufferSampleIdx, tempBuffer.getSample(channel, grainNumber * grainSize + sampleInGrainIdx));
                        granulizedBufferSampleIdx++;
                        continue;
                    }
                }
                //grain, ktery neni prvni nebo posledni
                if (sampleInGrainIdx < overlapInSamples) //preskakujeme zacatek grainu, uz je vyreseny minulym grainem
                {
                    continue;
                }
                if (sampleInGrainIdx < grainSize - overlapInSamples) //grainy, ktere jsou mimo prekryti, zapisujeme tak jak jsou
                {
                    granulizedBuffer.setSample(channel, granulizedBufferSampleIdx, tempBuffer.getSample(channel, grainNumber * grainSize + sampleInGrainIdx));
                    granulizedBufferSampleIdx++;
                    continue;
                }

                //PRO PREKRYTI NA KONCI ZASE UDELAME PRUMER POSLEDNICH VZORKU N-TEHO GRAINU A PRVNICH VZORKU N+1 GRAINU

                float average = (tempBuffer.getSample(channel, grainNumber * grainSize + sampleInGrainIdx) + tempBuffer.getSample(channel, (grainNumber + 1) * grainSize + sampleInGrainIdx - overlapInSamples - notOverlappedSamples)) / 2;
                //float addition = tempBuffer.getSample(channel, grainNumber * grainSize + sampleInGrainIdx) + tempBuffer.getSample(channel, (grainNumber + 1) * grainSize + sampleInGrainIdx - overlapInSamples - notOverlappedSamples);
                granulizedBuffer.setSample(channel, granulizedBufferSampleIdx, average);
                granulizedBufferSampleIdx++;
            }
        }
    }

    readWaveform(granulizedBuffer);

    //MUSIME SI TO ZASE ZAPSAT DO SOUBORU, PROTOZE FORMAT READER BUFFERY NECTE

    MemoryBlock memoryBlock;
    {
        WavAudioFormat wavFormat;
        std::unique_ptr<AudioFormatWriter> writer(wavFormat.createWriterFor(new MemoryOutputStream(memoryBlock, false), myFormatReader->sampleRate, granulizedBuffer.getNumChannels(), 24, {}, 0));

        if (writer != nullptr)
        {
            writer->writeFromAudioSampleBuffer(granulizedBuffer, 0, granulizedBuffer.getNumSamples());
        }
    }

    WavAudioFormat wavFormat;
    std::unique_ptr<AudioFormatReader> reader(wavFormat.createReaderFor(new MemoryInputStream(memoryBlock, false), true));

    ////////////////////////

    BigInteger range;
    int lowerLimit = myValTrSt.getRawParameterValue("ZONE LOWER LIMIT")->load();
    int upperLimit = myValTrSt.getRawParameterValue("ZONE UPPER LIMIT")->load();

    if (lowerLimit == upperLimit)
    {
        lowerLimit = upperLimit - 1;
    }
    if (upperLimit < lowerLimit)
    {
        std::swap(lowerLimit, upperLimit);
    }
    if (lowerLimit == upperLimit && lowerLimit == 0)
    {
        upperLimit = lowerLimit + 1;
    }

    range.setRange(lowerLimit, upperLimit - lowerLimit + 1, true);

    int rootNote = myValTrSt.getRawParameterValue("ROOT NOTE")->load();
    myGrannySynth.addSound(new SamplerSound("Sample", *reader, range, rootNote, 0.1, 0.1, 2400.0));
    updateADSR();

    //cisteni pameti
    myGrain.setSize(0, 0, 0, 1, 0);
    tempBuffer.setSize(0, 0, 0, 1, 0);
    granulizedBuffer.setSize(0, 0, 0, 1, 0);
}

void SynthGrannyAudioProcessor::granulisationByColour(float averageHue, float averageSaturation, float averageLightness)
{
    if (originalBuffer.getNumSamples() > 0)
    {
        myGrannySynth.clearSounds();

        float grainLengthInMs = averageHue;
        int grainSize = static_cast<int>((grainLengthInMs / 1000) * myFormatReader->sampleRate);

        float overlapInPercent = averageLightness;
        int overlapInSamples = static_cast<int>((grainSize / 100) * overlapInPercent);

        if (grainSize > originalBuffer.getNumSamples())
        {
            grainSize = originalBuffer.getNumSamples();
        }

        int numChannels = originalBuffer.getNumChannels();

        int numGrains = static_cast<int>(originalBuffer.getNumSamples() / grainSize) + 1;

        AudioBuffer<float> myGrain;
        AudioBuffer<float> tempBuffer;
        std::vector<int> tempIdxs;

        for (int i = 0; i < numGrains; i++)
        {
            tempIdxs.push_back(i);
        }

        /////nastaveni velikosti
        myGrain.setSize(numChannels, grainSize);
        tempBuffer.setSize(numChannels, numGrains * grainSize);

        /////nastaveni AD obalky pro grain

        float attackInPercent = averageSaturation;
        float decayInPercent = averageSaturation;
        int attackInSamples = static_cast<int>(grainSize * (attackInPercent / 100));
        int decayInSamples = static_cast<int>(grainSize * (decayInPercent / 100));

        int decayStartIdx = grainSize - decayInSamples;

        /////cyklus prochazeni a granulizace originalniho bufferu

        int grainIdx = 0;
        for (int sample = 0; sample < (numGrains - 1) * grainSize; sample++)
        {

            /////nacteni stereo samplu do transportniho bufferu o velikosti grainu

            for (int channel = 0; channel < numChannels; channel++)
            {
                myGrain.setSample(channel, grainIdx, originalBuffer.getSample(channel, sample));
            }
            if (grainIdx == grainSize - 1) //pokud je grain plny
            {
                /// vyber nahodneho indexu a nacteni do noveho bufferu
                int removeIdx = rand() % size(tempIdxs);
                int idx = tempIdxs[removeIdx];
                tempIdxs.erase(tempIdxs.begin() + removeIdx);
                for (int channel = 0; channel < numChannels; channel++)
                {
                    /// aplikace AD obalky
                    myGrain.applyGainRamp(0, attackInSamples, 0.0f, 1.0f);
                    myGrain.applyGainRamp(decayStartIdx, decayInSamples, 1.0f, 0.0f);
                    for (int i = 0; i < grainSize; i++)
                    {
                        tempBuffer.setSample(channel, idx * grainSize + i, myGrain.getSample(channel, i));
                    }
                }
                myGrain.clear();
                grainIdx = 0;
            }
            else //pokud grain neni plny
            {
                grainIdx++;
            }
        }
        /////sekce pro doplneni zbytku samplu, ktere prebyvaji a nezaplni cely grain

        int diff = grainSize - grainIdx;
        int lastGrainStartIdx = tempIdxs[0] * grainSize;

        for (int channel = 0; channel < numChannels; channel++)
        {
            for (int i = 0; i < grainIdx - 1; i++) //doplneni zbylych vzorku
            {
                tempBuffer.setSample(channel, lastGrainStartIdx + i, myGrain.getSample(channel, i));
            }
            int numMovedSamples = tempBuffer.getNumSamples() - lastGrainStartIdx - grainIdx;
        }

        /////////////////////////PREKRYTI

        AudioBuffer<float> granulizedBuffer;
        granulizedBuffer.setSize(tempBuffer.getNumChannels(), numGrains * grainSize - (numGrains - 1) * overlapInSamples);

        

        int notOverlappedSamples = grainSize - 2 * overlapInSamples;
        for (int channel = 0; channel < granulizedBuffer.getNumChannels(); channel++)
        {
            int granulizedBufferSampleIdx = 0;

            for (int grainNumber = 0; grainNumber < numGrains; grainNumber++) //projdi postupne vsechny grainy
            {
                for (int sampleInGrainIdx = 0; sampleInGrainIdx < grainSize; sampleInGrainIdx++) //projdi vsechny vzorky v grainu
                {
                    if (grainNumber == 0) //prvni grain (nechceme prekryti na zacatku)
                    {
                        if (sampleInGrainIdx < grainSize - overlapInSamples) //vsechny grainy do prekryti, pouze je prepis tak jak jsou
                        {
                            granulizedBuffer.setSample(channel, granulizedBufferSampleIdx, tempBuffer.getSample(channel, grainNumber * grainSize + sampleInGrainIdx));
                            granulizedBufferSampleIdx++;
                            continue;
                        }
                        //vzorky zasazene prekrytim se prumeruji se zacatkem dalsiho grainu
                        float average = (tempBuffer.getSample(channel, grainNumber * grainSize + sampleInGrainIdx) + tempBuffer.getSample(channel, (grainNumber + 1) * grainSize + sampleInGrainIdx - overlapInSamples - notOverlappedSamples)) / 2;
                        //float addition = tempBuffer.getSample(channel, grainNumber * grainSize + sampleInGrainIdx) + tempBuffer.getSample(channel, (grainNumber + 1) * grainSize + sampleInGrainIdx - overlapInSamples - notOverlappedSamples);
                        //a zapisuji
                        granulizedBuffer.setSample(channel, granulizedBufferSampleIdx, average);
                        granulizedBufferSampleIdx++;
                        continue;
                    }
                    if (grainNumber == numGrains - 1) //zde zpracovavame posledni grain 
                    {
                        if (sampleInGrainIdx < overlapInSamples) //prekryti ze zacatku preskakujeme, protoze uz bylo vyreseno v predposlednim grainu
                        {
                            continue;
                        }
                        granulizedBuffer.setSample(channel, granulizedBufferSampleIdx, tempBuffer.getSample(channel, grainNumber * grainSize + sampleInGrainIdx)); //ostatni vzorky zapiseme jak jsou
                        granulizedBufferSampleIdx++;
                        continue;
                    }
                    //grain, ktery neni prvni nebo posledni
                    if (sampleInGrainIdx < overlapInSamples) //preskakujeme zacatek grainu, uz je vyreseny
                    {
                        continue;
                    }
                    if (sampleInGrainIdx < grainSize - overlapInSamples) //grainy, ktere jsou mimo prekryti, zapisujeme tak jak jsou
                    {
                        granulizedBuffer.setSample(channel, granulizedBufferSampleIdx, tempBuffer.getSample(channel, grainNumber * grainSize + sampleInGrainIdx));
                        granulizedBufferSampleIdx++;
                        continue;
                    }
                    //pro prekryti na konci zase udelame prumer poslednich vzorku n-teho grainu a prvnich vzorku n+1 grainu
                    float average = (tempBuffer.getSample(channel, grainNumber * grainSize + sampleInGrainIdx) + tempBuffer.getSample(channel, (grainNumber + 1) * grainSize + sampleInGrainIdx - overlapInSamples - notOverlappedSamples)) / 2;
                    //float addition = tempBuffer.getSample(channel, grainNumber * grainSize + sampleInGrainIdx) + tempBuffer.getSample(channel, (grainNumber + 1) * grainSize + sampleInGrainIdx - overlapInSamples - notOverlappedSamples);

                    granulizedBuffer.setSample(channel, granulizedBufferSampleIdx, average);
                    granulizedBufferSampleIdx++;
                }
            }
        }

        readWaveform(granulizedBuffer);

        MemoryBlock memoryBlock;
        {
            WavAudioFormat wavFormat;
            std::unique_ptr<AudioFormatWriter> writer(wavFormat.createWriterFor(new MemoryOutputStream(memoryBlock, false), myFormatReader->sampleRate, granulizedBuffer.getNumChannels(), 24, {}, 0));

            if (writer != nullptr)
            {
                writer->writeFromAudioSampleBuffer(granulizedBuffer, 0, granulizedBuffer.getNumSamples());
            }
        }

        WavAudioFormat wavFormat;
        std::unique_ptr<AudioFormatReader> reader(wavFormat.createReaderFor(new MemoryInputStream(memoryBlock, false), true));

        ////////////////////////

        BigInteger range;
        int lowerLimit = myValTrSt.getRawParameterValue("ZONE LOWER LIMIT")->load();
        int upperLimit = myValTrSt.getRawParameterValue("ZONE UPPER LIMIT")->load();

        if (lowerLimit == upperLimit)
        {
            lowerLimit = upperLimit - 1;
        }
        if (upperLimit < lowerLimit)
        {
            std::swap(lowerLimit, upperLimit);
        }
        if (lowerLimit == upperLimit && lowerLimit == 0)
        {
            upperLimit = lowerLimit + 1;
        }

        range.setRange(lowerLimit, upperLimit - lowerLimit + 1, true);

        int rootNote = myValTrSt.getRawParameterValue("ROOT NOTE")->load();
        myGrannySynth.addSound(new SamplerSound("Sample", *reader, range, rootNote, 0.1, 0.1, 2400.0));
        updateADSR();

        //cisteni pameti
        myGrain.setSize(0, 0, 0, 1, 0);
        tempBuffer.setSize(0, 0, 0, 1, 0);
        granulizedBuffer.setSize(0, 0, 0, 1, 0);
    }
}

void SynthGrannyAudioProcessor::loadFileViaButton()
{
    FileChooser chooser{ "Vyberte soubor", {}, "*.wav;*.mp3;*.flac;*.aiff;*.aif;*.wma;*.ogg"};
    if (chooser.browseForFileToOpen())
    {
        auto file = chooser.getResult();
        myFormatReader = myFormatManager.createReaderFor(file);
    }
    else
    {
        return;
    }

    originalBuffer.setSize(myFormatReader->numChannels, myFormatReader->lengthInSamples);
    myFormatReader->read(&originalBuffer, 0, myFormatReader->lengthInSamples, 0, true, true);

    degranulize();
}

void SynthGrannyAudioProcessor::degranulize()
{
    myGrannySynth.clearSounds();
    if (originalBuffer.getNumSamples() > 0)
    {
        readWaveform(originalBuffer);

        BigInteger range;
        int lowerLimit = myValTrSt.getRawParameterValue("ZONE LOWER LIMIT")->load();
        int upperLimit = myValTrSt.getRawParameterValue("ZONE UPPER LIMIT")->load();

        if (lowerLimit == upperLimit)
        {
            lowerLimit = upperLimit - 1;
        }
        if (upperLimit < lowerLimit)
        {
            std::swap(lowerLimit, upperLimit);
        }
        if (lowerLimit == upperLimit && lowerLimit == 0)
        {
            upperLimit = lowerLimit + 1;
        }

        range.setRange(lowerLimit, upperLimit - lowerLimit + 1, true);

        int rootNote = myValTrSt.getRawParameterValue("ROOT NOTE")->load();
        myGrannySynth.addSound(new SamplerSound("Sample", *myFormatReader, range, rootNote, 0.1, 0.1, 2400.0));
        updateADSR();
    }
}

void SynthGrannyAudioProcessor::readWaveform(AudioBuffer<float> tempBuffer)
{
    myWave.makeCopyOf(tempBuffer, 1);
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

    parameters.push_back(std::make_unique<AudioParameterFloat>("GRAIN LENGTH", "Length", 0.1f, 100.0f, 50.0f));
    parameters.push_back(std::make_unique<AudioParameterFloat>("GRAIN ATTACK", "Attack", 0.0f, 100.0f, 50.0f));
    parameters.push_back(std::make_unique<AudioParameterFloat>("GRAIN DECAY", "Decay", 0.0f, 100.0f, 50.0f));
    parameters.push_back(std::make_unique<AudioParameterFloat>("GRAIN OVERLAP", "Overlap", 0.0f, 50.0f, 0.0f));
    parameters.push_back(std::make_unique<AudioParameterFloat>("GRAIN BALANCE", "Balance", -100.0f, 100.0f, 0.0f));

    parameters.push_back(std::make_unique<AudioParameterInt>("ROOT NOTE", "Root", 0, 127, 72));
    parameters.push_back(std::make_unique<AudioParameterInt>("ZONE LOWER LIMIT", "Lower limit", 0, 127, 0));
    parameters.push_back(std::make_unique<AudioParameterInt>("ZONE UPPER LIMIT", "Upper limit", 0, 127, 127));
    parameters.push_back(std::make_unique<AudioParameterFloat>("VOLUME", "Volume", 0.0f, 1.5f, 1.0f));

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
