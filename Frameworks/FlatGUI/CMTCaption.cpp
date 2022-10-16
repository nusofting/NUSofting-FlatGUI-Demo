//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2020
//****************************************************************************************************//

#pragma warning( disable : 4244)// 'initializing' : conversion from 'VSTGUI::CCoord' to 'const int', possible loss of data

#include "FlatGUI/CMTCaption.h"
#include "FlatGUI/editor_colours.hpp"
#include "FlatGUI/FontSizeFactors.hpp"
#include "../DashLibrary/mathFunctions.hpp"

#include "vstgui/lib/platform/iplatformfont.h"

//==============================================================================
//
// CMTCaption implementation
//

CMTCaption::CMTCaption (const CRect &size, 	ScaledFontFactory& fontFactory,  const ConfigSlider& configSlider) 
						: CView(size), m_configSlider(configSlider)
{
	setMsgs(0);
}
//------------------------------------------------------------------------
void CMTCaption::draw (CDrawContext *pContext)
{	
	CDrawContext* drawContext = pContext;
	drawContext->setFrameColor(m_configSlider.colFrame);
	drawContext->setDrawMode(kAliasing);
	drawContext->setLineWidth(1.0f);

	const CCoord ratio = 0.01*size.getWidth();
	CRect innerFrame = size;
	innerFrame.inset(12.0*ratio,12.0*ratio);
	drawContext->drawRect(innerFrame, kDrawStroked);
		
	drawContext->setFontColor(m_configSlider.colTrackFront); 
	drawContext->setFont(drawContext->getFont(), 8.0*ratio, 0);
	innerFrame.inset(2.0*ratio, -6.0*ratio);
	drawContext->drawString(msg1, innerFrame.offset(0.0, -5.0*ratio));
	drawContext->drawString(msg2, innerFrame.offset(0.0, 10.0*ratio));

	setDirty(false);
}