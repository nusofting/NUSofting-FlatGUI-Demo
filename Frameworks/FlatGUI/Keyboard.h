/*-----------------------------------------------------------------------------

© 2016 nusofting.com - Luigi Felici (aka Liqih)

-----------------------------------------------------------------------------*/

#pragma once
#include "vstgui/vstgui.h"

#if !defined(__Keyboard_H)
#define __Keyboard_H


//-----------------------------------------------------------------------------
class Keyboard : public CControl
{
public:
	Keyboard(const CRect &size, IControlListener *listener, long tag,
		int aNNotes, int aFirstNote = 0);  // aFirstNote = 0 means C

	~Keyboard();

	virtual void draw(CDrawContext *pContext);
	CMouseEventResult onMouseDown (CPoint &where, const CButtonState &buttons);
	CMouseEventResult onMouseUp (CPoint &where, const CButtonState &buttons);
	CMouseEventResult onMouseMoved (CPoint &where, const CButtonState &buttons);
	virtual void setViewSize (const CRect& newSize, bool invalid);
	void noteOn(int Note);
	void noteOff(int Note);
	void reset(void);
	int getKeyPressed();
	CLASS_METHODS(Keyboard, CControl)

private: 

	enum
	{
		kUpC,kUpD,kUpE,kUpS,kUpN,
		kDownC,kDownD,kDownE,kDownS,kDownN	
	};

	void drawKey (COffscreenContext* oc, const CRect& rect, bool playing, int iType = 0);
	void buildZones (const CRect& rect);

	CColor cFrameColour, cClearColour1, cClearColour2, cClearColour3, cDarkColour1,  cDarkColour2, cDarkColour3, cDarkColour4;
	CCoord witdhW, heightW, witdhB, heightB;
	CCoord witdhBhalf, witdhBside, ShadowHeightTop, ShadowHeightBottom;
	CRect rs[128];
	int NNotes;
	int FirstNote;
	int KeyPresed;
	int indexRect;
	int NZones;
	int Keys[128];
	CPoint p;	
	
CColor luminosity(CColor target, float value)
{
	target.red = uint8_t(target.red*value)&255;
	target.green = uint8_t(target.green*value)&255;
	target.blue = uint8_t(target.blue*value)&255;

	return target;
}

void bottomShadow (COffscreenContext* oc, const CRect& rect);

};

//-----------------------------------------------------------------------------

#endif
