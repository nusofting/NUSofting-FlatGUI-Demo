/*-----------------------------------------------------------------------------

© 2002 2015,   nusofting.com  - Liqih

__myeditor__.h
Written by Luigi Felici
Copyright (c) 2007 2015 NUSofting

-----------------------------------------------------------------------------*/

#if !defined(__inc___myeditor__)
#define __inc___myeditor__ __inc___myeditor__

#include "FlatGUI/CToolTip.h"
#include "FlatGUI/messages_multi_controls.hpp"
#include "FlatGUI/Controls.h"
#include "FlatGUI/ZoomableEditor.h"
#include "FlatGUI/CKickTwiceMenu.h"
#include "FlatGUI/CLabelViewsChange.hpp"
#include "FlatGUI/CEchobisAmpModPassiveDisplay.hpp"
#include "FlatGUI/CDisplayBitmap.h"

#include "beta_switch.hpp"

namespace Platform
{
	class Logger;
}
class AppearanceController;
class PreferencesRegistry;
class PresetManager;
class MeteringData;
class Modulations;
class MultiActionDialogButton;
class SaveDialog;
class BFRBackgroundBitmap;

class EditorContentView : public ZoomableEditorContentView, public IControlListener
{
public:
	EditorContentView(ZoomableEditorController& parentController,
					AppearanceController& appearanceController,
					PreferencesRegistry& prefs,
					ScaledFontFactory* scaledFontFactory,
					EditorToPluginCoreInterface& notifyPluginCore,
					//Modulations& modulations,
					const MeteringData& meteringData,
					float* sharedFloatPool);
	~EditorContentView();

	void CreateKnobView(int index, int type = 1);
	void CreateSwitchOnOff(int index);
	CSliderFlat* CreateSliderFlat(int index);
	void CreateSliderNano(int index);

	/// From ZoomableEditorContentView
	/// @{
	virtual bool open (CCoord width, CCoord height, CFrame* frame);
	virtual void close();
	virtual void idle ();
	virtual void refreshParameter(int index, float value);
	virtual void setDisplayParameterInt(int msgIndex, int value) { } 
	virtual void setColours();
	virtual void handleZoom(const CRect &newSize);
	/// @}


	// from IControlListener
	void valueChanged (CControl* pControl);

	MainToolBarRects& getRectsNeededForToolBar() { return m_rMainRects; }

private:
	MainToolBarRects m_rMainRects;
	
	ZoomableEditorController& m_parentController;
	CControl* controls[kNumParams];

	ScaledFontFactory* m_scaledFontFactory;

	/// Refreshes the display for a changed parameter, including changing labels, changing displays on dependent
	/// parameters and the mouse hover parameter displays. Handles parameters changes from both the GUI itself and
	/// parameters changes from the host, e.g. via automation or program change.
	void refreshDisplay(int index);
	void DisplayHelperAndValue(int index, CControl* pControl, CControl* controlDisplay = 0);

	CToolTip* m_anyinfo;

	CStereoPeaksView* m_ViewMeter1;
	CStereoPeaksView* m_ViewMeter2;

	CMovieButton* m_KickClearBypass;
    void setByass(int index, float value, bool state)
    {
        switch(index)
        {
            case kActive1:
            {
            static_cast<CMultiSwitch*>(controls[kActive1])->setActive(!state);
			controls[kActive1]->setMouseEnabled(!state);
            wasTurnedOffByClear1 = state;
            refreshParameter (kActive1, value);
            m_notifyPluginCore.paramValueChanged(kActive1, value);
            } break;
            case kActive2:
        {
            static_cast<CMultiSwitch*>(controls[kActive2])->setActive(!state);
			controls[kActive2]->setMouseEnabled(!state);
            wasTurnedOffByClear2 = state;
            refreshParameter (kActive2, value);
            m_notifyPluginCore.paramValueChanged(kActive2, value);
        }break;
       
        }
     
    }
	CRect rKickClearBypass;
	const CCoord setScaleBkgd(CCoord heightFrame)
	{
		return 2410/heightFrame; // value relative to Docs+PNGs/clearkick.png on Windows
	}
	bool wasTurnedOffByClear1;
	bool wasTurnedOffByClear2;


	CDisplayBitmap* m_divHelp;

	CRect rCtrls[kNumParams];

	void textureFrame(CFrame* frame);

	//void Mouse_Hover(CFrame* frame);

	AppearanceController& m_appearance;
	const MeteringData& m_meteringData;

	PreferencesRegistry& m_prefs;

	CTextEdit* m_LabelViewsDT1;
	CTextEdit* m_LabelViewsDT2;
	CParamDisplayValueToStringProc valueToString1;
	CRect rLabelTime1;
	CRect rLabelTime2;

	CEchobisAmpModPassiveDisplay* m_AmpMod1_Passive;
	CEchobisAmpModPassiveDisplay* m_AmpMod2_Passive;

	CRect rAmpMod1;
	CRect rAmpMod2;

	CEchobisAmpModPassiveDisplay* m_Bend1_Passive;
	CEchobisAmpModPassiveDisplay* m_Bend2_Passive;

	void modControlsUtil (int kDepthN, int kTimeModN, int kDivN);

	CFrame* frame;
	EditorToPluginCoreInterface& m_notifyPluginCore;

	/// Pool of floating point values (dangerously) shared between the DSP engine and the editor. Dangerous in the sense
	/// that this is an array of floats passed to both the DSP engine and editor by a simple pointer, and there is no
	/// bounds checking or thread synchronisation governing read or write access to this pool. So only index into this
	/// via the index ranges defined in the SharedFloatPoolIndexes enum defined in PluginDef.h.
	float* m_sharedFloatPool;

	Beta_Switch beta_switch;
};


//------------------------------------------------------------------------------

class EditorConfiguration : public ZoomableEditorConfiguration
{
public:
	EditorConfiguration(const MeteringData& meteringData);
	~EditorConfiguration();

private:
	/// Return the width of the editor's frame at zoom level 1x. This includes the space needed by the toolbar.
	virtual CCoord getReferenceWidth();

	/// Return the height of the editor's frame at zoom level 1x. This includes the space needed by the toolbar.
	virtual CCoord getReferenceHeight();

	/// Return the name of the custom font to use.
	virtual const char* getFontName();

	/// Return the full path to the font file for the custom font to use.
	virtual std::string getFontFilePath();

	virtual ToolBarOptions& getToolBarOptions();
	virtual ZoomableEditorContentView* getContentView(ZoomableEditorController& parentController,
	                                                  AppearanceController& appearanceController,
	                                                  PreferencesRegistry& prefs,
	                                                  ScaledFontFactory* scaledFontFactory,
	                                                  EditorToPluginCoreInterface& notifyPluginCore,
	                                                  float* sharedFloatPool);
	virtual void releaseContentView(ZoomableEditorContentView *view);

	const MeteringData& m_meteringData;

	EditorContentView* m_editorContentView;	
};

#endif

