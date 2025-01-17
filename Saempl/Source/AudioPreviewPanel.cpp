/*
 ==============================================================================
 
 AudioPreviewPanel.cpp
 Author:  Jonas Blome
 
 Some of the code in this class is adapted from the JUCE DemoRunner Demo
 that can be found under DemoRunner->Audio->AudioPlaybackDemo.h
 
 ==============================================================================
 */

#include "AudioPreviewPanel.h"
#include "BlomeFileTreeView.h"

AudioPreviewPanel::AudioPreviewPanel(SaemplAudioProcessor& inProcessor,
                                     Slider& inSlider,
                                     AudioPlayer& inAudioPlayer)
:
PanelBase(inProcessor),
audioPlayer(inAudioPlayer),
zoomSlider(inSlider),
mThumbnailCache(std::make_unique<AudioThumbnailCache>(5)),
mAudioPreview(std::make_unique<AudioThumbnail>(512, audioPlayer.getFormatManager(), *mThumbnailCache)),
isFollowingTransport(false)
{
    setSize(style->SAMPLE_PREVIEW_WIDTH, style->SAMPLE_PREVIEW_HEIGHT);
    setPanelComponents();
}

AudioPreviewPanel::~AudioPreviewPanel()
{
    mAudioPreviewScrollbar->removeListener(this);
    stopTimer();
}

void AudioPreviewPanel::paint(Graphics& g)
{
    auto previewArea = getLocalBounds().removeFromBottom(getHeight() - style->SAMPLE_PREVIEW_TITLE_HEIGHT);
    
    // Draw background
    g.setColour(style->COLOUR_ACCENT_DARK);
    g.fillRoundedRectangle(getLocalBounds()
                           .toFloat()
                           .withTrimmedTop(style->PANEL_MARGIN * 0.25)
                           .withTrimmedBottom(style->PANEL_MARGIN),
                           style->CORNER_SIZE_MEDIUM);
    drawDropShadow(g,
                   Rectangle<int>(40, getHeight() * 0.25, getWidth() - 80, 1),
                   0,
                   0,
                   getHeight() * 0.5,
                   style);
    
    // Draw audio preview
    previewArea = previewArea.withTrimmedBottom(mAudioPreviewScrollbar->getHeight() + 4 + style->PANEL_MARGIN);
    g.setColour(style->COLOUR_ACCENT_LIGHT);
    
    if (mAudioPreview->getTotalLength() > 0.0)
    {
        // Draw file name on title bar
        g.setFont(style->FONT_SMALL_BOLD);
        String fileName = URL::removeEscapeChars(lastFileDropped.getFileName());
        g.drawFittedText(fileName,
                         getLocalBounds()
                         .removeFromRight(getWidth() - style->PANEL_MARGIN)
                         .removeFromTop(style->SAMPLE_PREVIEW_TITLE_HEIGHT),
                         Justification::centredLeft,
                         2);
        
        // Draw audio preview
        mAudioPreview->drawChannels(g,
                                    previewArea,
                                    visibleRange.getStart(),
                                    visibleRange.getEnd(),
                                    1.0f);
    }
    else
    {
        // Draw hint to drag in audio file
        g.setFont(style->FONT_SMALL_BOLD);
        g.drawFittedText("No audio file selected",
                         getLocalBounds()
                         .removeFromRight(getWidth() - style->PANEL_MARGIN)
                         .removeFromTop(style->SAMPLE_PREVIEW_TITLE_HEIGHT),
                         Justification::centredLeft,
                         1);
        g.setColour(style->COLOUR_PANEL_TITLE_FONT);
        g.setFont(style->FONT_MEDIUM_BOLD);
        g.drawFittedText("No audio file selected",
                         getLocalBounds(),
                         Justification::centred,
                         2);
    }
}

