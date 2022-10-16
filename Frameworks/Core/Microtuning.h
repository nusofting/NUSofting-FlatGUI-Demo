//
//  Microtuning.h
//
//  Copyright 2017 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//
#pragma once

#include <string>
#include <vector>

struct MicrotuningScale
{
    enum
    {
        NumNotes = 128,
    };
    std::string name;
    float cents[NumNotes];
};

class MicrotuningFile
{

};

/// Encapsulates information about a microtuning scale, plus loads and saves
/// such scales from and to a file.
class Microtuning
{
public:
    Microtuning();
    ~Microtuning();
    void setFullName(const char* fullName);

private:
};

class MicrotuningCollection
{
public:
    MicrotuningCollection();
    ~MicrotuningCollection();
    std::string importFromFile(const char* fileName);
    bool importFromFile(const char* fileName, MicrotuningScale& importedScale);
    MicrotuningScale* scaleForName(const char* scaleName);

private:
    void createDefaultEqualTemperamentScale();
    std::vector<MicrotuningScale> m_scales;
};
