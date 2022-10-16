//
//  Appearance.cpp
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "Appearance.h"

#include <cassert>

Appearance::Appearance(size_t numColourItems, bool isFactory)
 : m_colourItems(numColourItems),
   m_decorationType(0),
   m_isFactory(isFactory)
{
}

Appearance::~Appearance()
{
}

void Appearance::setName(const std::string& name)
{
    m_name = name;
}

void Appearance::setName(const char* name)
{
    m_name = name ? name : "";
}

void Appearance::setColourItemValue(size_t itemId, const AppearanceColourItem& value)
{
    m_colourItems[itemId] = value;
}

const AppearanceColourItem& Appearance::getColourItemValue(size_t itemId) const
{
    return (m_colourItems)[itemId];
}

void Appearance::setDecorationType(size_t decorationType)
{
    m_decorationType = decorationType;
}

void Appearance::setFilePath(const std::string& filePath)
{
    m_filePath = filePath;
}

Appearance::Appearance(const Appearance& other)
 : m_name(other.m_name),
   m_colourItems(other.m_colourItems),
   m_isFactory(other.m_isFactory)
{
}

Appearance& Appearance::operator=(Appearance rhs)
{
    std::swap(m_name, rhs.m_name);
    std::swap(m_colourItems, rhs.m_colourItems);
    m_isFactory = rhs.m_isFactory;
    return *this;
}

