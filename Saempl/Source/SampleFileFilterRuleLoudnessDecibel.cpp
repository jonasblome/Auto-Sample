/*
  ==============================================================================

    SampleFileFilterRuleLoudness.cpp
    Author:  Jonas Blome

  ==============================================================================
*/

#include "SampleFileFilterRuleLoudnessDecibel.h"

SampleFileFilterRuleLoudnessDecibel::SampleFileFilterRuleLoudnessDecibel(String inRulePropertyName)
:
SampleFileFilterRuleBase(inRulePropertyName)
{
    mCompareValue = 0.0;
    mCompareOperator = GREATER_THAN;
}

SampleFileFilterRuleLoudnessDecibel::~SampleFileFilterRuleLoudnessDecibel()
{
    
}

bool SampleFileFilterRuleLoudnessDecibel::matches(SampleItem const & inSampleItem)
{
    int propertyValue = inSampleItem.getLoudnessDecibel();
    
    switch (mCompareOperator) {
        case LESS_THAN:
        {
            return propertyValue < mCompareValue;
            break;
        }
        case EQUAL_TO:
        {
            return propertyValue == mCompareValue;
            break;
        }
        case GREATER_THAN:
        {
            return propertyValue > mCompareValue;
            break;
        }
        case CONTAINS:
        {
            return false;
            break;
        }
        default:
            jassertfalse;
            return false;
    };
}

int SampleFileFilterRuleLoudnessDecibel::getCompareValue()
{
    return mCompareValue;
}

void SampleFileFilterRuleLoudnessDecibel::setCompareValue(int const & inCompareValue)
{
    mCompareValue = inCompareValue;
}
