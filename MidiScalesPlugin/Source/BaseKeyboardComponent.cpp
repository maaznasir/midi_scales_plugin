/*
  ==============================================================================

    BaseKeyboardComponent.cpp
    Created: 26 Jan 2021 1:27:52am
    Author:  Maaz

  ==============================================================================
*/

#include "BaseKeyboardComponent.h"

static const uint8 keyboard_whiteNotes[] = { 0, 2, 4, 5, 7, 9, 11 };
static const uint8 keyboard_blackNotes[] = { 1, 3, 6, 8, 10 };

struct BaseKeyboardComponent::UpDownButton  : public Button
{
    UpDownButton (BaseKeyboardComponent& c, int d)
    : Button ({}), owner (c), delta (d)
    {
    }
    
    void clicked() override
    {
        auto note = owner.getLowestVisibleKey();
        
        if (delta < 0)
            note = (note - 1) / 12;
        else
            note = note / 12 + 1;
        
        owner.setLowestVisibleKey (note * 12);
    }
    
    using Button::clicked;
    
    void paintButton (Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        owner.drawUpDownButton (g, getWidth(), getHeight(),
                                shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown,
                                delta > 0);
    }
    
private:
    BaseKeyboardComponent& owner;
    const int delta;
    
    JUCE_DECLARE_NON_COPYABLE (UpDownButton)
};

//==============================================================================
BaseKeyboardComponent::BaseKeyboardComponent (MidiKeyboardState& s, Orientation o)
: state (s), orientation (o)
{
    scrollDown.reset (new UpDownButton (*this, -1));
    scrollUp  .reset (new UpDownButton (*this, 1));
    
    addChildComponent (scrollDown.get());
    addChildComponent (scrollUp.get());
    
    // initialise with a default set of qwerty key-mappings..
    int note = 0;
    
    for (char c : "awsedftgyhujkolp;")
        setKeyPressForNote (KeyPress (c, 0, 0), note++);
    
    mouseOverNotes.insertMultiple (0, -1, 32);
    mouseDownNotes.insertMultiple (0, -1, 32);
    
    colourChanged();
    setWantsKeyboardFocus (true);
    
    state.addListener (this);
    
    startTimerHz (20);
}

BaseKeyboardComponent::~BaseKeyboardComponent()
{
    state.removeListener (this);
}

//==============================================================================
void BaseKeyboardComponent::setKeyWidth (float widthInPixels)
{
    jassert (widthInPixels > 0);
    
    if (keyWidth != widthInPixels) // Prevent infinite recursion if the width is being computed in a 'resized()' call-back
    {
        keyWidth = widthInPixels;
        resized();
    }
}

void BaseKeyboardComponent::setScrollButtonWidth (int widthInPixels)
{
    jassert (widthInPixels > 0);
    
    if (scrollButtonWidth != widthInPixels)
    {
        scrollButtonWidth = widthInPixels;
        resized();
    }
}

void BaseKeyboardComponent::setOrientation (Orientation newOrientation)
{
    if (orientation != newOrientation)
    {
        orientation = newOrientation;
        resized();
    }
}

void BaseKeyboardComponent::setAvailableRange (int lowestNote, int highestNote)
{
    jassert (lowestNote >= 0 && lowestNote <= 127);
    jassert (highestNote >= 0 && highestNote <= 127);
    jassert (lowestNote <= highestNote);
    
    if (rangeStart != lowestNote || rangeEnd != highestNote)
    {
        rangeStart = jlimit (0, 127, lowestNote);
        rangeEnd = jlimit (0, 127, highestNote);
        firstKey = jlimit ((float) rangeStart, (float) rangeEnd, firstKey);
        resized();
    }
}

void BaseKeyboardComponent::setLowestVisibleKey (int noteNumber)
{
    setLowestVisibleKeyFloat ((float) noteNumber);
}

void BaseKeyboardComponent::setLowestVisibleKeyFloat (float noteNumber)
{
    noteNumber = jlimit ((float) rangeStart, (float) rangeEnd, noteNumber);
    
    if (noteNumber != firstKey)
    {
        bool hasMoved = (((int) firstKey) != (int) noteNumber);
        firstKey = noteNumber;
        
        if (hasMoved)
            sendChangeMessage();
        
        resized();
    }
}

