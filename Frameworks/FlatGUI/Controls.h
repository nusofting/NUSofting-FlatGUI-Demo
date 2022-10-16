//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2015
//****************************************************************************************************//

#pragma warning( disable : 4244)// 'initializing' : conversion from 'VSTGUI::CCoord' to 'const int', possible loss of data

#pragma once

#include "FlatGUI/Config.h"

#include "vstgui/vstgui.h"




//-------- custom objects -------
#include "FlatGUI/CKickTwiceZoom.h"
#include "FlatGUI/CSimpleLabel.h"
#include "FlatGUI/CSliderFlat.h"

//------------------------------------------------------------------------------
class CStereoPeaksView : public CControl
{
public:

	CStereoPeaksView(const CRect& size, const ConfigGraphView& configGraphView, bool bMakeFrame = true, const CHoriTxtAlign hAlign = kCenterText);
	~CStereoPeaksView();
	void setString128(const char* string);
	void draw (CDrawContext *pContext);
	void setValues(float& var1, float& var2);
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	void resetPeak();
	void setDisplayFont(CFontRef asFont = 0);
	void setSingleChannel(const bool value)
	{
		bSingleChannel = value;
	}

	CLASS_METHODS(CStereoPeaksView, CControl)

private:

	char display[128];
	bool bDrawFrame;
	CHoriTxtAlign hAlign_xx;
	float decreaseValue;
	float value1,value2;
	float oldValue1,oldValue2;
	CCoord iMaxValue1,iMaxValue2;
	bool bSingleChannel;

	void Clamp(float& value);

	const ConfigGraphView& m_configGraphView;

	CFontRef myFont;

}; // end class CStereoPeaksView 

//------------------------------------------------------------------------------
class CKnobFlatSwitch : public CControl
{
public:

	CKnobFlatSwitch (const CRect &size, IControlListener* listener, long tag, const ConfigKnob& configKnob, int steps = 0);
	virtual	~CKnobFlatSwitch ();
	virtual	void setString32(char* string);
	virtual	void draw (CDrawContext *pContext);
	virtual	void drawHandle (CDrawContext *pContext);
	virtual	void valueToPointOffset (CPoint &point1, CPoint &point2, float value) const;
	virtual	void drawString (CDrawContext *pContext);
	virtual void setActive(bool var);
	virtual const bool getActive()
	{
		return bActive;
	}
	virtual bool onWheel (const CPoint& where, const float& distance, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;

	const char* getString()
	{
		if(sizeof(display) > 0)
			return display;
		else 
			return nullptr;
	}

	//void setValueQuantized(int var);

	//int getValueQuantized();

	virtual	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	virtual	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);
	virtual	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);
	virtual	CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons);
	virtual	void setStartAngle (float val);
	virtual	void setRangeAngle (float val);
	virtual	float getStartAngle () const { return startAngle; }
	virtual	float getRangeAngle () const { return rangeAngle; }
	virtual	void compute ();
	virtual	void valueToPoint (CPoint &point) const;
	virtual	void zeroToPoint (CPoint &point) const;
	virtual	void pointToValue (CPoint &point, float value, CCoord radius)  const;
	virtual	float valueFromPoint (CPoint &point) const; // used only with Alt+Mouse
	void setConfig(bool drawCircle2, CCoord cursOffsetRel);
	void setDisplayFont(CFontRef asFont = 0);

	CLASS_METHODS(CKnobFlatSwitch, CControl)

private:
	void quantizeValue();
	void drawEdgeLines(CDrawContext* pContext,CPoint &point1, CPoint &point2, float value) const;

protected:

	bool bDrawCirle2;

	int halfCurveSwitch;

	const ConfigKnob& m_configKnob;

	char display[32];

	CCoord iMaxPos, iMinPos;
	CCoord iCursOffset; CCoord iCursOffsetRel;


	CRect rectOld;

	bool bReverse;
	int steps;

	CFontRef myFont;

	CPoint offset;

	long     inset;
	float    startAngle, rangeAngle, halfAngle;
	float    aCoef, bCoef;
	float    radius;
	float    zoomFactor;
	bool	 bMouseOn;
	bool	bActive;

