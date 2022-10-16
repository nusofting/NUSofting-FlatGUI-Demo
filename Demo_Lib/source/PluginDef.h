/*-----------------------------------------------------------------------------

© 2022 NUSofting

-----------------------------------------------------------------------------*/

#pragma once

#define VstSynthName "demoLibVST"      // SynthName
/** Warning: the name of the product is demoLibVST 
*/

/// @todo Build into the framework a way for the about box (which is now the main thing remaining that uses VstSynthName
/// and VstSynthVersion) to get the name and version from the plugin properties struct.
#if !defined(VstSynthVersionNum)
#define VstSynthVersionNum "1.0.0" 
#endif

#if DEMO
#define VstSynthVersion "Version DEMO " VstSynthVersionNum
#else
#define VstSynthVersion "Version " VstSynthVersionNum
#endif

#define VstSynthCopyright "Copyright \xC2\xA9 2022 NUSofting All Rights Reserved"
#define VstSynthInfoFileName "Docs+PNGs/demoLibVST.html"

// to be fixed
#define VstSynthDefaultSkinIndex 3.0f
#define VstSynthDefaultAppearance "pure"
// currently hard coded in
   // const float kDefaultSkinIndex = 3.0f; // zoom
   // const char *kDefaultAppearanceFileName = "pure";

#define BETA 0
#define DEMO_time 19.0 // minutes
static float Repeat_seconds = 34.0; // between the spoken "Demo" outputs

enum
{
	// Parameters Tags
	kPre_Gain = 0,
	kMixChannels,
	kDelayTime1,
	kSync1,
	kActive1,
	kRate1,
	kDepth1,
	kDiv1,
	kFeedback1,
	kLP1,
	kHP1,
	kPitch1,
	kPhase1,
	kLevel1,
	kPan1,
	kDelayTime2,
	kSync2,
	kActive2,
	kRate2,
	kDepth2,
	kDiv2,
	kFeedback2,
	kLP2,
	kHP2,
	kPitch2,
	kPhase2,
	kLevel2,
	kPan2,
	kLinkLPHPs,
	kLinkFeeds,
	kLinkLevels,
	kCrossfeed,
	kDry_Gain,
	kTapeComp,
	kDrift,
	kTimeMod1,
	kTimeMod2,
	kTriplets1,
	kTriplets2,
	kNumHostParams, // Number of parameters known to the host. The remaining parameters are internal.
	kNumParams = kNumHostParams // Number of parameters including internal parameters.
};

/// Enum defining indexes and index ranges into the pool of floating point values (dangerously) shared between the DSP
/// engine and the editor. Dangerous in the sense that the shared float pool is an array of floats passed to both the
/// DSP engine and editor by a simple pointer, and there is no bounds checking or thread synchronisation governing read
/// or write access to this pool.
enum SharedFloatPoolIndexes
{
    kSFPI_WaveStart = 0,  // Just an example only - not yet used
    kSFPI_WaveEnd = 127,  // Just an example only - not yet used

    kSFPI_NumFloats = kSFPI_WaveEnd + 1 // Must be last and must be the last index value + 1
};
