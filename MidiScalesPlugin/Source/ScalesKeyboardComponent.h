/*
  ==============================================================================

    ScalesKeyboardComponent.h
    Created: 10 Jan 2021 5:10:16pm
    Author:  Maaz

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class ScalesKeyboardComponent : public juce::MidiKeyboardComponent
{
public:
    ScalesKeyboardComponent (juce::MidiKeyboardState& state,
                             juce::MidiKeyboardComponent::Orientation orientation);
    
    virtual juce::String getWhiteNoteText (int midiNoteNumber) override;
    
};