private:
	CPoint firstPoint;
	CPoint lastPoint;
	float  startValue;
	float  fEntryState;
	float  range;
	float  coef;
	CButtonState   oldButton;
	bool   modeLinear;

};// end class CKnobFlatSwitch

//------------------------------------------------------------------------------
class CKnobFlatBranis : public CKnobFlatSwitch
{
public:

	CKnobFlatBranis(const CRect &size, IControlListener* listener, long tag, const ConfigKnob& configKnob, int steps = 0);
	void draw (CDrawContext *pContext);
	void drawHandle (CDrawContext *pContext);
	void valueToPointOffset (CPoint &point1, CPoint &point2, float value, CCoord iCursLength) const;
	void drawString (CDrawContext *pContext);


	//void setValueQuantized(int var);

	//int getValueQuantized();

	void DoCurve(bool var, bool rev);
	void SeeValue(bool var);

	CLASS_METHODS(CKnobFlatBranis, CKnobFlatSwitch)

private:

	CCoord iCursLength;

	void quantizeValue();
	void drawEdgeLines(CDrawContext* pContext,CPoint &point1, CPoint &point2, float value, CCoord iCursLength = 0) const;
	void drawBarsLines(CDrawContext* pContext, bool bLeftRight) const;

	bool bMakeCurve;
	bool bShowValue;

};// end class CKnobFlatBranis

//------------------------------------------------------------------------------
class CSliderSpot : public CSlider
{
public:

	CSliderSpot (const CRect &size, IControlListener* listener, long tag, long iMinPos, long iMaxPos, const long style = kLeft|kHorizontal);
	virtual	~CSliderSpot();
	void setString32(const char* string);
	void draw (CDrawContext *pContext);
	void drawString (CDrawContext *pContext);
	void ColoursAndConfig(struct ConfigSlider& configSlider );
	void setScaleGUIx(float var);
	void setDisplayFont(CFontRef asFont = 0);
	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);
	CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons);
	CMouseEventResult onMouseExited (CPoint& where,const CButtonState& buttons);

	CLASS_METHODS(CSliderSpot, CSlider)

private:

	char display[32];

	float	scaleGUIx;

	CCoord iMaxPos, iMinPos;

	CRect rectOld;

	CColor colorBkground1, colorFrame, colorFont, colorHandle, colorFrameHandle;

	CFontRef myFont;
	bool bMouseOn;
};// end class CSliderSpot

//------------------------------------------------------------------------------
class CSimpleOnOff : public CControl
{
public:
	CSimpleOnOff(CRect& size, IControlListener* pListener, long tag, const ConfigSwitch& configSwitch);
	// Why do we need this? It should just be inherited: long getTag(){ return CControl::getTag(); }
	void draw(CDrawContext *pContext);
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	virtual void setActive(bool var);
	virtual void setString32(const char* inLabel = 0);
	void use3D(bool var) { bDoing3D = var; }

	CLASS_METHODS(CSimpleOnOff, CControl)

private:

	float step;
	CRect rectOld;
	bool bActive;
	const ConfigSwitch& m_configSwitch;
	bool bAddDisplay;
	char sString[32];
	bool bDoing3D;
	

}; // end class CSimpleOnOff

//------------------------------------------------------------------------------
class CMultiSwitch : public CControl // horizontal switch max 12 positions
{
public:
	CMultiSwitch(CRect& size, IControlListener* pListener, long tag, int pos, ScaledFontFactory& fontFactory, const ConfigSwitch& configSwitch);
	~CMultiSwitch();
	void setMomentary(const bool value);
	void setLabels(const char** Labels);
	void setHorizontal(bool mode);
	// needed?: long getTag(){ return CControl::getTag(); }
	void draw(CDrawContext *pContext);
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	virtual void setActive(bool var);
	void setLabelsGapV(CCoord var);
	virtual void setSwitchBkground(bool var);
	void setBkgdAlpha(int var);
	void setDrawLink(bool var)
	{
		bLinkDraw = var;
	}

