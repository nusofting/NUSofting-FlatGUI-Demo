//
// Demo_LibPlugin.cpp
//
// Copyright 2016 Bernie Maier & Luigi Felici. All rights reserved.
// Licensed to NUSofting.
//

#include "PluginDef.h"

#include "VstSynth.h"
#include "__myeditor__.h"

#include "Core/MeteringData.h"
#include "Core/Modulations.h"
#include "Core/PluginCore.h"
#include "Core/PluginFactory.h"
#include "AppearanceManager/AppearanceFactory.h"
#include "AppearanceManager/AppearanceManager.h"
#include "PresetManager/Program.h"

#include "FlatGUI/AppearanceController.h"
#include "FlatGUI/PreferencesRegistry.h"

#include "randomizerSelector.hpp"
static size_t kRandomizer = 11111111; // recall

/// The Demo_Lib-specific implementation subclass of the plugin core.
class Demo_LibPlugin : public PluginCore
{
public:
	Demo_LibPlugin(const PluginProperties& pluginProps,
	               Parameters& parameters,
	               MeteringData& meteringData,
	               VstSynth& dsp,
	               PluginFactory* factory,
	               float* sharedFloatPool);
	virtual ~Demo_LibPlugin();

	virtual EditorCore* getEditor();

	/// Notify parameter change from editor to core.
	virtual void changedDisplayParameterInt(int msgIndex, int value)
	{
		if(msgIndex == kRandomizer) // from Toolbar
		{
			for(size_t index = 0; index < kNumParams; ++index)
			{
				if(selectParam(index)) // do or do not
				{
					const float cf = value/1000.0f; // value from CRandWidget (new slider kRandomizer)

					static const float fix = 1.0f/(double(RAND_MAX)+1.0f);
					const float pingParam = float((double)rand()*fix);
				
					const float mixParam = (1.0-cf)*  m_synth.DmVal(index) + cf*pingParam;  // blends recursively with current param values
					// Note : DmVal(index) is now "public", it was protected
					float newRandomizedValue = softParam(index, mixParam);

					// Mistery code
					paramValueChanged(index, newRandomizedValue);

				}
			}		
		}
	}

private:
	void defineModulations();
	AppearanceFactory m_appearanceFactory;
	AppearanceManager m_appearanceManager;
	PreferencesRegistry m_prefs;
	AppearanceController m_pluginAppearance;
	EditorConfiguration m_editorConfig;
	ZoomableEditorController m_editorController;
	Modulations m_modulations;
	MeteringData& m_meteringData;
	VstSynth& m_synth;
};


/// The Demo_Lib-specific implementation subclass of the factory the creates and owns the major plugin-specific objects.
class Demo_LibPluginFactory : public PluginFactory
{
public:
	Demo_LibPluginFactory();
	virtual ~Demo_LibPluginFactory();
	virtual DspEngine& getDspEngine();
	virtual PluginCore* getPluginInstance();

private:
	MeteringData m_meteringData;
	/// Pool of floating point values (dangerously) shared between the DSP engine and the editor. Dangerous in the sense
	/// that this is an array of floats passed to both the DSP engine and editor by a simple pointer, and there is no
	/// bounds checking or thread synchronisation governing read or write access to this pool. So only index into this
	/// via the index ranges defined in the SharedFloatPoolIndexes enum defined in PluginDef.h.
	float *m_sharedFloatPool;
	VstSynth m_synth; // We can create this in-place because it has no initial dependencies.
	Demo_LibPlugin* m_plugin; // We have a pointer to this because the parameters have to be defined before we can construct it.
};


//==============================================================================

PluginFactory* PluginFactory::create()
{
	return new Demo_LibPluginFactory;
}

void PluginFactory::destroy(PluginFactory* factory)
{
	delete factory;
}


//==============================================================================

Demo_LibPluginFactory::Demo_LibPluginFactory()
 : m_meteringData(),
   m_sharedFloatPool(new float[kSFPI_NumFloats]),
   m_synth(m_meteringData),
   m_plugin(0)
{
	memset(&m_pluginProperties, 0, sizeof m_pluginProperties); // Initialise all members to zero.
	m_pluginProperties.numPrograms = 1; // Only report 1 program to the VST host because we do our own program management.
	m_pluginProperties.numParams = kNumParams;
	m_pluginProperties.numInputs = 2;
	m_pluginProperties.numOutputs = 2;
	m_pluginProperties.pluginID = 'NliB';
	m_pluginProperties.name = "Demo Lib by NUSofting";
	m_pluginProperties.nameAsId = "Demolib";
	m_pluginProperties.versionStr = VstSynthVersionNum;
	m_pluginProperties.mixerLabel = "out";
	m_pluginProperties.isSynth = false;
	// Optional properties: Demo_Lib is new and so does not need to support loading old Dash programs and banks.
	m_pluginProperties.canLoadOldPrograms = false;
	m_pluginProperties.oldNumPrograms = 0;
	m_pluginProperties.oldNumParams = 0;
	m_pluginProperties.paramDefinitionSize = 0;
	m_pluginProperties.programNameSize = 0;
}

