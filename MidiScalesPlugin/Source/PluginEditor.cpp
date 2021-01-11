/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MidiScalesPluginAudioProcessorEditor::MidiScalesPluginAudioProcessorEditor (MidiScalesPluginAudioProcessor& p)
    : AudioProcessorEditor (&p),
      m_audioProcessor (p),
      m_keyboardComponent(p.m_keyboardState,  juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);
    
    addAndMakeVisible (m_selectedChord);
    m_selectedChord.setText ("C Major - Happy, Innocent", juce::dontSendNotification);
    m_selectedChord.setFont (juce::Font (20.0f, juce::Font::plain));
    m_selectedChord.setColour (juce::Label::backgroundColourId, juce::Colours::white);
    m_selectedChord.setColour (juce::Label::textColourId, juce::Colours::black);
    m_selectedChord.setJustificationType (juce::Justification::centred);
    
    addAndMakeVisible (m_keyboardComponent);
    m_keyboardComponent.setScrollButtonsVisible(false);
    m_keyboardComponent.setAvailableRange(SCALES_OCTAVE_NORMALIZED_START,
                                          SCALES_OCTAVE_NORMALIZED_START +
                                          SCALES_OCTAVE_STEPS_RANGE - 1);
    m_keyboardComponent.setKeyWidth(46);
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
    const int iKeyboardTopSpacing = 30;
    const int iKeyboardHeight = 200;
    const int iEndKeyExcess = -9;
    
    m_selectedChord.setBounds (iLabelLeftRightSpacing, iLabelTopSpacing,
                               (getWidth() - 2*iLabelLeftRightSpacing) + iEndKeyExcess,  iLabelHeight);
    
    m_keyboardComponent.setBounds (iLabelLeftRightSpacing, iLabelTopSpacing + iLabelHeight + iKeyboardTopSpacing,
                                 (getWidth() - 2*iLabelLeftRightSpacing) + iEndKeyExcess, iKeyboardHeight);
}