void AudioPreviewPanel::setPanelComponents()
{
    // Add scrollbar for audio thumbnail
    mAudioPreviewScrollbar = std::make_unique<ScrollBar>(false);
    mAudioPreviewScrollbar->setRangeLimits(visibleRange);
    mAudioPreviewScrollbar->setAutoHide(false);
    mAudioPreviewScrollbar->addListener(this);
    addAndMakeVisible(*mAudioPreviewScrollbar);
    
    // Add position marker
    mAudioPositionMarker = std::make_unique<DrawableRectangle>();
    mAudioPositionMarker->setFill(style->COLOUR_ACCENT_MEDIUM);
    addAndMakeVisible(*mAudioPositionMarker);
    
    resizePanelComponents();
}

void AudioPreviewPanel::setURL(URL const & url)
{
    if (auto inputSource = std::make_unique<URLInputSource>(url))
    {
        mAudioPreview->setSource(inputSource.release());
        
        Range<double> newRange(0.0, mAudioPreview->getTotalLength());
        mAudioPreviewScrollbar->setRangeLimits(newRange);
        setRange(newRange);
        
        startTimerHz(40);
    }
}

URL AudioPreviewPanel::getLastDroppedFile() const noexcept
{
    return lastFileDropped;
}

void AudioPreviewPanel::setZoomFactor(double amount)
{
    // Set preview and scrollbar zoom
    if (mAudioPreview->getTotalLength() > 0)
    {
        auto newScale = jmax(0.001, mAudioPreview->getTotalLength() * (1.0 - jlimit(0.0, 0.99, amount)));
        auto timeAtCentre = xToTime((float) getWidth() / 2.0f);
        
        setRange({ timeAtCentre - newScale * 0.5, timeAtCentre + newScale * 0.5 });
    }
}

void AudioPreviewPanel::setRange(Range<double> newRange)
{
    // Set range for audio preview and scrollbar
    visibleRange = newRange;
    mAudioPreviewScrollbar->setCurrentRange(visibleRange);
    updateCursorPosition();
    repaint();
}

bool AudioPreviewPanel::getFollowsTransport()
{
    return isFollowingTransport;
}

void AudioPreviewPanel::setFollowsTransport(bool shouldFollow)
{
    isFollowingTransport = shouldFollow;
}

bool AudioPreviewPanel::isInterestedInFileDrag(StringArray const & files)
{
    return true;
}

void AudioPreviewPanel::filesDropped(StringArray const & files, int x, int y)
{
    File file = File(files[0]);
    
    if (!file.isDirectory() && isSupportedAudioFileFormat(file.getFileExtension()))
    {
        lastFileDropped = URL(file);
        sendChangeMessage();
    }
}

void AudioPreviewPanel::mouseDown(MouseEvent const & e)
{
    mouseDrag(e);
}

void AudioPreviewPanel::mouseDrag(MouseEvent const & e)
{
    if (canMoveTransport())
    {
        audioPlayer.setTransportSourcePosition(jmax(0.0, xToTime((float)e.x)));
    }
}

void AudioPreviewPanel::mouseUp(MouseEvent const & mouseEvent)
{
    // Show sample options pop up menu
    if (mouseEvent.mods.isRightButtonDown())
    {
        PopupMenu popupMenu;
        popupMenu.addItem("Show in Finder", [this] { showSampleInFinder(); });
        popupMenu.showMenuAsync(PopupMenu::Options{}.withMousePosition());
    }
    else
    {
        audioPlayer.start();
    }
}

void AudioPreviewPanel::mouseWheelMove(MouseEvent const &, MouseWheelDetails const & wheel)
{
    // Set zoom of audio preview if a file is loaded
    if (mAudioPreview->getTotalLength() > 0.0)
    {
        auto newStart = visibleRange.getStart() - wheel.deltaX * (visibleRange.getLength()) / 10.0;
        newStart = jlimit(0.0, jmax(0.0, mAudioPreview->getTotalLength() - (visibleRange.getLength())), newStart);
        
        if (canMoveTransport())
        {
            setRange({ newStart, newStart + visibleRange.getLength() });
        }
        
        if (wheel.deltaY != 0.0f)
        {
            zoomSlider.setValue(zoomSlider.getValue() - wheel.deltaY * 0.06);
        }
        
        repaint();
    }
}

