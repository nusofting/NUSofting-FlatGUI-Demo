//
//  Program.cpp
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "Program.h"

#include <cassert>

Program::Program(size_t numParams, bool isFactory)
 : m_initialParams(numParams, 0.0f),
   m_currentParams(numParams, 0.0f),
   m_activeParams(&m_currentParams),
   m_isDirty(false),
   m_isFactory(isFactory)
{
    //fprintf(stderr, "Creating program %p\n", this);
}

Program::~Program()
{
    //fprintf(stderr, "Destroying program %p\n", this);
}

void Program::setName(const std::string &name)
{
    m_name = name;
}

void Program::setName(const char *name)
{
    m_name = name ? name : "";
}

void Program::setCategory(const std::string& category)
{
    m_category = category;
}

const std::string Program::getFullName() const
{
    return std::string(m_category) + ": " + m_name;
}

void Program::markDirty(bool isDirty)
{
    m_isDirty = isDirty;
    if (!isDirty)
    {
        // Copy all the changed parameters into the initial parameters array,
        // since this is now the clean state.
        m_initialParams = m_currentParams;
    }
}

void Program::setInitialParameterValue(size_t paramId, float value)
{
    m_initialParams[paramId] = value;
    m_currentParams[paramId] = value;
}

void Program::changeParameterValue(size_t paramId, float value)
{
    m_currentParams[paramId] = value;
    if (m_initialParams[paramId] != value)
    {
        // Only change the dirty state if the parameter has really changed.
        m_isDirty = true;
    }
}

void Program::revert()
{
    m_currentParams = m_initialParams;
    m_isDirty = false;
}

void Program::selectValueSet(ValueType valueType)
{
    switch (valueType)
    {
    case VALUE_INITIAL:
        m_activeParams = &m_initialParams;
        break;
    case VALUE_CURRENT:
        m_activeParams = &m_currentParams;
        break;
    default:
        assert(valueType == VALUE_INITIAL || valueType == VALUE_CURRENT);
        break;
    }
}

Program::ValueType Program::getGetValueSetType() const
{
    return m_activeParams == &m_initialParams ? VALUE_INITIAL : VALUE_CURRENT;
}

float Program::getParameterValue(size_t paramId) const
{
    return (*m_activeParams)[paramId];
}

bool Program::isUnsavedChunk() const
{
    // By definition, if a program was loaded from a chunk it won't have a file name set.
    return m_filePath.empty();
}

void Program::convertToUserPreset()
{
    m_isFactory = false;
    // @todo Set category to user! (Currently done in PresetManager)
}

void Program::setFilePath(const std::string& filePath)
{
    m_filePath = filePath;
}

Program::Program(const Program& other)
 : m_name(other.m_name),
   m_category(other.m_category),
   m_initialParams(other.m_initialParams),
   m_currentParams(other.m_currentParams),
   m_activeParams(other.m_activeParams == &other.m_initialParams ? &m_initialParams : &m_currentParams),
   m_isDirty(other.m_isDirty),
   m_isFactory(other.m_isFactory)
{
    //fprintf(stderr, "Creating program %p via copy\n", this);
}

Program& Program::operator=(Program rhs)
{
    std::swap(m_name, rhs.m_name);
    std::swap(m_category, rhs.m_category);
    std::swap(m_initialParams, rhs.m_initialParams);
    std::swap(m_currentParams, rhs.m_currentParams);
    m_activeParams = rhs.m_activeParams == &rhs.m_initialParams ? &m_initialParams : &m_currentParams;
    m_isDirty = rhs.m_isDirty;
    m_isFactory = rhs.m_isFactory;
    //fprintf(stderr, "Creating program %p via assignment\n", this);
    return *this;
}

