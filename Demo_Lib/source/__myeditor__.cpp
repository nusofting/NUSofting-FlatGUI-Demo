/*-----------------------------------------------------------------------------

© 2002 2020,   nusofting.com  - Liqih

__myeditor__.cpp
Written by Luigi Felici
Copyright (c) 2020 NUSofting

-----------------------------------------------------------------------------*/

// effetto
#include "PluginDef.h"

// editor nor
#include "__myeditor__.h"
#include "BFRBackgroundBitmap.h"

#include "FlatGUI/AppearanceController.h"
#include "FlatGUI/PreferencesRegistry.h"
#include "Core/MeteringData.h"
#include "Core/Modulations.h"
#include "Platform/Platform.h"
#include "PresetManager/PresetManager.h"
#include "PresetManager/Program.h"
#include "VSTGUI-extension/Popups.h"

// menu font size scaling
#define menuFontTB 12
#define menuFontCC 5



enum EditorConst
{
	kKeymap = 14000,
	kEditorNoteDisplay = 15000,
	kBaseLearn = 0x100000,

	kOptionTempo = 19000,
	kSetupZoomOptions = 19001,	
	kToolbarSettingsOptions = 19002,

	kStereoWavsView = 19003,

	kDelayEdit1,
	kDelayEdit2,

	kClearBypass,
	kDivHelp
};

#ifdef WIN32
#include <windows.h>
std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}
#endif


//Avalanche offset athabasca Rg offset Lucida Sans Atlanta News comme cibreo chivo
static const char* thisFontUsed = "Arvin"; //Glober SemiBold Free  Kelson Sans Tahoma  cuprum gafata asap  gidole needs size increased


//#include "Mouse_Hover_inc.hpp"

static bool convertStringToValue  (UTF8StringPtr string, float& output, void* userData)
{
	output = UTF8StringView (string).toFloat ();
	if (output < 0.0f)
		output = 0.0f;
	else if (output > 2000.0f)
		output = 2000.0f;

	output = output/2000.0f;

	return true;
}

//-----------------------------------------------------------------------------------

EditorContentView::EditorContentView (ZoomableEditorController& parentController,
					AppearanceController& appearanceController,
					PreferencesRegistry& prefs,
					ScaledFontFactory* scaledFontFactory,
					EditorToPluginCoreInterface& notifyPluginCore,
					//Modulations& modulations,
					const MeteringData& meteringData,
					float* sharedFloatPool)
					: 
					m_parentController(parentController),
					m_scaledFontFactory(scaledFontFactory),
					m_appearance(appearanceController),
					m_meteringData(meteringData),
					m_notifyPluginCore(notifyPluginCore),
					m_sharedFloatPool(sharedFloatPool),
					m_prefs(prefs)
{
	m_anyinfo = 0;

	for (int i = 0; i < kNumParams; i++)
	{	controls[i]=0; }

	m_ViewMeter1 = 0;
	m_ViewMeter2 = 0;
	m_LabelViewsDT1 = 0;
	m_LabelViewsDT2 = 0;


	m_AmpMod1_Passive = 0;
	m_AmpMod2_Passive = 0;
	m_Bend1_Passive = 0;
	m_Bend2_Passive = 0;

    wasTurnedOffByClear1 = wasTurnedOffByClear2 = false;
	m_KickClearBypass = 0;

	m_divHelp = 0;


}

EditorContentView::~EditorContentView()
{
}

//-------- custom objects -------

void EditorContentView::CreateKnobView(int index, int type)
{
	if(rCtrls[index].getWidth() <= 0) return; else if (rCtrls[index].getWidth() == 52.0f) rCtrls[index].inset(-1,-1); else if (rCtrls[index].getWidth() == 41.0f) rCtrls[index].inset(-3,-3);
	CKnobFlatBranis* pKnob = new CKnobFlatBranis(rCtrls[index], this, index, m_appearance.getKnobTheme(), type);
	if(index == kHP1 || index == kHP2) pKnob->DoCurve(true, true);
	frame->addView (pKnob); 
	//-- remember our controls so that we can sync them with the state of the effect
	controls[index] = pKnob;		
}

