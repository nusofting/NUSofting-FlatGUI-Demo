//
// MeteringData.h
//
// Copyright 2016 Bernie Maier & Luigi Felici. All rights reserved.
// Licensed to NUSofting.
//


#pragma once

/// Encapsulates DSP engine data that can be display directly by the editor. The editor has a read-only view of this
/// data and is not permitted to modify it. This is mainly useful for features such as a waveform viewer and level
/// meters, for which it is unnecessary to have to pass through the core.
class MeteringData
{
public:
    MeteringData();
    ~MeteringData() {}

    /// Functions that the DSP engine calls to update the display data.
    /// @{
    void update(float in1, float in2, char MIDI_AT = 0, char MIDI_CC = 0, int MIDI_PB = 0);
    void clear();
    /// @}

    /// Functions that the editor calls to retrieve the display data.
    /// @{

    /// Polls for current VU meter levels. These aren't constantly available (the data is analysed in chunks) and so
    /// this function encapsulates the action of querying whether level data is available and, if so, returning that
    /// data.
    /// @return
    ///		Returns true if level data is available and the passed level variables have been updated.
	bool pollLevels(float &vu_cd1, float &vu_cd2) const;
	bool pollMIDIData(char& MIDI_AT, char& MIDI_CC, int& MIDI_PB) const;

	const float* getWaveData(int& atSample) const;

    /// @}

	enum { bufferLength = 4096 };

private:
	float vu_ch1;
	float vu_ch2;
	float fMaxLevel1;
	float fMaxLevel2;

	char MIDI_AT_;
	char MIDI_CC_;
	int  MIDI_PB_;

	/// This is mutable to allow it to be updated when the editor (which only sees a const view of this) successfully
	/// polls for levels.
	mutable bool bMaxLevelReady;
	mutable bool bMIDIDataReady;

	float filledData[bufferLength];
	float insMergePrev;
	int sampleOuts;
	int counter;
};
		
