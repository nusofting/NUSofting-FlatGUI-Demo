//#define CLAMP(x,a,b) ( ((x)<(a))? (a) : ( ((x)>(b))? (b) : (x) ) );

/*-----------------------------------------------------------------------------

(c) 2015 -  - Liqih 
(c) 2020 - nusofting.com - Written by Luigi Felici

-----------------------------------------------------------------------------*/

#ifndef __VstSynth__
#include "VstSynth.h"
#endif

#include "../../Frameworks/Core/MeteringData.h"
#include "../../Frameworks/Core/Modulations.h" // shouldn't this go into VstSynth.h ?


//int VstSynth::m_param_to_cc_map[kNumParams];

enum
{
	kNumFrequencies = 128,	// 128 midi notes
};


static float freqtab[kNumFrequencies];

//-----------------------------------------------------------------------------------------
VstSynth::VstSynth (MeteringData& meteringData)
	: DspEngine(),
	m_meteringData(meteringData)
{
}

//-----------------------------------------------------------------------------------------
void VstSynth::initProcess ()
{
	//Out = NULL;
	//if(Out == NULL){
	//	Out = new float[2];
	//	Out[0] = Out[1] = 0;
	//}


	// make frequency (Hz) table
	double k = 1.059463094359;	// 12th root of 2
	double a = 6.875;	// a
	a *= k;	// b
	a *= k;	// bb
	a *= k;	// c, frequency of midi note 0
	for (int i = 0; i < kNumFrequencies; i++)	// 128 midi notes
	{
		freqtab[i] = (float)a;
		a *= k;
	}

	//////*****//////

	if(Samplerate < 22050.0) Samplerate = 22050.0;

	// set values
	const float sr = Samplerate;





	for (int i = 0; i < 4; ++i)	
	{
		gainBuffer[i] = 0.0f;
	}
	goOut = false; // Used to check for AudioOut or in
	inputSampleCount = 0;

	fPre_Gain = 0.0f;
}

//-----------------------------------------------------------------------------------------
VstSynth::~VstSynth ()
{

}	
//-----------------------------------------------------------------------------------------
void VstSynth::DoProcess (float **inputs, float **outputs, VstInt32 sampleFrames)
{


	float* in1 = inputs[0];
	float* in2 = inputs[1];
	float* out1 = outputs[0];
	float* out2 = outputs[1];




	float Out[2] = {0};
	Out[0] = 0.0f;
	Out[1] = 0.0f;

	for (long samples=0; samples < sampleFrames; ++samples)
	{	


		EventsTick(samples);



		Out[0] = (*in1++); 
		Out[1] = (*in2++); 



			float meterSign1 = Out[0];
			float meterSign2 = Out[1];

			m_meteringData.update(meterSign1,meterSign2);
		
#ifdef _DEBUG
		//checkFloat(Out[0], 1);
		//checkFloat(Out[1], 2);
#endif

		(*out1++) = Out[0];
		(*out2++) = Out[1];
	}
}

//-----------------------------------------------------------------------------------------
void VstSynth :: onStartRenderingNextBlock() 
{

}

//-----------------------------------------------------------------------------------------
void VstSynth::EventsTickSynth(char midiStatus, char midiData1, char midiData2)
{
	long note;
	long velocity;
	long ni = 0;
	long notexi = 0;

	long status = midiStatus & 0xf0;
	float value = (float)((float)(midiData2 & 0x7f)/127);
	float value2 = (float)((float)(midiData1 & 0x7f)/127);
	//	bIdle = false;
	//	IdleCounter = 0;

	// E0 = Pitch Bend
	if (status == 0xE0)
	{
		//const char lsb = midiData1; /* LSB */
		//const char msb = midiData2; /* MSB */

		//float pitch=(float)(msb*128+lsb);

		//if(pitch>8192+3)
		//{
		//	float p=pitch/8192 - 1;
		//	pitch=p*(float)pow(1.059463094359,m_max_pitchBd)+1-p;
		//}
		//else if(pitch<8192-3)
		//{
		//	float p=(8192-pitch)/8192;
		//	pitch=1/(p*(float)pow(1.059463094359,m_max_pitchBd)+1-p);
		//}
		//else
		//	pitch=1;

		////m_pitch_bend = 1.0f/pitch;
		//m_pitch_bend = pitch;
	}
	else if (status == 0xD0) //monoAT
	{ 
		//m_modulations->inputFromSource(fromAftertouch, NoZero(value2)); // AT output to selected target jack slot
	}
	else if (status == 0xA0) //polyAT
	{
		//m_modulations->inputFromSource(fromAftertouch, NoZero(value)); // AT output to selected target jack slot

		//long note = midiData1 & 0x7f;
		//long value = midiData2 & 0x7f;
		//RecAT[note] = value;
	}

	else if (status == 0xC0) // Program Change
	{
		m_notifyPluginCore->setProgram(midiData1);
	}
	//B0 = MIDI CCs
	else if (status == 0xB0) {

		//if (midiData1 == 1)
		//	m_modulations->inputFromSource(fromModWheel, NoZero(value));

		if (midiData1 == 11) { fVolPedal = value; }
		if (midiData1 == 0x7B ||midiData1 == 0x78)
		{	// all notes off //
		}

		if (midiData1 == 0x42 ||midiData1 == 0x40)
		{
			//bSusPedal = value > 0.5f;

			//for (int j = 0; j < fVoices; j++)	
			//{
			//	env1[j].setSusPedal(bSusPedal);  env2[j].setSusPedal(bSusPedal);
			//}
		}

		if (MidiLearn != -1) {
			// MIDI learn is not implemented yet in the new framework, but we no longer
			// have the necessary infrastructure in the old framework to handle MIDI learn.
			assert(0);
		}

	}
	else if (status == 0x90 || status == 0x80)	// ignoring channel we only look at notes
	{
		note = midiData1 & 0x7f;
		velocity = midiData2 & 0x7f;
		if (status == 0x80 || velocity == 0)
		{

			setNote(note,velocity,false); 

		} else {
			//if(RecAT[note] != -1 && noChannelAT) // if status == 0xA0 was used
			//{
			//	//if(fAT1 && fAftertouch < 1.5f)
			//	//	fAftertouch = (float)((float)RecAT[note]/127) * 1.4f*fAT1;// DATA BYTE 2
			//	//else
			//	//	fAftertouch = 0;
			//}

			goOut = true; // Used to check for AudioOut
			setNote(note,velocity,true);
		}			
	}
}

//-----------------------------------------------------------------------------------------
void VstSynth::setNote(int note, int velocity, bool Add)
{
	goOut = true; // Used to check for AudioOut
}

//-----------------------------------------------------------------------------------------
void VstSynth::resume ()
{
	// The framework guarantees that initialisation and first program load is complete *before* the host even receives
	// the newly created plugin instance. Therefore, resume without checking that we are still loading, because we know
	// we have finished loading.


	m_meteringData.clear();


}
//-----------------------------------------------------------------------------------------
void VstSynth::suspend ()
{
	m_meteringData.clear();
}
