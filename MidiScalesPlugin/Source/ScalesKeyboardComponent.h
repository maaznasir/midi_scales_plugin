/*
  ==============================================================================

    ScalesKeyboardComponent.h
    Created: 10 Jan 2021 5:10:16pm
    Author:  Maaz

  ==============================================================================
*/

#pragma once

#include "Utilities.h"

class ScalesKeyboardComponent : public juce::MidiKeyboardComponent
{
public:
    ScalesKeyboardComponent (juce::MidiKeyboardState& state,
                             juce::MidiKeyboardComponent::Orientation orientation);
    
    virtual juce::String getWhiteNoteText (int midiNoteNumber) override;
    
    juce::String getBlackNoteText (int midiNoteNumber);
    
    //==============================================================================
    /** Draws a white note in the given rectangle.
     
     isOver indicates whether the mouse is over the key, isDown indicates whether the key is
     currently pressed down.
     
     When doing this, be sure to note the keyboard's orientation.
     */
    virtual void drawWhiteNote (int midiNoteNumber,
                                juce::Graphics& g, juce::Rectangle<float> area,
                                bool isDown, bool isOver,
                                juce::Colour lineColour, juce::Colour textColour) override;
    
    /** Draws a black note in the given rectangle.
     
     isOver indicates whether the mouse is over the key, isDown indicates whether the key is
     currently pressed down.
     
     When doing this, be sure to note the keyboard's orientation.
     */
    virtual void drawBlackNote (int midiNoteNumber,
                                juce::Graphics& g, juce::Rectangle<float> area,
                                bool isDown, bool isOver,
                                juce::Colour noteFillColour) override;
    
};
