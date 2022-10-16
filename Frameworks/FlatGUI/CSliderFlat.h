#pragma once
#include "vstgui/vstgui.h"
#include "FlatGUI/Config.h"

class CSliderFlat : public CSlider
{
public:

	CSliderFlat (const CRect &size, IControlListener* listener, long tag, long iMinPos, long iMaxPos,
	             ScaledFontFactory& fontFactory, const ConfigSlider& configSlider, const long style = kLeft|kHorizontal);
	
	virtual	void setString32(char* string);

	void draw (CDrawContext *pContext);

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);	
	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);
	CMouseEventResult onMouseEntered (CPoint& where, const CButtonState& buttons);
	CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons);
	bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;

	virtual void setActive(bool var)
	{
		bActive = var;
		//if(bActive)
		//	setMouseEnabled(true);
		//else
		//	setMouseEnabled(false);

		setDirty();
		invalid();
	}

	virtual void reverseColours(bool var)
	{
		bSwapDisplayColours = var;
	}

	virtual void addLayer(bool var)
	{
		bUseSliderLayerFlat = var;
	}

	virtual void addSideLabels(bool var, const char* lb1, const char* lb2)
	{
		if(strlen(lb1) > 1) strncpy(sideLeft,lb1,31);
		if(strlen(lb2) > 1) strncpy(sideRight,lb2,31);

		bUseSideLabels = var;
	}

	void setScaleGUIx(float var)
	{
		scaleGUIx = var;
	}

	void setFontBlack()
	{
		static_cast<CColor>(m_configSlider.colFont) = MakeCColor(0,0,0,255);
	}

	void setDefaultValue(float val)
	{
		CControl::setDefaultValue(val);
	}
	

	CLASS_METHODS(CSliderFlat, CSlider)

private:

	char display[32];

	float	scaleGUIx;

	CCoord iMaxPos, iMinPos;

	CRect rectOld;

	const ConfigSlider& m_configSlider;

	float oldVal;
	float curVal;
	float delta;

	bool bActive;
	CCoord startHPoint,startVPoint;
	ScaledFontFactory& m_scaledFontFactory;
	bool bSwapDisplayColours;
	bool bUseSliderLayerFlat;
	bool bUseSideLabels;
	char sideLeft[32];
	char sideRight[32];
	CCoord glyphsVGap;
	bool bMouseOn;

	void drawMouseOverFrame(CDrawContext* drawContext);

};// end class CSliderFlat
