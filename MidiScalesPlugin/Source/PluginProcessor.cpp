/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Utilities.h"
#include "PressedChord.h"

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
    m_ScaleNotes.ensureStorageAllocated(SCALES_OCTAVE_STEPS);
    
    m_iScaleNote = -1;
    m_ScaleType = Scales::Type::Invalid;
    m_ChordType.set(Chords::Type::Invalid);
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
    int iSamplePosition;
    juce::MidiMessage m;
    
    // Start - Atomic Variable Access
    
    Chords::Type::eType chordType = m_ChordType.get();
    
    // End - Atomic Variable Access
    
    if(chordType == Chords::Type::Invalid)
        return;
    
    for (juce::MidiBuffer::Iterator i (midiMessages); i.getNextEvent (m, iSamplePosition);)
    {
        if(m.isNoteOn())
        {
            if(m_currentChord.IsValid())
            {
                m_currentChord.GenerateMidi(false , iSamplePosition-1, m.getTimeStamp(), processedMidi, keyboardStateMidi);
                m_currentChord.Reset();
            }
            
            const bool bIsNoteInScale = IsNoteInScaleSafe(m.getNoteNumber());
            if(bIsNoteInScale)
            {
                m_currentChord.Setup(m.getNoteNumber(), m.getChannel(), chordType, m.getVelocity(), m.getTimeStamp());
                m_currentChord.GenerateMidi(true , iSamplePosition, m.getTimeStamp(), processedMidi, keyboardStateMidi);
            }
            else
            {
                // Generate UI Message
                juce::MidiMessage k = juce::MidiMessage::noteOn(KEYBOARD_UI_NOTE_CHANNEL, m.getNoteNumber() % SCALES_DOUBLE_OCTAVE_STEPS, m.getVelocity());
                k.setTimeStamp(m.getTimeStamp());
                keyboardStateMidi.addEvent(k, iSamplePosition);
            }
        }
        else if(m.isNoteOff())
        {
            if(m_currentChord.IsValid() && m_currentChord.GetRootNote() == m.getNoteNumber())
            {
                m_currentChord.GenerateMidi(false, iSamplePosition, m.getTimeStamp(), processedMidi, keyboardStateMidi);
                m_currentChord.Reset();
            }
            
            const bool bIsNoteInScale = IsNoteInScaleSafe(m.getNoteNumber());
            if(!bIsNoteInScale)
            {
                // Generate UI Message
                juce::MidiMessage k = juce::MidiMessage::noteOff(KEYBOARD_UI_NOTE_CHANNEL, m.getNoteNumber() % SCALES_DOUBLE_OCTAVE_STEPS, uint8_t(0));
                k.setTimeStamp(m.getTimeStamp());
                keyboardStateMidi.addEvent(k, iSamplePosition);
            }
        }
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

void MidiScalesPluginAudioProcessor::SetScaleSafe(int iScaleNote, Scales::Type::eType scaleType)
{
    juce::ScopedLock lock(m_ScalesLock);
    
    m_iScaleNote = iScaleNote;
    m_ScaleType = scaleType;
    
    Helpers::GetScaleSequence(m_ScaleType, m_ScaleNotes);
    const int iNumNotes = m_ScaleNotes.size();
    
    if(m_iScaleNote >= 0)
    {
        for(int i=0; i<iNumNotes; i++)
        {
            m_ScaleNotes.set(i, (m_ScaleNotes[i] + m_iScaleNote) % SCALES_OCTAVE_STEPS);
        }
    }
}

bool MidiScalesPluginAudioProcessor::IsNoteInScaleSafe(int iMidiNote)
{
    juce::ScopedLock lock(m_ScalesLock);
    return  m_ScaleNotes.contains(iMidiNote % SCALES_OCTAVE_STEPS);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MidiScalesPluginAudioProcessor();
}


