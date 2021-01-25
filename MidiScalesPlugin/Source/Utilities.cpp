/*
  ==============================================================================

    Utilities.cpp
    Created: 24 Jan 2021 2:48:39pm
    Author:  Maaz

  ==============================================================================
*/

#include "Utilities.h"

namespace Helpers
{
    // Hard code scales in integer notation. Should come from data in future!
    void GetScaleSequence(Scales::Type::eType scaleType, ScaleNotes& scaleComposition)
    {
        scaleComposition.clearQuick();
        switch (scaleType)
        {
            case Scales::Type::Major:
            {
                scaleComposition.add(0, 2, 4, 5, 7, 9, 11);
            }
                break;
            case Scales::Type::NaturalMinor:
            {
                scaleComposition.add(0, 2, 3, 5, 7, 8, 10);
            }
                break;
            case Scales::Type::HarmonicMinor:
            {
                scaleComposition.add(0, 2, 3, 5, 7, 8, 11);
            }
                break;
            case Scales::Type::MelodicMinor:
            {
                scaleComposition.add(0, 2, 3, 5, 7, 9 ,11);
            }
                break;
            default:
                break;
                //Do Nothing
        }
    }
    
    // Hard code chords in integer notation. Should come from data in future!
    void GetChordSequence(Chords::Type::eType chordType, ChordNotes& chordComposition)
    {
        chordComposition.clearQuick();
        switch (chordType)
        {
            case Chords::Type::MajorTriad:
            {
                chordComposition.add(0, 4, 7);
            }
                break;
            case Chords::Type::MinorTriad:
            {
                chordComposition.add(0, 3, 7);
            }
                break;
            case Chords::Type::MajorSeventh:
            {
                chordComposition.add(0, 4, 7, 11);
            }
                break;
            case Chords::Type::MinorSeventh:
            {
                chordComposition.add(0, 3, 7, 10);
            }
                break;
            default:
                break;
                //Do Nothing
        }
    }
    
    int GetNoteNumber(Notes::Type::eType eType)
    {
        const static int s_noteIndices[] = {0, 2, 4, 5, 7, 9, 11};
        return s_noteIndices[eType];
    }
    
    // TODO: These names should come from data
    juce::String GetScaleTypeString(Scales::Type::eType scaleType)
    {
        switch (scaleType)
        {
            case Scales::Type::Major:
                return "Major/Ionian";
            case Scales::Type::NaturalMinor:
                return "Natural Minor";
            case Scales::Type::HarmonicMinor:
                return "Harmonic Minor";
            case Scales::Type::MelodicMinor:
                return "Melodic Minor";
            default:
                return "Invalid";
                //Do Nothing
        }
    }
    
    // TODO: These names should come from data
    juce::String GetChordTypeString(Chords::Type::eType chordType)
    {
        switch (chordType)
        {
            case Chords::Type::MajorTriad:
                return "Major Triad";
            case Chords::Type::MinorTriad:
                return "Minor Triad";
            case Chords::Type::MajorSeventh:
                return "Major Seventh";
            case Chords::Type::MinorSeventh:
                return "Minor Seventh";
            default:
                return "Invalid";
                //Do Nothing
        }
    }
    
    juce::String GetNoteString(Notes::Type::eType noteType)
    {
        const static char* s_noteText[] = {"C", "D", "E", "F", "G", "A", "B"};
        return s_noteText[noteType];
    }
    
    int GetNoteType(const char* note)
    {
        const static char* s_noteText[] = {"C", "D", "E", "F", "G", "A", "B"};
        
        for(int i=0; i<Notes::Type::Total; i++)
        {
            if(std::strcmp(s_noteText[i], note) == 0)
                return i;
        }
        
        return -1;
    }
}