void EditorContentView::CreateSwitchOnOff(int index)
{
	if(rCtrls[index].getWidth() <= 0) return;
	CSimpleOnOff* pSwitch = new CSimpleOnOff(rCtrls[index], this, index, m_appearance.getSimpleSwitchTheme());
	frame->addView (pSwitch);
	//-- remember our controls so that we can sync them with the state of the effect
	controls[index] = pSwitch;
}
CSliderFlat* EditorContentView::CreateSliderFlat(int index)
{
	CSliderFlat* pTemp = new CSliderFlat(rCtrls[index],this, index,0,rCtrls[index].getHeight(),
		*m_scaledFontFactory,m_appearance.getSliderTheme(),kNoFrame|kVertical);
	frame->addView(pTemp);
	controls[index] = pTemp;
	return pTemp;
}
void EditorContentView::CreateSliderNano(int index)
{
	CSliderNano* pTemp = new CSliderNano(rCtrls[index],this, index, m_appearance.getSliderTheme(),kBottom|kVertical);
	pTemp->setShadow(false);
	frame->addView(pTemp);
	controls[index] = pTemp;
}

//-----------------------------------------------------------------------------

bool EditorContentView::open(CCoord width, CCoord height, CFrame* newFrame)
{
    
    //CView::kDirtyCallAlwaysOnMainThread = true;

	float scaleGUIx = (float)width/refWidth;
	float scaleGUIy = (float)height/refHeight;
	frame = newFrame;  
	textureFrame(frame); // fill rCtrls


	CreateKnobView(kDelayTime1, 1);
	CreateKnobView(kDelayTime2, 1);
	CreateKnobView(kFeedback1, 1);
	CreateKnobView(kFeedback2, 1);

	CreateKnobView(kRate1, 1);
	CreateKnobView(kTimeMod1, -1);	
	CreateKnobView(kDepth1, 1);
	CreateKnobView(kDiv1, 6);

	CreateKnobView(kLP1, 1);
	CreateKnobView(kHP1, 1);
	CreateKnobView(kPitch1, -1);

	CreateKnobView(kRate2, 1);
	CreateKnobView(kDepth2, 1);
	CreateKnobView(kTimeMod2, -1);
	CreateKnobView(kDiv2, 6);

	CreateKnobView(kLP2, 1);
	CreateKnobView(kHP2, 1);
	CreateKnobView(kPitch2, -1);	
	
	CreateSliderNano(kPre_Gain);
	CreateSliderNano(kMixChannels);
	static_cast<CSliderNano*>(controls[kMixChannels])->setValueUpDownDir(true);
	CreateSliderNano(kLevel1);
	CreateSliderNano(kLevel2);	
	CreateSliderNano(kDry_Gain);

	CreateKnobView(kPan1, -1);
	CreateKnobView(kPan2, -1);

	CreateSwitchOnOff(kPhase1);
	CreateSwitchOnOff(kPhase2);

	CreateSwitchOnOff(kSync1);
	CreateSwitchOnOff(kSync2);

	static const char* LabelsActive1[1] = {"delay 1"}; 
	CMultiSwitch* pActive1 = new CMultiSwitch(rCtrls[kActive1],this,kActive1,1,*m_scaledFontFactory, m_appearance.getMultiSwitchTheme());
	pActive1->setLabels(LabelsActive1);
	pActive1->setSwitchBkground(true);
	newFrame->addView(pActive1);
	controls[kActive1] = pActive1;

	static const char* LabelsActive2[1] = {"delay 2"}; 
	CMultiSwitch* pActive2 = new CMultiSwitch(rCtrls[kActive2],this,kActive2,1,*m_scaledFontFactory, m_appearance.getMultiSwitchTheme());
	pActive2->setLabels(LabelsActive2);
	pActive2->setSwitchBkground(true);
	newFrame->addView(pActive2);
	controls[kActive2] = pActive2;

	
	static const char* LabelsTriplets[1] = {"24"}; 
	CMultiSwitch* pTriplets1 = new CMultiSwitch(rCtrls[kTriplets1],this,kTriplets1,1,*m_scaledFontFactory, m_appearance.getMultiSwitchTheme());
	pTriplets1->setLabels(LabelsTriplets);
	pTriplets1->setSwitchBkground(true);
	newFrame->addView(pTriplets1);
	controls[kTriplets1] = pTriplets1;

	CMultiSwitch* 	pTriplets2 = new CMultiSwitch(rCtrls[kTriplets2],this,kTriplets2,1,*m_scaledFontFactory, m_appearance.getMultiSwitchTheme());
	pTriplets2->setLabels(LabelsTriplets);
	pTriplets2->setSwitchBkground(true);
	newFrame->addView(pTriplets2);
	controls[kTriplets2] = pTriplets2;
	

	static const char* LabelsLink[1] = {"æ"}; // not unused because of pLink->setDrawLink(true);
	CMultiSwitch* pLink = new CMultiSwitch(rCtrls[kLinkFeeds],this,kLinkFeeds,1,*m_scaledFontFactory, m_appearance.getMultiSwitchTheme());
	pLink->setLabels(LabelsLink);
	pLink->setSwitchBkground(true);
	pLink->setDrawLink(true);
	pLink->setBkgdAlpha(128);
	newFrame->addView(pLink);
	controls[kLinkFeeds] = pLink;

	pLink = new CMultiSwitch(rCtrls[kLinkLevels],this,kLinkLevels,1,*m_scaledFontFactory, m_appearance.getMultiSwitchTheme());
	pLink->setLabels(LabelsLink);
	pLink->setSwitchBkground(true);
	pLink->setDrawLink(true);
	pLink->setBkgdAlpha(128);
	newFrame->addView(pLink);
	controls[kLinkLevels] = pLink;

	pLink = new CMultiSwitch(rCtrls[kLinkLPHPs],this,kLinkLPHPs,1,*m_scaledFontFactory, m_appearance.getMultiSwitchTheme());
	pLink->setLabels(LabelsLink);
	pLink->setSwitchBkground(true);
	pLink->setDrawLink(true);
	pLink->setBkgdAlpha(128);
	newFrame->addView(pLink);
	controls[kLinkLPHPs] = pLink;


	CGain2OnOff* pSwitch = new CGain2OnOff(rCtrls[kTapeComp], this, kTapeComp, m_appearance.getSimpleSwitchTheme());
	frame->addView (pSwitch);
	//-- remember our controls so that we can sync them with the state of the effect
	controls[kTapeComp] = pSwitch;

	 pSwitch = new CGain2OnOff(rCtrls[kDrift], this, kDrift, m_appearance.getSimpleSwitchTheme());
	frame->addView (pSwitch);
	//-- remember our controls so that we can sync them with the state of the effect
	controls[kDrift] = pSwitch;
	
	CSliderNano* pSliH = new CSliderNano(rCtrls[kCrossfeed],this,kCrossfeed, m_appearance.getSliderTheme());
	//pSliH->setSquareHandle(true);
	pSliH->setShadow(false);
	newFrame->addView(pSliH);
	controls[kCrossfeed] = pSliH;
	
		// variable label
		m_LabelViewsDT1 = new CTextEdit(rLabelTime1, this, kDelayEdit1);
		m_LabelViewsDT1->setFont(m_scaledFontFactory->getScaledSmallFont());
		//m_LabelViewsDT1->setStringToValueProc(&convertStringToValue, NULL);
		m_LabelViewsDT1->setBackColor(MakeCColor(1,1,1,98));
		newFrame->addView(m_LabelViewsDT1);

		m_LabelViewsDT2 = new CTextEdit(rLabelTime2, this, kDelayEdit2);
		m_LabelViewsDT2->setFont(m_scaledFontFactory->getScaledSmallFont());
		//m_LabelViewsDT2->setStringToValueProc(&convertStringToValue, NULL);
		m_LabelViewsDT2->setBackColor(MakeCColor(1,1,1,98));
		newFrame->addView(m_LabelViewsDT2);


		m_AmpMod1_Passive = new CEchobisAmpModPassiveDisplay(rAmpMod1, true);
		newFrame->addView(m_AmpMod1_Passive);
		m_AmpMod2_Passive = new CEchobisAmpModPassiveDisplay(rAmpMod2, true);
		newFrame->addView(m_AmpMod2_Passive);

		m_Bend1_Passive = new CEchobisAmpModPassiveDisplay(rAmpMod1.offset(0.0f, -60.0f*scaleGUIy), false);
		newFrame->addView(m_Bend1_Passive);
		m_Bend2_Passive = new CEchobisAmpModPassiveDisplay(rAmpMod2.offset(0.0f, -60.0f*scaleGUIy), false);
		newFrame->addView(m_Bend2_Passive);
		
	//CRect rToolTip(rEnvDisplay);
	//rToolTip.inset(-4*scaleGUIx,0);
	//m_anyinfo = new CToolTip(rToolTip,false,kLeftText); 
	//newFrame->addView (m_anyinfo);
	//m_anyinfo->setDisplayFont(0, thisFontUsed);

	CRect peaksViewRect(0,0,19*scaleGUIx, 104*scaleGUIy);
	peaksViewRect.offset(rCtrls[kLevel1].left-35*scaleGUIx, rCtrls[kFeedback1].top+2*scaleGUIy);
	m_ViewMeter1 = new CStereoPeaksView(peaksViewRect, m_appearance.getGraphViewTheme());
	m_ViewMeter1->setSingleChannel(true);
	newFrame->addView (m_ViewMeter1);

	m_ViewMeter2 = new CStereoPeaksView(peaksViewRect.offset(0, 180*scaleGUIy), m_appearance.getGraphViewTheme());
	m_ViewMeter2->setSingleChannel(true);
	newFrame->addView (m_ViewMeter2);
	
	CBitmap* bkgdClear = createScaledBitmap("Docs+PNGs/clearkick.png",  setScaleBkgd(height), false);
	if(bkgdClear)
	{
		m_KickClearBypass =  new CMovieButton(rKickClearBypass, this, kClearBypass,  bkgdClear->getHeight()*0.5, bkgdClear, CPoint(0.0));
		newFrame->addView (m_KickClearBypass);
	}

	CRect rDivHelp(rCtrls[kSync1]); 
	rDivHelp.inset(2.0,2.0);
	rDivHelp.offset(-26*scaleGUIx, 0.0f);
	CBitmap* bkgdMouse = createScaledBitmap("Docs+PNGs/callMouse_128.png", 4400/width, false);	
	CBitmap* helpBmp = createScaledBitmap("Docs+PNGs/divs_examples_128.png", 1600/width, false);	
	if(bkgdMouse && helpBmp)
	{
		m_divHelp = new CDisplayBitmap(rDivHelp, this, kDivHelp, bkgdMouse, helpBmp);
		newFrame->addView (m_divHelp);
	}


	for(size_t index = 0; index < kNumParams; ++index)
	{
		if(controls[index] != nullptr)
		controls[index]->setDefaultValue(m_notifyPluginCore.requestDefaultValue(index));
	}

#if BETA
	beta_switch.CheckDate_betaProtection2();

	if(beta_switch.getState())
	{	
		for(size_t index = 0; index < kNumParams; ++index)
		{
			if(controls[index] != nullptr)
				controls[index]->setVisible(false);
		}
	}
#endif

	return true;
}

