//
// Vst2Plugin.cpp
//
// Copyright 2016 Bernie Maier. All rights reserved.
// Licensed to NUSofting.
//

#include "Vst2Plugin.h"

#include "DspEngine.h"
#include "EditorCore.h"
#include "PluginCore.h"
#include "Core/Parameters.h"
#include "Platform/Platform.h"
#include "PresetManager/PresetManager.h"
#include "PresetManager/Program.h"

#include "vst2.4/public.sdk/source/vst2.x/aeffeditor.h"

#include <cassert>


/// Helper function for initialising the VST 2.4 SDK object representing an
/// effect or synth plugin.
void vstSetup(AudioEffectX* vstEffect, const PluginProperties& pluginProps)
{
    vstEffect->programsAreChunks();
    vstEffect->setNumInputs(pluginProps.numInputs);
    vstEffect->setNumOutputs(pluginProps.numOutputs);
    vstEffect->canProcessReplacing();
    if (pluginProps.isSynth)
    {
        vstEffect->isSynth();
    }
    vstEffect->setUniqueID(pluginProps.pluginID);
    vstEffect->DECLARE_VST_DEPRECATED(canMono(true));
}

/// Helper function for copying strings into buffers supplied by the VST interface.
/// VST limits these to 24 bytes.
void vstStringCopy(const std::string &src, char *dest)
{
    const size_t kVstStringBufLen = 24;
    /// @todo Make this UTF-8 aware so that we don't split a UTF-8 character when truncating.
    strncpy(dest, src.c_str(), kVstStringBufLen);
    dest[kVstStringBufLen - 1] = 0;
}


//==============================================================================
class VstEditor : public AEffEditor
{
public:
//-------------------------------------------------------------------------------------------------------
    VstEditor(EditorCore& notifyEditor);
    virtual ~VstEditor();
    virtual bool getRect(ERect** rect);
    virtual bool open(void* ptr);
    virtual void close();
    virtual bool isOpen();
    virtual void idle();

private:
    EditorCore& m_notifyEditor;
    ERect m_rect;
};


//==============================================================================
Vst2Plugin::Vst2Plugin(const PluginProperties& pluginProps,
                       PluginCore& core,
                       DspEngine& dspEngine,
                       PresetManager& presetManager)
 : AudioEffectX(0, pluginProps.numPrograms, pluginProps.numParams),
   m_pluginProperties(pluginProps),
   m_parameters(core.getParameters()),
   m_core(core),
   m_dspEngine(dspEngine),
   m_presetManager(presetManager),
   m_initialising(true)
{
    vstSetup(this, pluginProps);
}

Vst2Plugin::~Vst2Plugin()
{
    m_core.shutdown();
}

void Vst2Plugin::connectHost(audioMasterCallback hostAudioMaster)
{
    audioMaster = hostAudioMaster;
}

void Vst2Plugin::connectEditor(EditorCore& notifyEditor)
{
	 VstEditor* vstEditor = new VstEditor(notifyEditor);
    setEditor(vstEditor);
}

//==============================================================================
//
// VST interface
//

//-----------------------------------------------------------------------------------------
bool Vst2Plugin::getEffectName(char* name)
{
    vst_strncpy(name, m_pluginProperties.name, kVstMaxEffectNameLen);
    return true;
}

//-----------------------------------------------------------------------------------------
bool Vst2Plugin::getProductString(char* text)
{
    vst_strncpy(text, m_pluginProperties.name, kVstMaxProductStrLen);
    return true;
}

//-----------------------------------------------------------------------------------------
bool Vst2Plugin::getVendorString(char* text)
{
    vst_strncpy(text, "nusofting.com", kVstMaxVendorStrLen);
    return true;
}

//-----------------------------------------------------------------------------------------
VstInt32 Vst2Plugin::canDo(char* text)
{
    if (!strcmp (text, "receiveVstEvents"))
        return 1;
    if (!strcmp (text, "receiveVstMidiEvent"))
        return 1;
    if (!strcmp (text, "receiveVstTimeInfo"))
        return 1;
	 //if (!strcmp (text, "sizeWindow")) this is a Host canDo
  //      return 1;

	return -1;
}

//-----------------------------------------------------------------------------------------
VstInt32 Vst2Plugin::getNumMidiInputChannels()
{
    return 1;
}

//-----------------------------------------------------------------------------------------
VstInt32 Vst2Plugin::getNumMidiOutputChannels()
{
    return 0;
}

//-----------------------------------------------------------------------------------------
bool Vst2Plugin::getOutputProperties(VstInt32 index, VstPinProperties* properties)
{
    if (index < 2) 
    {
        properties->flags = kVstPinIsStereo|kVstPinIsActive;
        strcpy (properties->label, m_pluginProperties.mixerLabel);
        return true;
    }
    else
        if (index < 6)
        {
            properties->flags = kVstPinIsActive;
            strcpy (properties->label, m_pluginProperties.mixerLabel);
            return true;
        }
        else
            return false;
}

void Vst2Plugin::open()
{
}

VstInt32 Vst2Plugin::startProcess()
{
    return 0;
}

VstInt32 Vst2Plugin::stopProcess()
{
    return 0;
}

//-----------------------------------------------------------------------------------------
void Vst2Plugin::suspend()
{
    m_dspEngine.suspend();
}

