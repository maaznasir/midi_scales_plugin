/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "Utilities.h"
#include "PressedChord.h"

//==============================================================================
/**
*/

class MidiScalesPluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    MidiScalesPluginAudioProcessor();
    ~MidiScalesPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void SetScaleSafe(int iScaleNote, Scales::Type::eType scaleType);
    bool IsNoteInScaleSafe(int iMidiNote);
    
    juce::MidiKeyboardState m_keyboardState;
    juce::Atomic<Chords::Type::eType> m_ChordType;

private:
    juce::CriticalSection m_ScalesLock;
    int m_iScaleNote;
    Scales::Type::eType m_ScaleType;
    PressedChord m_currentChord;
    ScaleNotes m_ScaleNotes;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiScalesPluginAudioProcessor)
};