void EditorContentView::close()
{

	if(m_KickClearBypass && m_KickClearBypass->getValue() >= 0.5f)
	{
		if(wasTurnedOffByClear1)
		{
			setByass(kActive1, 1.0f, false);
		}
		if(wasTurnedOffByClear2)
		{
			setByass(kActive2, 1.0f, false);
		}
	}
	
	m_anyinfo = 0;
	m_ViewMeter1 = 0;
	m_ViewMeter2 = 0;
	m_LabelViewsDT1 = 0;
	m_LabelViewsDT2 = 0;
	m_AmpMod1_Passive = 0;
	m_AmpMod2_Passive = 0;
	m_Bend1_Passive = 0;
	m_Bend2_Passive = 0;
	m_KickClearBypass = 0;
	m_divHelp = 0;

	frame = 0;
}

void EditorContentView::DisplayHelperAndValue(int index, CControl* pControl, CControl* controlDisplay)
{
	char display[128] = {0};
	m_notifyPluginCore.requestForFormattedParameterString(index,display,sizeof display);
	if(pControl->isTypeOf("CKnobFlatBranis"))
		static_cast<CKnobFlatBranis*>(pControl)->setString32(display);

	if(pControl->isTypeOf("CSliderFlat"))
		static_cast<CSliderFlat*>(pControl)->setString32(display);

	if(index == kSync1 || index == kDelayTime1)
	{
		char display[128] = {0};
		strcpy(display,static_cast<CKnobFlatBranis*>(EditorContentView::controls[kDelayTime1])->getString());
		if(m_LabelViewsDT1) m_LabelViewsDT1->setText(display);

		controls[kTriplets1]->setVisible(controls[kSync1]->getValue() >= 0.5f);
	}
	if(index == kSync2 || index == kDelayTime2)
	{
		char display[128] = {0};
		strcpy(display,static_cast<CKnobFlatBranis*>(EditorContentView::controls[kDelayTime2])->getString());
		if(m_LabelViewsDT2) m_LabelViewsDT2->setText(display);

		controls[kTriplets2]->setVisible(controls[kSync2]->getValue() >= 0.5f);
	}
	if(index == kDepth1 || index == kDiv1) 
	{
		const int value = 1 + int(controls[kDiv1]->getValue()*5.0f+0.5f);
		if(m_AmpMod1_Passive)m_AmpMod1_Passive->setDepth(controls[kDepth1]->getValue(), value);
	}
	if(index == kDepth2 || index == kDiv2) 
	{
		const int value = 1 + int(controls[kDiv2]->getValue()*5.0f+0.5f);
		if(m_AmpMod2_Passive) m_AmpMod2_Passive->setDepth(controls[kDepth2]->getValue(), value);
	}
	if(index == kTimeMod1 || index == kDiv1) 
	{
		const int value = 1 + int(controls[kDiv1]->getValue()*5.0f+0.5f);
		if(m_Bend1_Passive) m_Bend1_Passive->setDepth(controls[kTimeMod1]->getValue(), value);
	}
	if(index == kTimeMod2 || index == kDiv2) 
	{
		const int value = 1 + int(controls[kDiv2]->getValue()*5.0f+0.5f);
		if(m_Bend2_Passive) m_Bend2_Passive->setDepth(controls[kTimeMod2]->getValue(), value);
	}
}