void BaseKeyboardComponent::setScrollButtonsVisible (bool newCanScroll)
{
    if (canScroll != newCanScroll)
    {
        canScroll = newCanScroll;
        resized();
    }
}

void BaseKeyboardComponent::colourChanged()
{
    setOpaque (findColour (whiteNoteColourId).isOpaque());
    repaint();
}

//==============================================================================
void BaseKeyboardComponent::setMidiChannel (int midiChannelNumber)
{
    jassert (midiChannelNumber > 0 && midiChannelNumber <= 16);
    
    if (midiChannel != midiChannelNumber)
    {
        resetAnyKeysInUse();
        midiChannel = jlimit (1, 16, midiChannelNumber);
    }
}

void BaseKeyboardComponent::setMidiChannelsToDisplay (int midiChannelMask)
{
    midiInChannelMask = midiChannelMask;
    shouldCheckState = true;
}

void BaseKeyboardComponent::setVelocity (float v, bool useMousePosition)
{
    velocity = jlimit (0.0f, 1.0f, v);
    useMousePositionForVelocity = useMousePosition;
}

//==============================================================================
Range<float> BaseKeyboardComponent::getKeyPosition (int midiNoteNumber, float targetKeyWidth) const
{
    jassert (midiNoteNumber >= 0 && midiNoteNumber < 128);
    
    static const float notePos[] = { 0.0f, 1 - blackNoteWidthRatio * 0.6f,
        1.0f, 2 - blackNoteWidthRatio * 0.4f,
        2.0f,
        3.0f, 4 - blackNoteWidthRatio * 0.7f,
        4.0f, 5 - blackNoteWidthRatio * 0.5f,
        5.0f, 6 - blackNoteWidthRatio * 0.3f,
        6.0f };
    
    auto octave = midiNoteNumber / 12;
    auto note   = midiNoteNumber % 12;
    
    auto start = (float) octave * 7.0f * targetKeyWidth + notePos[note] * targetKeyWidth;
    auto width = MidiMessage::isMidiNoteBlack (note) ? blackNoteWidthRatio * targetKeyWidth : targetKeyWidth;
    
    return { start, start + width };
}

Range<float> BaseKeyboardComponent::getKeyPos (int midiNoteNumber) const
{
    return getKeyPosition (midiNoteNumber, keyWidth)
    - xOffset
    - getKeyPosition (rangeStart, keyWidth).getStart();
}

Rectangle<float> BaseKeyboardComponent::getRectangleForKey (int note) const
{
    jassert (note >= rangeStart && note <= rangeEnd);
    
    auto pos = getKeyPos (note);
    auto x = pos.getStart();
    auto w = pos.getLength();
    
    if (MidiMessage::isMidiNoteBlack (note))
    {
        auto blackNoteLength = getBlackNoteLength();
        
        switch (orientation)
        {
            case horizontalKeyboard:            return { x, 0, w, blackNoteLength };
            case verticalKeyboardFacingLeft:    return { (float) getWidth() - blackNoteLength, x, blackNoteLength, w };
            case verticalKeyboardFacingRight:   return { 0, (float) getHeight() - x - w, blackNoteLength, w };
            default:                            jassertfalse; break;
        }
    }
    else
    {
        switch (orientation)
        {
            case horizontalKeyboard:            return { x, 0, w, (float) getHeight() };
            case verticalKeyboardFacingLeft:    return { 0, x, (float) getWidth(), w };
            case verticalKeyboardFacingRight:   return { 0, (float) getHeight() - x - w, (float) getWidth(), w };
            default:                            jassertfalse; break;
        }
    }
    
    return {};
}

float BaseKeyboardComponent::getKeyStartPosition (int midiNoteNumber) const
{
    return getKeyPos (midiNoteNumber).getStart();
}

float BaseKeyboardComponent::getTotalKeyboardWidth() const noexcept
{
    return getKeyPos (rangeEnd).getEnd();
}

int BaseKeyboardComponent::getNoteAtPosition (Point<float> p)
{
    float v;
    return xyToNote (p, v);
}

