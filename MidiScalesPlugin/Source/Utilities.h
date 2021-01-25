/*
  ==============================================================================

    Utilities.h
    Created: 24 Jan 2021 1:42:57pm
    Author:  Maaz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>

#define CHORD_MAX_NOTES 5
#define SCALES_OCTAVE_STEPS 12

#define SCALES_OCTAVE_STEPS_RANGE 25
#define SCALES_DOUBLE_OCTAVE_STEPS 24
#define SCALES_OCTAVE_NORMALIZED_START 0
#define SCALES_TOTAL_STEPS 128

typedef juce::Array<int> ChordNotes;
typedef juce::Array<int> ScaleNotes;

namespace Chords
{
    namespace Type
    {
        enum eType
        {
            Invalid = 0,
            MajorTriad,
            MinorTriad,
            MajorSeventh,
            MinorSeventh,
            Total = MinorSeventh
        };
    };
};

namespace Scales
{
    namespace Type
    {
        enum eType
        {
            Invalid = 0,
            Major,
            NaturalMinor,
            HarmonicMinor,
            MelodicMinor,
            Total = MelodicMinor
        };
    };
};

namespace Notes
{
    namespace Type
    {
        enum eType
        {
            C = 0,
            D,
            E,
            F,
            G,
            A,
            B,
            Total
        };
    };
};

namespace Helpers
{
    void GetScaleSequence(Scales::Type::eType scaleType, ScaleNotes& scaleComposition);
    void GetChordSequence(Chords::Type::eType chordType, ChordNotes& chordComposition);
    int GetNoteNumber(Notes::Type::eType);
    
    juce::String GetScaleTypeString(Scales::Type::eType scaleType);
    juce::String GetChordTypeString(Chords::Type::eType chordType);
    juce::String GetNoteString(Notes::Type::eType noteType);
    int GetNoteType(const char* note);
}