void EditorContentView::modControlsUtil (int kDepthN, int kTimeModN, int kDivN)
{
		bool state = controls[kDepthN]->getValue() >= 0.01f;
		float is = controls[kTimeMod2]->getValue();
		state |= fabs(is-0.5f) <= 0.016f? false : is > 0.516f? true : true;			
		static_cast<CKnobFlatBranis*>(controls[kDivN])->setActive(state);
}

void EditorContentView::refreshParameter (int index, float value)
{
	if(index < 0) return;

	//-- refreshParameter is called when the host automates one of the effects parameters
	//-- and when the program is changed
	//-- The UI should reflect this state.
	if (frame && index < kNumParams && controls[index] != NULL)
	{
		controls[index]->setValue(value); // moves controls

		refreshDisplay(index);
	}
}

void EditorContentView::refreshDisplay (int index)
{
	// for value display on knobs
	DisplayHelperAndValue(index, EditorContentView::controls[index]);

	if(index == kSync1) {DisplayHelperAndValue(kDelayTime1, EditorContentView::controls[kDelayTime1]);}
	if(index == kSync2) {DisplayHelperAndValue(kDelayTime2, EditorContentView::controls[kDelayTime2]);}

	if(EditorContentView::controls[kSync1]->getValue() > 0.0f)
	{
		m_LabelViewsDT1->setMouseEnabled(false);
		m_LabelViewsDT1->setTransparency(true);
	}
	else
	{
		m_LabelViewsDT1->setMouseEnabled(true);
		m_LabelViewsDT1->setTransparency(false);
	}

	if(EditorContentView::controls[kSync2]->getValue() > 0.0f)
	{
		m_LabelViewsDT2->setMouseEnabled(false);
		m_LabelViewsDT2->setTransparency(true);
	}
	else
	{
		m_LabelViewsDT2->setMouseEnabled(true);
		m_LabelViewsDT2->setTransparency(false);
	}

	// set active or grayed our views
	switch(index)
	{
	case kLinkFeeds: {
		const bool state = !(controls[kLinkFeeds]->getValue() >= 0.5f);
		static_cast<CKnobFlatBranis*>(controls[kFeedback2])->setActive(state);
					 }break;
	case kLinkLevels: {
		const bool state = !(controls[kLinkLevels]->getValue() >= 0.5f);
		static_cast<CSliderNano*>(controls[kLevel2])->setActive(state);
					  }break;
	case kPitch1: {
		const bool state = controls[kPitch1]->getValue() != 0.5f;
		static_cast<CKnobFlatBranis*>(controls[kRate1])->setActive(state);
				  }break;
	case kPitch2: {
		const bool state = controls[kPitch2]->getValue() != 0.5f;
		static_cast<CKnobFlatBranis*>(controls[kRate2])->setActive(state);
				  }break;
	case kActive1 : {
		bool state = controls[kActive1]->getValue() >= 0.5f;
		static_cast<CKnobFlatBranis*>(controls[kDelayTime1])->setActive(state);
		static_cast<CKnobFlatBranis*>(controls[kFeedback1])->setActive(state);
		static_cast<CKnobFlatBranis*>(controls[kLP1])->setActive(state);
		static_cast<CKnobFlatBranis*>(controls[kHP1])->setActive(state);
		static_cast<CKnobFlatBranis*>(controls[kPitch1])->setActive(state);	
		static_cast<CKnobFlatBranis*>(controls[kTimeMod1])->setActive(state);
		static_cast<CKnobFlatBranis*>(controls[kDepth1])->setActive(state);
		static_cast<CKnobFlatBranis*>(controls[kDiv1])->setActive(state);
		if(state) modControlsUtil (kDepth1, kTimeMod1, kDiv1);
		static_cast<CSimpleOnOff*>(controls[kSync1])->setActive(state);
		static_cast<CSimpleOnOff*>(controls[kPhase1])->setActive(state);
		state &= controls[kPitch1]->getValue() != 0.5f;
		static_cast<CKnobFlatBranis*>(controls[kRate1])->setActive(state);
					}break;
	case kActive2 : {
		bool state = controls[kActive2]->getValue() >= 0.5f;
		static_cast<CKnobFlatBranis*>(controls[kDelayTime2])->setActive(state);
		static_cast<CKnobFlatBranis*>(controls[kFeedback2])->setActive(state);
		static_cast<CKnobFlatBranis*>(controls[kLP2])->setActive(state);
		static_cast<CKnobFlatBranis*>(controls[kHP2])->setActive(state);	
		static_cast<CKnobFlatBranis*>(controls[kPitch2])->setActive(state);
		static_cast<CKnobFlatBranis*>(controls[kTimeMod2])->setActive(state);
		static_cast<CKnobFlatBranis*>(controls[kDepth2])->setActive(state);
		static_cast<CKnobFlatBranis*>(controls[kDiv2])->setActive(state);
		if(state) modControlsUtil (kDepth2, kTimeMod2, kDiv2);
		static_cast<CSimpleOnOff*>(controls[kSync2])->setActive(state);
		static_cast<CSimpleOnOff*>(controls[kPhase2])->setActive(state);
		state &= controls[kPitch2]->getValue() != 0.5f;
		static_cast<CKnobFlatBranis*>(controls[kRate2])->setActive(state);		
					}break;
	}

	if(index ==	kDepth1 || index == kTimeMod1)
	{
		modControlsUtil (kDepth1, kTimeMod1, kDiv1);
	}
	if(index ==	kDepth2 || index == kTimeMod2)
	{
		modControlsUtil (kDepth2, kTimeMod2, kDiv2);
	}
}

