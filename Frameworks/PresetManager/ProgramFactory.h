//
//  ProgramFactory.h
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#pragma once

#include <map>
#include <string>

#include "YAML/YamlParser.h"

namespace Platform
{
    class Logger;
}
class ParameterIdMap;
class Parameters;
class Program;
class ProgramFactory;


class PresetYamlParser: public YamlParser
{
public:
    PresetYamlParser(ProgramFactory& programFactory, Platform::Logger& errorLog);
    virtual ~PresetYamlParser() { }

protected:
    virtual const char* headerSectionName() const  { return "Preset"; }
    virtual void startDocument(const std::string& name) const;
    virtual void abandonDocument() const {}
    virtual void endDocument() const {}
    virtual size_t numDetailSections() const { return 1; }
    virtual const char* detailSectionName(size_t sectionNum) const { return "Params"; }
    virtual bool isValidDetailSectionValue(size_t sectionNum, const YAML::Node& valueNode) const;
    virtual void interpretKeyValuePair(const std::string& key, const YAML::Node& valueNode) const;

private:
    ProgramFactory& m_programFactory;
    YamlErrorLogger m_errorLog;
};

/// Defines an interface for creating a program from a textual description, e.g.
/// a YAML preset, that associates parameter name strings with value strings.
class ProgramFactory
{
public:
    /// Constructor
    ///
    /// @param parameterDefinitions
    ///     Object defining the parameter names and default values used by this plugin.
    /// @param pluginName
    ///     The simple plugin name, i.e. an internal name that doesn't needed to
    ///     have extra friendly text to display to the user. Should be the same as
    ///     the name used as preferences registry keys etc..
    /// @param pluginVersion
    ///     A string with the undecorated digits of the version number (i.e. doesn't
    ///     include strings like "version" and "beta".
    /// @param errorLog
    ///     The logger used for reporting parsing errors to the user.
    ProgramFactory(const Parameters& parameterDefinitions,
                   const char *pluginName, const char *pluginVersion,
                   Platform::Logger& errorLog);

    ~ProgramFactory();

    /// Informs the program factory to create any new programs as factory presets.
    void createFactoryPresets();

    /// Informs the program factory to create any new programs as user programs.
    void createUserPrograms();

    Program *loadYamlPreset(const std::string &fileName);
    Program* loadYamlChunk(const std::string& chunk);

    /// Loads a program chunk created before the new framework was introduced. Such chunks have parameters in a
    /// fixed size array.
    ///
    /// @param data
    ///     Pointer to the chunk data, past the standard FXB header and chunk size fields.
    /// @param byteSize
    ///     The number of bytes taken by the data pointed to via the data pointer.
    /// @param parameterIdMap
    ///     The mapping from the parameter IDs used in the old version of the plugin to those used in current versions.
    ///
    /// @return
    ///     The number of bytes actually read and interpreted from the chunk.
    size_t loadOldStyleProgramChunk(const char* data, size_t byteSize, const ParameterIdMap& parameterIdMap);

    std::string getYamlForProgram(const Program* program, bool compact);
    bool saveProgramToYamlFile(const Program* program, const std::string& fileName);

    void startNewProgram();

    /// Abandons the current program being creating, e.g. due to a parsing error.
    void abandonProgram();
    void setName(const std::string &name);
    void setName(const char *name);
    /// Sets the named parameter to the specified value.
    /// @ return
    ///     true if the parameter name is in the list of names originally provided,
    ///     otherwise false (i.e. the parameter name is not valid).
    bool setParameterValue(const char *paramName, float value);
    /// @todo MIDI learn, anything else?

    Program *getProgram();

private:
    const Parameters& m_parameterDefinitions;
    const char *m_pluginName;
    const char *m_pluginVersion;
    typedef std::map<std::string, size_t> ParamNameMap;
    ParamNameMap m_paramNameToIndex;
    bool m_createFactoryPresets;
    Program *m_currentProgram;
    Platform::Logger& m_errorLog;
    PresetYamlParser m_presetParser;

    /// Prevent copying.
    ProgramFactory(const ProgramFactory &);

    /// Prevent assignment.
    ProgramFactory &operator=(const ProgramFactory &);
};