float AudioPreviewPanel::timeToX(double const time) const
{
    if (visibleRange.getLength() <= 0)
    {
        return 0;
    }
    
    return (float) getWidth() * (float) ((time - visibleRange.getStart()) / visibleRange.getLength());
}

double AudioPreviewPanel::xToTime(float const x) const
{
    return (x / (float) getWidth()) * (visibleRange.getLength()) + visibleRange.getStart();
}

bool AudioPreviewPanel::canMoveTransport() const noexcept
{
    return !(isFollowingTransport && audioPlayer.isPlaying());
}

void AudioPreviewPanel::scrollBarMoved(ScrollBar* scrollbar, double newRangeStart)
{
    if (scrollbar == &*mAudioPreviewScrollbar)
    {
        if (!(isFollowingTransport && audioPlayer.isPlaying()))
        {
            setRange(visibleRange.movedToStartAt(newRangeStart));
        }
    }
}

void AudioPreviewPanel::timerCallback()
{
    // Update cursor position or audio preview range
    if (canMoveTransport())
    {
        updateCursorPosition();
    }
    else
    {
        setRange(visibleRange.movedToStartAt(audioPlayer.getCurrentPosition() - (visibleRange.getLength() / 2.0)));
    }
}

void AudioPreviewPanel::updateCursorPosition()
{
    mAudioPositionMarker->setVisible(audioPlayer.isPlaying() || isMouseButtonDown());
    mAudioPositionMarker->setRectangle(Rectangle<float>(timeToX(audioPlayer.getCurrentPosition())
                                                        - 0.75f,
                                                        style->SAMPLE_PREVIEW_TITLE_HEIGHT,
                                                        1.5f,
                                                        (float) (getHeight()
                                                                 - style->PANEL_MARGIN * 1.5
                                                                 - style->SAMPLE_PREVIEW_TITLE_HEIGHT
                                                                 - mAudioPreviewScrollbar->getHeight())));
}

void AudioPreviewPanel::showAudioResource()
{
    URL resource = getLastDroppedFile();
    showAudioResource(resource);
}

/**
 Shows the given resource in the audio preview
 */
void AudioPreviewPanel::showAudioResource(URL inResource)
{
    // Load resource into preview and player
    lastFileDropped = inResource;
    
    if (loadURLIntoTransport(inResource))
    {
        mCurrentAudioFile = std::move(inResource);
    }
    
    zoomSlider.setValue(0, dontSendNotification);
    setURL(mCurrentAudioFile);
}

void AudioPreviewPanel::startOrStop()
{
    audioPlayer.startOrStop();
}

bool AudioPreviewPanel::loadURLIntoTransport(URL const & audioURL)
{
    return audioPlayer.loadURLIntoTransport(audioURL);
}

void AudioPreviewPanel::emptyAudioResource()
{
    audioPlayer.emptyTransport();
    lastFileDropped = URL();
    zoomSlider.setValue(0, dontSendNotification);
    mAudioPreview->clear();
    Range<double> newRange;
    setRange(newRange);
    mAudioPreviewScrollbar->setRangeLimits(newRange);
}

void AudioPreviewPanel::resizePanelComponents()
{
    if (mAudioPreviewScrollbar != nullptr)
    {
        mAudioPreviewScrollbar->setBounds(getLocalBounds()
                                          .withTrimmedTop(style->PANEL_MARGIN * 0.25)
                                          .withTrimmedBottom(style->PANEL_MARGIN)
                                          .removeFromBottom(11)
                                          .reduced(2));
    }
}

void AudioPreviewPanel::showSampleInFinder()
{
    if (mCurrentAudioFile.isEmpty())
    {
        return;
    }
    
    File(mCurrentAudioFile.getLocalFile()).revealToUser();
}