void EditorContentView::valueChanged (CControl* pControl)
{
	long tag = pControl->getTag();
	float value = pControl->getValue();
	//-- valueChanged is called whenever the user changes one of the controls in the User Interface (UI)
	//   These controls can represent raw VST parameter values or they can represent other higher-level
	//   concepts based on menu selection, such as modulations. We want to treat these separately,
	//   because (for example) menu selections fall outside of the standard VST -1.0 to 1.0 value
	//   range and therefore we don't want / need the host to potentially automate them.

	if (tag >= 0 && tag < kNumParams)
	{
		m_notifyPluginCore.paramValueChanged(tag, value);

		if(controls[kLinkFeeds]->getValue() >= 0.5f)
		{
			if(tag == kFeedback1)
			{
				m_notifyPluginCore.paramValueChanged(kFeedback2, value);
				refreshParameter(kFeedback2, value);
			}
			else if(tag == kFeedback2)
			{
				m_notifyPluginCore.paramValueChanged(kFeedback1, value);
				refreshParameter(kFeedback1, value);
			}
		}
		if(controls[kLinkLevels]->getValue() >= 0.5f)
		{
			if(tag == kLevel1)
			{
				m_notifyPluginCore.paramValueChanged(kLevel2, value);
				refreshParameter(kLevel2, value);
			}
			else if(tag == kLevel2)
			{
				m_notifyPluginCore.paramValueChanged(kLevel1, value);
				refreshParameter(kLevel1, value);
			}
			else if(tag == kPan1)
			{
				m_notifyPluginCore.paramValueChanged(kPan2, 1.0f-value);
				refreshParameter(kPan2, 1.0f-value);
			}
			else if(tag == kPan2)
			{
				m_notifyPluginCore.paramValueChanged(kPan1, 1.0f-value);
				refreshParameter(kPan1, 1.0f-value);
			}

		}
		if(controls[kLinkLPHPs]->getValue() >= 0.5f)
		{
			if(tag == kLP1)
			{
				m_notifyPluginCore.paramValueChanged(kLP2, value);
				refreshParameter(kLP2, value);
			}
			else if(tag == kLP2)
			{
				m_notifyPluginCore.paramValueChanged(kLP1, value);
				refreshParameter(kLP1, value);
			}
			else if(tag == kHP1)
			{
				m_notifyPluginCore.paramValueChanged(kHP2, value);
				refreshParameter(kHP2, value);
			}
			else if(tag == kHP2)
			{
				m_notifyPluginCore.paramValueChanged(kHP1, value);
				refreshParameter(kHP1, value);
			}
		}

		// for value display on knobs
		refreshDisplay(tag);
	}

	if (tag == kDelayEdit1 && !(EditorContentView::controls[kSync1]->getValue() > 0.0f))	
	{
		m_notifyPluginCore.paramValueChanged(kDelayTime1, m_LabelViewsDT1->getValue());
		refreshParameter(kDelayTime1, m_LabelViewsDT1->getValue());
	}
	if (tag == kDelayEdit2 && !(EditorContentView::controls[kSync2]->getValue() > 0.0f))	
	{
		m_notifyPluginCore.paramValueChanged(kDelayTime2, m_LabelViewsDT2->getValue());
		refreshParameter(kDelayTime2, m_LabelViewsDT2->getValue());
	}

	if(tag == kTriplets1)
	{
		refreshDisplay(kDelayTime1);
	}
	if(tag == kTriplets2 )
	{
		refreshDisplay(kDelayTime2);
	}
    
    
    if(tag == kClearBypass && m_KickClearBypass)
    {
        if(value >= 0.5f)
        {
            if(controls[kActive1]->getValue() >= 0.5f)
            {
                setByass(kActive1, 0.0f, true);
            }
            if(controls[kActive2]->getValue() >= 0.5f)
            {
                setByass(kActive2, 0.0f, true);
            }
        }
        else
        {
            if(wasTurnedOffByClear1)
            {
                setByass(kActive1, 1.0f, false);
            }
            if(wasTurnedOffByClear2)
            {
                setByass(kActive2, 1.0f, false);
            }
        }
    }
    

	m_parentController.checkIfPresetBecameDirty();   

}


