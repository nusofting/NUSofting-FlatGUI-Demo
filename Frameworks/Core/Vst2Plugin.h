//
// Vst2Plugin.h
//
// Copyright 2016 Bernie Maier. All rights reserved.
// Licensed to NUSofting.
//
#pragma once

#include "PluginFactory.h"

#include "vst2.4/public.sdk/source/vst2.x/audioeffectx.h"

class DspEngine;
class EditorCore;
class Parameters;
class PluginCore;
class PresetManager;
class Program;
class ProgramFactory;


class Vst2Plugin : public AudioEffectX
{
public:
    Vst2Plugin(const PluginProperties& pluginProps,
               PluginCore& core,
               DspEngine& dspEngine,
               PresetManager& presetManager);
    virtual ~Vst2Plugin();
    void connectHost(audioMasterCallback hostAudioMaster);
    void connectEditor(EditorCore& notifyEditor);
    /// @{
    /// VST 2.4 interface functions (AudioEffect)
    virtual bool getEffectName(char* name);
    virtual bool getProductString(char* text);
    virtual bool getOutputProperties(VstInt32 index, VstPinProperties* properties);
    virtual bool getVendorString(char* text);
    virtual VstInt32 getVendorVersion() {return 1;}
    virtual VstInt32 canDo(char* text);
    virtual VstInt32 getNumMidiInputChannels();
    virtual VstInt32 getNumMidiOutputChannels();

    virtual void open();
    virtual VstInt32 startProcess();
    virtual VstInt32 stopProcess();
    virtual void resume();
    virtual void suspend();

    virtual void setSampleRate(float sampleRate);
    virtual void setBlockSize(VstInt32 blockSize);

    virtual void setProgram(VstInt32 program);
    virtual void setProgramName(char *name);
    virtual void getProgramName(char *name);
    virtual void setParameter(VstInt32 index, float value);
    virtual void setParameterAutomated (VstInt32 index, float value);
    virtual float getParameter(VstInt32 index);
    virtual void getParameterLabel(VstInt32 index, char *label);
    virtual void getParameterDisplay(VstInt32 index, char *text);
    virtual void getParameterName(VstInt32 index, char *text);
    virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char *text);
    virtual bool copyProgram (VstInt32 destination);
    virtual VstInt32 getChunk(void **data, bool isSingleProgram);
    virtual VstInt32 setChunk(void *data, VstInt32 byteSize, bool isSingleProgram = false);
    virtual void processReplacing(float **inputs, float **outputs, VstInt32 sampleframes);
    VstInt32 processEvents (VstEvents* ev);
    /// @}

private:
    PluginProperties m_pluginProperties;
    const Parameters& m_parameters;
    PluginCore& m_core;
    DspEngine& m_dspEngine;
    PresetManager& m_presetManager;
    bool m_initialising;
};
