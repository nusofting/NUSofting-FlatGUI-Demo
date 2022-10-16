#pragma once
#include "vstgui/vstgui.h"
#include "FlatGUI/Config.h"

class CSliderDuos : public CControl
{
public:

	CSliderDuos (const CRect &size, IControlListener* listener, long tag1, long tag2, long tagN,
		ScaledFontFactory& fontFactory, const ConfigSlider& configSlider, const long style = kLeft|kHorizontal);

	void draw (CDrawContext *pContext);

	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);	
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseEntered (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons);
	bool checkDefaultValue (CButtonState button, CPoint& where);

	virtual void setValue1(float value)
	{
		getListener()->valueChanged(this);
		changed (kMessageValueChanged);
		//setValue(value);
		curVal1 = value;
	}
	virtual void setValue2(float value)
	{
		getListener()->valueChanged(this);
    	changed (kMessageValueChanged);
		//setValue(value);
		curVal2 = value; 
	}
	virtual float getValue1()
	{
		return curVal1;
	}
	virtual float getValue2()
	{
		return curVal2;
	}
	void setValueM(float value)
	{
		curValM = value; 
	}
	float getValueM()
	{
		return curValM;
	}

	virtual	void setString32A(char* string);
	virtual	void setString32B(char* string);

	void setActive(bool var)
	{
		bActive = var;
		setDirty();
	}

	void reverseColours(bool var)
	{
		bSwapDisplayColours = var;
	}

	CLASS_METHODS(CSliderDuos, CView)

private:

	CControl* pCtrl1;
	CControl* pCtrl2;

	CRect rHandle1, rHandle2;
	CRect rDisplay;
	CRect rMiddle;
	CRect rectValue;

	bool bMouseOverMiddle;

	CCoord iMaxPos1, iMinPos1;
	CCoord iMaxPos2, iMinPos2;

	CCoord rangeHandle, widthOfSlider; 
	CCoord widthControl, heightControl;

	float oldVal1;
	float curVal1;
	float delta1;
	float oldVal2;
	float curVal2;
	float curValM;
	float delta2;
	float delta3;

	bool bActive;
	ScaledFontFactory& m_scaledFontFactory;
	bool bSwapDisplayColours;
	const ConfigSlider& m_configSlider;
	char sideLeft[32];
	char sideRight[32];
	CCoord glyphsVGap;
	bool use1,use2,use3;

	int32_t tag1_, tag2_;

	int32_t oldButton;

	void initRects();

	void bounceValue (float& value)
	{
		if (value > 1.0f) 	    value = 1.0f;
		else if (value < 0.0f) 	value = 0.0f;

		//value = 1.0f - value;
	}

	char display1[32];
	char display2[32];

};// end class CSliderDuos