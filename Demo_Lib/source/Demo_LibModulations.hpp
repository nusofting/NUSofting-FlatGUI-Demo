
//Modulation sources and targets defined for Echo_Grain synth


enum ModSourceIDs 
{
    fromNone,
    fromLfo1,
    fromModWheel,
    fromAftertouch,
    fromKeyTrack
};
enum ModTargetIDs 
{								  // Menu Labels:
	send2none,					  // off	
	send2bits,					  // "bits"                    
	send2noise,  				  // "noise"  
	send2amplitude,				  // "amplitude"
	send2lags,					  // "lags"                
	send2fdbk_amnt,				  // "fdbk amnt"   
	send2BR_Hz,					  // "BR Hz"                    
	send2LFO_depth,				  // "LFO depth"                   
	send2LFO_speed,				  // "LFO speed"                   
	send2pitch_pos,				  // "pitch +"                  
	send2pitch_neg,				  // "pitch -"                   
	send2pitch,					  // "pitch"                  
	send2pitchPLUSamplitude,      // "pitch+amplitude"           
	send2harmonicsPLUSamplitude,  // "harmonics+amplitude"
	send2noisePLUSamplitude,	  // "noise+amplitude"
	send2pan 					  // "pan"
};	

static const int iModSources = 5; 
static const int iModTargets = 16;
