/*
  ==============================================================================

    SampleEditor.h
    Created: 21 May 2023 3:00:50pm
    Author:  Jonas Blome

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "AudioPlayer.h"

class SampleEditor
{
public:
    // Constructors
    SampleEditor();
    ~SampleEditor();
    
    // Methods
    AudioFormatManager& getAudioFormatManager();
    void startOrStopAudio();
    void setAudioReadheadPosition(double position);
    void startAudio();
    bool isPlaying();
    double getCurrentReadheadPosition();
    void stopAudio();
    bool loadURLIntoTransport(URL const & audioURL, TimeSliceThread& inThread);
    
private:
    JUCE_HEAVYWEIGHT_LEAK_DETECTOR(SampleEditor)
    
    // Fields
    std::unique_ptr<AudioPlayer> mAudioPlayer;
    
    // Methods
    
};