	CLASS_METHODS(CMultiSwitch, CControl)

private:

	float step;
	CRect rectOld;
	int pos;
	char* labels[12];
	bool bActive;
	ScaledFontFactory& m_scaledFontFactory;
	const ConfigSwitch& m_configSwitch;
	CCoord labelsGapV;
	bool bSwitchBkground;
	bool bHorizontal;
	bool bMomentarySwitch;
	uint8_t iAlpha;
	bool bLinkDraw;

}; // end class CMultiSwitch

//------------------------------------------------------------------------------
class CLFODisplay : public CView
{
public:
	CLFODisplay(const CRect &size);
	void draw (CDrawContext *pContext);
	void setShape(int var);

	CLASS_METHODS(CLFODisplay, CView)

private:

	class LFOx;

	LFOx* lfo1;
	int XS;
	bool bSmoothed;

};

//------------------------------------------------------------------------------
class CSliderNano : public CSlider
{
public:

	CSliderNano (const CRect &size, IControlListener* listener, long tag, const ConfigSlider& configSlider, const long style = kLeft|kHorizontal);
	void draw (CDrawContext *pContext);
	void setScaleGUIx(float var);
	virtual void setActive(bool var);

	void setSquareHandle(bool mode)
	{
		bSquareView = mode;
	}
	void setShadow(bool mode)
	{
		bDrawLittleShadows = mode;
		setDirty();
	}

	void setValueUpDownDir(bool mode)
	{
	    bUpDownDir = mode;
		setDirty();
	}

	CLASS_METHODS(CSliderNano, CSlider)

private:

	//float	scaleGUIx;

	//CCoord iMaxPos, iMinPos;

	CRect rectOld;

	const ConfigSlider& m_configSlider;

	bool bSquareView;
	bool bActive;
	bool bDrawLittleShadows;
	bool bUpDownDir;

};// end class CSliderNano


//------------------------------------------------------------------------------
class CFilterPlot : public CControl
{
public:
	CFilterPlot(const CRect &size);
	void draw (CDrawContext *pContext);
	void setType(int var);

	CLASS_METHODS(CFilterPlot, CControl)

private:

	int iType;
}; // end class CFilterPlot



//------------------------------------------------------------------------------
class CKickPreviousNext : public CControl
{
public:
	CKickPreviousNext(CRect& size, IControlListener* pListener, long tag);
	~CKickPreviousNext();
	// needed?: long getTag(){ return CControl::getTag(); }
	void draw(CDrawContext *pContext);
	void drawPoints(CDrawContext *pContext, CRect rect, int which);
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons);
	void ColoursAndConfig(struct ConfigKick& configKick );

	CLASS_METHODS(CKickPreviousNext, CControl)

private:

	CRect* zone[2];
	float step;
	CRect rectOld;
	CColor colorBkground1;
	CColor colorBkground2;
	CColor colorFrame; 
	CColor colorFont;
	bool bMouseOn1,bMouseOn2;

}; // end class CKickPreviousNext

//------------------------------------------------------------------------------
class CPlotDisplay : public CView
{
public:
	CPlotDisplay(const CRect &size, const ConfigGraphView& configGraphView);
	void draw (CDrawContext *pContext);
	void setPlotEQ(const float* inArray);

	CLASS_METHODS(CPlotDisplay, CView)

private:

	float arrayPlotEQ[512];
	const ConfigGraphView& m_configGraphView;

};


//------------------------------------------------------------------------------
class CGain2OnOff : public CControl
{
public:
	CGain2OnOff(CRect& size, IControlListener* pListener, long tag, const ConfigSwitch& configSwitch);
	// needed?: long getTag(){ return CControl::getTag(); }
	void draw(CDrawContext *pContext);
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	virtual void setActive(bool var);

	CLASS_METHODS(CGain2OnOff, CControl)

private:

	float step;
	CRect rectOld;
	bool bActive;
	const ConfigSwitch& m_configSwitch;

}; // end class CGain2OnOff
