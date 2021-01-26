/*
  ==============================================================================

    ScalesKeyboardComponent.cpp
    Created: 10 Jan 2021 5:10:29pm
    Author:  Maaz

  ==============================================================================
*/

#include "ScalesKeyboardComponent.h"

ScalesKeyboardComponent::ScalesKeyboardComponent (juce::MidiKeyboardState& state,
                                                  BaseKeyboardComponent::Orientation orientation)
: BaseKeyboardComponent(state, orientation),
m_keyboardState(state)
{
    m_iScaleRootNote = -1;
    m_iScaleBaseNote = -1;
    m_eScaleType = Scales::Type::Invalid;
    
    m_ScaleNotes.ensureStorageAllocated(SCALES_OCTAVE_STEPS);
}

juce::String ScalesKeyboardComponent::getWhiteNoteText (int midiNoteNumber)
{
    if(!HasValidScale())
        return "";
    
    const int iNoteNumber = midiNoteNumber % SCALES_OCTAVE_STEPS;
    const int idx = m_ScaleNotes.indexOf(iNoteNumber);
    
    if(idx >= 0)
    {
        const int iShiftedMajorNote = (idx + m_iScaleBaseNote) % Notes::Type::Total;
        
        juce::String majorNoteType(Helpers::GetNoteString((Notes::Type::eType) iShiftedMajorNote));
        
        const int iMajorNoteNumber = Helpers::GetNoteNumber((Notes::Type::eType) iShiftedMajorNote);
        int iNoteDiff =  iNoteNumber - iMajorNoteNumber;
        if(abs(iNoteDiff) > 5)
            iNoteDiff = iNoteDiff > 0 ? iNoteDiff - SCALES_OCTAVE_STEPS : iNoteDiff + SCALES_OCTAVE_STEPS;
        
        juce::String noteDiff;
        if(iNoteDiff == 1)
            noteDiff = "#";
        else if(iNoteDiff > 1)
            noteDiff = "##";
        else if(iNoteDiff == -1)
            noteDiff = "b";
        else if(iNoteDiff < -1)
            noteDiff = "bb";
        else
            noteDiff ="";
        
        return majorNoteType + noteDiff;
    }
 
    return "";
}

juce::String ScalesKeyboardComponent::getBlackNoteText (int midiNoteNumber)
{
    return getWhiteNoteText(midiNoteNumber);
}


void ScalesKeyboardComponent::drawBlackNote (int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                                             bool isDown, bool isOver, juce::Colour noteFillColour)
{
    auto c = noteFillColour;
    
    juce::Colour pressedColour = juce::Colours::lightblue;
    juce::Colour invalidColour = juce::Colours::lightgrey;
    //juce::Colour overColour = juce::Colours::lightblue;

    if (isDown)
    {
        const bool bNoteActive = m_keyboardState.isNoteOn(KEYBOARD_UI_NOTE_CHANNEL, midiNoteNumber);
        if(bNoteActive && !m_ScaleNotes.contains(midiNoteNumber % SCALES_OCTAVE_STEPS))
        {
            c = c.overlaidWith ( invalidColour );
        }
        else
        {
            c = c.overlaidWith ( pressedColour );
        }
    }
    
    //if (isOver)  c = c.overlaidWith ( overColour );
    
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
        

        const bool bRootNoteOn = isDown ? m_keyboardState.isNoteOn(KEYBOARD_UI_NOTE_CHANNEL, midiNoteNumber) : false;
        
        juce::Colour textColour(isDown ? (bRootNoteOn ? juce::Colours::red : juce::Colours::black) : juce::Colours::white);
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
    juce::Colour invalidColour = juce::Colours::lightgrey;
    //juce::Colour overColour = juce::Colours::lightblue;
    
    if (isDown)
    {
        const bool bNoteActive = m_keyboardState.isNoteOn(KEYBOARD_UI_NOTE_CHANNEL, midiNoteNumber);
        if(bNoteActive && !m_ScaleNotes.contains(midiNoteNumber % SCALES_OCTAVE_STEPS))
        {
            c = c.overlaidWith ( invalidColour );
        }
        else
        {
            c = c.overlaidWith ( pressedColour );
        }
    }
    
    //if (isOver)  c = c.overlaidWith (overColour);
    
    g.setColour (c);
    g.fillRect (area);
    
    auto text = getWhiteNoteText (midiNoteNumber);
    
    if (text.isNotEmpty())
    {
        auto fontHeight = juce::jmin (16.0f, getKeyWidth() * 0.9f);
        const bool bRootNoteOn = isDown ? m_keyboardState.isNoteOn(KEYBOARD_UI_NOTE_CHANNEL, midiNoteNumber) : false;
        
        juce::Colour updatedTextColour(bRootNoteOn ? juce::Colours::red : juce::Colours::black);
        g.setColour (updatedTextColour);
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

void ScalesKeyboardComponent::SetScale(int iRootNote, int iBaseNote, Scales::Type::eType eScaleType)
{
    m_iScaleRootNote = iRootNote;
    m_iScaleBaseNote = iBaseNote;
    m_eScaleType = eScaleType;
    
    Helpers::GetScaleSequence(m_eScaleType, m_ScaleNotes);
    const int iNumNotes = m_ScaleNotes.size();
    
    if(m_iScaleRootNote >= 0)
    {
        for(int i=0; i<iNumNotes; i++)
        {
            m_ScaleNotes.set(i, (m_ScaleNotes[i] + m_iScaleRootNote) % SCALES_OCTAVE_STEPS);
        }
    }
    
    repaint();
}

bool ScalesKeyboardComponent::HasValidScale()
{
    return m_iScaleBaseNote >= 0 && m_iScaleRootNote >= 0 && m_ScaleNotes.size() > 0;
}

