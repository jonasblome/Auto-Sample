/*
  ==============================================================================

    BlomeHelperFunctions.h
    Created: 22 May 2023 10:47:09am
    Author:  Jonas Blome

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

#if JUCE_WINDOWS
    static const juce::String DIRECTORY_SEPARATOR = "\\";
#elif JUCE_MAC
    static const juce::String DIRECTORY_SEPARATOR = "/";
#endif

#define SAMPLE_FILE_EXTENSION ".bsf"

const StringArray supportedAudioFileFormat = StringArray({ ".mp3", ".wav", ".aiff", ".m4a" });

inline bool isSupportedAudioFileFormat(String inExtension)
{
    for (String extension : supportedAudioFileFormat)
    {
        if (extension == inExtension) {
            return true;
        }
    }
    
    return false;
}

inline String restoreSpacesFromURLString(String urlString)
{
    return urlString.replace("%20", " ");
}