Demo_LibPluginFactory::~Demo_LibPluginFactory()
{
	delete m_plugin;
	delete [] m_sharedFloatPool;
}

DspEngine& Demo_LibPluginFactory::getDspEngine()
{
	return m_synth;
}

PluginCore* Demo_LibPluginFactory::getPluginInstance()
{
	if (!m_plugin)
	{
		m_plugin = new Demo_LibPlugin(m_pluginProperties, m_parameters, m_meteringData, m_synth, this, m_sharedFloatPool);
	}
	return m_plugin;
}


//==============================================================================

Demo_LibPlugin::Demo_LibPlugin(const PluginProperties& pluginProps,
                               Parameters& parameters,
                               MeteringData& meteringData,
                               VstSynth& dsp,
                       PluginFactory* factory,
                       float* sharedFloatPool)
 : PluginCore(pluginProps, parameters, dsp, factory),
   m_appearanceFactory(m_errorLog),
   m_appearanceManager(m_appearanceFactory),
   m_prefs(m_pluginProperties.nameAsId),
   m_pluginAppearance(m_appearanceFactory, m_appearanceManager, m_prefs),
   m_editorConfig(meteringData),
   m_editorController(m_presetManager, *this, m_appearanceManager, m_errorLog, m_pluginAppearance, m_prefs,
                      m_editorConfig, sharedFloatPool),
   m_modulations(),
   m_meteringData(meteringData),
   m_synth(dsp)
{
}

Demo_LibPlugin::~Demo_LibPlugin()
{
}

EditorCore* Demo_LibPlugin::getEditor()
{
	return &m_editorController;
}

void Demo_LibPlugin::defineModulations()
{
	//m_modulations.defineSource(fromNone,          "None"); // curently unused
	//m_modulations.defineSource(fromLfo1,          "Lfo1");
	//m_modulations.defineSource(fromModWheel,      "ModWheel");
	//m_modulations.defineSource(fromAftertouch,	  "Aftertouch");
	//m_modulations.defineSource(fromKeyTrack,      "KeyTrack"); // curently unused

	//m_modulations.defineTarget(send2none,                   "off");
	//m_modulations.defineTarget(send2bits,              "harmonics");
	//m_modulations.defineTarget(send2noise,                  "noise");
	//m_modulations.defineTarget(send2amplitude,              "amplitude");
	//m_modulations.defineTarget(send2lags,                   "lags");
	//m_modulations.defineTarget(send2fdbk_amnt,              "fdbk amnt");
	//m_modulations.defineTarget(send2BR_Hz,                  "BR Hz");
	//m_modulations.defineTarget(send2LFO_depth,              "LFO depth");
	//m_modulations.defineTarget(send2LFO_speed,              "LFO speed");
	//m_modulations.defineTarget(send2pitch_pos,              "pitch +");
	//m_modulations.defineTarget(send2pitch_neg,              "pitch -");
	//m_modulations.defineTarget(send2pitch,                  "pitch");
	//m_modulations.defineTarget(send2pitchPLUSamplitude,     "pitch+amplitude");
	//m_modulations.defineTarget(send2harmonicsPLUSamplitude, "harmonics+amplitude");
	//m_modulations.defineTarget(send2noisePLUSamplitude,     "noise+amplitude");
	//m_modulations.defineTarget(send2pan,                    "pan");

	//// Define the allowed targets for each source. Some synths / effects may
	//// allow any source to be mapped to any target, but others may want a more
	//// restricted mapping.
	//m_modulations.addTargetForSource(fromLfo1, send2none);
	//m_modulations.addTargetForSource(fromLfo1, send2bits);
	//m_modulations.addTargetForSource(fromLfo1, send2noise);
	//m_modulations.addTargetForSource(fromLfo1, send2pitch);
	//m_modulations.addTargetForSource(fromLfo1, send2amplitude);
	//m_modulations.addTargetForSource(fromLfo1, send2lags);
	//m_modulations.addTargetForSource(fromLfo1, send2fdbk_amnt);
	//m_modulations.addTargetForSource(fromLfo1, send2BR_Hz);
	//m_modulations.addTargetForSource(fromLfo1, send2pan);

	//m_modulations.addTargetForSource(fromModWheel, send2none);
	//m_modulations.addTargetForSource(fromModWheel, send2bits);
	//m_modulations.addTargetForSource(fromModWheel, send2noise);	
	//m_modulations.addTargetForSource(fromModWheel, send2fdbk_amnt); 
	//m_modulations.addTargetForSource(fromModWheel, send2LFO_depth);

	//m_modulations.addTargetForSource(fromAftertouch, send2none);
	//m_modulations.addTargetForSource(fromAftertouch, send2bits);
	//m_modulations.addTargetForSource(fromAftertouch, send2noise);
	//m_modulations.addTargetForSource(fromAftertouch, send2pitch);
	//m_modulations.addTargetForSource(fromAftertouch, send2amplitude);
	//m_modulations.addTargetForSource(fromAftertouch, send2fdbk_amnt); 
	//m_modulations.addTargetForSource(fromAftertouch, send2LFO_depth);

	//m_modulations.createMatrix();
}

