//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2020
//****************************************************************************************************//

#pragma warning( disable : 4244)// 'initializing' : conversion from 'VSTGUI::CCoord' to 'const int', possible loss of data

#include "FlatGUI/CKSDisplay.h"
#include "FlatGUI/editor_colours.hpp"
#include "FlatGUI/FontSizeFactors.hpp"
#include "../DashLibrary/mathFunctions.hpp"

#include <stdio.h>
#include <math.h>
#undef max
#include <algorithm>

#include "vstgui/lib/platform/iplatformfont.h"



//==============================================================================
//
// CKSDisplay implementation
//

CKSDisplay::CKSDisplay(const CRect &size, IControlListener* listener, int32_t tag, const ConfigGraphView& configGraphView) :
	CControl(size, listener, tag), m_configGraphView(configGraphView)
{
	XS = size.getWidth(); 
	for (size_t j = 0; j < noteDisplayed; ++j)
	{ 
		levels[j] = 0.5f;
	}
	fCenterNoteParm_ = 12;

	bMouseOn = false;
}

void CKSDisplay::draw (CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;
	drawContext->setDrawMode(kAntiAliasing);
	const int w = size.getWidth();
	const int h = size.getHeight();

	//draw grid

	const CCoord stepX = w/80.0f;
	drawContext->setLineWidth(stepX);
	for(int x = 0; x <= w; x += int(2.0*stepX+0.5)) 
	{
		CPoint Y2 = CPoint(x, h);
		CPoint Y1 = CPoint(x, 0);
		Y2.offset(size.left, size.top);
		Y1.offset(size.left, size.top);
		drawContext->setFrameColor(m_configGraphView.colBkground1);
		drawContext->drawLine(Y1, Y2);		
		Y2.offset(stepX, 0);
		Y1.offset(stepX, 0);
		const CColor cStep2 = m_configGraphView.colBkground1;
		drawContext->setFrameColor(brightnessColour(cStep2, 0.1f));
		drawContext->drawLine(Y1, Y2);	
	}

	drawContext->setLineWidth(1.1);
	const CPoint X1(size.left,size.top+h*0.5+1.6);
	const CPoint X2(size.right,size.top+h*0.5+1.6);
	drawContext->drawLine(X1, X2);	


	// draw variables

	drawContext->setDrawMode(kAntiAliasing | kNonIntegralMode);
	drawContext->setFrameColor(m_configGraphView.colSignal);
	CPoint start = CPoint(0.0,  h*( 1.0 - levels[20] ) + 1.0);
	start.offset(size.left, size.top);

	for(int x = 0; x <= w; x += int(stepX+0.5))
	{	
		const int indexArray = int(x/stepX+21.5f); // displays the range supported from 20 to 100
		CPoint Y2 ( x, h*( 1.0 - levels[indexArray&127] ) + 1.0 ); 
		Y2.offset(size.left, size.top);
		drawContext->setLineWidth(1.6);
		drawContext->drawLine(start, Y2);
		start = Y2;	
	}

	const CCoord fCenterNoteX = size.left+stepX*(fCenterNoteParm_- 20.0);
	const CPoint top(fCenterNoteX,size.top);
	const CPoint bottom(fCenterNoteX,size.bottom);
	drawContext->setLineWidth(2.1);
	drawContext->setFrameColor(m_configGraphView.colClip);
	drawContext->drawLine(top, bottom);

	if(bMouseOn)
	{
		CRect hover = CRect(size);
		drawContext->drawRect( hover.inset(0.5,0.0));	
	}


	setDirty(false);
}

void CKSDisplay::setCurve(float fCenterNoteParm, float fKeytrkParm, int mode)
{
	fCenterNoteParm_ = 20.0f + 80.0f*fCenterNoteParm; // 20.0 .. 100.0 MIDI note : value zero for index of "G#0" in noteNumberToDisplay()
	linExpWrap(levels, fCenterNoteParm_, fKeytrkParm, mode);

	for (size_t j = 0; j < 128; ++j) // fit to display
	{ 
		levels[j] -= 0.5f;
		levels[j] = CLAMP(levels[j], 0.05f, 0.99f);	
	}

	setDirty();
}


CMouseEventResult CKSDisplay::onMouseEntered (CPoint& where, const CButtonState& buttons) 
{
	bMouseOn = true;
	setDirty();
	return  kMouseEventHandled;
}
CMouseEventResult CKSDisplay::onMouseExited (CPoint& where, const CButtonState& buttons) 
{
	bMouseOn = false;
	setDirty();
	return  kMouseEventHandled;
}
CMouseEventResult CKSDisplay::onMouseDown (CPoint& where, const CButtonState& buttons) 
{
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	value = oldValue;

	beginEdit ();	
	if(value == 0.0f) value = 1.0f; else value = 0.0f;
	if(listener) listener->valueChanged(this);
	endEdit();

	setDirty();
	return  kMouseEventHandled;
}