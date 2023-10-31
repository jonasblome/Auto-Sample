/*
  ==============================================================================

    SampleFileFilterRuleLength.h
    Created: 28 Oct 2023 5:24:42pm
    Author:  Jonas Blome

  ==============================================================================
*/

#pragma once

#include "SampleFileFilterRuleBase.h"

class SampleFileFilterRuleLength
:   public SampleFileFilterRuleBase
{
public:
    // Constructors
    SampleFileFilterRuleLength(String inRuleType);
    ~SampleFileFilterRuleLength();
    
    // Methods
    bool matches(SampleItem const & inSampleItem) override;
    double getCompareValue();
    void setCompareValue(double const & inCompareValue);
    
private:
    JUCE_HEAVYWEIGHT_LEAK_DETECTOR(SampleFileFilterRuleLength)
    
    // Fields
    double mCompareValue;
    
    // Methods
    
};