void Vst2Plugin::resume()
{
    if (m_initialising)
    {
        // First resume after creating, opening and initialising.
        //// Only now are we guaranteed that the sample rate and block size have been set, so now we can initialise the
        //// DSP code.
        //m_synth.initProcess(); Commented out because we actually need to initialise before the first resume, otherwise
        //                       the synth object initialisation code would need to be split between object construction
        //                       and later initialisation, which is inconvenient.
        m_initialising = false;
    }
    wantEvents();
    m_dspEngine.resume();
}

void Vst2Plugin::setSampleRate(float sampleRate)
{
    AudioEffectX::setSampleRate(sampleRate);
    m_dspEngine.setSampleRate(sampleRate);
}

void Vst2Plugin::setBlockSize(VstInt32 blockSize)
{
    AudioEffectX::setBlockSize(blockSize);
}

void Vst2Plugin::setProgram(VstInt32 program)
{
    m_core.setProgramFromHost(program);
}

void Vst2Plugin::setProgramName(char *name)
{
    //std::string nameStr(name);
    /// @todo Decide whether or not to implement this. Do we want the host to set the program name? We have it in the YAML anyway.
}

void Vst2Plugin::getProgramName(char *name)
{
    vstStringCopy(m_presetManager.getCurrentProgramName(), name);
}

void Vst2Plugin::setParameter(VstInt32 index, float value)
{
    m_core.setParameterFromHost(index, value);
}

void Vst2Plugin::setParameterAutomated(VstInt32 index, float value)
{
    // Copy part of the implementation from the VST SDK.
    // We do not want the SDK's default behaviour, because that also calls
    // setParameter(), which will end up being indistinguishable from host
    // automation. We want to control our response to setParameter more finely.
    if (audioMaster)
    {
        audioMaster (&cEffect, audioMasterAutomate, index, 0, 0, value);    // value is in opt
    }
}


float Vst2Plugin::getParameter(VstInt32 index)
{
    return m_presetManager.getCurrentProgram()->getParameterValue(index);
}

void Vst2Plugin::getParameterLabel(VstInt32 index, char *label)
{
    label[0] = 0;
    if (m_parameters.hasLabel(index))
    {
        if (m_parameters.labelIsDynamic(index))
        {
            m_dspEngine.getParameterLabel(index, label, kVstMaxParamStrLen);
        }
        else
        {
            vstStringCopy(m_parameters.getLabel(index).c_str(), label);
        }
    }
}

void Vst2Plugin::getParameterDisplay(VstInt32 index, char *text)
{
    m_dspEngine.getParameterDisplay(index, text, kVstMaxParamStrLen);
}

void Vst2Plugin::getParameterName(VstInt32 index, char *text)
{
    assert(index < m_pluginProperties.numParams);
    vstStringCopy(m_parameters.getShortName(index), text);
}

bool Vst2Plugin::getProgramNameIndexed(VstInt32 category, VstInt32 index, char *text)
{
    vstStringCopy(m_presetManager.getCurrentProgramName(), text);
    return true;
}

bool Vst2Plugin::copyProgram(VstInt32 destination)
{
    // copyProgram is deprecated, we do not support it.
    return false;
}

VstInt32 Vst2Plugin::getChunk(void **data, bool isSingleProgram)
{
    return m_core.getChunk(data, isSingleProgram);
}

VstInt32 Vst2Plugin::setChunk(void *data, VstInt32 byteSize, bool isSingleProgram)
{
    return m_core.setChunk(data, byteSize, isSingleProgram);
}

void Vst2Plugin::processReplacing (float **inputs, float **outputs, VstInt32 sampleFrames)
{
    VstTimeInfo *vsti = getTimeInfo(kVstTransportChanged | kVstTransportPlaying);

    if (vsti && (vsti->flags & kVstTransportChanged || vsti->flags & kVstTransportPlaying))
    {
        vsti = getTimeInfo(kVstTempoValid | kVstPpqPosValid);
        m_dspEngine.processReplacing(inputs, outputs, sampleFrames, vsti->ppqPos, vsti->tempo);
    }
    else
    {
        m_dspEngine.processReplacing(inputs, outputs, sampleFrames);
    }
}

VstInt32 Vst2Plugin::processEvents (VstEvents* ev)
{
    return m_dspEngine.processEvents(ev);
}


//==============================================================================
//
// VST editor implementation
//
VstEditor::VstEditor(EditorCore& notifyEditor)
 : AEffEditor(*this),
   m_notifyEditor(notifyEditor)
{
    m_rect.left   = 0;
    m_rect.top    = 0;
    m_rect.right  = m_notifyEditor.getWidth();
    m_rect.bottom = m_notifyEditor.getHeight();
}

VstEditor::~VstEditor()
{
}

bool VstEditor::getRect(ERect** rect)
{
    m_rect.right  = m_notifyEditor.getWidth();
    m_rect.bottom = m_notifyEditor.getHeight();
    *rect = &m_rect;
    return true; // was  return false;
}

bool VstEditor::open(void* ptr)
{
    AEffEditor::open(ptr);
    return m_notifyEditor.open(ptr);
}

void VstEditor::close()
{
    m_notifyEditor.close();
    AEffEditor::close(); 
}

bool VstEditor::isOpen()
{
    return AEffEditor::isOpen();
}

void VstEditor::idle()
{
    m_notifyEditor.idle(); 
}
