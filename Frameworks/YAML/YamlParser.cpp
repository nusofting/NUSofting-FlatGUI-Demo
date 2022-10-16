//
//  YamlParser.cpp
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "YamlParser.h"

#include <fstream>
#include <sstream>
#include <string>

#include "Platform/Platform.h"
#include "yaml-cpp/yaml.h"


//==============================================================================
//

const char *YamlParser::kProductKey = "product";
const char *YamlParser::kVersionKey = "version";
const char *YamlParser::kFormatKey = "format";
const char *YamlParser::kNameKey = "name";

YamlParser::YamlParser(YamlErrorLogger& errorLog)
 : m_errorLog(errorLog)
{
}

bool YamlParser::parseFile(const std::string& fileName)
{
    m_errorLog.clear();
    m_errorLog.setFilename(fileName);
    std::ifstream fin(fileName);
    return parse(fin);
}

bool YamlParser::parseString(const std::string& yamlContent)
{
    m_errorLog.clear();
    std::stringstream ss(yamlContent);
    return parse(ss);
}

bool YamlParser::parse(std::istream& input)
{
    try
    {
        YAML::Parser parser(input);
        YAML::Node doc;
        if (parser.GetNextDocument(doc)) {
            return parse(doc);
        }
    }
    catch (YAML::Exception &e)
    {
        std::stringstream ss;
        ss << "YAML error at line " << e.mark.line + 1 << ", column " << e.mark.column + 1 << ": " << e.msg;
        m_errorLog.log(ss.str());
    }
    catch (...)
    {
        m_errorLog.log("YAML parsing failure");
    }
    return false;
}

bool YamlParser::parse(const YAML::Node& doc)
{
    const YAML::Node& headerNode = doc[headerSectionName()];
    const YAML::Node& productNode = headerNode[kProductKey];
    const YAML::Node& versionNode = headerNode[kVersionKey];
    const YAML::Node& formatNode = headerNode[kFormatKey];
    const YAML::Node& nameNode = headerNode[kNameKey];
    int format = formatNode.to<int>();
    // We are not checking the specific format right now because changes so far have been compatible.
    std::string name = nameNode.to<std::string>();
    startDocument(name);
    m_errorLog.setYamlDocname(name);
    size_t numSections = numDetailSections();
    for (auto i = 0; i < numSections; ++i)
    {
        const YAML::Node& detailNode = doc[detailSectionName(i)];
        if (!isValidDetailSectionValue(i, detailNode))
        {
            m_errorLog.log("Invalid YAML document structure");
            abandonDocument();
            return false;
        }
        for (auto it = detailNode.begin();
             it != detailNode.end();
             ++it)
        {
            auto key = it.first().to<std::string>();
            m_errorLog.setKey(key);
            auto& valueNode = it.second();
            interpretKeyValuePair(key, valueNode);
        }
    }
    endDocument();
    return true;
}


//==============================================================================
//

YamlErrorLogger::YamlErrorLogger(Platform::Logger& errorLog)
 : m_errorLog(errorLog)
{
}

void YamlErrorLogger::clear()
{
    m_filename = "";
    m_docname = "";
    m_key = "";
}

void YamlErrorLogger::log(const std::string& message) const
{
    Platform::LogLine(m_errorLog) << message;
    if (!m_key.empty())
    {
        Platform::LogLine(m_errorLog) << "    key: " << m_key;
    }
    if (!m_docname.empty())
    {
        Platform::LogLine(m_errorLog) << "    preset/theme: " << m_docname;
    }
    if (!m_filename.empty())
    {
        Platform::LogLine(m_errorLog) << "    file: " << m_filename;
    }
}
