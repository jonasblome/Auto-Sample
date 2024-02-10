/*
 ==============================================================================
 
 SampleLibraryManager.cpp
 Author:  Jonas Blome
 
 ==============================================================================
 */

#include "SampleLibraryManager.h"

SampleLibraryManager::SampleLibraryManager(OwnedArray<SampleItem>& inAllSampleItems, OwnedArray<SampleItem>& inPaletteSampleItems)
:
ThreadWithProgressWindow("Synching sample library", true, true, 10000, "Stop loading", nullptr),
allSampleItems(inAllSampleItems),
paletteSampleItems(inPaletteSampleItems)
{
    // Initialize sample analyser
    mSampleAnalyser = std::make_unique<SampleAnalyser>();
}

SampleLibraryManager::~SampleLibraryManager()
{
    
}

void SampleLibraryManager::updateSampleLibraryFile(File& inLibraryDirectory)
{
    // Create sample library file as xml and store path to library
    XmlElement sampleLibraryXml("SampleLibrary");
    sampleLibraryXml.setAttribute("LibraryPath", inLibraryDirectory.getFullPathName());
    
    // Adding sample items xml to sample library xml
    XmlElement* sampleItemsXml = new XmlElement("SampleItems");
    
    for (SampleItem* sampleItem : allSampleItems)
    {
        XmlElement* sampleItemXml = new XmlElement("SampleItem");
        
        // Adding file path xml to sample item xml
        sampleItemXml->setAttribute("FilePath", sampleItem->getFilePath());
        
        // Adding sample properties xml to sample item xml
        XmlElement* samplePropertiesXml = new XmlElement("SampleProperties");
        
        // Adding title property
        XmlElement* samplePropertyXml = new XmlElement(PROPERTY_NAMES[0]);
        samplePropertyXml->setAttribute("PropertyValue", sampleItem->getTitle());
        samplePropertiesXml->prependChildElement(samplePropertyXml);
        
        // Adding length property
        samplePropertyXml = new XmlElement(PROPERTY_NAMES[1]);
        samplePropertyXml->setAttribute("PropertyValue", sampleItem->getLength());
        samplePropertiesXml->prependChildElement(samplePropertyXml);
        
        // Adding decibel loudness property
        samplePropertyXml = new XmlElement(PROPERTY_NAMES[2]);
        samplePropertyXml->setAttribute("PropertyValue", sampleItem->getLoudnessDecibel());
        samplePropertiesXml->prependChildElement(samplePropertyXml);
        
        // Adding LUFS loudness property
        samplePropertyXml = new XmlElement(PROPERTY_NAMES[3]);
        samplePropertyXml->setAttribute("PropertyValue", sampleItem->getLoudnessLUFS());
        samplePropertiesXml->prependChildElement(samplePropertyXml);
        
        // Adding tempo property
        samplePropertyXml = new XmlElement(PROPERTY_NAMES[4]);
        samplePropertyXml->setAttribute("PropertyValue", sampleItem->getTempo());
        samplePropertiesXml->prependChildElement(samplePropertyXml);
        
        // Adding key property
        samplePropertyXml = new XmlElement(PROPERTY_NAMES[5]);
        samplePropertyXml->setAttribute("PropertyValue", sampleItem->getKey());
        samplePropertiesXml->prependChildElement(samplePropertyXml);
        
        // Adding feature vector
        std::vector<float> featureVector = sampleItem->getFeatureVector();
        
        for (int d = 0; d < featureVector.size(); d++)
        {
            String childName = "FV" + std::to_string(d);
            samplePropertyXml = new XmlElement(childName);
            samplePropertyXml->setAttribute("PropertyValue", featureVector[d]);
            samplePropertiesXml->prependChildElement(samplePropertyXml);
        }
        
        sampleItemXml->prependChildElement(samplePropertiesXml);
        
        sampleItemsXml->prependChildElement(sampleItemXml);
    }
    
    sampleLibraryXml.prependChildElement(sampleItemsXml);
    
    // Write sample library xml to external file
    File sampleLibraryFile = File(mLibraryFilesDirectoryPath
                                  + DIRECTORY_SEPARATOR
                                  + inLibraryDirectory.getFileNameWithoutExtension()
                                  + SAMPLE_LIBRARY_FILE_EXTENSION);
    writeXmlToFile(sampleLibraryXml, sampleLibraryFile);
}

