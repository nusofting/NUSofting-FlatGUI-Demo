//
//  DspEngine.h
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#pragma once

typedef int VstInt32; /// @todo Get rid of this from the framework's interfaces
struct VstEvents;


class Parameters;

/// Abstract base class defining the functions the DSP code can use to communicate with the plugin core. The DSP code
/// must not communicate with the core via any other way (including VST). Anything that the DSP needs to communicate
/// that isn't here needs to be added here in some form.
class DspToPluginCoreInterface
{
public:
    virtual void setProgram(size_t programNum) = 0;
    virtual void reloadCurrentProgram() = 0;
};

/// Abstract base class defining the functions the DSP code can use to communicate with the editor. The DSP code must
/// not communicate with the editor via any other way (including VST). Anything that the DSP needs to communicate
/// that isn't here needs to be added here in some form.
class DspToEditorInterface
{
public:
    virtual void postDisplayParameterInt(int msgIndex, int value) = 0;
};

/// Abstract base class defining the functions that must be implemented by the synth or effect-specific DSP class
/// (the class that was typically named VstSynth in the plugins based on the Dash library).
class DspInterface
{
public:
    virtual void defineParameters(Parameters &parameters) = 0;
    virtual void initProcess() = 0;
    virtual void resume() = 0;
    virtual void suspend() = 0;

    /// Informs the DSP code that all the parameters are about to be set due to a new program being set.
    /// This is useful for any DSP code that could react badly (i.e. with noise) to sudden parameter changes, and
    /// allows the option to reset the DSP code (e.g. quieten voices) before all the parameter changes come.
    virtual void handleNewProgram() = 0;

    virtual void setParameter(VstInt32 index, float value) = 0;
    virtual void getParameterLabel(VstInt32 index, char *label, size_t len) = 0;
    virtual void getParameterDisplay(VstInt32 index, char *text, size_t len) = 0;

protected:
    virtual void DoProcess(float **inputs, float **outputs, VstInt32 sampleFrames) = 0;
    virtual void onStartRenderingNextBlock() = 0;
    virtual void EventsTickSynth(char midiStatus, char midiData1, char midiData2) = 0;
    virtual void setNote(int note, int velocity, bool Add) = 0; // Should this be in the interface or just in the synth?
    virtual float DmVal(long index) = 0;
    virtual void handleSampleRateChanged() = 0;
};

/// Base class for the DSP code that implements the basic event processing common to all synth and effect plugins.
class DspEngine : public DspInterface
{
public:
    DspEngine();
    virtual ~DspEngine();

    /// Functions called by the plugin core.
    /// @{
    void connectToCore(DspToPluginCoreInterface& plugin, DspToEditorInterface& editor);
    VstInt32 processEvents(VstEvents* events);
    void processReplacing(float **inputs, float **outputs, VstInt32 sampleframes, double posPPQ, double tempoBPM);
    void processReplacing(float **inputs, float **outputs, VstInt32 sampleframes);

    void globalInit(void);
    void setSampleRate(float sampleRate);
    /// @}

    /// Functions called by the DSP subclass (VstSynth etc.).
    /// @{
    virtual void EventsTick(VstInt32 sampleFrame = 0);

protected:

    /// @{
    /// @todo Audit these member variables. Are they all still needed? Which should be private? Most of these, if still
    ///       used, belong in the plugin-specific subclass.
    int MidiLearn;
    float* out1;
    float* out2;
    float* input1;
    float* input2;
    float Signal;
    long counterdm;
    long wTiming;
	float gainBuffer[4]; // for goOut
    bool goOut;
	int inputSampleCount;
    double Samplerate;
    float fMaxVoices;
    int   fVoices;
    float *Out;
    float fRegVolume;
    float pBend;
    bool  bIdle;
    bool  demoPop;
    float fGlideAuto;
    float fVolPedal;
    float fBend;
    float fVolType;
    float fVolumeT;
    float fVol;
    bool LoadingPreset;
    double fPPQ;
    double fBPM;
	bool isProcessing;
	// @}

    DspToPluginCoreInterface* m_notifyPluginCore;
    DspToEditorInterface* m_notifyEditor;

private:

    VstEvents *VSTEvents;
    VstInt32 VSTEventsNow;	
	VstInt32 VSTEventsCounter;	
	//char* midiData;	
};