void EditorContentView::handleZoom(const CRect &newSize)
{
	textureFrame(frame);
	m_ViewMeter1->resetPeak();
	m_ViewMeter2->resetPeak();

	CRect changedSize = frame->getViewSize();
	CBitmap* bkgdClear = createScaledBitmap("Docs+PNGs/clearkick.png",  setScaleBkgd(changedSize.getHeight()), false);
	if(m_KickClearBypass && bkgdClear)
	{
		m_KickClearBypass->getBackground()->forget();
		m_KickClearBypass->setBackground(bkgdClear);
		m_KickClearBypass->setHeightOfOneImage(bkgdClear->getHeight()*0.5);
		m_KickClearBypass->invalid ();
	}	

	CBitmap* helpBmp = createScaledBitmap("Docs+PNGs/divs_examples_128.png", 1600/changedSize.getWidth(), false);	
	if(m_divHelp && helpBmp)
	{
		m_divHelp->doResize(helpBmp);
	}
}

void EditorContentView::setColours()
{
	frame->invalidRect(frame->getViewSize());

	textureFrame(frame); // background 
	
	//m_appearance.styleToolTip(m_anyinfo);

	if(m_AmpMod1_Passive && m_AmpMod2_Passive && m_Bend1_Passive && m_Bend2_Passive)
	{
	 m_AmpMod1_Passive->setDisplayColour(m_appearance.getColourById(CLR_LABEL_FONT));
	 m_AmpMod2_Passive->setDisplayColour(m_appearance.getColourById(CLR_LABEL_FONT));
	 m_Bend1_Passive->setDisplayColour(m_appearance.getColourById(CLR_LABEL_FONT));
	 m_Bend2_Passive->setDisplayColour(m_appearance.getColourById(CLR_LABEL_FONT));
	}
}

