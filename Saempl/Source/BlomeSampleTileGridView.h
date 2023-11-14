/*
 ==============================================================================
 
 BlomeSampleTileGridView.h
 Created: 14 Nov 2023 1:29:30pm
 Author:  Jonas Blome
 
 ==============================================================================
 */

#pragma once

#include "JuceHeader.h"
#include "BlomeStyleDefinitions.h"
#include "SampleLibrary.h"
#include "BlomeSampleItemTileView.h"

class BlomeSampleTileGridView
:
public Component,
public ChangeListener,
public FileDragAndDropTarget
{
public:
    BlomeSampleTileGridView(SampleLibrary& inSampleLibrary, SampleItemPanel& inSampleItemPanel);
    ~BlomeSampleTileGridView();
    
private:
    SampleLibrary& sampleLibrary;
    BlomeStyling::Ptr style;
    std::unique_ptr<Grid> mSampleTileGrid;
    SampleItemPanel& linkedSampleItemPanel;
    OwnedArray<BlomeSampleItemTileView> mSampleItemTiles;
    
    void paint(Graphics& g) override;
    void changeListenerCallback(ChangeBroadcaster* source) override;
    /**
     Creates tile components for each sample item and adds them to a square grid.
     */
    void setupGrid();
    void filesDropped(StringArray const & files, int x, int y) override;
    bool isInterestedInFileDrag(StringArray const & files) override;
};
