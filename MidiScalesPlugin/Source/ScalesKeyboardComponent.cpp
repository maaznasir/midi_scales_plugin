/*
  ==============================================================================

    ScalesKeyboardComponent.cpp
    Created: 10 Jan 2021 5:10:29pm
    Author:  Maaz

  ==============================================================================
*/

#include "ScalesKeyboardComponent.h"

ScalesKeyboardComponent::ScalesKeyboardComponent (juce::MidiKeyboardState& state,
                                                  juce::MidiKeyboardComponent::Orientation orientation)
: juce::MidiKeyboardComponent(state, orientation)
{
}

juce::String ScalesKeyboardComponent::getWhiteNoteText (int /*midiNoteNumber*/)
{
    return juce::String();
}
