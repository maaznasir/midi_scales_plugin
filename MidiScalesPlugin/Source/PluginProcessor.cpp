/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MidiScalesPluginAudioProcessor::MidiScalesPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    m_keyboardState.reset();
}

MidiScalesPluginAudioProcessor::~MidiScalesPluginAudioProcessor()
{
}

//==============================================================================
const juce::String MidiScalesPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MidiScalesPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MidiScalesPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MidiScalesPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MidiScalesPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MidiScalesPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MidiScalesPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MidiScalesPluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MidiScalesPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void MidiScalesPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MidiScalesPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void MidiScalesPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MidiScalesPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

void MidiScalesPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();

    juce::MidiBuffer processedMidi;
    juce::MidiBuffer keyboardStateMidi;
    int time;
    juce::MidiMessage m, m1, m2, m3, k1, k2, k3;
    
    for (juce::MidiBuffer::Iterator i (midiMessages); i.getNextEvent (m, time);)
    {
        //Custom messaging
        
        // Major Triad Indices
        const int iFirstIdx = 0;
        const int iThirdIdx = 4;
        const int iFifthIdx = 7;
        
        if (m.isNoteOn())
        {
            const int iFirstNote = (m.getNoteNumber() + iFirstIdx) % SCALES_TOTAL_STEPS;
            const int iThirdNote = (m.getNoteNumber() + iThirdIdx) % SCALES_TOTAL_STEPS;
            const int iFifthNote = (m.getNoteNumber() + iFifthIdx) % SCALES_TOTAL_STEPS;
            
            m1 = juce::MidiMessage::noteOn(m.getChannel(), iFirstNote, m.getVelocity());
            m1.setTimeStamp(m.getTimeStamp());
            m2 = juce::MidiMessage::noteOn(m.getChannel(), iThirdNote, m.getVelocity());
            m2.setTimeStamp(m.getTimeStamp());
            m3 = juce::MidiMessage::noteOn(m.getChannel(), iFifthNote, m.getVelocity());
            m3.setTimeStamp(m.getTimeStamp());
            
            const int iAdjustedFirst = ((m.getNoteNumber() % SCALES_DOUBLE_OCTAVE_STEPS) + iFirstIdx) % SCALES_OCTAVE_STEPS_RANGE;
            const int iAdjustedThird = ((m.getNoteNumber() % SCALES_DOUBLE_OCTAVE_STEPS) + iThirdIdx) % SCALES_OCTAVE_STEPS_RANGE;
            const int iAdjustedFifth = ((m.getNoteNumber() % SCALES_DOUBLE_OCTAVE_STEPS) + iFifthIdx) % SCALES_OCTAVE_STEPS_RANGE;
            
            k1 = juce::MidiMessage::noteOn(m.getChannel(), iAdjustedFirst, m.getVelocity());
            k1.setTimeStamp(m.getTimeStamp());
            k2 = juce::MidiMessage::noteOn(m.getChannel(), iAdjustedThird, m.getVelocity());
            k2.setTimeStamp(m.getTimeStamp());
            k3 = juce::MidiMessage::noteOn(m.getChannel(), iAdjustedFifth, m.getVelocity());
            k3.setTimeStamp(m.getTimeStamp());
        }
        else if (m.isNoteOff())
        {
            const int iFirstNote = (m.getNoteNumber() + iFirstIdx) % SCALES_TOTAL_STEPS;
            const int iThirdNote = (m.getNoteNumber() + iThirdIdx) % SCALES_TOTAL_STEPS;
            const int iFifthNote = (m.getNoteNumber() + iFifthIdx) % SCALES_TOTAL_STEPS;
            
            m1 = juce::MidiMessage::noteOff(m.getChannel(), iFirstNote, m.getVelocity());
            m1.setTimeStamp(m.getTimeStamp());
            m2 = juce::MidiMessage::noteOff(m.getChannel(), iThirdNote, m.getVelocity());
            m2.setTimeStamp(m.getTimeStamp());
            m3 = juce::MidiMessage::noteOff(m.getChannel(), iFifthNote, m.getVelocity());
            m3.setTimeStamp(m.getTimeStamp());
            
            const int iAdjustedFirst = ((m.getNoteNumber() % SCALES_DOUBLE_OCTAVE_STEPS) + iFirstIdx) % SCALES_OCTAVE_STEPS_RANGE;
            const int iAdjustedThird = ((m.getNoteNumber() % SCALES_DOUBLE_OCTAVE_STEPS) + iThirdIdx) % SCALES_OCTAVE_STEPS_RANGE;
            const int iAdjustedFifth = ((m.getNoteNumber() % SCALES_DOUBLE_OCTAVE_STEPS) + iFifthIdx) % SCALES_OCTAVE_STEPS_RANGE;
            
            k1 = juce::MidiMessage::noteOff(m.getChannel(), iAdjustedFirst, m.getVelocity());
            k1.setTimeStamp(m.getTimeStamp());
            k2 = juce::MidiMessage::noteOff(m.getChannel(), iAdjustedThird, m.getVelocity());
            k2.setTimeStamp(m.getTimeStamp());
            k3 = juce::MidiMessage::noteOff(m.getChannel(), iAdjustedFifth, m.getVelocity());
            k3.setTimeStamp(m.getTimeStamp());
        }
        
        processedMidi.addEvent (m1, time);
        processedMidi.addEvent (m2, time);
        processedMidi.addEvent (m3, time);
        keyboardStateMidi.addEvent (k1, time);
        keyboardStateMidi.addEvent (k2, time);
        keyboardStateMidi.addEvent (k3, time);
    }
    
    midiMessages.swapWith (processedMidi);
    
    m_keyboardState.processNextMidiBuffer (keyboardStateMidi, 0, buffer.getNumSamples(), true);
}

//==============================================================================
bool MidiScalesPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MidiScalesPluginAudioProcessor::createEditor()
{
    return new MidiScalesPluginAudioProcessorEditor (*this);
}

//==============================================================================
void MidiScalesPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MidiScalesPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MidiScalesPluginAudioProcessor();
}
