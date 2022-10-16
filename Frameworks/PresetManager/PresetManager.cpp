//
//  PresetManager.cpp
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "PresetManager.h"
#include "Program.h"
#include "ProgramFactory.h"

#include <algorithm>
#include <cassert>
#include <sys/stat.h>

#include "Platform/Platform.h"
#include "Utils/Utils.h"


namespace
{
    // We want the "special" categories used for uncategorised presets to appear at the end of the list and
    // also to appear visually distinctive. To do this, we enclose these names inside a pair of Unicode angle
    // quotation marks. We add a special case to the category name sorting function to ensure that strings
    // starting with the special opening mark will sort after any "normal" category names.
    // Note that we don't currently support category names to contain any international (i.e. non 7-bit traditional
    // ASCII) characters because category names are the source directory names, and since international characters
    // are encoded differently between Windows and OS X file systems we just make the rule that we don't use them.
    // In practice, they show up OK on OS X but are truncated in Windows.
    #define kUtf8LeftMark  "\xC2\xAB"   // Left-pointing double angle quotation mark (U+00AB)
    #define kUtf8RightMark "\xC2\xBB"   // Right-pointing double angle quotation mark (U+00BB)
    const char* kUserCategory    = kUtf8LeftMark "User presets" kUtf8RightMark;
    const char* kFactoryCategory = kUtf8LeftMark "Factory presets" kUtf8RightMark;
    const char* kProjectCategory = kUtf8LeftMark "Project presets" kUtf8RightMark;
    const char* kConvertedCategory = kUtf8LeftMark "Converted old presets" kUtf8RightMark;
    // The following category is only used if no preset files are loaded.
    const char* kInitCategory = kUtf8LeftMark "Init" kUtf8RightMark;
}

PresetManager::PresetManager(ProgramFactory &programFactory)
 : m_programFactory(programFactory),
   m_categoryIndex(0),
   m_programIndexInCategory(0),
   m_currentProgram(0),
   m_defaultProgram(0),
   m_allPresetFilesValid(true)
{
}

PresetManager::~PresetManager()
{
    /// @todo delete the programs in the vector.
    delete m_defaultProgram;
}

bool compareProgramNames(const Program* prog1, const Program* prog2)
{
    return Platform::utf8CaseInsensitiveCompare(prog1->getName(), prog2->getName()) < 0;
}

bool compareCategoryNames(const std::string& str1, const std::string& str2)
{
    bool isLess = false;
    bool str1IsSpecial = Utils::startsWith(kUtf8LeftMark, str1.c_str());
    bool str2IsSpecial = Utils::startsWith(kUtf8LeftMark, str2.c_str());
    // We want the strings that represent the "special" categories for uncategorised presets to appear
    // at the end of the list. However, the character we use to prefix the special categories does not
    // necessarily sort in the desired order, therefore we add an explicit check for the prefix.
    // If neither or both strings represent the "special" categories for uncategorised presets,
    // then perforrm an internationalised case-insensitive comparison. Otherwise (i.e. only one of the
    // category strings represents a special category, the construct the result to force the special
    // category string to appear at the end of the list.
    if (str1IsSpecial == str2IsSpecial)
    {
        isLess = Platform::utf8CaseInsensitiveCompare(str1, str2) < 0;
    }
    else
    {
        isLess = str2IsSpecial;
    }
    return isLess;
}

