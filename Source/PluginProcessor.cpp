#include "PluginProcessor.h"
#include "PluginEditor.h"

SynthAudioProcessor::SynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters(*this, nullptr, "PARAMETERS", initializeGUI())
#endif
{
    initSynth();
}

juce::AudioProcessorValueTreeState::ParameterLayout SynthAudioProcessor::initializeGUI()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
    //VOLUMEN SLIDER
    params.push_back(std::make_unique<juce::AudioParameterFloat>("VOLUME_ID",
                                                                 "VOLUME_NAME",
                                                                 0.0f,
                                                                 1.0f,
                                                                 0.5f));
    
    //ATTACK SLIDER
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ATTACK_ID",
                                                                 "ATTACK_NAME",
                                                                 0.1f,
                                                                 3.0f,
                                                                 0.5f));
    //SUSTAIN SLIDER
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SUSTAIN_ID",
                                                                 "SUSTAIN_NAME",
                                                                 0.1f,
                                                                 3.0f,
                                                                 0.5f));
    //DECAY SLIDER
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DECAY_ID",
                                                                 "DECAY_NAME",
                                                                 0.1f,
                                                                 3.0f,
                                                                 0.5f));
    //RELEASE SLIDER
    params.push_back(std::make_unique<juce::AudioParameterFloat>("RELEASE_ID",
                                                                 "RELEASE_NAME",
                                                                 0.1f,
                                                                 3.0f,
                                                                 0.5f));
    //DELAY TIME SLIDER
    params.push_back(std::make_unique<juce::AudioParameterFloat>("TIME_ID",
                                                                 "TIME_NAME",
                                                                 0.1f,
                                                                 3.0f,
                                                                 0.5f));
    //FEEDBACK SLIDER
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FEEDBACK_ID",
                                                                 "FEEDBACK_NAME",
                                                                 0.01f,
                                                                 0.99f,
                                                                 0.5f));
    
    return {params.begin(),params.end()};
}

void SynthAudioProcessor::initSynth()
{
    mySynth.addSound(new synth_Sound());
    mySynth.addVoice(new synth_Voice());
}

SynthAudioProcessor::~SynthAudioProcessor(){}

const juce::String SynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SynthAudioProcessor::getNumPrograms()
{
    return 1;
}

int SynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SynthAudioProcessor::setCurrentProgram (int index){}

const juce::String SynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void SynthAudioProcessor::changeProgramName (int index, const juce::String& newName){}

void SynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mySynth.setCurrentPlaybackSampleRate(sampleRate);
    
    for (int i = 0; i < mySynth.getNumVoices(); i++)
    {
        if(auto voice = dynamic_cast<synth_Voice*>(mySynth.getVoice(i)))
            voice->prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    }
}

void SynthAudioProcessor::releaseResources(){}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    for (int i = 0; i < mySynth.getNumVoices(); i++)
    {
        if(auto voice = dynamic_cast<synth_Voice*>(mySynth.getVoice(i)))
        {
            voice->getParams(*parameters.getRawParameterValue("VOLUME_ID"),
                             *parameters.getRawParameterValue("ATTACK_ID"),
                             *parameters.getRawParameterValue("DECAY_ID"),
                             *parameters.getRawParameterValue("SUSTAIN_ID"),
                             *parameters.getRawParameterValue("RELEASE_ID"),
                             *parameters.getRawParameterValue("TIME_ID"),
                             *parameters.getRawParameterValue("FEEDBACK_ID"));
        }
    }
    
    mySynth.renderNextBlock(buffer,
                            midiMessages,
                            0,
                            buffer.getNumSamples());
}

bool SynthAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SynthAudioProcessor::createEditor()
{
    return new SynthAudioProcessorEditor (*this);
}

void SynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData){}

void SynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes){}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynthAudioProcessor();
}
