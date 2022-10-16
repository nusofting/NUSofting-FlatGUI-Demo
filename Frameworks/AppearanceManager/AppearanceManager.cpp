//
//  AppearanceManager.cpp
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "AppearanceManager.h"
#include "Appearance.h"
#include "AppearanceFactory.h"

#include <algorithm>
#include <cassert>

#include "Platform/Platform.h"
#include "Utils/Utils.h"


AppearanceManager::AppearanceManager(AppearanceFactory& appearanceFactory)
 : m_appearanceFactory(appearanceFactory),
   m_defaultAppearance(0),
   m_currentAppearance(0),
   m_currentAppearanceIndex(0),
   m_allAppearanceFilesValid(true)
{
}

AppearanceManager::~AppearanceManager()
{
    clearThemes();
}

class AppearanceManager::YamlFileCallback : public Platform::DirTreeCallback
{
public:
    YamlFileCallback(AppearanceManager& appearanceManager)
     : m_appearanceManager(appearanceManager)
    { }
    virtual void run(const std::string& filePath)
    {
        m_appearanceManager.loadYamlAppearanceFile(filePath);
    }
    virtual const char* enterSubdir(const char* leafSubdir) { return 0; }
    virtual void leaveSubdir(const char* restoreData) { }
private:
    AppearanceManager& m_appearanceManager;
};

bool compareAppearanceNames(const Appearance* appearance1, const Appearance* appearance2)
{
    return Platform::utf8CaseInsensitiveCompare(appearance1->getFilePath(), appearance2->getFilePath()) < 0;
}

void AppearanceManager::loadThemes()
{
    // Reset the flag used to indicate whether or not any errors occurred loading any of the files.
    m_allAppearanceFilesValid = true;
    clearThemes();
    m_appearanceFactory.createUserAppearances();
    std::string userPresetsDir = Platform::getAppearanceDirectory(true);
    YamlFileCallback yamlUserPresetFileCallback(*this);
    Platform::forEachFileInDir(userPresetsDir, yamlUserPresetFileCallback, ".yaml");
    m_appearanceFactory.createFactoryAppearances();
    std::string factoryPresetsDir = Platform::getAppearanceDirectory(false);
    YamlFileCallback yamlFactoryPresetFileCallback(*this);
    Platform::forEachFileInDir(factoryPresetsDir, yamlFactoryPresetFileCallback, ".yaml");
    if (m_appearances.empty())
    {
        createDefaultAppearance();
    }
    else
    {
        std::sort(m_appearances.begin(), m_appearances.end(), compareAppearanceNames);
        Appearances::iterator endIt = m_appearances.end();
        size_t index = 0;
        for (Appearances::iterator appearanceIt = m_appearances.begin();
             appearanceIt != endIt;
             ++appearanceIt, ++index)
        {
            Appearance* item = *appearanceIt;
            m_appearancesByName[item->getFilePath()] = index;
            m_appearancesByName[item->getName()] = index;
        }
    }
}

void AppearanceManager::clearThemes()
{
    m_defaultAppearance = 0;
    m_currentAppearance = 0;
    m_currentAppearanceIndex = 0;
    m_appearancesByName.clear();
    Appearances::iterator endIt = m_appearances.end();
    for (Appearances::iterator appearanceIt = m_appearances.begin(); appearanceIt != endIt; ++appearanceIt)
    {
        Appearance* item = *appearanceIt;
        delete item;
    }
    m_appearances.clear();
}

void AppearanceManager::loadYamlAppearanceFile(const std::string& filePath)
{
    Appearance* newAppearance = m_appearanceFactory.loadYamlRepresentation(filePath);
    if (newAppearance)
    {
        addAppearance(newAppearance);
    }
    else
    {
        m_allAppearanceFilesValid = false;
    }
}

void AppearanceManager::addAppearance(Appearance* newAppearance)
{
    size_t newIndex = m_appearances.size();
    m_appearances.push_back(newAppearance);
}

Appearance* AppearanceManager::selectAppearance(size_t itemId)
{
    if (itemId < m_appearances.size())
    {
        m_currentAppearance = m_appearances[itemId];
        m_currentAppearanceIndex = itemId;
    }
    else
    {
        m_currentAppearance = m_defaultAppearance;
        m_currentAppearanceIndex = 0;
    }
    return m_currentAppearance;
}

Appearance* AppearanceManager::selectAppearance(const std::string& fileName, const std::string& name)
{
    NameToIndexMap::iterator it = m_appearancesByName.find(fileName);
    if (it == m_appearancesByName.end())
    {
        // File of that name no longer exists, try looking up by just the appearance name.
        it = m_appearancesByName.find(name);
    }
    size_t foundIndex = (it != m_appearancesByName.end()) ? it->second : 0;
    return selectAppearance(foundIndex);
}

Appearance* AppearanceManager::getCurrentAppearance() const
{
    // Either a current Appearance must have been set or a default Appearance created.
    // Normally even if only a default Appearance has been created it will be made
    // current, so this is truly a check against future programming errors.
    assert(m_currentAppearance || m_defaultAppearance);
    return m_currentAppearance ? m_currentAppearance : m_defaultAppearance;
}

std::string AppearanceManager::getCurrentAppearanceName() const
{
    return m_currentAppearance ? m_currentAppearance->getName() : std::string("Init");
}

std::string AppearanceManager::getAppearanceName(size_t itemId) const
{
    assert(itemId < m_appearances.size());
    return m_appearances[itemId]->getName();
}

size_t AppearanceManager::numThemes() const
{
    return m_appearances.size();
}

void AppearanceManager::createDefaultAppearance()
{
    if (!m_defaultAppearance)
    {
        // The Appearance factory knows how to create a Appearance using default parameter values.
        m_appearanceFactory.startNewAppearance();
        m_defaultAppearance = m_appearanceFactory.getAppearance();
        if (m_defaultAppearance)
        {
            m_defaultAppearance->setName("Init");
            addAppearance(m_defaultAppearance);
            m_appearancesByName[m_defaultAppearance->getFilePath()] = 0;
            m_appearancesByName[m_defaultAppearance->getName()] = 0;
            selectAppearance(0);
        }
        else
        {
            fprintf(stderr, "Failed to load any appearance themes or even create a default one. This is likely to be terminal.\n");
        }
    }
}
