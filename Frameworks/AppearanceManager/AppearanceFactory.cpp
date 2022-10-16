//
//  AppearanceFactory.cpp
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "AppearanceFactory.h"
#include "Appearance.h"

#include <cassert>

#include "Platform/Platform.h"
#include "YAML/YamlParser.h"
#include "yaml-cpp/yaml.h"


AppearanceFactory::AppearanceFactory(Platform::Logger& errorLog)
 : m_createFactoryAppearances(false),
   m_currentAppearance(0),
   m_errorLog(errorLog),
   m_appearanceParser(*this, errorLog)
{
}

AppearanceFactory::~AppearanceFactory()
{

}

void AppearanceFactory::defineColourItem(size_t itemNum, const char* name, const AppearanceColourItem& defaultValue)
{
    m_colourItemDefinitions.push_back(ColourItemDefinition(itemNum, name, defaultValue));
    assert(m_colourItemDefinitions.size() - 1 == itemNum);
    m_itemNameToIndex[name] = itemNum;
}

void AppearanceFactory::defineDecorationName(size_t itemNum, const char* name)
{
    m_decorationNames.push_back(name);
    assert(m_decorationNames.size() - 1 == itemNum);
    m_decorationNameToIndex[name] = itemNum;
}

void AppearanceFactory::createFactoryAppearances()
{
    m_createFactoryAppearances = true;
}

void AppearanceFactory::createUserAppearances()
{
    m_createFactoryAppearances = false;
}


Appearance* AppearanceFactory::loadYamlRepresentation(const std::string& fileName)
{
    m_currentAppearance = 0;
    bool fileContainsValidData = m_appearanceParser.parseFile(fileName);
    if (fileContainsValidData)
    {
        m_currentAppearance->setFilePath(fileName);
    }
    else
    {
        abandonAppearance();
    }
    return m_currentAppearance;
}

void AppearanceFactory::startNewAppearance()
{
    size_t numItems = m_colourItemDefinitions.size();
    m_currentAppearance = new Appearance(numItems, m_createFactoryAppearances);
    // Set up default values for each parameter.
    for (size_t i = 0; i < numItems; ++i)
    {
        m_currentAppearance->setColourItemValue(i, m_colourItemDefinitions[i].m_defaultValue);
    }
}

void AppearanceFactory::abandonAppearance()
{
    delete m_currentAppearance;
    m_currentAppearance = 0;
}

void AppearanceFactory::setName(const std::string& name)
{
    assert(m_currentAppearance);
    m_currentAppearance->setName(name);
}

void AppearanceFactory::setName(const char* name)
{
    assert(m_currentAppearance);
    m_currentAppearance->setName(name);
}

bool AppearanceFactory::setColourValue(const std::string& itemName, const AppearanceColourItem& colourValue)
{
    assert(m_currentAppearance);
    assert(!m_itemNameToIndex.empty());
    ItemNameMap::iterator itemNameEntry = m_itemNameToIndex.find(itemName);
    bool itemNameValid = itemNameEntry != m_itemNameToIndex.end();
    if (itemNameValid)
    {
        m_currentAppearance->setColourItemValue(itemNameEntry->second, colourValue);
    }
    return itemNameValid;
}

bool AppearanceFactory::setDecorationValue(const std::string& decorationName)
{
    assert(m_currentAppearance);
    assert(!m_decorationNameToIndex.empty());
    ItemNameMap::iterator decorationNameEntry = m_decorationNameToIndex.find(decorationName);
    bool decorationNameValid = decorationNameEntry != m_decorationNameToIndex.end();
    if (decorationNameValid)
    {
        m_currentAppearance->setDecorationType(decorationNameEntry->second);
    }
    return decorationNameValid;
}

Appearance* AppearanceFactory::getAppearance()
{
    return m_currentAppearance;
}

//==============================================================================
//

class ColourSeq
{
public:
    ColourSeq(const YAML::Node& seqNode);
    bool addColourItem(const std::string& name, AppearanceFactory& appearanceFactory);
    bool isValid() const
    {
        return m_isValid;
    }

private:
    bool m_isValid;
    int m_red;
    int m_green;
    int m_blue;
    float m_alpha;
};

ColourSeq::ColourSeq(const YAML::Node& seqNode)
 : m_isValid(false),
   m_red(0),
   m_green(0),
   m_blue(0),
   m_alpha(0.0)
{
    if (seqNode.size() == 3 || seqNode.size() == 4)
    {
        m_red = seqNode[0].to<int>();
        m_green = seqNode[1].to<int>();
        m_blue = seqNode[2].to<int>();
        m_alpha = (seqNode.size() == 4) ? seqNode[3].to<float>() : 1.0;
        m_isValid = true;
    }
}

bool ColourSeq::addColourItem(const std::string& name, AppearanceFactory& appearanceFactory)
{
    if (m_isValid)
    {
        return appearanceFactory.setColourValue(name, AppearanceColourItem(m_red, m_green, m_blue, m_alpha));
    }
    return false;
}

//==============================================================================
//

AppearanceYamlParser::AppearanceYamlParser(AppearanceFactory& appearanceFactory, Platform::Logger& errorLog)
 : YamlParser(m_errorLog),
   m_appearanceFactory(appearanceFactory),
   m_errorLog(errorLog)
{
}

void AppearanceYamlParser::startDocument(const std::string& name) const
{
    m_appearanceFactory.startNewAppearance();
    m_appearanceFactory.setName(name);
}

bool AppearanceYamlParser::isValidDetailSectionValue(size_t sectionNum, const YAML::Node& valueNode) const
{
    return sectionNum == 0 && valueNode.Type() == YAML::NodeType::Map;
}

void AppearanceYamlParser::interpretKeyValuePair(const std::string& key, const YAML::Node& valueNode) const
{
    if (   valueNode.Type() == YAML::NodeType::Sequence
        && (valueNode.size() == 3 || valueNode.size() == 4))
    {
        getColour(key, valueNode);
    }
    else if (key == "decoType")
    {
        getDecoration(valueNode);
    }
    else
    {
        m_errorLog.log("Warning: unrecognised item name: " + key);
    }
}

void AppearanceYamlParser::getColour(const std::string& name, const YAML::Node& valueNode) const
{
    ColourSeq colour(valueNode);
    if (colour.isValid())
    {
        if (!colour.addColourItem(name, m_appearanceFactory))
        {
            m_errorLog.log("Warning: unrecognised colour item name: " + name);
        }
    }
}

void AppearanceYamlParser::getDecoration(const YAML::Node& valueNode) const
{
    auto name = valueNode.to<std::string>();
    if (!m_appearanceFactory.setDecorationValue(name))
    {
        m_errorLog.log("Warning: unrecognised decoration name: " + name);
    }
}
