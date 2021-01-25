/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Utilities.h"

//==============================================================================
MidiScalesPluginAudioProcessorEditor::MidiScalesPluginAudioProcessorEditor (MidiScalesPluginAudioProcessor& p)
    : AudioProcessorEditor (&p),
      m_audioProcessor (p),
      m_keyboardComponent(p.m_keyboardState,  juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);
    
    /*addAndMakeVisible (m_selectedChord);
    m_selectedChord.setText ("C Major - Happy, Innocent", juce::dontSendNotification);
    m_selectedChord.setFont (juce::Font (16.0f, juce::Font::plain));
    m_selectedChord.setColour (juce::Label::backgroundColourId, juce::Colours::white);
    m_selectedChord.setColour (juce::Label::textColourId, juce::Colours::black);
    m_selectedChord.setJustificationType (juce::Justification::centred);*/
    
    addAndMakeVisible (m_ScaleLabel);
    m_ScaleLabel.setText ("Scale: ", juce::dontSendNotification);
    m_ScaleLabel.setFont (juce::Font (16.0f, juce::Font::plain));
    m_ScaleLabel.setColour (juce::Label::backgroundColourId, juce::Colours::white);
    m_ScaleLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    
    addAndMakeVisible (m_ChordLabel);
    m_ChordLabel.setText ("Chord: ", juce::dontSendNotification);
    m_ChordLabel.setFont (juce::Font (16.0f, juce::Font::plain));
    m_ChordLabel.setColour (juce::Label::backgroundColourId, juce::Colours::white);
    m_ChordLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    
    addAndMakeVisible (m_keyboardComponent);
    m_keyboardComponent.setScrollButtonsVisible(false);
    m_keyboardComponent.setAvailableRange(SCALES_OCTAVE_NORMALIZED_START,
                                          SCALES_OCTAVE_NORMALIZED_START +
                                          SCALES_OCTAVE_STEPS_RANGE - 1);
    m_keyboardComponent.setKeyWidth(46);
    
    
    addAndMakeVisible (m_ChordType);
    for(int i = 1; i <= Chords::Type::Total; i++)
    {
        Chords::Type::eType chordType = (Chords::Type::eType) i;
        m_ChordType.addItem (Helpers::GetChordTypeString(chordType),  chordType);
    }
    m_ChordType.setSelectedId(Chords::Type::MajorTriad);
    m_ChordType.onChange = [this] { ChordTypeComboChanged(); };
    m_ChordType.onChange();
    
    addAndMakeVisible (m_ScaleType);
    for(int i = 1; i <= Scales::Type::Total; i++)
    {
        Scales::Type::eType scaleType = (Scales::Type::eType) i;
        m_ScaleType.addItem (Helpers::GetScaleTypeString(scaleType),  scaleType);
    }
    m_ScaleType.setSelectedId(Scales::Type::Major);
    m_ScaleType.onChange = [this] { ScaleTypeComboChanged(); };
    m_ScaleType.onChange();
    
    addAndMakeVisible (m_ScaleNote);
    for(int i = 1; i <= SCALES_OCTAVE_STEPS; i++)
    {
        m_ScaleNote.addItem (juce::MidiMessage::getMidiNoteName (i-1, true, false, m_keyboardComponent.getOctaveForMiddleC()),  i);
    }
    m_ScaleNote.setSelectedId(1);
    m_ScaleNote.onChange = [this] { ScaleNoteComboChanged(); };
    m_ScaleNote.onChange();
    
    m_ToggleLookAndFeel.setColour(juce::ToggleButton::textColourId, juce::Colours::black);
    m_ToggleLookAndFeel.setColour(juce::ToggleButton::tickColourId, juce::Colours::black);
    m_ToggleLookAndFeel.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::black);
    
    addAndMakeVisible(m_ToggleSharps);
    
    m_ToggleSharps.setToggleState(true, juce::dontSendNotification);
    m_ToggleSharps.setLookAndFeel(&m_ToggleLookAndFeel);
    m_ToggleSharps.onClick = [this] { SharpsToggleClicked(); };
}

MidiScalesPluginAudioProcessorEditor::~MidiScalesPluginAudioProcessorEditor()
{
}

//==============================================================================
void MidiScalesPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::lightgrey);

    //g.setColour (juce::Colours::white);
    //g.setFont (15.0f);
    //g.drawFittedText ("Let the Chord Progressions begin!", getLocalBounds(), juce::Justification::centred, 1);
}

void MidiScalesPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    const int iLabelTopSpacing = 20;
    const int iLabelLeftRightSpacing = 50;
    const int iLabelHeight = 50;
    const int iKeyboardTopSpacing = 10;
    const int iKeyboardHeight = 200;
    const int iEndKeyExcess = -9;
    
    const int iScaleNoteWidth = 90;
    const int iScaleChordLabelWidth = 90;
    const int iScaleTypeWidth = 200;
    //Unused due to last item in width requiring full alignment
    //const int iChordTypeWidth = 200;
    
    const int iEffectiveWidth = (getWidth() - 2*iLabelLeftRightSpacing) + iEndKeyExcess;
    
    int iCurrentLeftSpacing = iLabelLeftRightSpacing;
    m_ScaleLabel.setBounds(iCurrentLeftSpacing, iLabelTopSpacing, iScaleChordLabelWidth, iLabelHeight);
    
    iCurrentLeftSpacing += iScaleChordLabelWidth;
    m_ScaleNote.setBounds(iCurrentLeftSpacing, iLabelTopSpacing, iScaleNoteWidth, iLabelHeight);
    
    iCurrentLeftSpacing += iScaleNoteWidth;
    m_ScaleType.setBounds(iCurrentLeftSpacing, iLabelTopSpacing, iScaleTypeWidth, iLabelHeight);
    
    iCurrentLeftSpacing += iScaleTypeWidth;
    m_ChordLabel.setBounds(iCurrentLeftSpacing, iLabelTopSpacing, iScaleChordLabelWidth, iLabelHeight);
    
    iCurrentLeftSpacing += iScaleChordLabelWidth;
    m_ChordType.setBounds(iCurrentLeftSpacing, iLabelTopSpacing, iEffectiveWidth - (iCurrentLeftSpacing - iLabelLeftRightSpacing), iLabelHeight);
    
    const int iCheckboxHeight = 25;
    iCurrentLeftSpacing = iLabelLeftRightSpacing;
    int iCurrentVerticleSpacing = iKeyboardTopSpacing + iLabelTopSpacing + iLabelHeight;
    
    m_ToggleSharps.setBounds(iCurrentLeftSpacing, iCurrentVerticleSpacing, 200, iCheckboxHeight);
    
    /*m_selectedChord.setBounds (iLabelLeftRightSpacing, iLabelTopSpacing,
                               iEffectiveWidth,  iLabelHeight);*/
    
    iCurrentVerticleSpacing += iCheckboxHeight + iKeyboardTopSpacing;
    m_keyboardComponent.setBounds (iCurrentLeftSpacing, iCurrentVerticleSpacing,
                                  iEffectiveWidth, iKeyboardHeight);
}

void MidiScalesPluginAudioProcessorEditor::ScaleNoteComboChanged()
{
    int iSelectedId = m_ScaleNote.getSelectedId();
    int scaleNote = iSelectedId > 0 ? iSelectedId - 1 : -1;
    m_audioProcessor.m_iScaleNote.set(scaleNote);
    
    SetKeyboardScale();
}

void MidiScalesPluginAudioProcessorEditor::ScaleTypeComboChanged()
{
    Scales::Type::eType selectedType = (Scales::Type::eType) m_ScaleType.getSelectedId();
    m_audioProcessor.m_ScaleType.set(selectedType);
    
    SetKeyboardScale();
}

void MidiScalesPluginAudioProcessorEditor::ChordTypeComboChanged()
{
    Chords::Type::eType selectedType = (Chords::Type::eType) m_ChordType.getSelectedId();
    m_audioProcessor.m_ChordType.set(selectedType);
}

void MidiScalesPluginAudioProcessorEditor::SetKeyboardScale()
{
    int iSelectedId = m_ScaleNote.getSelectedId();
    int scaleNote = iSelectedId > 0 ? iSelectedId - 1 : -1;
    juce::String selectedText = m_ScaleNote.getItemText(m_ScaleNote.getSelectedItemIndex());
    selectedText = selectedText.replace("#", "").replace("b", "");
    int iBaseNote = Helpers::GetNoteType(selectedText.toRawUTF8());
    
    m_keyboardComponent.SetScale(scaleNote, iBaseNote, (Scales::Type::eType) m_ScaleType.getSelectedId());
}

void MidiScalesPluginAudioProcessorEditor::SharpsToggleClicked()
{
    int iPreviousSelectedId = m_ScaleNote.getSelectedItemIndex();
    for(int i = 1; i <= SCALES_OCTAVE_STEPS; i++)
    {
        m_ScaleNote.changeItemText (i, juce::MidiMessage::getMidiNoteName (i-1, m_ToggleSharps.getToggleState(), false,         m_keyboardComponent.getOctaveForMiddleC()));
    }
    m_ScaleNote.setSelectedItemIndex(iPreviousSelectedId);
}
