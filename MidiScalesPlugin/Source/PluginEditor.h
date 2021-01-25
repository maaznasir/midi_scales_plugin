/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include "Utilities.h"

#include "ScalesKeyboardComponent.h"

//==============================================================================
/**
*/
class MidiScalesPluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    MidiScalesPluginAudioProcessorEditor (MidiScalesPluginAudioProcessor&);
    ~MidiScalesPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void ScaleNoteComboChanged();
    void ScaleTypeComboChanged();
    void ChordTypeComboChanged();

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MidiScalesPluginAudioProcessor& m_audioProcessor;
    
    ScalesKeyboardComponent m_keyboardComponent;
    juce::Label m_selectedChord;
    juce::Label m_ChordLabel;
    juce::Label m_ScaleLabel;
    juce::ComboBox m_ChordType;
    juce::ComboBox m_ScaleType;
    juce::ComboBox m_ScaleNote;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiScalesPluginAudioProcessorEditor)
};
