//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2020
//****************************************************************************************************//

#pragma warning( disable : 4244)// 'initializing' : conversion from 'VSTGUI::CCoord' to 'const int', possible loss of data

#include "FlatGUI/CRandWidget.h"
#include "FlatGUI/editor_colours.hpp"
#include "FlatGUI/FontSizeFactors.hpp"
//#include "../DashLibrary/mathFunctions.hpp"

#include <stdio.h>
#include <math.h>
#undef max
#include <algorithm>

#include "vstgui/lib/platform/iplatformfont.h"

//==============================================================================
//
// CRandWidget implementation
//

CRandWidget::CRandWidget (const CRect &size, IControlListener* listener, ScaledFontFactory& fontFactory)
	: CControl(size,listener,-1)	

{
	w = size.getWidth();
	h = size.getHeight();
	ratio = 0.00235*w; // for zoom	
	moveX = 0.0;
	mouseStartPoint = CPoint(0.0,0.0);
	delta = 0.0f;
	actualPos = 0.0;

	sizeOld = size;

	bMouseOn = false;
}
//------------------------------------------------------------------------
void CRandWidget::draw (CDrawContext *pContext)
{	
	CRect trueSize = CRect(size);
	//trueSize.left += 1.0;
	trueSize.bottom -= 0.5;

	if(sizeOld != size)
	{
		w = trueSize.getWidth();
		h = trueSize.getHeight();
	}
	sizeOld = size;

	CDrawContext* drawContext = pContext;

	drawContext->setDrawMode(kAntiAliasing|kNonIntegralMode);
	drawContext->setFrameColor(kGreyCColor);
	drawContext->setFillColor(kDarkCColor);
	drawContext->setLineWidth(5.0*ratio);

	drawContext->drawRect(trueSize, kDrawFilledAndStroked);	

	drawContext->setLineWidth(9.0*ratio);
	drawContext->setFrameColor(kLightCColor);

	for(int j = 0; j < int(w)-1; ++j)
	{
		static const double fix = 1.0/(double(RAND_MAX)+1.0);
		const float dither = float(h*rand()*fix);
		double scale = j/w;
		scale = scale*scale;
		CPoint pDot(j, 0.5*h*(1.0-scale) + scale*dither);
		pDot.offset(trueSize.getTopLeft());
		const CPoint pEnd(pDot.x-1.6,pDot.y-1.6);
		drawContext->drawLine(pDot, pEnd);			
	}

	if(bMouseOn) 
	{
		CRect rSpot(0.0, 0.0, h-1.0, h-1.0);
		rSpot.offset(trueSize.getTopLeft());
		rSpot.inset(2.5, 2.5);
		rSpot.offset(moveX, 1.0);	

		normValue01 = 1.2f* (normValue+0.6666666667f)/1.666666667f;

		normValue01 = normValue01 < 0.0f? 0.0f : normValue01 > 1.0f ? 1.0f : normValue01;

		const float r = sinf(normValue01*(3.14f*0.5f));
		const float g = cosf(normValue01*(3.14f*0.5f));	

		CColor kSpotCColor;
		kSpotCColor.red = int(r * 255.0f);
		kSpotCColor.green = int(g * 255.0f);
		kSpotCColor.blue = 0;
		kSpotCColor.alpha = 189;

		drawContext->setFrameColor(kSpotCColor);	
		drawContext->drawEllipse(rSpot);
		drawContext->setLineWidth(5.0*ratio);
		drawContext->drawEllipse(rSpot.extend(1.4,1.4));
	}

	//drawContext->setFontColor(kRedCColor);
	//char value[128] = {0};
	//sprintf(value, "%1.2f", normValue01*normValue01);
	//drawContext->drawString(value, size);

	setDirty(false);
}
//------------------------------------------------------------------------
CMouseEventResult CRandWidget::onMouseMoved(CPoint& where, const CButtonState& buttons) 
{
	setDirty();

	if(bMouseOn)
	{
		CCoord rangeHandle = w - h - 3.0; // because of borders
		delta = calculateDelta (where, rangeHandle);
		normValue = (float)(where.x - delta) / (float)rangeHandle;

		moveX = normValue*(w-h)+h;
		if(moveX < 1.0 ) moveX =  1.0;
		else if(moveX > (w-h) ) moveX = (w-h);

		getListener()->valueChanged(this);

		return  kMouseEventHandled;
	}	

	return  kMouseEventNotHandled;

}
//------------------------------------------------------------------------
CMouseEventResult CRandWidget::onMouseDown (CPoint& where, const CButtonState& buttons) 
{
	if (buttons.isLeftButton ())
	{
		if(size.pointInside(where))
		{
			getFrame()->setCursor(kCursorSizeAll);
			bMouseOn = true;

			CCoord rangeHandle = w - h - 3.0;
			delta = calculateDelta (where, rangeHandle);
			mouseStartPoint = where;

			return onMouseMoved (where, buttons);
		}
	}	

	return  kMouseEventNotHandled;
}
//------------------------------------------------------------------------
CMouseEventResult CRandWidget::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	if(bMouseOn)
	{
		getFrame()->setCursor(kCursorDefault);
		bMouseOn = false;

	}
	invalidRect(size);
	setDirty();

	return kMouseEventHandled;
}
//------------------------------------------------------------------------

float CRandWidget::calculateDelta (const CPoint& where, float rangeHandle)
{
	CCoord result = getViewSize ().left;	
	result += w / 2.0 - 1.0;

	return (float)(result);
}
