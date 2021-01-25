/*
  ==============================================================================

    PressedChord.cpp
    Created: 24 Jan 2021 1:19:37pm
    Author:  Maaz

  ==============================================================================
*/

#include "PressedChord.h"

PressedChord::PressedChord()
{
    m_notesPressed.ensureStorageAllocated(CHORD_MAX_NOTES);
    Reset();
}

PressedChord::~PressedChord()
{
    m_notesPressed.clear();
}

void PressedChord::Reset()
{
    m_eChordType = Chords::Type::Invalid;
    m_iRootNote = -1;
    m_dTimeStamp = 0;
    m_uVelocity = 0;
    m_iChannel = -1;
    m_notesPressed.clearQuick();
}

bool PressedChord::IsValid()
{
    return m_iRootNote > 0 && m_iRootNote < SCALES_TOTAL_STEPS && m_notesPressed.size() > 0 && m_iChannel >= 0;
}

void PressedChord::Setup(int iRootNote, int iChannel, Chords::Type::eType eChordType, juce::uint8 uVelocity, double dTimeStamp)
{
    m_eChordType = eChordType;
    m_iRootNote = iRootNote;
    m_dTimeStamp = dTimeStamp;
    m_uVelocity = uVelocity;
    m_iChannel = iChannel;
    
    Helpers::GetChordSequence(eChordType, m_notesPressed);
}

void PressedChord::GenerateMidi(bool bNoteOnOff, int iSamplePosition, double fCurrentTimeStamp, juce::MidiBuffer& processedMidi, juce::MidiBuffer& keyboardStateMidi)
{
    if(!IsValid())
        return;
    
    for(auto iChordNote : m_notesPressed)
    {
        juce::MidiMessage p, k;
        
        if(bNoteOnOff)
        {
            p = juce::MidiMessage::noteOn(m_iChannel, (m_iRootNote + iChordNote) % SCALES_TOTAL_STEPS , m_uVelocity);
            p.setTimeStamp(fCurrentTimeStamp);
            processedMidi.addEvent (p, iSamplePosition);
            
            k = juce::MidiMessage::noteOn(m_iChannel, ((m_iRootNote % SCALES_DOUBLE_OCTAVE_STEPS) + iChordNote) % SCALES_OCTAVE_STEPS_RANGE, m_uVelocity);
            k.setTimeStamp(m_dTimeStamp);
            keyboardStateMidi.addEvent(k, iSamplePosition);
        }
        else
        {
            const juce::uint8 uZeroVelocity = 0;
            
            p = juce::MidiMessage::noteOff(m_iChannel, (m_iRootNote + iChordNote) % SCALES_TOTAL_STEPS , uZeroVelocity);
            p.setTimeStamp(m_dTimeStamp);
            processedMidi.addEvent (p, iSamplePosition);
            
            k = juce::MidiMessage::noteOff(m_iChannel, ((m_iRootNote % SCALES_DOUBLE_OCTAVE_STEPS) + iChordNote) % SCALES_OCTAVE_STEPS_RANGE, uZeroVelocity);
            k.setTimeStamp(m_dTimeStamp);
            keyboardStateMidi.addEvent(k, iSamplePosition);
        }

    }
}