bool has_suffix(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

class PresetManager::YamlFileCallback : public Platform::DirTreeCallback
{
public:
    YamlFileCallback(PresetManager &presetManager, const char* defaultCategory)
     : m_presetManager(presetManager),
       m_category(defaultCategory)
    { }
    virtual void run(const std::string &filePath)
    {
        m_presetManager.loadYamlPresetFile(filePath, m_category);
    }
    virtual const char* enterSubdir(const char* leafSubdir)
    {
        // We return the previous category set, to restore when leaving this sudbirectoy.
        const char* previousCategory = m_category;
        // We use the subdirectory name as the preset category.
        m_category = leafSubdir;
        return previousCategory;
    }
    virtual void leaveSubdir(const char* restoreData)
    {
        m_category = restoreData;
    }
private:
    PresetManager &m_presetManager;
    const char* m_category;
};


void PresetManager::loadPresets()
{
    // Reset the flag used to indicate whether or not any errors occurred loading any of the files.
    m_allPresetFilesValid = true;
    m_programFactory.createFactoryPresets();
    std::string factoryPresetsDir = Platform::getPresetsDirectory(false);
    YamlFileCallback yamlFactoryPresetFileCallback(*this, kFactoryCategory);
    Platform::forEachFileInDir(factoryPresetsDir, yamlFactoryPresetFileCallback, ".yaml");
    m_programFactory.createUserPrograms();
    std::string userPresetsDir = Platform::getPresetsDirectory(true);
    YamlFileCallback yamlUserPresetFileCallback(*this, kUserCategory);
    Platform::forEachFileInDir(userPresetsDir, yamlUserPresetFileCallback, ".yaml");
    if (m_programs.empty())
    {
        createDefaultProgram();
    }
    else
    {
        std::sort(m_categories.begin(), m_categories.end(), compareCategoryNames);
        for (CategorisedPrograms::iterator it = m_categorisedPrograms.begin();
             it != m_categorisedPrograms.end();
             ++it)
        {
            Programs& programsForCategory = it->second;
            std::sort(programsForCategory.begin(), programsForCategory.end(), compareProgramNames);
        }
    }
}

Program* PresetManager::loadYamlChunk(const std::string& chunk)
{
    Program *newProgram = m_programFactory.loadYamlChunk(chunk);
    if (newProgram)
    {
        addProgram(newProgram, kProjectCategory, true);
    }
    return newProgram;
}

void PresetManager::loadYamlPresetFile(const std::string& filePath, const std::string& category)
{
    Program *newProgram = m_programFactory.loadYamlPreset(filePath);
    if (newProgram)
    {
        addProgram(newProgram, category, false);
    }
    else
    {
        m_allPresetFilesValid = false;
    }
}

void PresetManager::addProgram(Program* newProgram, const std::string& category, bool makeCurrent)
{
    newProgram->setCategory(category);
    addProgram(newProgram, makeCurrent);
}

void PresetManager::addProgram(Program* newProgram, bool makeCurrent)
{
    m_programs.push_back(newProgram);
    const std::string& category = newProgram->getCategory();
    Programs& programsForCategory = m_categorisedPrograms[category];
    if (programsForCategory.empty())
    {
        m_categories.push_back(category);
    }
    programsForCategory.push_back(newProgram);
    if (makeCurrent)
    {
        m_currentProgram = newProgram;
        std::sort(programsForCategory.begin(), programsForCategory.end(), compareProgramNames);
        std::vector<std::string>::iterator categoryIt = std::find(m_categories.begin(), m_categories.end(), category);
        m_categoryIndex = std::distance(m_categories.begin(), categoryIt);
        Programs::iterator programIt = std::find(programsForCategory.begin(), programsForCategory.end(), m_currentProgram);
        m_programIndexInCategory = std::distance(programsForCategory.begin(), programIt);
    }
}

void PresetManager::addConvertedProgram(Program* newProgram, bool convertedToUserProgram)
{
    std::string category(convertedToUserProgram ? kUserCategory : kConvertedCategory);
    addProgram(newProgram, category, false);
}

std::string PresetManager::getYamlChunkForProgram(const Program *program)
{
    return m_programFactory.getYamlForProgram(program, false);
}

Program* PresetManager::getCurrentProgram()
{
    // Either a current program must have been set or a default program created.
    // Normally even if only a default program has been created it will be made
    // current, so this is truly a check against future programming errors.
    assert(m_currentProgram || m_defaultProgram);
    return m_currentProgram ? m_currentProgram : m_defaultProgram;
}

Program* PresetManager::getProgram(size_t programIndex)
{
    // Either programs must have been loaded or a default program created.
    assert(!m_programs.empty() || m_defaultProgram);
    return programIndex < m_programs.size() ? m_programs[programIndex] : m_defaultProgram;
}

Program* PresetManager::getDefaultProgram()
{
    if (!m_defaultProgram)
    {
        createDefaultProgram();
    }
    return m_defaultProgram;
}

Program* PresetManager::copyCurrentProgram(const char* newName)
{
    assert(m_currentProgram);
    Program* newProgram = new Program(*m_currentProgram);
    newProgram->setName(newName);
    newProgram->convertToUserPreset();
    std::string userCategory(kUserCategory);
    newProgram->setCategory(userCategory);
    // A copied program isn't saved, so therefore it must be dirty.
    newProgram->markDirty();
    return newProgram;
}

void PresetManager::selectProgram(size_t categoryIndex, size_t programIndexInCategory)
{
    if (m_currentProgram)
    {
        m_currentProgram->revert();
    }
    if (categoryIndex < m_categories.size())
    {
        m_categoryIndex = categoryIndex;
        const std::string& categoryStr = m_categories[categoryIndex];
        Programs& programs = m_categorisedPrograms[categoryStr];
        if (programIndexInCategory < programs.size())
        {
            m_programIndexInCategory = programIndexInCategory;
            m_currentProgram = programs[programIndexInCategory];
        }
    }
}

bool PresetManager::saveNew(Program* program, const std::string& directory)
{
    if (!program)
    {
        return false;
    }
    std::string filePath(Platform::buildSafeFileName(directory.empty() ? Platform::getPresetsDirectory(true) : directory,
                                                     program->getName(),
                                                     "yaml"));
    bool saved = savePresetFile(program, filePath);
    if (saved)
    {
        program->setFilePath(filePath);
    }
    return saved;
}

bool PresetManager::overwrite(Program* program)
{
    if (!program)
    {
        return false;
    }
    const std::string &filePath = program->getFilePath();
    return savePresetFile(program, filePath);
}

bool PresetManager::savePresetFile(Program* program, const std::string& filePath)
{
    // Make sure we save the *edited* parameters.
    program->selectValueSet(Program::VALUE_CURRENT);
    bool saved = m_programFactory.saveProgramToYamlFile(program, filePath);
    if (saved)
    {
        program->markDirty(false);
    }
    return saved;
}

bool PresetManager::userPresetNameExists(const char* name)
{
    bool nameExists = false;
    std::string userCategory(kUserCategory);
    Programs& userPrograms = m_categorisedPrograms[userCategory];
    for (Programs::iterator it = userPrograms.begin();
         !nameExists && it != userPrograms.end();
         ++it)
    {
        Program* program = *it;
        nameExists = (program->getName().compare(name) == 0);
    }
    return nameExists;
}

std::string PresetManager::getCurrentProgramName()
{
    return m_currentProgram ? m_currentProgram->getName() : std::string("Init");
}

void PresetManager::setCurrentProgramName(const char* newName)
{
    if (m_currentProgram)
    {
        m_currentProgram->setName(newName);
        m_currentProgram->markDirty();
    }
}

size_t PresetManager::numPresets() const
{
    return m_programs.size();
}

size_t PresetManager::numCategories() const
{
    return m_categories.size();
}

std::vector<std::string> PresetManager::getCategories()
{
    return m_categories;
}

Programs PresetManager::getProgramsInCategory(const char* category)
{
    std::string categoryStr(category ? category : ""); // Convert null category to empy string
    return m_categorisedPrograms[categoryStr];
}

const std::vector<std::string> &PresetManager::getIndexedFactoryProgramNames()
{
    if (m_indexedFactoryPrograms.empty())
    {
        for (size_t categoryIndex = 0; categoryIndex < m_categories.size(); ++categoryIndex)
        {
            //const char *categoryName = categories[categoryIndex].c_str();
            Programs& programs = m_categorisedPrograms[m_categories[categoryIndex]];
            size_t programIndex = 0;
            for (Programs::iterator it = programs.begin();
                 it != programs.end();
                 ++it, ++programIndex)
            {
                Program* program = *it;
                if (program->isFactoryPreset())
                {
                    IndexedProgram progData = { categoryIndex, programIndex, program };
                    m_indexedFactoryPrograms.push_back(progData);
                    m_indexedFactoryProgramNames.push_back(std::string(program->getFullName()));
                }
            }
        }
    }
    return m_indexedFactoryProgramNames;
}

void PresetManager::selectFactoryProgramByIndex(size_t programIndex)
{
    selectProgram(m_indexedFactoryPrograms[programIndex].m_categoryIndex,
                  m_indexedFactoryPrograms[programIndex].m_programIndex);
}


void PresetManager::createDefaultProgram()
{
    if (!m_defaultProgram)
    {
        // The program factory knows how to create a program using default parameter values.
        m_programFactory.startNewProgram();
        m_defaultProgram = m_programFactory.getProgram();
        if (m_defaultProgram)
        {
            m_defaultProgram->setName("Init");
            std::string initCategory(kInitCategory);
            m_defaultProgram->setCategory(initCategory);
            // By definition, the default program hasn't yet been saved anywhere, so mark it dirty.
            m_defaultProgram->markDirty();
            // Only add the default program to m_programs if there are no other programs there already.
            // Sometimes we may need to create a default program even when we have loaded actual factory/user programs.
            if (m_programs.empty())
            {
                addProgram(m_defaultProgram, true);
            }
        }
        else
        {
            fprintf(stderr, "Failed to load any presets or even create a default program. This is likely to be terminal.\n");
        }
    }
}
