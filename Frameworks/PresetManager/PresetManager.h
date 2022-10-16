//
//  PresetManager.h
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#pragma once

#include <map>
#include <string>
#include <vector>


class ParameterIdMap;
class Program;
class ProgramFactory;

// Helper typedef for an array (vector) of pointers to Program objects.
typedef std::vector<Program*> Programs;

/// Encapsulates the collection of presets and currently actual program by
/// loading the presets from the file system and providing a mechanism to
/// navigate through the collection and set a current program. A program is a
/// snapshot of the values of each published parameter of the plugin, while a
/// preset is a stored copy of a program, i.e. all presets are programs but not
/// all programs are presets.
///
/// This class also maintains the state of the current program (e.g whether or
/// not the user has altered it) and provides the ability to switch between and
/// altered and original program, save changes back to the file system etc.
/// This class is thus repsonsible for managing preset and program data, but not
/// the UI. The UI uses this interface to present the preset information.
class PresetManager
{
public:
    PresetManager(ProgramFactory &programFactory);

    ~PresetManager();

    void loadPresets();
    bool allPresetFilesValid() const { return m_allPresetFilesValid; }
    Program* loadYamlChunk(const std::string& chunk);
    std::string getYamlChunkForProgram(const Program *program); // Direct access for AU
    Program* getCurrentProgram();
    Program* getProgram(size_t programIndex);
    Program* getDefaultProgram();
    Program* copyCurrentProgram(const char* newName);
    void addProgram(Program* newProgram, bool makeCurrent);
    void addConvertedProgram(Program* newProgram, bool convertedToUserProgram);
    void selectProgram(size_t categoryIndex, size_t programIndexInCategory);
    size_t getCurrentCategoryIndex() const { return m_categoryIndex; }
    size_t getCurrentProgramIndexInCategory() const { return m_programIndexInCategory; }
    std::string getCurrentProgramName();
    void setCurrentProgramName(const char* newName);

    /// Saves the program into a new, uniquely named preset file with the file
    /// name being the program name transformed to use only "safe" characters
    /// and digits added in case of duplicated names.
    bool saveNew(Program* program, const std::string& directory = "");

    /// Overwrites the preset file the program was originally loaded from, with
    /// the current contents of the preset.
    bool overwrite(Program* program);

    /// Checks whether or not a preset with the specified name already exists at the root
    /// of the user presets tree. Note that unless the user adds subdirectories and
    /// categorises files, all the user presets will be at the root.
    bool userPresetNameExists(const char* name);

    size_t numPresets() const;
    size_t numCategories() const;
    std::vector<std::string> getCategories();
    Programs getProgramsInCategory(const char* category);

    /// Provide an alternative view into the factory presets, for host APIs that request a straight array / vector of
    /// presets rather than the categorised view used with our own preset browser.
    const std::vector<std::string> &getIndexedFactoryProgramNames();
    void selectFactoryProgramByIndex(size_t programIndex);

private:
    void addProgram(Program* newProgram, const std::string& category, bool makeCurrent);
    void loadYamlPresetFile(const std::string& filePath, const std::string& category);
    bool savePresetFile(Program* program, const std::string& filePath);
    void createDefaultProgram();

    class YamlFileCallback;
    ProgramFactory &m_programFactory;
    Programs m_programs;
    std::vector<std::string> m_categories;
    typedef std::map<std::string, Programs> CategorisedPrograms;
    CategorisedPrograms m_categorisedPrograms;
    size_t m_categoryIndex;
    size_t m_programIndexInCategory;
    Program* m_currentProgram;
    Program* m_defaultProgram;
    bool m_allPresetFilesValid;

    struct IndexedProgram
    {
        size_t m_categoryIndex;
        size_t m_programIndex;
        Program *m_program;
    };
    std::vector<IndexedProgram> m_indexedFactoryPrograms;
    std::vector<std::string> m_indexedFactoryProgramNames;

    /// Prevent copying.
    PresetManager(const PresetManager &);

    /// Prevent assignment.
    PresetManager &operator=(const PresetManager &);
};
