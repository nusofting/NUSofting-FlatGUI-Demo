//
//  Microtuning.cpp
//
//  Copyright 2017 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "Microtuning.h"

#include <cassert>


ParameterDefinition::ParameterDefinition(size_t id, const char* shortName, float defaultValue, bool dynamicLabel)
 : m_id(id),
   m_shortName(shortName ? shortName : ""),
   m_fullName(),
   m_label(),
   m_defaultValue(defaultValue),
   m_minimumValue(0.0),
   m_maximumValue(1.0),
   m_labelIsDynamic(dynamicLabel)
{
}

ParameterDefinition::ParameterDefinition(size_t id, const char* shortName, float defaultValue, const char *label)
 : m_id(id),
   m_shortName(shortName ? shortName : ""),
   m_fullName(""),
   m_label(label ? label : ""),
   m_defaultValue(defaultValue),
   m_minimumValue(0.0),
   m_maximumValue(1.0),
   m_labelIsDynamic(false)
{
}

ParameterDefinition::~ParameterDefinition()
{
}

void ParameterDefinition::setFullName(const char* fullName)
{
    m_fullName = std::string(fullName ? fullName : ""); // Guard against setting to a null pointer
}

void ParameterDefinition::setValidValueRange(float minimumValue, float maximumValue)
{
    m_minimumValue = minimumValue;
    m_maximumValue = maximumValue;
}

void ParameterDefinition::addDisplayDependent(size_t id)
{
    m_redisplayDependents.push_back(id);
}

const std::string& ParameterDefinition::getFullName() const
{
    return m_fullName.empty() ? m_shortName : m_fullName;
}


//==============================================================================

Parameters::Parameters()
{
}

Parameters::~Parameters()
{
}

ParameterDefinition* Parameters::define(size_t id, const char* shortName, float defaultValue, bool dynamicLabel)
{
    ParameterDefinition* paramDefinition = new ParameterDefinition(id, shortName, defaultValue, dynamicLabel);
    m_definitions.push_back(paramDefinition);
    assert(m_definitions.size() - 1 == id);
    return paramDefinition;
}

ParameterDefinition* Parameters::define(size_t id, const char* shortName, float defaultValue, const char *label)
{
    ParameterDefinition* paramDefinition = new ParameterDefinition(id, shortName, defaultValue, label);
    m_definitions.push_back(paramDefinition);
    assert(m_definitions.size() - 1 == id);
    return paramDefinition;
}

const std::string& Parameters::getShortName(size_t id) const
{
    return m_definitions[id]->getShortName();
}

const std::string& Parameters::getFullName(size_t id) const
{
    return m_definitions[id]->getFullName();
}

const std::string& Parameters::getLabel(size_t id) const
{
    return m_definitions[id]->getLabel();
}

bool Parameters::hasLabel(size_t id) const
{
    return m_definitions[id]->hasLabel();
}

bool Parameters::labelIsDynamic(size_t id) const
{
    return m_definitions[id]->labelIsDynamic();
}

float Parameters::getDefaultValue(size_t id) const
{
    return m_definitions[id]->getDefaultValue();
}

float Parameters::getMinimumValue(size_t id) const
{
    return m_definitions[id]->getMinimumValue();
}

float Parameters::getMaximumValue(size_t id) const
{
    return m_definitions[id]->getMaximumValue();
}


//==============================================================================

ParameterIdMap::ParameterIdMap()
{
}

ParameterIdMap::~ParameterIdMap()
{
}

void ParameterIdMap::define(size_t oldId, size_t newId)
{
    m_newParameterIds.push_back(newId);
    assert(m_newParameterIds.size() - 1 == oldId);
}

void ParameterIdMap::ignore(size_t oldId)
{
    m_newParameterIds.push_back(kIgnored);
    assert(m_newParameterIds.size() - 1 == oldId);
}

size_t ParameterIdMap::getNewIdForOldParameter(size_t oldParameterId) const
{
    return m_newParameterIds[oldParameterId];
}