int BaseKeyboardComponent::xyToNote (Point<float> pos, float& mousePositionVelocity)
{
    if (! reallyContains (pos.toInt(), false))
        return -1;
    
    auto p = pos;
    
    if (orientation != horizontalKeyboard)
    {
        p = { p.y, p.x };
        
        if (orientation == verticalKeyboardFacingLeft)
            p = { p.x, (float) getWidth() - p.y };
        else
            p = { (float) getHeight() - p.x, p.y };
    }
    
    return remappedXYToNote (p + Point<float> (xOffset, 0), mousePositionVelocity);
}

int BaseKeyboardComponent::remappedXYToNote (Point<float> pos, float& mousePositionVelocity) const
{
    auto blackNoteLength = getBlackNoteLength();
    
    if (pos.getY() < blackNoteLength)
    {
        for (int octaveStart = 12 * (rangeStart / 12); octaveStart <= rangeEnd; octaveStart += 12)
        {
            for (int i = 0; i < 5; ++i)
            {
                auto note = octaveStart + keyboard_blackNotes[i];
                
                if (note >= rangeStart && note <= rangeEnd)
                {
                    if (getKeyPos (note).contains (pos.x - xOffset))
                    {
                        mousePositionVelocity = jmax (0.0f, pos.y / blackNoteLength);
                        return note;
                    }
                }
            }
        }
    }
    
    for (int octaveStart = 12 * (rangeStart / 12); octaveStart <= rangeEnd; octaveStart += 12)
    {
        for (int i = 0; i < 7; ++i)
        {
            auto note = octaveStart + keyboard_whiteNotes[i];
            
            if (note >= rangeStart && note <= rangeEnd)
            {
                if (getKeyPos (note).contains (pos.x - xOffset))
                {
                    auto whiteNoteLength = (orientation == horizontalKeyboard) ? getHeight() : getWidth();
                    mousePositionVelocity = jmax (0.0f, pos.y / (float) whiteNoteLength);
                    return note;
                }
            }
        }
    }
    
    mousePositionVelocity = 0;
    return -1;
}

//==============================================================================
void BaseKeyboardComponent::repaintNote (int noteNum)
{
    if (noteNum >= rangeStart && noteNum <= rangeEnd)
        repaint (getRectangleForKey (noteNum).getSmallestIntegerContainer());
}

void BaseKeyboardComponent::paint (Graphics& g)
{
    g.fillAll (findColour (whiteNoteColourId));
    
    auto lineColour = findColour (keySeparatorLineColourId);
    auto textColour = findColour (textLabelColourId);
    
    for (int octave = 0; octave < 128; octave += 12)
    {
        for (int white = 0; white < 7; ++white)
        {
            auto noteNum = octave + keyboard_whiteNotes[white];
            
            if (noteNum >= rangeStart && noteNum <= rangeEnd)
                drawWhiteNote (noteNum, g, getRectangleForKey (noteNum),
                               state.isNoteOnForChannels (midiInChannelMask, noteNum),
                               mouseOverNotes.contains (noteNum), lineColour, textColour);
        }
    }
    
    float x1 = 0.0f, y1 = 0.0f, x2 = 0.0f, y2 = 0.0f;
    auto width = getWidth();
    auto height = getHeight();
    
    if (orientation == verticalKeyboardFacingLeft)
    {
        x1 = (float) width - 1.0f;
        x2 = (float) width - 5.0f;
    }
    else if (orientation == verticalKeyboardFacingRight)
        x2 = 5.0f;
    else
        y2 = 5.0f;
    
    auto x = getKeyPos (rangeEnd).getEnd();
    auto shadowCol = findColour (shadowColourId);
    
    if (! shadowCol.isTransparent())
    {
        g.setGradientFill (ColourGradient (shadowCol, x1, y1, shadowCol.withAlpha (0.0f), x2, y2, false));
        
        switch (orientation)
        {
            case horizontalKeyboard:            g.fillRect (0.0f, 0.0f, x, 5.0f); break;
            case verticalKeyboardFacingLeft:    g.fillRect ((float) width - 5.0f, 0.0f, 5.0f, x); break;
            case verticalKeyboardFacingRight:   g.fillRect (0.0f, 0.0f, 5.0f, x); break;
            default: break;
        }
    }
    
    if (! lineColour.isTransparent())
    {
        g.setColour (lineColour);
        
        switch (orientation)
        {
            case horizontalKeyboard:            g.fillRect (0.0f, (float) height - 1.0f, x, 1.0f); break;
            case verticalKeyboardFacingLeft:    g.fillRect (0.0f, 0.0f, 1.0f, x); break;
            case verticalKeyboardFacingRight:   g.fillRect ((float) width - 1.0f, 0.0f, 1.0f, x); break;
            default: break;
        }
    }
    
    auto blackNoteColour = findColour (blackNoteColourId);
    
    for (int octave = 0; octave < 128; octave += 12)
    {
        for (int black = 0; black < 5; ++black)
        {
            auto noteNum = octave + keyboard_blackNotes[black];
            
            if (noteNum >= rangeStart && noteNum <= rangeEnd)
                drawBlackNote (noteNum, g, getRectangleForKey (noteNum),
                               state.isNoteOnForChannels (midiInChannelMask, noteNum),
                               mouseOverNotes.contains (noteNum), blackNoteColour);
        }
    }
}

