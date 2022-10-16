//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2015
//****************************************************************************************************//

#pragma warning( disable : 4244)// 'initializing' : conversion from 'VSTGUI::CCoord' to 'const int', possible loss of data

#include "FlatGUI/CEnvDisplay.h"
#include "FlatGUI/editor_colours.hpp"
#include "FlatGUI/FontSizeFactors.hpp"

#include <stdio.h>
#include <math.h>
#undef max
#include <algorithm>

#include "vstgui/lib/platform/iplatformfont.h"



//==============================================================================
//
// CEnvDisplay implementation
//

CEnvDisplay::CEnvDisplay(const CRect &size, const ConfigGraphView& configGraphView): CView(size), m_configGraphView(configGraphView)
{
	env1.Reset();
	env1.setSR(size.getWidth()*0.25f);
	env1.setAttackTime(0.5f); // seconds
	env1.setDecayTime(0.5f); // seconds
	env1.setSustainLevel(0.5f);
	env1.setReleaseTime(0.5f); // seconds
	XS = size.getWidth(); 
	bBackLayer = false;
	iNumber = 1;
}

void CEnvDisplay::draw (CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;
	//CColor locCol = m_configGraphView.colBkground1;
	//locCol.alpha = 154;
	//drawContext->setFillColor(locCol);
	drawContext->setFrameColor(m_configGraphView.colFrame);
	//drawContext->setDrawMode(kAliasing);
	//drawContext->drawRect(size,kDrawFilled);

	drawContext->setDrawMode(kAntiAliasing | kNonIntegralMode);
	CPoint start(size.getBottomLeft().offset(2,-5));	
	drawContext->setFrameColor(m_configGraphView.colHandle);
	drawContext->setLineWidth(bBackLayer? 0.25f : 1.4f);
	const int w = size.getWidth();

	//env1.Reset();
	env1.keyOn(127);
	for(int x = 2; x < w; ++x)
	{			
		float level = env1.tick();
		const int h = size.getHeight()-5;
		CPoint Y2 = CPoint(x, h-env1.tick()*h + 1);
		Y2.offset(size.left, size.top);

		if(x == w/2+10) 
		{
			env1.keyOff(); // 10 of sustain
		}
	
		if(!bBackLayer)
		{
			drawContext->setFrameColor(kGreyCColor);
			CPoint s1 = start;
			CPoint s2 = Y2;
			drawContext->drawLine(s1.offset(1.0f, 1.4f), s2.offset(1.0f, 1.4f));
			drawContext->setFrameColor(m_configGraphView.colHandle);
		}
		drawContext->setLineWidth(bBackLayer? 0.25f : 1.4f);
		drawContext->drawLine(start, Y2);

		start = Y2;

		if (level < 0.0001f)
		{break;}
	}


	if(!bBackLayer)
	{
		const double c1 = w*0.96;
		const double c2 = w*0.08;
		CPoint pNumber(c1, c2);
		pNumber.offset(size.left, size.top);	
		drawContext->setFontColor(m_configGraphView.colHandle);
		drawContext->drawString(iNumber == 2? "2" : "1",  pNumber);
	}

	setDirty(false);
}

void CEnvDisplay::attack(float var)
{
	env1.setAttackTime(var+0.05f); // seconds , offset for the draw
	setDirty();
}
void CEnvDisplay::decay(float var)
{
	env1.setDecayTime(var); // seconds
	setDirty();
}
void CEnvDisplay::sustain(float var)
{
	env1.setSustainLevel(var+0.001f);
	setDirty();
}
void CEnvDisplay::release(float var)
{
	env1.setReleaseTime(var); // seconds
	setDirty();
}
void CEnvDisplay::ratio(float var)
{
	env1.setTargetRatio(var); // seconds
	setDirty();
}




