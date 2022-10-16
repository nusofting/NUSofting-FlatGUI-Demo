//
//  DspEngine.cpp
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "DspEngine.h"

#include "vst2.4/public.sdk/source/vst2.x/aeffectx.h"


DspEngine::DspEngine()
{
	isProcessing = false;
}

DspEngine::~DspEngine()
{
}

//-----------------------------------------------------------------------------------------
void DspEngine::connectToCore(DspToPluginCoreInterface& plugin, DspToEditorInterface& editor)
{
	m_notifyPluginCore = &plugin;
	m_notifyEditor = &editor;
}

//-----------------------------------------------------------------------------------------
void DspEngine::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames, double posPPQ, double tempoBPM)
{
	isProcessing = true;
	fBPM = tempoBPM;
	fPPQ = posPPQ;
	DoProcess(inputs, outputs, sampleFrames);
	isProcessing = false;
}

//-----------------------------------------------------------------------------------------
void DspEngine::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames)
{


	// The framework guarantees that initialisation and first program load is complete *before* the host even receives
	// the newly created plugin instance. Therefore, we can just directly call the DSP processing code without checking
	// that we are still loading, because we know we have finished loading.
	DoProcess(inputs, outputs, sampleFrames);
	// Reset VSTEvents
	VSTEventsNow = 0;
	VSTEventsCounter = -1;
	VSTEvents = NULL;
	isProcessing = false;
}

// ----------------------------------------------------------
void DspEngine::globalInit(void)
{
	LoadingPreset = false;
	MidiLearn = -1;
	fGlideAuto = 0;
	fRegVolume = 1;
	fVolumeT = 0.5f;
	fMaxVoices = 1;
	Samplerate = 44100.0; // Sample rate cannot be definitely known until the host sets it, which is after this point.
	wTiming = int((Samplerate*60)*30);
	Signal = 0;
	demoPop = false;
	bIdle = true;
	counterdm = 0;
	fVolPedal = 1;
	VSTEventsNow = 0;
	VSTEventsCounter = -1;
	VSTEvents = NULL;
	fPPQ = 0.0;
	fBPM = 120.0;
	isProcessing = false;
	//fVoices = kVoices-1;
}

//-----------------------------------------------------------------------------------------
void DspEngine::setSampleRate(float sampleRate)
{
	if (sampleRate != Samplerate)
	{
		Samplerate = sampleRate;
		handleSampleRateChanged();
	}
}

//-----------------------------------------------------------------------------------------
VstInt32 DspEngine::processEvents(VstEvents* ev)
{
	if (counterdm >= wTiming) return 1;

	if(!isProcessing && ev)
	{
		if (ev->numEvents <= 0)
		{
			VSTEvents = NULL;
			VSTEventsNow =  0;
			VSTEventsCounter = -1;
		} else {
			VSTEvents = ev;
			VSTEventsNow = ev->numEvents;
			VSTEventsCounter = 0;
		}

		onStartRenderingNextBlock();
	}

	return 1;
}

//-----------------------------------------------------------------------------------------
void DspEngine::EventsTick(VstInt32 sampleFrame) // read one sample position at time
{
	if(VSTEvents == NULL) return;

	if (VSTEventsCounter == -1 || VSTEventsNow == 0 || VSTEventsCounter >= VSTEventsNow ) return;

	if ( (VSTEvents->events[VSTEventsCounter])->deltaFrames != 0
		&& sampleFrame < (VSTEvents->events[VSTEventsCounter])->deltaFrames ) return;

	while (true) 
	{
		if ((VSTEvents->events[VSTEventsCounter])->deltaFrames == 0 // zero is only for no MIDI sequencing from DAW
			|| sampleFrame == (VSTEvents->events[VSTEventsCounter])->deltaFrames)
		{
			if ((VSTEvents->events[VSTEventsCounter])->type != kVstMidiType) 
			{
				VSTEventsCounter++; // skip other VST events 
			} 
			else  // then trigger the MIDI event in "sample sync"
			{
				VstMidiEvent*  event = (VstMidiEvent*)VSTEvents->events[VSTEventsCounter];
				char* midiData = NULL;
				if(event != NULL)  midiData = event->midiData;
				if(midiData != NULL)  EventsTickSynth(midiData[0], midiData[1], midiData[2]);
			}      

			VSTEventsCounter++;
		}

		if (VSTEventsCounter >= VSTEventsNow) break;
		if ((VSTEvents->events[VSTEventsCounter])->deltaFrames != 0
			&& sampleFrame < (VSTEvents->events[VSTEventsCounter])->deltaFrames) break;
	}  // END LOOP //      
}