void BaseKeyboardComponent::drawWhiteNote (int midiNoteNumber, Graphics& g, Rectangle<float> area,
                                           bool isDown, bool isOver, Colour lineColour, Colour textColour)
{
    auto c = Colours::transparentWhite;
    
    if (isDown)  c = findColour (keyDownOverlayColourId);
    if (isOver)  c = c.overlaidWith (findColour (mouseOverKeyOverlayColourId));
    
    g.setColour (c);
    g.fillRect (area);
    
    auto text = getWhiteNoteText (midiNoteNumber);
    
    if (text.isNotEmpty())
    {
        auto fontHeight = jmin (12.0f, keyWidth * 0.9f);
        
        g.setColour (textColour);
        g.setFont (Font (fontHeight).withHorizontalScale (0.8f));
        
        switch (orientation)
        {
            case horizontalKeyboard:            g.drawText (text, area.withTrimmedLeft (1.0f).withTrimmedBottom (2.0f), Justification::centredBottom, false); break;
            case verticalKeyboardFacingLeft:    g.drawText (text, area.reduced (2.0f), Justification::centredLeft,   false); break;
            case verticalKeyboardFacingRight:   g.drawText (text, area.reduced (2.0f), Justification::centredRight,  false); break;
            default: break;
        }
    }
    
    if (! lineColour.isTransparent())
    {
        g.setColour (lineColour);
        
        switch (orientation)
        {
            case horizontalKeyboard:            g.fillRect (area.withWidth (1.0f)); break;
            case verticalKeyboardFacingLeft:    g.fillRect (area.withHeight (1.0f)); break;
            case verticalKeyboardFacingRight:   g.fillRect (area.removeFromBottom (1.0f)); break;
            default: break;
        }
        
        if (midiNoteNumber == rangeEnd)
        {
            switch (orientation)
            {
                case horizontalKeyboard:            g.fillRect (area.expanded (1.0f, 0).removeFromRight (1.0f)); break;
                case verticalKeyboardFacingLeft:    g.fillRect (area.expanded (0, 1.0f).removeFromBottom (1.0f)); break;
                case verticalKeyboardFacingRight:   g.fillRect (area.expanded (0, 1.0f).removeFromTop (1.0f)); break;
                default: break;
            }
        }
    }
}

void BaseKeyboardComponent::drawBlackNote (int /*midiNoteNumber*/, Graphics& g, Rectangle<float> area,
                                           bool isDown, bool isOver, Colour noteFillColour)
{
    auto c = noteFillColour;
    
    if (isDown)  c = c.overlaidWith (findColour (keyDownOverlayColourId));
    if (isOver)  c = c.overlaidWith (findColour (mouseOverKeyOverlayColourId));
    
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
        
        switch (orientation)
        {
            case horizontalKeyboard:            g.fillRect (area.reduced (w * sideIndent, 0).removeFromTop   (h * topIndent)); break;
            case verticalKeyboardFacingLeft:    g.fillRect (area.reduced (0, h * sideIndent).removeFromRight (w * topIndent)); break;
            case verticalKeyboardFacingRight:   g.fillRect (area.reduced (0, h * sideIndent).removeFromLeft  (w * topIndent)); break;
            default: break;
        }
    }
}

