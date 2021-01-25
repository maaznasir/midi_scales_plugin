/*
  ==============================================================================

    PressedChord.h
    Created: 24 Jan 2021 1:19:27pm
    Author:  Maaz

  ==============================================================================
*/

#pragma once
#include "Utilities.h"

class PressedChord
{
public:
    PressedChord();
    ~PressedChord();
    
    void Reset();
    bool IsValid();

    void Setup(int iRootNote, int iChannel, Chords::Type::eType eChordType, juce::uint8 uVelocity, double dTimeStamp);
    // bNoteOnOff: TRUE -> On, FALSE -> Off
    void GenerateMidi(bool bNoteOnOff, int iSamplePosition, double fCurrentTimeStamp, juce::MidiBuffer& processedMidi, juce::MidiBuffer& keyboardStateMidi);
    
    int GetRootNote() const { return m_iRootNote; }

private:
    ChordNotes m_notesPressed;
    Chords::Type::eType m_eChordType;
    int m_iRootNote;
    double m_dTimeStamp;
    juce::uint8 m_uVelocity;
    int m_iChannel;
};