void EditorContentView::idle()
{	
#if BETA
	if(frame)
	beta_switch.sendBetaMesssage();
	if(beta_switch.getState()) return;
#endif

	//setAutosizingEnabled (true);
	//Mouse_Hover(frame); // calls ToolTips

	if(m_ViewMeter1 && m_ViewMeter2)
	{
		float vu_cd1 = 0.0f;
		float vu_cd2 = 0.0f;
		if (m_meteringData.pollLevels(vu_cd1, vu_cd2))
		{
			m_ViewMeter1->setValues(vu_cd1, vu_cd1); // only left
			m_ViewMeter2->setValues(vu_cd2, vu_cd2); // only left
		}
	}

	if(m_KickClearBypass && frame)
	{
		if(m_KickClearBypass->getValue() >= 0.5f)
		{
			CPoint XY;
			if(frame->getCurrentMouseButtons () != CButton::kLButton && frame->getCurrentMouseLocation(XY))
			{			
				if(!m_KickClearBypass->getViewSize().pointInside(XY))
				{
					m_KickClearBypass->setValue(0.0f);
					m_KickClearBypass->setDirty();
					valueChanged (m_KickClearBypass);
				}
			}
		}
	}

	if(m_divHelp) m_divHelp->idleRun();
}

void EditorContentView::textureFrame(CFrame* frame)
{

	//IPlatformFrame* pFrame = frame->getPlatformFrame();
	//CPoint pos(0,0);
	//pFrame->getGlobalPosition(pos);
	
	CCoord x;
	CCoord y;
	CPoint XY(0,0);	
	if(frame->getPosition (x, y))
	{
		XY.x = x;
		XY.y = y;
	}
	
	CRect size = frame->getViewSize();
	COffscreenContext* oc = COffscreenContext::create (frame, size.getWidth(), size.getHeight());
	oc->beginDraw();
	BFRBackgroundBitmap* bk = new BFRBackgroundBitmap(size.getWidth(), size.getHeight(), m_appearance, thisFontUsed);
	bk->draw (oc, frame->getViewSize(), XY, 1.0f);
	bk->getAllrCtrls(rCtrls); 
	m_rMainRects = bk->sharedToolBarRects.rAllRectsNeeded;
	rLabelTime1 = bk->rLabelTime1;
	rLabelTime2 = bk->rLabelTime2;
	rAmpMod1 = bk->rAmpMod1;
	rAmpMod2 = bk->rAmpMod2;
	rKickClearBypass = bk->rKickClearBypass;

	oc->endDraw();
	CBitmap* background = oc->getBitmap();
	frame->setBackground(background);
	oc->forget();
	bk->forget();

	frame->getEditor()->beforeSizeChange(size, size);
	frame->parentSizeChanged();
}

