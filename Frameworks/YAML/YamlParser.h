//
//  YamlParser.h
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#pragma once

#include <ios>
#include <string>


namespace Platform
{
    class Logger;
}

namespace YAML
{
    class Node;
}
class YamlErrorLogger;

class YamlParser
{
public:
    YamlParser(YamlErrorLogger& errorLog);
    virtual ~YamlParser() { }
    bool parseFile(const std::string& fileName);
    bool parseString(const std::string& yamlContent);

protected:
    virtual const char* headerSectionName() const = 0;
    virtual void startDocument(const std::string& name) const = 0;
    virtual void abandonDocument() const = 0;
    virtual void endDocument() const = 0;
    virtual size_t numDetailSections() const = 0;
    virtual const char* detailSectionName(size_t sectionNum) const = 0;
    virtual bool isValidDetailSectionValue(size_t sectionNum, const YAML::Node& valueNode) const = 0;
    virtual void interpretKeyValuePair(const std::string& key, const YAML::Node& valueNode) const = 0;

private:
    bool parse(std::istream& input);
    bool parse(const YAML::Node& doc);

    static const char *kProductKey;
    static const char *kVersionKey;
    static const char *kFormatKey;
    static const char *kNameKey;
    YamlErrorLogger& m_errorLog;

    /// Prevent copying.
    YamlParser(const YamlParser &);

    /// Prevent assignment.
    YamlParser &operator=(const YamlParser &);
};

/// Encapsulates an error logging facility for YAML parsing errors.
class YamlErrorLogger
{
public:
    YamlErrorLogger(Platform::Logger& errorLog);
    ~YamlErrorLogger() { }
    void setFilename(const std::string& filename) { m_filename = filename; }
    void setYamlDocname(const std::string& docname) { m_docname = docname; }
    void setKey(const std::string& key) { m_key = key;}
    void clear();
    void log(const std::string& message) const;
private:
    Platform::Logger& m_errorLog;
    std::string m_filename;
    std::string m_docname;
    std::string m_key;

    /// Prevent copying.
    YamlErrorLogger(const YamlErrorLogger &);

    /// Prevent assignment.
    YamlErrorLogger &operator=(const YamlErrorLogger &);
};

