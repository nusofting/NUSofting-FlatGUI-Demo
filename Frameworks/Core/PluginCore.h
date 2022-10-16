//
// PluginCore.h
//
// Copyright 2016 Bernie Maier. All rights reserved.
// Licensed to NUSofting.
//
#pragma once

#include "DspEngine.h"
#include "EditorCore.h"
#include "Parameters.h"

#include "Platform/Platform.h"
#include "PresetManager/PresetManager.h"
#include "PresetManager/ProgramFactory.h"

#include <string>


//------------------------------------------------------------------------------

class Parameters;
class PluginCore;
class PluginFactory;
struct PluginProperties;
class Vst2Plugin;


/// Specific implementation class used by the plugin core to allow the DSP code to communicate with the plugin core.
/// This is an implementation detail which I'd prefer not to be in this header file.
///
/// @todo Consider simplifying this to omit the need to inherit from DspToPluginCoreInterface.
/// i.e. Just define this class, and put a forward declaration to it in DspEngine.h.
class DspToPlugin : public DspToPluginCoreInterface
{
public:
    DspToPlugin(PluginCore& plugin);
    virtual void setProgram(size_t programNum);
    virtual void reloadCurrentProgram();

private:
    PluginCore& m_pluginCore;
};

/// Specific implementation class used by the plugin core to allow the DSP code to communicate with the editor.
/// This is an implementation detail which I'd prefer not to be in this header file.
///
/// @todo Consider simplifying this to omit the need to inherit from DspToEditorInterface.
/// i.e. Just define this class, and put a forward declaration to it in DspEngine.h.
class DspToEditor : public DspToEditorInterface
{
public:
    // We always communicate to the editor via the plugin core, because the core needs to know everything that is
    // happening.
    DspToEditor(PluginCore& plugin);
    //virtual void setNoteOn(size_t noteNum, bool used);
    //virtual void setNoteOff(size_t noteNum, bool used);
    virtual void postDisplayParameterInt(int msgIndex, int value);

private:
    PluginCore& m_pluginCore;
};

//------------------------------------------------------------------------------

/// Encapsulates the core of the plugin, that mediates between the classes representing the major components of the
/// plugin: the DSP, the editor and the VST SDK / host communication. This is also the base class of a class implemented
/// for each specific plugin, that provides plugin-specific details.
class PluginCore: public EditorToPluginCoreInterface
{
public:
    PluginCore(const PluginProperties& pluginProps, Parameters& parameters, DspEngine& dsp, PluginFactory* factory);
    virtual ~PluginCore();

    /// Entry point to the plugin. This is called by either the VST SDK's call to create the effect instance or the
    /// Symbiosis AU wrapper that has been heavily modified to take advantage of more direct access to the capabilities
    /// of this framework.
    static PluginFactory* entry();
    void init();
    virtual EditorCore* getEditor() = 0;
    void connectVst2Plugin(Vst2Plugin* vst2Plugin);
    void shutdown();

    void setProgramFromHost(size_t program);
    void selectFactoryProgramByIndex(size_t index);
    void setParameterFromHost(size_t index, float value);
    const Program *getProgramForProjectSave();
    VstInt32 getChunk(void **data, bool isSingleProgram);
    VstInt32 setChunk(void *data, VstInt32 byteSize, bool isSingleProgram);
    Program* loadYamlChunk(const std::string& chunk); // Direct access for AU; no need to use awkward VST "setChunk" API


    /// @{
    /// EditorToPluginCoreInterface

    /// Notify that the currently selected program / preset has changed in the
    /// preset manager. Query the preset manager for current values.
    virtual void programChanged();

    /// Notify that the current program's value set (i.e. initial vs currently
    /// edited parameter values) has changed. Query the current program in the
    /// preset manager for the current values to use.
    virtual void paramValueSetChanged();

    /// Notify that the value of a parameter in the current program  has changed.
    virtual void paramValueChanged(size_t index, float value);

    /// Notify the core that the editor is changing its window size.
    virtual bool notifySizeChange(unsigned int newWidth, unsigned int newHeight);

    /// Notify the core that a control is starting to be manipulated in the
    /// editor and thus its value is likely to change.
    virtual void notifyBeginEdit(size_t index);

    /// Notify the core that a control is no longer being manipulated in the
    /// editor.
    virtual void notifyEndEdit(size_t index);

    /// Request that a preset or bank file from an older plugin version be converted.
    /// The file can be a DashLibrary .bnk or a VST FXB or FXP file.
    virtual void requestPresetsFileConversion(const char* filePath, Platform::Logger& logger);

	/// Request that a preset from other Sinamd version be converted (re-saved into ruiing plugin).
    /// The file can be a YAML file only
    virtual void requestLoadAndAutosaveYamls(const char* filePath, Platform::Logger& logger);

    /// Request that the formatted value of the indexed parameter is copied to the specified buffer.
    /// This is expected to be called from the GUI thread, and can access cached parameter values in the DSP engine.
    virtual void requestForFormattedParameterString(int index, char* buffer, size_t len);

	virtual float requestDefaultValue(int index);

    /// @}

    enum ParamValueSource
    {
        SRC_HOST_AUTOMATION,
        SRC_SET_PROGRAM,
        SRC_HOST_CHUNK,
        SRC_INTERNAL,
        SRC_EDITOR,
        SRC_MIDI_PC,
        SRC_RELOAD_TO_DSP
    };
    void notifyParameterChange(size_t index, float value, ParamValueSource source);
    void midiProgramChange(size_t programNum);
    void reloadCurrentProgram();
    /// A bit hacky, but this is a placeholder and will have to do for the moment.
    void pushToGui(unsigned int msgIndex, int value);

    void handleCloseFromHost();

    PresetManager& getPresetManager() { return m_presetManager; }
    ProgramFactory& getProgramFactory() { return m_programFactory; }
    const Parameters& getParameters() { return m_parameters; }

private:
    void setActiveProgram(Program* newProgram, ParamValueSource source);
    void defineModulations();
    virtual void defineParameterIdMappings() {}
    virtual void remapPresetParameters(Program* program) {}
    void convertPresets(const char* data, size_t byteSize, bool autoSave, Platform::Logger& logger);
	void doLoadAndAutosaveYamls(Program *newProgram, bool autoSave, Platform::Logger& logger);

protected:
    const PluginProperties& m_pluginProperties;
    Parameters& m_parameters;
    size_t m_numParams; // Convenience variable to save repeatedly querying size from m_parameters
    Platform::Logger m_errorLog;
    ProgramFactory m_programFactory;
    PresetManager m_presetManager;
    std::string m_mostRecentChunk;
    EditorCore* m_notifyEditor;
    ParameterIdMap m_oldToNewParamIds;
private:
    DspToPlugin m_dspToPluginNotifier;
    DspToEditor m_dspToEditorNotifier;
    DspEngine& m_dsp;
    Vst2Plugin* m_vst2Plugin;
    PluginFactory* m_factory;
};
