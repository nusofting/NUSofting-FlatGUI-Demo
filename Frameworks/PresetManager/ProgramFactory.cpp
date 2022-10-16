//
//  ProgramFactory.cpp
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "ProgramFactory.h"
#include "Program.h"

#include "Core/Parameters.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

#include "yaml-cpp/yaml.h"
#include "Platform/Platform.h"


//==============================================================================
//

ProgramFactory::ProgramFactory(const Parameters& parameterDefinitions,
                               const char *pluginName,
                               const char *pluginVersion,
                               Platform::Logger& errorLog)
 : m_parameterDefinitions(parameterDefinitions),
   m_pluginName(pluginName),
   m_pluginVersion(pluginVersion),
   m_createFactoryPresets(false),
   m_currentProgram(0),
   m_errorLog(errorLog),
   m_presetParser(*this, errorLog)
{
    for (size_t i = 0; i < parameterDefinitions.getNumParameters(); ++i)
    {
        m_paramNameToIndex[m_parameterDefinitions.getFullName(i)] = i;
    }
}

ProgramFactory::~ProgramFactory()
{

}

void ProgramFactory::createFactoryPresets()
{
    m_createFactoryPresets = true;
}

void ProgramFactory::createUserPrograms()
{
    m_createFactoryPresets = false;
}


Program *ProgramFactory::loadYamlPreset(const std::string &fileName)
{
    m_currentProgram = 0;
    bool fileContainsValidData = m_presetParser.parseFile(fileName);
    if (fileContainsValidData)
    {
        m_currentProgram->setFilePath(fileName);
    }
    else
    {
        abandonProgram();
    }
    return m_currentProgram;
}

Program* ProgramFactory::loadYamlChunk(const std::string& chunk)
{
    // Chunks are treated like factory presets because they can't be saved in their current
    // location via the plugin's UI. (They can be saved by the host's UI, but we have no
    // knowledge of that, nor do we need our save logic for that.)
    bool saveCreateFactoryPresetsSetting = m_createFactoryPresets;
    m_createFactoryPresets = true;

    m_currentProgram = 0;
    bool chunkContainsValidPreset = m_presetParser.parseString(chunk);
    if (!chunkContainsValidPreset)
    {
        abandonProgram();
    }
    m_createFactoryPresets = saveCreateFactoryPresetsSetting;
    return m_currentProgram;
}

size_t ProgramFactory::loadOldStyleProgramChunk(const char* data, size_t byteSize, const ParameterIdMap& parameterIdMap)
{
    size_t bytesUsed = 0;
    // Old style chunk starts with up to 24 bytes for name.
    setName(data);
    data += 24;
    bytesUsed += 24;
    const float* params = reinterpret_cast<const float*>(data);
    const size_t kParamValueSize = sizeof params[0];
    size_t numChunkParams = parameterIdMap.getNumOldParameters();
    for (size_t i = 0; i < numChunkParams; ++i)
    {
        size_t newId = parameterIdMap.getNewIdForOldParameter(i);
        if (newId != ParameterIdMap::kIgnored)
        {
            m_currentProgram->setInitialParameterValue(parameterIdMap.getNewIdForOldParameter(i), params[i]);
        }
#if 0
        else
        {
            fprintf(stderr, "Ignoring old parameter ID %zu with value %f\n", i, params[i]);
        }
#endif
        bytesUsed += kParamValueSize;
    }
    return bytesUsed;
}

void ProgramFactory::startNewProgram()
{
    size_t numParams = m_parameterDefinitions.getNumParameters();
    m_currentProgram = new Program(numParams, m_createFactoryPresets);
    // Set up default values for each parameter.
    for (size_t i = 0; i < numParams; ++i)
    {
        m_currentProgram->setInitialParameterValue(i, m_parameterDefinitions.getNormalisedDefaultValue(i));
    }
}

void ProgramFactory::abandonProgram()
{
    delete m_currentProgram;
    m_currentProgram = 0;
}

void ProgramFactory::setName(const std::string &name)
{
    assert(m_currentProgram);
    m_currentProgram->setName(name);
}

void ProgramFactory::setName(const char *name)
{
    assert(m_currentProgram);
    m_currentProgram->setName(name);
}

bool ProgramFactory::setParameterValue(const char *paramName, float value)
{
    assert(m_currentProgram);
    assert(!m_paramNameToIndex.empty());
    ParamNameMap::iterator paramNameEntry = m_paramNameToIndex.find(paramName);
    bool paramNameValid = paramNameEntry != m_paramNameToIndex.end();
    if (paramNameValid)
    {
        m_currentProgram->setInitialParameterValue(paramNameEntry->second, value);
    }
    return paramNameValid;
}

Program *ProgramFactory::getProgram()
{
    return m_currentProgram;
}

std::string ProgramFactory::getYamlForProgram(const Program* program, bool compact)
{
    if (!program)
    {
        Platform::LogLine(m_errorLog) << "No program to save";
        return std::string("");
    }

    YAML::Emitter out;
    out << YAML::BeginDoc;
    out << YAML::BeginMap;
    out << YAML::Key << "Preset"
        << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "product"
            << YAML::Value << m_pluginName;
        out << YAML::Key << "version"
            << YAML::Value << m_pluginVersion;
        out << YAML::Key << "format"
            << YAML::Value << 1;
        out << YAML::Key << "name"
            << YAML::Value << program->getName();
        out << YAML::EndMap;
    out << YAML::Key << "Params"
        << YAML::Value << YAML::BeginMap;
        for (size_t i = 0; i < m_parameterDefinitions.getNumParameters(); ++i)
        {
            out << YAML::Key << m_parameterDefinitions.getFullName(i)
                << YAML::Value << program->getParameterValue(i);
        }
        out << YAML::EndMap;
    out << YAML::EndMap;
    return std::string(out.c_str());
}

bool ProgramFactory::saveProgramToYamlFile(const Program* program, const std::string& fileName)
{
    if (!program)
    {
        return false;
    }

    std::ofstream output;
    output.open(fileName.c_str());
    if (!output)
    {
        Platform::LogLine(m_errorLog) << "Failed to create preset file " << fileName
                                      << " for program " << program->getName();
        return false;
    }

    std::string yaml = getYamlForProgram(program, false);
    if (!yaml.empty())
    {
        output << yaml;
    }
    output.close();

    return !yaml.empty();
}


//==============================================================================
//

PresetYamlParser::PresetYamlParser(ProgramFactory& programFactory, Platform::Logger& errorLog)
 : YamlParser(m_errorLog),
   m_programFactory(programFactory),
   m_errorLog(errorLog)
{
}

void PresetYamlParser::startDocument(const std::string& name) const
{
    m_programFactory.startNewProgram();
    m_programFactory.setName(name);
}

bool PresetYamlParser::isValidDetailSectionValue(size_t sectionNum, const YAML::Node& valueNode) const
{
    return sectionNum == 0 && valueNode.Type() == YAML::NodeType::Map;
}

void PresetYamlParser::interpretKeyValuePair(const std::string& key, const YAML::Node& valueNode) const
{
    if (valueNode.Type() == YAML::NodeType::Scalar)
    {
        auto value = valueNode.to<float>();
        if (!m_programFactory.setParameterValue(key.c_str(), value))
        {
            m_errorLog.log("Warning: ignoring unrecognised parameter");
        }
    }
    else
    {
        m_errorLog.log("Warning: unrecognised key (" + key + ") or bad value");
    }
}