//-----------------------------------------------------------------------------------

EditorConfiguration::EditorConfiguration(const MeteringData& meteringData)
	: m_meteringData(meteringData),
	m_editorContentView(0)
{
}

EditorConfiguration::~EditorConfiguration()
{
#ifdef WIN32
	std::wstring stemp = s2ws(Platform::getPathForAppResource("Fonts/Arvin.ttf")); 
	LPCWSTR result = stemp.c_str();
	RemoveFontResourceEx(result, FR_NOT_ENUM, 0);
#endif


	delete m_editorContentView;
}

CCoord EditorConfiguration::getReferenceWidth()
{
	return refWidth;
}

CCoord EditorConfiguration::getReferenceHeight()
{
	return refHeight;
}

const char* EditorConfiguration::getFontName()
{
	return thisFontUsed;
}

std::string EditorConfiguration::getFontFilePath()
{

#ifdef WIN32
	std::wstring stemp = s2ws(Platform::getPathForAppResource("Fonts/Arvin.ttf")); 
	LPCWSTR result = stemp.c_str();
	int i = 0;
	AddFontResourceEx(result, FR_NOT_ENUM, 0);
#endif

	return Platform::getPathForAppResource("Fonts/Arvin.ttf"); // unused?
}

ToolBarOptions& EditorConfiguration::getToolBarOptions()
{
	static ToolBarOptions tbOptions(kToolbarSettingsOptions,
                                    kSetupZoomOptions,
                                    VstSynthName,
                                    VstSynthVersion,
                                    VstSynthCopyright,
                                    VstSynthInfoFileName);

	tbOptions.m_rMainRects = m_editorContentView->getRectsNeededForToolBar();

	return tbOptions;
}

ZoomableEditorContentView* EditorConfiguration::getContentView(ZoomableEditorController& parentController,
                                                               AppearanceController& appearanceController,
                                                               PreferencesRegistry& prefs,
                                                               ScaledFontFactory* scaledFontFactory,
                                                               EditorToPluginCoreInterface& notifyPluginCore,
                                                               float* sharedFloatPool)
{
	m_editorContentView = new EditorContentView(parentController,
	                                            appearanceController,
												prefs,
	                                            scaledFontFactory,
	                                            notifyPluginCore,
	                                            m_meteringData,
	                                            sharedFloatPool);
		

	return m_editorContentView;
}

void EditorConfiguration::releaseContentView(ZoomableEditorContentView *view)
{
	delete m_editorContentView;
	m_editorContentView = 0;
}
