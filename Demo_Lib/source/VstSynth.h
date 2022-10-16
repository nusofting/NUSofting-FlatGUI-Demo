
#pragma warning (disable:4244) //conversion from 'double' to 'float', conversion from 'int' to 'float', possible loss of data

/*-----------------------------------------------------------------------------

© 2015 NUSofting

-----------------------------------------------------------------------------*/

#ifndef __VstSynth__
#define __VstSynth__
#define CLAMP(x,a,b) ( ((x)<(a))? (a) : ( ((x)>(b))? (b) : (x) ) );

#include "PluginDef.h"
// these includes are not needed, already in the Library

//#include <sys/stat.h>
//#include <float.h>
//#include <string.h>
//#include <complex>
//#include <stdio.h>
#include <algorithm>
#include <sys/types.h>
#include <assert.h>
#include <math.h>

#define masterLPsHz 10000.0

#include "../../Frameworks/Platform/Platform.h"

#include "../../Frameworks/Core/DspEngine.h"


#if DEMO
#include "../../DashLibrary/DemoRoutines.hpp"
#endif

/** Warning: the name of the product is demoLibVST
*/
#if !defined(MAC)
#ifdef _WIN64
#define DLL_NAME "demoLibVST.dll" 
#else
#define DLL_NAME "demoLibVST.dll"
#endif
#else
#define DLL_NAME "demoLibVST"
#endif


//#ifndef pi
//#define pi 3.1415926535897932384626433832795f
//#endif

class Parameters;

class MeteringData;

//------------------------------------------------------------------------------------------

class VstSynth : public DspEngine
{
public:
	VstSynth(MeteringData& meteringData);
	~VstSynth();
	virtual void defineParameters(Parameters &parameters);

	virtual void DoProcess (float **inputs, float **outputs, VstInt32 sampleFrames);
	virtual void EventsTickSynth(char midiStatus, char midiData1, char midiData2);
	virtual void setNote(int note, int velocity, bool Add); // Should this be in the interface or just in the synth?
	virtual void resume();
	virtual void suspend();
	virtual void handleNewProgram();
	virtual void setParameter(VstInt32 index, float value);
	virtual void getParameterLabel(VstInt32 index, char *label, size_t len);
	virtual void getParameterDisplay(VstInt32 index, char *text, size_t len);
	virtual void handleSampleRateChanged();
	virtual void onStartRenderingNextBlock();

	virtual void initProcess();

	virtual float DmVal(long index);


private:
	/// The DSP engine keeps a cache of the current parameter values for easy access.
	float parametersCache[kNumParams];

	/// The read-only view over DSP data that is displayed in the editor.
	MeteringData& m_meteringData;


	float acc;
	int idxAcc;
	
	float fPre_Gain;


};
#endif
