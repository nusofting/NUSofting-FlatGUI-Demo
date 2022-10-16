/*-----------------------------------------------------------------------------

(c) 2020 - nusofting.com - Written by Luigi Felici

-----------------------------------------------------------------------------*/

#ifndef __VstSynth__
#include "VstSynth.h"
#endif

#include "../../Frameworks/Core/Parameters.h"
#include "../../Frameworks/Core/Modulations.h" // shouldn't this go into VstSynth.h ?

#undef min
#undef max
#include <algorithm>

extern bool oome;

void VstSynth::defineParameters(Parameters &parameters)
{
	parameters.define(kPre_Gain,"input", 0.786f);	
	parameters.define(kMixChannels, "mix", 0.5f);	
	parameters.define(kDelayTime1, "time 1", 1.0f/8.0f - 2.0f/96.0f);	
	parameters.define(kSync1, "sync 1", 1.0f);	
	parameters.define(kActive1, "Line 1 On", 1.0f);
	parameters.define(kRate1, "rate 1", 0.5f, "Hz");	
	parameters.define(kDepth1, "depth 1", 0.0f);
	parameters.define(kDiv1, "div 1", 0.0f);
	parameters.define(kFeedback1, "feed 1", 0.46f);
	parameters.define(kLP1, "LP1", 1.0f, "Hz");	
	parameters.define(kHP1,  "HP1", 0.1f, "Hz");
	parameters.define(kPitch1, "pitch 1", 0.5f, "st");
	parameters.define(kPhase1, "phase 1", 0.0f);
	parameters.define(kLevel1, "level 1", 0.5f);
	parameters.define(kPan1, "pan 1", 0.5f);	
	parameters.define(kDelayTime2, "time 2", 1.0f/8.0f - 2.0f/96.0f);	
	parameters.define(kSync2, "sync 2", 1.0f);	
	parameters.define(kActive2, "Line 2 On", 1.0f);
	parameters.define(kRate2, "rate 2", 0.5f, "Hz");
	parameters.define(kDepth2, "depth 2", 0.0f);
	parameters.define(kDiv2, "div 2", 0.0f);
	parameters.define(kFeedback2, "feed 2", 0.46f);
	parameters.define(kLP2, "LP2", 1.0f, "Hz");	
	parameters.define(kHP2,  "HP2", 0.1f, "Hz");
	parameters.define(kPitch2, "pitch 2", 0.5f, "st");
	parameters.define(kPhase2, "phase 2", 0.0f);
	parameters.define(kLevel2, "level 2", 0.5f);
	parameters.define(kPan2, "pan 2", 0.5f);	
	parameters.define(kLinkLPHPs, "link LPHPs", 0.0f);
	parameters.define(kLinkFeeds, "link Feeds", 0.0f);
	parameters.define(kLinkLevels, "link levels", 0.0f);
	parameters.define(kCrossfeed, "crossfeed", 0.0f);
	parameters.define(kDry_Gain, "dry", 0.8f);
	parameters.define(kTapeComp,"tape comp", 0.5f);
	parameters.define(kDrift,"ana drift", 0.0f);
	parameters.define(kTimeMod1,"env time 1", 0.5f);
	parameters.define(kTimeMod2,"env time 2", 0.5f);
	parameters.define(kTriplets1,"triplets 1", 0.0f);
	parameters.define(kTriplets2,"triplets 2", 0.0f);
}

//-----------------------------------------------------------------------------------------
//float VstSynth::BlendMixCalc(float value)
//{
//	const float var = pow(value,0.3333333f);
//	const float dBin = -100.0f*(1.0f-var)+5.0f*var;
//	float output = pow(10.0f,dBin/20.0f);
//	if(output <= 0.000011f) output = 0.0f; // max 1.7783f
//	return output;
//}
////-----------------------------------------------------------------------------------------
//float VstSynth::BlendMixConv(float is)
//{
//	const float var = pow(is,0.3333333f);
//	float exp = -100.0f*(1.0f-var)+5.0f*var;
//	return exp;
//}
//-----------------------------------------------------------------------------------------
void VstSynth::handleNewProgram()
{

}
//-----------------------------------------------------------------------------------------
void VstSynth::setParameter (VstInt32 index, float value)
{
	if (index < kNumParams) 
	{
		parametersCache[index] = value;	
	}

	//dBout = 20.0f*log(lin)/log(10.0f)
	//const float dBin = -52.0f*fCompAmnt;
	//const float lin = pow(10.0f,dBin/20.0f);// 1.0 to 0.0025118864315095801111 Thres for - 52 db

	// 0.5 = exp(log(10.0)*(-6.0/20.0)) dB to lin




}

float VstSynth::DmVal(long index)
{
	return parametersCache[index];
}

//-----------------------------------------------------------------------------------------
void VstSynth::getParameterLabel (VstInt32 index, char *label, size_t len)
{	
	*label = '\0';
}


#define MAKE_Hz(a,b,c)  a * pow(b*(1.0f/a), c)

//-----------------------------------------------------------------------------------------
void VstSynth::getParameterDisplay (VstInt32 index, char *text, size_t len)
{

	const float is = DmVal(index);

	switch(index)
	{
	case kLP1: sprintf(text, "%1.1f",MAKE_Hz(500.0f,20000.0f,is)); break;
	case kHP1: sprintf(text, "%1.1f",MAKE_Hz(20.0f,6000.0f,is)); break;
	case kLP2: sprintf(text, "%1.1f",MAKE_Hz(500.0f,20000.0f,is)); break;
	case kHP2: sprintf(text, "%1.1f",MAKE_Hz(20.0f,6000.0f,is)); break;


	case kDiv1 : sprintf(text, "%d",1 + int(is*5.0f+0.5f)); break;
	case kDiv2 : sprintf(text, "%d",1 + int(is*5.0f+0.5f)); break;

	case kPan1: sprintf(text, "%1.2f", fabs(2.0f*is-1.0f)); break;
	case kPan2: sprintf(text, "%1.2f", fabs(2.0f*is-1.0f)); break;
	case kTimeMod1: 
	case kTimeMod2: sprintf(text, "%s", fabs(is-0.5f) <= 0.016f? "0" : is > 0.516f? "bend up" : "bend down");  break;
	case kDepth1:
	case kDepth2: sprintf(text, "%s", is < 0.01? "0" : "amp mod");   break;
	default: sprintf(text, "%1.2f", is); 
	}
}

//-----------------------------------------------------------------------------------------
void VstSynth::handleSampleRateChanged()
{

	// Reload the current program so that any sample-rate dependent code for each parameter is re-run.
	m_notifyPluginCore->reloadCurrentProgram();	

}
