//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2020
//****************************************************************************************************//

#pragma warning( disable : 4244)// 'initializing' : conversion from 'VSTGUI::CCoord' to 'const int', possible loss of data

#include "FlatGUI/CMTSliders.h"
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
// CMTDisplay implementation
//

CMTSliders::CMTSliders (const CRect &size, IControlListener* listener, const int32_t tags[12], 
						ScaledFontFactory& fontFactory,  const ConfigSlider& configSlider) 
						: CView(size), m_configSlider(configSlider)
{

	for (size_t j = 0; j < noteDisplayed; ++j)
	{ 
		centsParam[j] = 0.0f;
		cents[j] = 0;
		tags_[j] = tags[j];
	}
	tagEdit = -1;

	w = size.getWidth();
	h = size.getHeight();
	ratio = 0.00235*w; // for zoom	

	bMouseOn = false;

	Make_Slider_Rect;
	pSlider = new CSliderFlat(rSlider, listener, tags_[0] ,0, rSlider.getHeight(), fontFactory, m_configSlider, kNoFrame|kVertical);
	pSlider->attached(this);
	pSlider->setVisible(bMouseOn);
	pSlider->setMouseEnabled(bMouseOn);

	myFont = fontFactory.getScaledSmallFont();
}
//------------------------------------------------------------------------
void CMTSliders::draw (CDrawContext *pContext)
{	
	CDrawContext* drawContext = pContext;
	CRect fuckingFrame = size; // VSTGUI is crap
	drawContext->setClipRect(fuckingFrame.extend(1.0,0.0));
	CRect labelRectMT = size;
	labelRectMT.setHeight(102.0*ratio);
	drawContext->setFillColor(m_configSlider.colBkground1);
	drawContext->drawRect(labelRectMT, kDrawFilled);

	if(bMouseOn) // adjusting a cent value
	{

		drawContext->setDrawMode(kAliasing);
		drawContext->setLineWidth(1.0f);
		drawContext->setFrameColor(m_configSlider.colFrame);
		drawContext->setFontColor(m_configSlider.colFont); 
		drawContext->setFont(myFont, 29.0*ratio, 0);
		std::string s = "Change offset for  ";
		s.append(MTlabels[tagEdit]);
		s.append("  notes");
		drawContext->drawString(s.c_str(), size.getTopLeft().offset(14.0*ratio, 29.0*ratio));
		s = "drag up-down or left-right ";	
		drawContext->drawString(s.c_str(), size.getTopLeft().offset(14.0*ratio, 56.0*ratio));
		pSlider->draw(pContext);
	}
	else
	{

		labelRectMT.setHeight(29.0*ratio);
		drawContext->setFontColor(m_configSlider.colFont);
		drawContext->setFont(myFont, 26.0*ratio);
		drawContext->drawString("microtuning", labelRectMT);
		drawBackground(drawContext);

		drawContext->setDrawMode(kAntiAliasing);
		drawContext->setFillColor(m_configSlider.colTrackBack);
		rShift = CRect(0.0,0.0,32.0*ratio,32.0*ratio);
		rPTuning = rShift;
		rShift.offset(size.getTopLeft());
		drawContext->drawRect(rShift, kDrawFilled);
		drawContext->drawString(">>", rShift);
		rPTuning.offset(size.getTopRight().offset(-32.0*ratio, 0.0));
		drawContext->drawRect(rPTuning, kDrawFilled);
		drawContext->drawString("PT", rPTuning);
	}

	setDirty(false);
}
//------------------------------------------------------------------------
CMouseEventResult CMTSliders::onMouseMoved(CPoint& where, const CButtonState& buttons) 
{
	return pSlider->onMouseMoved(where,  buttons);

#if 0
	if(bMouseOn)
	{
		centsParam[tagEdit%12] = pSlider->getValue(); // store new value for this note
		return pSlider->onMouseMoved(where,  buttons);
	}
	else
		return  kMouseEventNotHandled;
#endif
}
//------------------------------------------------------------------------
CMouseEventResult CMTSliders::onMouseDown (CPoint& where, const CButtonState& buttons) 
{
	if (buttons & kControl) // reset
	{
		bMouseOn = false;
		for (size_t j = 0; j < noteDisplayed; ++j)
		{ 
			centsParam[j] = 0.5f;
			cents[j] = 0;

			pSlider->beginEdit();
			pSlider->setTag(tags_[j]);
			pSlider->setValue(centsParam[j]); 
			pSlider->getListener()->valueChanged(pSlider); //?
			pSlider->endEdit();
		}	

		setDirty();
		return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}	

	if (buttons.isLeftButton ())
	{
		if(rShift.pointInside(where))
		{
			float storeCP[noteDisplayed]; 
			int storeCs[noteDisplayed];
			memcpy(storeCP,centsParam, 12*sizeof(float));
			memcpy(storeCs,cents, 12*sizeof(int));

			for (size_t j = 1; j < noteDisplayed; ++j)
			{ 			
				centsParam[j] = storeCP[j-1];
				cents[j] = storeCs[j-1];						
			}
			centsParam[0] = storeCP[11];
			cents[0] = storeCs[11];	
			setDirty();

			for (size_t j = 0; j < noteDisplayed; ++j)
			{ 
				pSlider->beginEdit();
				pSlider->setTag(tags_[j]);
				pSlider->setValue(centsParam[j]); 
				pSlider->getListener()->valueChanged(pSlider); //?
				pSlider->endEdit();
			}	

			return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}
		if(rPTuning.pointInside(where))
		{
			for (size_t j = 0; j < noteDisplayed; ++j)
			{ 
				centsParam[j] = (pureTuningCentsOffset[j]+100.0f)/200.0f; // C scale 12 pure
				cents[j] = pureTuningCentsOffset[j];
				setDirty();

				pSlider->beginEdit();
				pSlider->setTag(tags_[j]);
				pSlider->setValue(centsParam[j]); 
				pSlider->getListener()->valueChanged(pSlider); //?
				pSlider->endEdit();
			}	

			return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}

		
		

		for(int j = 0; j < noteDisplayed; ++j)
		{
			tagEdit = -1;

			if(rSlider[j].pointInside(where) || rDisplay[j].pointInside(where))
			{	
				getFrame()->setCursor(kCursorSizeAll);
				bMouseOn = true;

				invalidRect(size);

				tagEdit = j;

				pSlider->beginEdit();
				pSlider->setTag(tags_[j]);
				pSlider->setValue(centsParam[tagEdit%12]); // read old value for this note
				pSlider->getListener()->valueChanged(pSlider); // set string on slider display
				pSlider->setMouseEnabled(bMouseOn);	

				Make_Slider_Rect;
				pSlider->CControl::setViewSize(rSlider, true);
				pSlider->setVisible(bMouseOn);
				pSlider->onMouseDown (where,  buttons);			

				this->setMouseEnabled(!bMouseOn);	

				return pSlider->onMouseMoved(where,  buttons);
			}				
		}
	}
	return  kMouseEventNotHandled;
}
//------------------------------------------------------------------------
CMouseEventResult CMTSliders::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	pSlider->getListener()->valueChanged(pSlider); //?
	centsParam[tagEdit%12] = pSlider->getValue(); // // store new value for this note
	tagEdit = -1;

	if(bMouseOn)
	{
		pSlider->endEdit();
		bMouseOn = false;
		pSlider->setMouseEnabled(bMouseOn);
		pSlider->setVisible(bMouseOn);
		this->setMouseEnabled(!bMouseOn);
		getFrame()->setCursor(kCursorDefault);
	}
	invalidRect(size);
	setDirty();

	return kMouseEventHandled;
}
//------------------------------------------------------------------------
void CMTSliders::setString32(char* string)
{
	pSlider->setString32(string);
	pSlider->setDirty();
}
