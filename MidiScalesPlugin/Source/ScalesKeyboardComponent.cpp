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

juce::String ScalesKeyboardComponent::getWhiteNoteText (int midiNoteNumber)
{
    return juce::MidiMessage::getMidiNoteName (midiNoteNumber, true, false, getOctaveForMiddleC());
}

juce::String ScalesKeyboardComponent::getBlackNoteText (int midiNoteNumber)
{
    return juce::MidiMessage::getMidiNoteName (midiNoteNumber, true, false, getOctaveForMiddleC());
}


void ScalesKeyboardComponent::drawBlackNote (int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                                             bool isDown, bool isOver, juce::Colour noteFillColour)
{
    auto c = noteFillColour;
    
    juce::Colour pressedColour = juce::Colours::lightblue;
    juce::Colour overColour = juce::Colours::lightblue;

    if (isDown)  c = c.overlaidWith ( pressedColour );
    if (isOver)  c = c.overlaidWith ( overColour );
    
    g.setColour (c);
    g.fillRect (area);
    
    if (isDown)
    {
        g.setColour (noteFillColour);
        g.drawRect (area);
    }
    else
    {
        g.setColour (c.brighter());
        auto sideIndent = 1.0f / 8.0f;
        auto topIndent = 7.0f / 8.0f;
        auto w = area.getWidth();
        auto h = area.getHeight();
        
        switch (getOrientation())
        {
            case horizontalKeyboard:            g.fillRect (area.reduced (w * sideIndent, 0).removeFromTop   (h * topIndent)); break;
            case verticalKeyboardFacingLeft:    g.fillRect (area.reduced (0, h * sideIndent).removeFromRight (w * topIndent)); break;
            case verticalKeyboardFacingRight:   g.fillRect (area.reduced (0, h * sideIndent).removeFromLeft  (w * topIndent)); break;
            default: break;
        }
    }
    
    auto text = getBlackNoteText(midiNoteNumber);
    
    if (text.isNotEmpty())
    {
        auto fontHeight = juce::jmin (16.0f, getKeyWidth() * 0.9f);
        
        juce::Colour textColour(isDown ? juce::Colours::black : juce::Colours::white);
        g.setColour (textColour);
        g.setFont (juce::Font (fontHeight).withHorizontalScale (1.0f));
        
        switch (getOrientation())
        {
            case horizontalKeyboard:            g.drawText (text, area.withTrimmedLeft (1.0f).withTrimmedBottom (2.0f), juce::Justification::centredBottom, false); break;
            case verticalKeyboardFacingLeft:    g.drawText (text, area.reduced (2.0f), juce::Justification::centredLeft,   false); break;
            case verticalKeyboardFacingRight:   g.drawText (text, area.reduced (2.0f), juce::Justification::centredRight,  false); break;
            default: break;
        }
    }
    
}

void ScalesKeyboardComponent::drawWhiteNote (int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                                             bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour)
{
    auto c = juce::Colours::transparentWhite;
    
    juce::Colour pressedColour = juce::Colours::lightblue;
    juce::Colour overColour = juce::Colours::lightblue;
    
    if (isDown)  c = pressedColour;
    if (isOver)  c = c.overlaidWith (overColour);
    
    g.setColour (c);
    g.fillRect (area);
    
    auto text = getWhiteNoteText (midiNoteNumber);
    
    if (text.isNotEmpty())
    {
        auto fontHeight = juce::jmin (16.0f, getKeyWidth() * 0.9f);
        
        g.setColour (textColour);
        g.setFont (juce::Font (fontHeight).withHorizontalScale (1.0f));
        
        switch (getOrientation())
        {
            case horizontalKeyboard:            g.drawText (text, area.withTrimmedLeft (1.0f).withTrimmedBottom (2.0f), juce::Justification::centredBottom, false); break;
            case verticalKeyboardFacingLeft:    g.drawText (text, area.reduced (2.0f), juce::Justification::centredLeft,   false); break;
            case verticalKeyboardFacingRight:   g.drawText (text, area.reduced (2.0f), juce::Justification::centredRight,  false); break;
            default: break;
        }
    }
    
    if (! lineColour.isTransparent())
    {
        g.setColour (lineColour);
        
        switch (getOrientation())
        {
            case horizontalKeyboard:            g.fillRect (area.withWidth (1.0f)); break;
            case verticalKeyboardFacingLeft:    g.fillRect (area.withHeight (1.0f)); break;
            case verticalKeyboardFacingRight:   g.fillRect (area.removeFromBottom (1.0f)); break;
            default: break;
        }
        
        if (midiNoteNumber == getRangeEnd())
        {
            switch (getOrientation())
            {
                case horizontalKeyboard:            g.fillRect (area.expanded (1.0f, 0).removeFromRight (1.0f)); break;
                case verticalKeyboardFacingLeft:    g.fillRect (area.expanded (0, 1.0f).removeFromBottom (1.0f)); break;
                case verticalKeyboardFacingRight:   g.fillRect (area.expanded (0, 1.0f).removeFromTop (1.0f)); break;
                default: break;
            }
        }
    }
}

