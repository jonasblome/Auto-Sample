/*
 ==============================================================================
 
 BlomeFileFilterRuleViewBase.cpp
 Author:  Jonas Blome
 
 ==============================================================================
 */

#include "BlomeFileFilterRuleViewBase.h"

BlomeFileFilterRuleViewBase::BlomeFileFilterRuleViewBase(SampleFileFilterRuleBase& inFilterRule,
                                                         SampleLibrary& inSampleLibrary)
:
sampleLibrary(inSampleLibrary),
filterRule(inFilterRule)
{
    setComponents();
}

BlomeFileFilterRuleViewBase::~BlomeFileFilterRuleViewBase()
{
    mCompareOperatorChooser->removeListener(this);
}

void BlomeFileFilterRuleViewBase::paint(Graphics& g)
{
    // Paint rule title
    Rectangle<int> area = getLocalBounds()
        .removeFromLeft(style->FILTER_RULE_TITLE_WIDTH + style->BUTTON_SIZE_SMALL)
        .removeFromRight(style->FILTER_RULE_TITLE_WIDTH);
    area.reduce(style->PANEL_MARGIN / 2.0, 0);
    g.setFont(style->FONT_SMALL_BOLD);
    g.setColour(style->COLOUR_ACCENT_LIGHT);
    g.drawFittedText(getFilterRule().getRulePropertyName(),
                     area,
                     Justification::right,
                     1);
}

void BlomeFileFilterRuleViewBase::setComponents()
{
    // Add button to activate filter rule
    mActivateRuleButton = std::make_unique<ToggleButton>("ActivateRuleButton");
    mActivateRuleButton->setToggleState(filterRule.getIsActive(), NotificationType::dontSendNotification);
    mActivateRuleButton->setTooltip("Activate/deactivate this filter rule");
    mActivateRuleButton->onClick = [this]
    {
        bool couldHaveEffect = sampleLibrary.getFileFilter().canHaveEffect();
        filterRule.setIsActive(mActivateRuleButton->getToggleState());
        
        if (sampleLibrary.getFileFilter().canHaveEffect() || couldHaveEffect)
        {
            sampleLibrary.refreshLibrary();
        }
    };
    addAndMakeVisible(*mActivateRuleButton);
    
    // Add combo box for compare operator
    mCompareOperatorChooser = std::make_unique<ComboBox>("CompareOperatorChooser");
    mCompareOperatorChooser->addListener(this);
    addAndMakeVisible(*mCompareOperatorChooser);
    
    // Add button for deleting filter rule
    mDeleteRuleButton = std::make_unique<BlomeImageButton>("Delete", true);
    mDeleteRuleButton->setImages(false,
                                 true,
                                 true,
                                 ImageCache::getFromMemory(BinaryData::delete_FILL0_wght400_GRAD0_opsz24_png,
                                                           BinaryData::delete_FILL0_wght400_GRAD0_opsz24_pngSize),
                                 style->BUTTON_IS_DEFAULT_ALPHA,
                                 style->COLOUR_SYMBOL_BUTTON,
                                 Image(),
                                 style->BUTTON_IS_OVER_ALPHA,
                                 style->COLOUR_SYMBOL_BUTTON,
                                 Image(),
                                 style->BUTTON_IS_DOWN_ALPHA,
                                 style->COLOUR_SYMBOL_BUTTON);
    mDeleteRuleButton->setTooltip("Delete this filter rule");
    mDeleteRuleButton->onClick = [this]
    {
        
    };
    addAndMakeVisible(*mDeleteRuleButton);
}

void BlomeFileFilterRuleViewBase::resized()
{
    mActivateRuleButton->setBounds(style->FILTER_RULE_HEIGHT / 2.0
                                   - style->BUTTON_SIZE_SMALL / 2.0
                                   + style->PANEL_MARGIN / 2.0,
                                   style->FILTER_RULE_HEIGHT / 2.0
                                   - style->BUTTON_SIZE_SMALL / 2.0
                                   + style->PANEL_MARGIN / 4.0,
                                   style->BUTTON_SIZE_SMALL - style->PANEL_MARGIN,
                                   style->BUTTON_SIZE_SMALL - style->PANEL_MARGIN);
    
    mCompareOperatorChooser->setBounds(style->BUTTON_SIZE_SMALL
                                       + style->FILTER_RULE_TITLE_WIDTH,
                                       0,
                                       style->COMBO_BOX_WIDTH_MEDIUM
                                       - style->PANEL_MARGIN / 4.0,
                                       getHeight());
    
    mDeleteRuleButton->setBounds(getWidth() - getHeight(),
                                 0,
                                 getHeight(),
                                 getHeight());
}

SampleFileFilterRuleBase& BlomeFileFilterRuleViewBase::getFilterRule()
{
    return filterRule;
}

void BlomeFileFilterRuleViewBase::addDeleteButtonListener(Button::Listener* inListener)
{
    mDeleteRuleButton->addListener(inListener);
}

void BlomeFileFilterRuleViewBase::removeDeleteButtonListener(Button::Listener* inListener)
{
    mDeleteRuleButton->removeListener(inListener);
}
