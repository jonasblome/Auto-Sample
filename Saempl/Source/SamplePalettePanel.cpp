/*
 ==============================================================================
 
 SamplePalettePanel.cpp
 Author:  Jonas Blome
 
 ==============================================================================
 */

#include "SamplePalettePanel.h"

SamplePalettePanel::SamplePalettePanel(SaemplAudioProcessor& inProcessor, SampleItemPanel& inSampleItemPanel)
:
PanelBase(inProcessor),
sampleLibrary(currentProcessor.getSampleLibrary()),
sampleItemPanel(inSampleItemPanel)
{
    setSize(style->SAMPLE_PALETTE_PANEL_WIDTH, style->SAMPLE_PALETTE_PANEL_HEIGHT);
    setPanelComponents();
}

SamplePalettePanel::~SamplePalettePanel()
{
    
}

void SamplePalettePanel::paint(Graphics& g)
{
    // Draw background
    g.setColour(style->COLOUR_ACCENT_MEDIUM);
    g.fillRoundedRectangle(getLocalBounds()
                           .toFloat()
                           .withTrimmedLeft(style->PANEL_MARGIN * 0.25)
                           .withTrimmedTop(style->PANEL_MARGIN * 0.25)
                           .withTrimmedRight(style->PANEL_MARGIN * 0.5)
                           .withTrimmedBottom(style->PANEL_MARGIN * 0.5),
                           style->CORNER_SIZE_MEDIUM);
    
    // Draw Title
    drawDropShadow(g,
                   Rectangle<int>(0, 0, getWidth(), 20),
                   0,
                   0,
                   70,
                   style);
    g.setColour(style->COLOUR_ACCENT_DARK);
    g.fillRoundedRectangle(getLocalBounds()
                           .removeFromTop(style->PANEL_TITLE_HEIGHT)
                           .withTrimmedLeft(style->PANEL_MARGIN * 0.75)
                           .withTrimmedTop(style->PANEL_MARGIN * 0.75)
                           .withTrimmedRight(style->PANEL_MARGIN)
                           .withTrimmedBottom(style->PANEL_MARGIN * 0.25)
                           .toFloat(),
                           style->CORNER_SIZE_MEDIUM);
    g.setFont(style->FONT_MEDIUM_SMALL_BOLD);
    g.setColour(style->COLOUR_PANEL_TITLE_FONT);
    g.drawFittedText("Favorites",
                     0,
                     0,
                     getWidth(),
                     style->PANEL_TITLE_HEIGHT,
                     Justification::centred,
                     1);
    
    mSampleTable->updateContent();
}

void SamplePalettePanel::setPanelComponents()
{
    // Set sample table component
    mSampleTable = std::make_unique<BlomeTableViewPalette>(currentProcessor, sampleItemPanel);
    addAndMakeVisible(*mSampleTable);
    
    resizePanelComponents();
}

void SamplePalettePanel::resizePanelComponents()
{
    if (mSampleTable != nullptr)
    {
        mSampleTable->setBounds(style->PANEL_MARGIN * 0.75,
                                style->PANEL_TITLE_HEIGHT + style->PANEL_MARGIN * 0.25,
                                getWidth() - style->PANEL_MARGIN * 1.75,
                                getHeight() - style->PANEL_TITLE_HEIGHT - style->PANEL_MARGIN * 1.25);
    }
}