void SampleLibraryManager::synchWithLibraryDirectory()
{
    launchThread();
}

void SampleLibraryManager::run()
{
    // Go through all files in directory, check if a corresponding sample item
    // already exists in the sample item list, if not add it
    Array<File> allSampleFiles = libraryDirectory.findChildFiles(File::findFiles,
                                                                 true,
                                                                 SUPPORTED_AUDIO_FORMATS_WILDCARD);
    int numItemsToProcess = allSampleFiles.size() + allSampleItems.size();
    int numProcessedItems = 0;
    setProgress(0.0);
    
    // Go through all current sample items,
    // check if corresponding audio file still exists
    // and if not, delete sample item
    // from all items and palette collection
    for (SampleItem* sampleItem : allSampleItems)
    {
        if (threadShouldExit())
        {
            break;
        }
        
        if (!File(sampleItem->getFilePath()).exists())
        {
            addedFilePaths.removeString(sampleItem->getFilePath());
            paletteSampleItems.removeObject(sampleItem, false);
            allSampleItems.removeObject(sampleItem);
        }
        
        numProcessedItems++;
        setProgress(numProcessedItems / (double) numItemsToProcess);
    }
    
    // All files have already been loaded
    if (addedFilePaths.size() == allSampleFiles.size())
    {
        setProgress(1.0);
        return;
    }
    
    // Go through all files in directory and add sample items
    // for all that are not yet loaded into the library
    bool isLoadingNewLibrary = allSampleItems.size() == 0;
    
    for (File const & sampleFile : allSampleFiles)
    {
        if (threadShouldExit())
        {
            break;
        }
        
        if (isLoadingNewLibrary || !addedFilePaths.contains(sampleFile.getFullPathName()))
        {
            createSampleItem(sampleFile);
        }
        
        numProcessedItems++;
        setProgress(numProcessedItems / (double) numItemsToProcess);
    }
}

void SampleLibraryManager::threadComplete(bool userPressedCancel)
{
    sendChangeMessage();
}

void SampleLibraryManager::loadSampleLibrary(File& inLibraryDirectory)
{
    libraryDirectory = inLibraryDirectory;
    addedFilePaths.clear();
    File libraryFile = File(mLibraryFilesDirectoryPath
                            + DIRECTORY_SEPARATOR
                            + libraryDirectory.getFileNameWithoutExtension()
                            + SAMPLE_LIBRARY_FILE_EXTENSION);
    
    // Check if sample library file exists
    if (libraryFile.exists())
    {
        // Get data from library file
        XmlElement libraryXml = loadFileAsXml(libraryFile);
        XmlElement* libraryXmlPointer = &libraryXml;
        
        if (libraryXmlPointer)
        {
            XmlElement* sampleItemsXml = libraryXmlPointer->getChildByName("SampleItems");
            
            // Go over all sample items
            for (XmlElement const * sampleItemXml : sampleItemsXml->getChildIterator())
            {
                // Create new sample item
                SampleItem* sampleItem = allSampleItems.add(new SampleItem());
                String filePath = sampleItemXml->getStringAttribute("FilePath");
                sampleItem->setFilePath(filePath);
                addedFilePaths.add(filePath);
                
                // Adding properties to item
                XmlElement* samplePropertiesXml = sampleItemXml->getChildByName("SampleProperties");
                
                // Adding title property to item
                XmlElement* samplePropertyXml = samplePropertiesXml->getChildByName(PROPERTY_NAMES[0]);
                String title = samplePropertyXml->getStringAttribute("PropertyValue");
                sampleItem->setTitle(title);
                
                // Adding length property to item
                samplePropertyXml = samplePropertiesXml->getChildByName(PROPERTY_NAMES[1]);
                double length = samplePropertyXml->getDoubleAttribute("PropertyValue");
                sampleItem->setLength(length);
                
                // Adding decibel property to item
                samplePropertyXml = samplePropertiesXml->getChildByName(PROPERTY_NAMES[2]);
                double loudnessDecibel = samplePropertyXml->getDoubleAttribute("PropertyValue");
                sampleItem->setLoudnessDecibel(loudnessDecibel);
                
                // Adding LUFS property to item
                samplePropertyXml = samplePropertiesXml->getChildByName(PROPERTY_NAMES[3]);
                double loudnessLUFS = samplePropertyXml->getDoubleAttribute("PropertyValue");
                sampleItem->setLoudnessLUFS(loudnessLUFS);
                
                // Adding tempo property to item
                samplePropertyXml = samplePropertiesXml->getChildByName(PROPERTY_NAMES[4]);
                int tempo = samplePropertyXml->getIntAttribute("PropertyValue");
                sampleItem->setTempo(tempo);
                
                // Adding key property to item
                samplePropertyXml = samplePropertiesXml->getChildByName(PROPERTY_NAMES[5]);
                int key = samplePropertyXml->getIntAttribute("PropertyValue");
                sampleItem->setKey(key);
                
                // Adding feature vector to item
                int numDimensions = NUM_CHROMA + NUM_FEATURES + NUM_SPECTRAL_BANDS;
                std::vector<float> featureVector(numDimensions);
                
                for (int d = 0; d < numDimensions; d++)
                {
                    String childName = "FV" + std::to_string(d);
                    samplePropertyXml = samplePropertiesXml->getChildByName(childName);
                    featureVector[d] = samplePropertyXml->getDoubleAttribute("PropertyValue");
                }
                
                sampleItem->setFeatureVector(featureVector);
            }
        }
    }
    
    synchWithLibraryDirectory();
}

