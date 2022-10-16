//
// PluginFactory.h
//
// Copyright 2016 Bernie Maier. All rights reserved.
// Licensed to NUSofting.
//
#pragma once


#include "Parameters.h"


/// Structure encapsulating the various static properties of a plugin, such as its name, ID, number of parameters etc.
/// This covers information that used to be specific via #defines in VstSynth.h.
struct PluginProperties
{
    unsigned int numPrograms; /// @todo Do we actually need this? With the framework, we manage our own presets and only report 1 to the host.
    unsigned int numParams; /// @todo Do we need to distinguish between the number of internal params vs the number reported to the host?
    unsigned int numInputs;
    unsigned int numOutputs;
    unsigned int pluginID;
    const char* name;
    const char* nameAsId;
    const char* versionStr;
    const char* mixerLabel;
    bool isSynth;
    /// Legacy chunk / file formats
    /// Assumed to be:
    /// 1. Optional array of parameter definitions
    /// 2. Array of programs, where each program is:
    ///    - Fixed-size program name buffer
    ///    - Array of floating point parameter values
    bool canLoadOldPrograms;
    unsigned int oldNumPrograms;
    unsigned int oldNumParams;
    size_t paramDefinitionSize; // Size of a single, old-style parameter definition
    size_t programNameSize;
};

//------------------------------------------------------------------------------------------

class DspEngine;
class PluginCore;
class PluginCoreToEditorInterface;

/// Encapsulates an object that contains and builds the various single main components of the plugin. This allows the
/// major components to be constructed in a consistent and orderly manner, coordinated via the plugin core, but with
/// each specific plugin providing the classes and subclasses specific to the plugin itself.
/// That is, each plugin needs to subclass this and implement these functions to initialise the plugin-specific features
/// and capabilities.
/// This class also organises the ownership and thus memory management of the major components, so that everything can
/// be shut down and destroyed in an orderly fashion when the plugin instance is removed from the project.
class PluginFactory
{
public:
    /// Implement this function in a plugin-specific source file to create the subclass factory object itself.
    static PluginFactory* create();

    /// Implement this function in a plugin-specific source file to destroy the subclass factory object created earlier.
    static void destroy(PluginFactory* factory);

    virtual ~PluginFactory() { }
    const PluginProperties& getProperties() { return m_pluginProperties; }
    Parameters& getParameters() { return m_parameters; }

    /// Implement this function in a plugin-specific factory subclass to return a reference to the plugin-specific DSP
    /// class (e.g. VstSynth).
    virtual DspEngine& getDspEngine() = 0;

    /// Implement this function in a plugin-specific factory subclass to return a reference to the plugin-specific
    /// plugin core class (e.g. PetiPlugin, SinnahPlugin).
    virtual PluginCore* getPluginInstance() = 0;

protected:
    PluginProperties m_pluginProperties;
    Parameters m_parameters;
};