void BaseKeyboardComponent::setOctaveForMiddleC (int octaveNum)
{
    octaveNumForMiddleC = octaveNum;
    repaint();
}

String BaseKeyboardComponent::getWhiteNoteText (int midiNoteNumber)
{
    if (midiNoteNumber % 12 == 0)
        return MidiMessage::getMidiNoteName (midiNoteNumber, true, true, octaveNumForMiddleC);
    
    return {};
}

void BaseKeyboardComponent::drawUpDownButton (Graphics& g, int w, int h,
                                              bool mouseOver,
                                              bool buttonDown,
                                              bool movesOctavesUp)
{
    g.fillAll (findColour (upDownButtonBackgroundColourId));
    
    float angle = 0;
    
    switch (orientation)
    {
        case horizontalKeyboard:            angle = movesOctavesUp ? 0.0f  : 0.5f;  break;
        case verticalKeyboardFacingLeft:    angle = movesOctavesUp ? 0.25f : 0.75f; break;
        case verticalKeyboardFacingRight:   angle = movesOctavesUp ? 0.75f : 0.25f; break;
        default:                            jassertfalse; break;
    }
    
    Path path;
    path.addTriangle (0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    path.applyTransform (AffineTransform::rotation (MathConstants<float>::twoPi * angle, 0.5f, 0.5f));
    
    g.setColour (findColour (upDownButtonArrowColourId)
                 .withAlpha (buttonDown ? 1.0f : (mouseOver ? 0.6f : 0.4f)));
    
    g.fillPath (path, path.getTransformToScaleToFit (1.0f, 1.0f, (float) w - 2.0f, (float) h - 2.0f, true));
}

void BaseKeyboardComponent::setBlackNoteLengthProportion (float ratio) noexcept
{
    jassert (ratio >= 0.0f && ratio <= 1.0f);
    
    if (blackNoteLengthRatio != ratio)
    {
        blackNoteLengthRatio = ratio;
        resized();
    }
}

float BaseKeyboardComponent::getBlackNoteLength() const noexcept
{
    auto whiteNoteLength = orientation == horizontalKeyboard ? getHeight() : getWidth();
    return (float) whiteNoteLength * blackNoteLengthRatio;
}

void BaseKeyboardComponent::setBlackNoteWidthProportion (float ratio) noexcept
{
    jassert (ratio >= 0.0f && ratio <= 1.0f);
    
    if (blackNoteWidthRatio != ratio)
    {
        blackNoteWidthRatio = ratio;
        resized();
    }
}

void BaseKeyboardComponent::resized()
{
    auto w = getWidth();
    auto h = getHeight();
    
    if (w > 0 && h > 0)
    {
        if (orientation != horizontalKeyboard)
            std::swap (w, h);
        
        auto kx2 = getKeyPos (rangeEnd).getEnd();
        
        if ((int) firstKey != rangeStart)
        {
            auto kx1 = getKeyPos (rangeStart).getStart();
            
            if (kx2 - kx1 <= (float) w)
            {
                firstKey = (float) rangeStart;
                sendChangeMessage();
                repaint();
            }
        }
        
        scrollDown->setVisible (canScroll && firstKey > (float) rangeStart);
        
        xOffset = 0;
        
        if (canScroll)
        {
            auto scrollButtonW = jmin (scrollButtonWidth, w / 2);
            auto r = getLocalBounds();
            
            if (orientation == horizontalKeyboard)
            {
                scrollDown->setBounds (r.removeFromLeft  (scrollButtonW));
                scrollUp  ->setBounds (r.removeFromRight (scrollButtonW));
            }
            else if (orientation == verticalKeyboardFacingLeft)
            {
                scrollDown->setBounds (r.removeFromTop    (scrollButtonW));
                scrollUp  ->setBounds (r.removeFromBottom (scrollButtonW));
            }
            else
            {
                scrollDown->setBounds (r.removeFromBottom (scrollButtonW));
                scrollUp  ->setBounds (r.removeFromTop    (scrollButtonW));
            }
            
            auto endOfLastKey = getKeyPos (rangeEnd).getEnd();
            
            float mousePositionVelocity;
            auto spaceAvailable = w;
            auto lastStartKey = remappedXYToNote ({ endOfLastKey - (float) spaceAvailable, 0 }, mousePositionVelocity) + 1;
            
            if (lastStartKey >= 0 && ((int) firstKey) > lastStartKey)
            {
                firstKey = (float) jlimit (rangeStart, rangeEnd, lastStartKey);
                sendChangeMessage();
            }
            
            xOffset = getKeyPos ((int) firstKey).getStart();
        }
        else
        {
            firstKey = (float) rangeStart;
        }
        
        scrollUp->setVisible (canScroll && getKeyPos (rangeEnd).getStart() > (float) w);
        repaint();
    }
}

//==============================================================================
void BaseKeyboardComponent::handleNoteOn (MidiKeyboardState*, int /*midiChannel*/, int /*midiNoteNumber*/, float /*velocity*/)
{
    shouldCheckState = true; // (probably being called from the audio thread, so avoid blocking in here)
}

void BaseKeyboardComponent::handleNoteOff (MidiKeyboardState*, int /*midiChannel*/, int /*midiNoteNumber*/, float /*velocity*/)
{
    shouldCheckState = true; // (probably being called from the audio thread, so avoid blocking in here)
}

//==============================================================================
void BaseKeyboardComponent::resetAnyKeysInUse()
{
    if (! keysPressed.isZero())
    {
        for (int i = 128; --i >= 0;)
            if (keysPressed[i])
                state.noteOff (midiChannel, i, 0.0f);
        
        keysPressed.clear();
    }
    
    for (int i = mouseDownNotes.size(); --i >= 0;)
    {
        auto noteDown = mouseDownNotes.getUnchecked(i);
        
        if (noteDown >= 0)
        {
            state.noteOff (midiChannel, noteDown, 0.0f);
            mouseDownNotes.set (i, -1);
        }
        
        mouseOverNotes.set (i, -1);
    }
}

void BaseKeyboardComponent::updateNoteUnderMouse (const MouseEvent& e, bool isDown)
{
    updateNoteUnderMouse (e.getEventRelativeTo (this).position, isDown, e.source.getIndex());
}

void BaseKeyboardComponent::updateNoteUnderMouse (Point<float> pos, bool isDown, int fingerNum)
{
    float mousePositionVelocity = 0.0f;
    auto newNote = xyToNote (pos, mousePositionVelocity);
    auto oldNote = mouseOverNotes.getUnchecked (fingerNum);
    auto oldNoteDown = mouseDownNotes.getUnchecked (fingerNum);
    auto eventVelocity = useMousePositionForVelocity ? mousePositionVelocity * velocity : velocity;
    
    if (oldNote != newNote)
    {
        repaintNote (oldNote);
        repaintNote (newNote);
        mouseOverNotes.set (fingerNum, newNote);
    }
    
    if (isDown)
    {
        if (newNote != oldNoteDown)
        {
            if (oldNoteDown >= 0)
            {
                mouseDownNotes.set (fingerNum, -1);
                
                if (! mouseDownNotes.contains (oldNoteDown))
                    state.noteOff (midiChannel, oldNoteDown, eventVelocity);
            }
            
            if (newNote >= 0 && ! mouseDownNotes.contains (newNote))
            {
                state.noteOn (midiChannel, newNote, eventVelocity);
                mouseDownNotes.set (fingerNum, newNote);
            }
        }
    }
    else if (oldNoteDown >= 0)
    {
        mouseDownNotes.set (fingerNum, -1);
        
        if (! mouseDownNotes.contains (oldNoteDown))
            state.noteOff (midiChannel, oldNoteDown, eventVelocity);
    }
}

void BaseKeyboardComponent::mouseMove (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);
}

