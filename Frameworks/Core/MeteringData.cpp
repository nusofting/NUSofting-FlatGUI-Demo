//
// MeteringData.h
//
// Copyright 2017 Bernie Maier & Luigi Felici. All rights reserved.
// Licensed to NUSofting.
//

#include "MeteringData.h"
#include "../FlatGUI/undenormalize_macros.hpp"


MeteringData::MeteringData() :
	vu_ch1(0.0f),
	vu_ch2(0.0f),
	fMaxLevel1(0.0f),
	fMaxLevel2(0.0f),
	bMaxLevelReady(false),
	bMIDIDataReady(false),
	sampleOuts(0),
	counter(0)
{
	for(int i = 0; i < bufferLength; i++)
		filledData[i] = 0.0f;

	clear();
}

// runs per sample in loop
void MeteringData::update(float in1,float in2, char MIDI_AT, char MIDI_CC, int MIDI_PB)
{
	if(sampleOuts >= bufferLength)
	{
		sampleOuts = 0;
	}
	const float k = 0.84f; // smoothing curve factor
	float insMerge = (1.0f-k)*(in1+in2) + k*insMergePrev;
	SNAPPING_ZERO(insMerge);
	insMergePrev = insMerge;

	filledData[sampleOuts] = insMerge; 
	++sampleOuts;

	if(bMaxLevelReady == false)
	{
		if(counter++ < 1024) // samples to analyze
		{
			if(in1 >= fMaxLevel1)
				fMaxLevel1 = in1;

			if(in2 >= fMaxLevel2)
				fMaxLevel2 = in2;
		}
		else
		{
			vu_ch1 = fMaxLevel1;
			vu_ch2 = fMaxLevel2;
			bMaxLevelReady = true;
			fMaxLevel1 = fMaxLevel2 = 0.0f;
			counter = 0;
		}
	}
	if(bMIDIDataReady == false && counter == 2)
	{

		MIDI_AT_ = MIDI_AT;
		MIDI_CC_ = MIDI_CC;
		MIDI_PB_ = MIDI_PB;
		bMIDIDataReady = true;
	}
}

void MeteringData::clear()
{
	MIDI_AT_ = 0;
	MIDI_CC_ = 0;
	MIDI_PB_ = 0;
	vu_ch1 = vu_ch2 = 0;
	insMergePrev = 0.0f; 
}

bool MeteringData::pollLevels(float &vu_cd1, float &vu_cd2) const
{
	bool gotLevels = bMaxLevelReady;
	if (gotLevels)
	{
		vu_cd1 = vu_ch1;
		vu_cd2 = vu_ch2;
		bMaxLevelReady = false;
	}
	return gotLevels;
}

bool MeteringData::pollMIDIData(char& MIDI_AT, char& MIDI_CC, int& MIDI_PB) const
{
	bool gotLevels = bMIDIDataReady;
	if (gotLevels)
	{
		MIDI_AT = MIDI_AT_;
		MIDI_CC = MIDI_CC_;
		MIDI_PB = MIDI_PB_;
		bMIDIDataReady = false;
	}
	return gotLevels;
}

const float* MeteringData::getWaveData(int& atSample) const
{
	atSample = sampleOuts;
	return filledData;
}