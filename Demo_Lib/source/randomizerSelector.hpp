/*-----------------------------------------------------------------------------

© 2020 NUSofting

Written by Luigi Felici
Copyright (c) 2020 NUSofting

-----------------------------------------------------------------------------*/

#pragma once

#include "PluginDef.h"


static void snapAndClamp(float& value)
{
	value = (value < 0.000001f)? 0.0f : (value > 1.0f)? 1.0f : value;
}


static bool selectParam(size_t index) // allowed or not
{
	switch(index)
	{
	case kPre_Gain: return false;
	case kActive1: return false;
	case kActive2: return false;
	case kPan1: return false;
	case kPan2: return false;
	case kLinkLPHPs: return false;
	case kLinkFeeds: return false;
	case kLinkLevels:return false;
	case kSync1:return false;
	case kFeedback1:return false;
	case kLevel1:return false;
	case kSync2:return false;
	case kFeedback2:return false;
	case kLevel2:return false;
	default: return true;
	}

	return true;
}

static float softParam(size_t index, float value) //scaled
{
	snapAndClamp(value);

	switch(index)
	{
	case kDelayTime1:   return 0.55f*value*value;
	case kDelayTime2:   return 0.55f*value;
	case kDepth1:		return 0.8f*value;
	case kDepth2:		return 0.8f*value;
	case kTimeMod1:		return 0.4f+0.2f*value;
	case kTimeMod2:		return 0.4f+0.2f*value;
	case kDiv1:			return 0.8f*value;
	case kDiv2:			return 0.8f*value;

	case kLP1:		 return 0.2f+0.8f*value;
	case kHP1:		 return 0.8f*value;
	case kLP2:		 return 0.2f+0.8f*value;
	case kHP2:		 return 0.8f*value;
	case kCrossfeed: return 0.8f*value;	
	case kDry_Gain:  return 0.4f+0.5f*value;

	case kRate1:		 return 0.14f+0.25f*value;
	case kRate2:		 return 0.14f+0.25f*value;
	case kPitch1:		 return 0.4f+0.2f*value;
	case kPitch2:		 return 0.4f+0.2f*value;
	

	default: return value;
	}

	return value;
}