String SampleLibraryManager::getLastOpenedDirectory()
{
    String lastOpenedDirectoryPath = mDefaultLibraryDirectoryPath;
    
    File saemplDataFile = File(mSaemplDataFilePath);
    
    if (!saemplDataFile.exists())
    {
        return lastOpenedDirectoryPath;
    }
    
    // Get data from library file
    XmlElement saemplDataXml = loadFileAsXml(saemplDataFile);
    XmlElement* saemplDataXmlPointer = &saemplDataXml;
    
    if (saemplDataXmlPointer)
    {
        lastOpenedDirectoryPath = saemplDataXml.getStringAttribute("LastOpenedDirectory");
    }
    
    return lastOpenedDirectoryPath;
}

void SampleLibraryManager::storeLastOpenedDirectory(String& inDirectoryPath)
{
    File saemplDataFile = File(mSaemplDataFilePath);
    XmlElement saemplDataXml = XmlElement("SaemplData");
    saemplDataXml.setAttribute("LastOpenedDirectory", inDirectoryPath);
    writeXmlToFile(saemplDataXml, saemplDataFile);
}

XmlElement SampleLibraryManager::loadFileAsXml(File& inFile)
{
    const InterProcessLock::ScopedLockType scopedLock(mFileLock);
    
    MemoryBlock fileData;
    inFile.loadFileAsData(fileData);
    XmlElement fileXml = *AudioPluginInstance::getXmlFromBinary(fileData.getData(), (int) fileData.getSize());
    
    return fileXml;
}

void SampleLibraryManager::writeXmlToFile(XmlElement& inXml, File& inFile)
{
    const InterProcessLock::ScopedLockType scopedLock(mFileLock);
    
    MemoryBlock destinationData;
    AudioPluginInstance::copyXmlToBinary(inXml, destinationData);
    inFile.replaceWithData(destinationData.getData(), destinationData.getSize());
}

SampleItem* SampleLibraryManager::createSampleItem(File const & inFile)
{
    SampleItem* newItem = allSampleItems.add(new SampleItem());
    newItem->setFilePath(inFile.getFullPathName().convertToPrecomposedUnicode());
    newItem->setTitle(inFile.getFileNameWithoutExtension().convertToPrecomposedUnicode());
    analyseSampleItem(*newItem, inFile, false);
    addedFilePaths.add(newItem->getFilePath());
    
    return newItem;
}

SampleItem* SampleLibraryManager::getSampleItemWithFilePath(String const & inFileName)
{
    for (SampleItem* sampleItem : allSampleItems)
    {
        if (sampleItem->getFilePath().convertToPrecomposedUnicode().compare(inFileName.convertToPrecomposedUnicode()) == 0)
        {
            return sampleItem;
        }
    }
    
    return nullptr;
}

bool SampleLibraryManager::fileHasBeenAdded(String const & inFilePath)
{
    return addedFilePaths.contains(inFilePath);
}

void SampleLibraryManager::analyseSampleItem(SampleItem& inSampleItem, File const & inFile, bool forceAnalysis)
{
    mSampleAnalyser->analyseSample(inSampleItem, inFile, forceAnalysis);
}
