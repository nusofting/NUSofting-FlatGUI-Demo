//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2020
//****************************************************************************************************//

#pragma warning( disable : 4244)// 'initializing' : conversion from 'VSTGUI::CCoord' to 'const int', possible loss of data

#pragma once

#include "FlatGUI/Config.h"

#include "vstgui/vstgui.h"


const CColor kDarkCColor = CColor (45, 45, 45, 255);
const CColor kLightCColor = CColor (155, 155, 155, 255);

//------------------------------------------------------------------------------
class CRandWidget : public CControl
{
public:
	CRandWidget (const CRect &size, IControlListener* listener,	ScaledFontFactory& fontFactory);
	~CRandWidget()
	{

	}
	void draw (CDrawContext *pContext);

	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);	
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);	
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);
	
	float getValueNormalized() { return normValue01*normValue01; }

	void setViewSize(CRect& newSize)
	{
		CView::setViewSize(newSize);

		size = newSize;
		w = newSize.getWidth();
		h = newSize.getHeight();
		ratio = 0.00235*w; // for zoom	
	}


	CLASS_METHODS(CRandWidget , CControl)

private:
	

	int32_t XS;
	bool bMouseOn;
	CCoord moveX;
	CPoint mouseStartPoint;
	float delta;
	CCoord actualPos;

	float normValue;

	float normValue01;

	CCoord w;
	CCoord h;
	CCoord ratio;

	CRect sizeOld;

float calculateDelta (const CPoint& where, float rangeHandle);
	


};