void BaseKeyboardComponent::mouseDrag (const MouseEvent& e)
{
    float mousePositionVelocity;
    auto newNote = xyToNote (e.position, mousePositionVelocity);
    
    if (newNote >= 0 && mouseDraggedToKey (newNote, e))
        updateNoteUnderMouse (e, true);
}

bool BaseKeyboardComponent::mouseDownOnKey    (int, const MouseEvent&)  { return true; }
bool BaseKeyboardComponent::mouseDraggedToKey (int, const MouseEvent&)  { return true; }
void BaseKeyboardComponent::mouseUpOnKey      (int, const MouseEvent&)  {}

void BaseKeyboardComponent::mouseDown (const MouseEvent& e)
{
    float mousePositionVelocity;
    auto newNote = xyToNote (e.position, mousePositionVelocity);
    
    if (newNote >= 0 && mouseDownOnKey (newNote, e))
        updateNoteUnderMouse (e, true);
}

void BaseKeyboardComponent::mouseUp (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);
    
    float mousePositionVelocity;
    auto note = xyToNote (e.position, mousePositionVelocity);
    
    if (note >= 0)
        mouseUpOnKey (note, e);
}

void BaseKeyboardComponent::mouseEnter (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);
}

void BaseKeyboardComponent::mouseExit (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);
}

