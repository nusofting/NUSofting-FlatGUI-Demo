//
//  AppearanceFactory.h
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#pragma once

#include "Appearance.h"

#include "YAML/YamlParser.h"

#include <map>
#include <string>
#include <vector>

namespace Platform
{
    class Logger;
}
class AppearanceFactory;


class AppearanceYamlParser: public YamlParser
{
public:
    AppearanceYamlParser(AppearanceFactory& appearanceFactory, Platform::Logger& errorLog);
    virtual ~AppearanceYamlParser() { }

protected:
    virtual const char* headerSectionName() const  { return "Theme"; }
    virtual void startDocument(const std::string& name) const;
    virtual void abandonDocument() const {}
    virtual void endDocument() const {}
    virtual size_t numDetailSections() const { return 1; }
    virtual const char* detailSectionName(size_t sectionNum) const { return "Appearance"; }
    virtual bool isValidDetailSectionValue(size_t sectionNum, const YAML::Node& valueNode) const;
    virtual void interpretKeyValuePair(const std::string& key, const YAML::Node& valueNode) const;

private:
    void getColour(const std::string& name, const YAML::Node& valueNode) const;
    void getDecoration(const YAML::Node& valueNode) const;

    AppearanceFactory& m_appearanceFactory;
    YamlErrorLogger m_errorLog;
};


/// Defines an interface for creating an appearance theme from a textual description, e.g.
/// a YAML file, that associates appearance item name strings with value strings.
class AppearanceFactory
{
public:
    /// Constructor
    ///
    /// @param errorLog
    ///     The logger used for reporting parsing errors to the user.
    AppearanceFactory(Platform::Logger& errorLog);

    ~AppearanceFactory();

    /// Defines a single colour item that can be read from the appearances theme file. In the file the item is
    /// identified by name, but within the application it is identified by a numeric identifier. This definition
    /// associates the ID with the name, plus a default value for the item if it is missing from the file.
    /// The numeric IDs are used as indexes and so must start at 0 and be contiguous. This function must be called
    /// with ID values in increasing order.
    void defineColourItem(size_t itemNum, const char* name, const AppearanceColourItem& defaultValue);

    /// Defines a single decoration name that can be read as a value from the appearances theme file.
    /// This is handled in a similar way to the colour item, except that the defined colour names are
    /// keys in the appearance file whereas the decoration name is a value.
    void defineDecorationName(size_t itemNum, const char* name);

    /// Informs the appearance factory to identify any new appearance themes as factory themes.
    void createFactoryAppearances();

    /// Informs the appearance factory to identify any new appearance themes as user themes.
    void createUserAppearances();

    Appearance* loadYamlRepresentation(const std::string& fileName);

    void startNewAppearance();

    /// Abandons the current appearance being creating, e.g. due to a parsing error.
    void abandonAppearance();
    void setName(const std::string& name);
    void setName(const char* name);
    /// Sets the named colour item to the specified value.
    /// @ return
    ///     true if the item name is in the list of names originally provided,
    ///     otherwise false (i.e. the item name is not valid).
    bool setColourValue(const std::string& itemName, const AppearanceColourItem& colourValue);
    bool setDecorationValue(const std::string& decorationName);

    Appearance* getAppearance();

private:
    struct ColourItemDefinition
    {
        size_t m_id;
        std::string m_name;
        AppearanceColourItem m_defaultValue;
        ColourItemDefinition(size_t id, const char* name, const AppearanceColourItem& defaultValue)
          : m_id(id), m_name(name), m_defaultValue(defaultValue) { }
    };
    std::vector<ColourItemDefinition> m_colourItemDefinitions;
    typedef std::map<std::string, size_t> ItemNameMap;
    ItemNameMap m_itemNameToIndex;
    std::vector<std::string> m_decorationNames;
    ItemNameMap m_decorationNameToIndex;
    bool m_createFactoryAppearances;
    Appearance* m_currentAppearance;
    Platform::Logger& m_errorLog;
    AppearanceYamlParser m_appearanceParser;

    /// Prevent copying.
    AppearanceFactory(const AppearanceFactory&);

    /// Prevent assignment.
    AppearanceFactory& operator=(const AppearanceFactory&);
};