void BaseKeyboardComponent::mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel)
{
    auto amount = (orientation == horizontalKeyboard && wheel.deltaX != 0)
    ? wheel.deltaX : (orientation == verticalKeyboardFacingLeft ? wheel.deltaY
                      : -wheel.deltaY);
    
    setLowestVisibleKeyFloat (firstKey - amount * keyWidth);
}

void BaseKeyboardComponent::timerCallback()
{
    if (shouldCheckState)
    {
        shouldCheckState = false;
        
        for (int i = rangeStart; i <= rangeEnd; ++i)
        {
            bool isOn = state.isNoteOnForChannels (midiInChannelMask, i);
            
            // MN_JUCE_HACK: Undesirable functionality here blocking paint calls at appropriate timings
            // due to chord presses. Consider copying this entire class into project for custom use,
            // rather than relying on JUCE in this case
            //if (keysCurrentlyDrawnDown[i] != isOn)
            {
                keysCurrentlyDrawnDown.setBit (i, isOn);
                repaintNote (i);
            }
        }
    }
}

//==============================================================================
void BaseKeyboardComponent::clearKeyMappings()
{
    resetAnyKeysInUse();
    keyPressNotes.clear();
    keyPresses.clear();
}

void BaseKeyboardComponent::setKeyPressForNote (const KeyPress& key, int midiNoteOffsetFromC)
{
    removeKeyPressForNote (midiNoteOffsetFromC);
    
    keyPressNotes.add (midiNoteOffsetFromC);
    keyPresses.add (key);
}

void BaseKeyboardComponent::removeKeyPressForNote (int midiNoteOffsetFromC)
{
    for (int i = keyPressNotes.size(); --i >= 0;)
    {
        if (keyPressNotes.getUnchecked (i) == midiNoteOffsetFromC)
        {
            keyPressNotes.remove (i);
            keyPresses.remove (i);
        }
    }
}

void BaseKeyboardComponent::setKeyPressBaseOctave (int newOctaveNumber)
{
    jassert (newOctaveNumber >= 0 && newOctaveNumber <= 10);
    
    keyMappingOctave = newOctaveNumber;
}

bool BaseKeyboardComponent::keyStateChanged (bool /*isKeyDown*/)
{
    bool keyPressUsed = false;
    
    for (int i = keyPresses.size(); --i >= 0;)
    {
        auto note = 12 * keyMappingOctave + keyPressNotes.getUnchecked (i);
        
        if (keyPresses.getReference(i).isCurrentlyDown())
        {
            if (! keysPressed[note])
            {
                keysPressed.setBit (note);
                state.noteOn (midiChannel, note, velocity);
                keyPressUsed = true;
            }
        }
        else
        {
            if (keysPressed[note])
            {
                keysPressed.clearBit (note);
                state.noteOff (midiChannel, note, 0.0f);
                keyPressUsed = true;
            }
        }
    }
    
    return keyPressUsed;
}

bool BaseKeyboardComponent::keyPressed (const KeyPress& key)
{
    return keyPresses.contains (key);
}

void BaseKeyboardComponent::focusLost (FocusChangeType)
{
    resetAnyKeysInUse();
}
