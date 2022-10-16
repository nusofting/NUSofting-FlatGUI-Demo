//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2015
//****************************************************************************************************//

#include "FlatGUI/CSliderFlat.h"
#include "FlatGUI/editor_colours.hpp"
#include "FlatGUI/FontSizeFactors.hpp"

CSliderFlat::CSliderFlat (const CRect &size, IControlListener* listener, long tag, long iMinPos, long iMaxPos,
						  ScaledFontFactory& fontFactory, const ConfigSlider& configSlider, const long style)
						  : CSlider (size, listener, tag,  iMinPos,  iMaxPos, NULL, NULL, CPoint (0, 0), style),
						  m_configSlider(configSlider),
						  m_scaledFontFactory(fontFactory)
{
	setMode(kTouchMode); // in VSTGUI 4.0.1 this was: bFreeClick = false;

	if (style & kHorizontal)
	{
		widthOfSlider  = size.getWidth ()-iMaxPos; // handle 
		heightOfSlider = size.getHeight ();
	}
	else
	{
		widthOfSlider  = size.getWidth (); // handle 
		heightOfSlider = size.getHeight ()-iMaxPos;
	}

	widthControl  = size.getWidth (); // background
	heightControl = size.getHeight ();

	if (style & kHorizontal)
	{
		minPos = iMinPos - size.left;
		rangeHandle = (CCoord)iMaxPos - iMinPos;
		CPoint p (0, 0);
		setOffsetHandle (p);
	}
	else
	{
		minPos = iMinPos + size.top;
		rangeHandle = (CCoord)iMaxPos - iMinPos;
		CPoint p (0, 0);
		setOffsetHandle (p);
	}

	if(style & kNoFrame) // only draggable number
	{
		minPos = iMinPos + size.top;
		rangeHandle = 12.0*size.getHeight ();
		oldVal = getValueNormalized ();
		delta = getViewSize ().top + offsetHandle.y;
		heightControl = rangeHandle;
	}

	setOffsetHandle (offsetHandle);

	zoomFactor = 15.2f;

	setWantsFocus (true);

	curVal = getValueNormalized ();

	memset(display,0,sizeof(char)*32);	

	scaleGUIx = 1.0f;

	this->iMaxPos = iMaxPos;
	this->iMinPos = iMinPos;

	rectOld = size;

	bActive = true;
	startHPoint = 0.0;
	startVPoint = 0.0;

	bSwapDisplayColours = false;
	bUseSliderLayerFlat = false;
	bUseSideLabels = false;

	memset(sideLeft,0,sizeof(sideLeft));
	memset(sideRight,0,sizeof(sideRight));

	glyphsVGap = 1.0;// good for Windows

	bMouseOn = false;
}


void CSliderFlat::setString32(char* string)
{
	if(strlen(string) > 0)
		strncpy(display,string,32);
	else
		strncpy(display,"",1);
}

void CSliderFlat::draw (CDrawContext *pContext)
{
	if(rectOld != size)
	{
		CRect newSize(size);

		iMaxPos = (newSize.right - newSize.left)-32*scaleGUIx;

		widthOfSlider  = newSize.getWidth ()-iMaxPos; // handle 
		heightOfSlider = newSize.getHeight ();

		widthControl  = newSize.getWidth (); // background
		heightControl = newSize.getHeight ();

		if (style & kHorizontal)
		{
			minPos = iMinPos - newSize.left;
			rangeHandle = (CCoord)iMaxPos - iMinPos;
			CPoint p (0, 0);
			setOffsetHandle (p);
		}
		else
		{
			minPos = iMinPos - newSize.top;
			rangeHandle = (CCoord)iMaxPos - iMinPos;
			CPoint p (0, 0);
			setOffsetHandle (p);
		}	

		if(style & kNoFrame) // only draggable number
		{
			minPos = iMinPos - newSize.top;
			rangeHandle = 12.0*newSize.getHeight();
			delta = getViewSize ().top + offsetHandle.y;
			heightControl =  rangeHandle;
		}
	}

	rectOld = size;

	CDrawContext* drawContext = pContext;

	float fValue;
	if (style & kLeft || style & kTop)
		fValue = value;
	else 
		fValue = 1.0f - value;


	if(style & kNoFrame) // only draggable number
	{	
		drawContext->setLineWidth(1);
		drawContext->setDrawMode(kAliasing);	
		drawContext->setFrameColor(m_configSlider.colFrame);
		drawContext->setFillColor((bSwapDisplayColours)? m_configSlider.colFont : m_configSlider.colBkground1);
		drawContext->drawRect(size,kDrawFilledAndStroked);
		drawContext->setFontColor((bActive)? ((bSwapDisplayColours)? m_configSlider.colBkground1 : m_configSlider.colFont)  : m_configSlider.colFontInactive);
		drawContext->setFont(m_scaledFontFactory.getScaledSmallFont());


		CRect rDisplay = CRect(size);
		//sprintf(display, "%1.2f", getValueNormalized ());
		drawContext->drawString(display, rDisplay.offset(0.0,glyphsVGap));

		if(bUseSliderLayerFlat)
		{
			CRect rSlider = CRect(size);
			rSlider.inset(1.0,1.0);
			rSlider.setWidth(value*(widthControl-2.0));
			CColor cSlider = m_configSlider.colFont;
			cSlider.alpha = 128;
			drawContext->setFillColor(cSlider);
			drawContext->drawRect(rSlider,kDrawFilled);
		}

		if(bUseSideLabels )
		{
			CRect rSide = CRect(size);
			rSide.setWidth(0.5*widthControl);
			drawContext->drawString(sideLeft,rSide.offset(0.0,glyphsVGap),kLeftText);
			rSide.offset(0.5*widthControl,0.0);
			drawContext->drawString(sideRight,rSide,kRightText);
		}

	drawMouseOverFrame(drawContext);

		setDirty (false);

		return; //<<<<<<<<<< only drawString() used
	}

	// (re)draw background
	CRect rect (0, 0, widthControl, heightControl);
	rect.offset (size.left, size.top);
	drawContext->setFrameColor(m_configSlider.colFrame);
	drawContext->setFillColor(m_configSlider.colBkground1);
	drawContext->drawRect(rect,kDrawFilledAndStroked);


	// calc new coords of slider
	CRect  rectNew;
	if (style & kHorizontal)
	{	
		CRect rectValue (rect);
		rectValue.setWidth(widthControl/3);	
		rectValue.setHeight(heightControl-7);
		drawContext->setFontColor(m_configSlider.colFont);
		drawContext->setFont(m_scaledFontFactory.getScaledSmallFont());
		const int iHspace = widthControl/19;			
		if(value > 0.5f){
			rectValue.setTopLeft(rect.getTopLeft());
			rectValue.offset(iHspace,kVerticalCentreTextFudge);
			drawContext->drawString(display,rectValue,kLeftText);
		}else{
			rectValue.setTopLeft(rect.getTopLeft());
			rectValue.offset(widthControl-rectValue.getWidth()-iHspace,kVerticalCentreTextFudge);
			drawContext->drawString(display,rectValue,kRightText);
		}

		rectNew.top    = offsetHandle.y;
		rectNew.bottom = rectNew.top + heightOfSlider;	

		rectNew.left   = offsetHandle.x + (int)(fValue * rangeHandle);
		rectNew.left   = (rectNew.left < minTmp) ? minTmp : rectNew.left;

		rectNew.right  = rectNew.left + widthOfSlider;
		//rectNew.right  = (rectNew.right > maxTmp) ? maxTmp : rectNew.right;

		//minTmp = offsetHandle.x + minPos;
		//maxTmp = minTmp + rangeHandle + widthOfSlider;
	}
	else
	{
		rectNew.left   = offsetHandle.x;
		rectNew.right  = rectNew.left + widthOfSlider;	

		rectNew.top    = offsetHandle.y + (int)(fValue * rangeHandle);
		//rectNew.top    = (rectNew.top < minTmp) ? minTmp : rectNew.top;

		rectNew.bottom = rectNew.top + heightOfSlider;
		//rectNew.bottom = (rectNew.bottom > maxTmp) ? maxTmp : rectNew.bottom;
	}


	rectNew.offset (size.left, size.top);

	CRect rectBar (size); rectBar.left -= 1; rectBar.top -= 1;

	if (style & kHorizontal)
	{			
		rectBar.right = rectNew.left;
	}
	else
	{
		rectBar.top = rectNew.bottom;
	}
	drawContext->setFillColor(MyColours::kTransparentCColor);
	drawContext->drawRect(rectBar,kDrawFilled);

	// draw slider handle at new position		

	if (style & kHorizontal)
	{
		drawContext->setFrameColor(m_configSlider.colFrame);
		drawContext->setFillColor(m_configSlider.colHandle);	
		drawContext->drawRect(rectNew,kDrawFilledAndStroked);
		CPoint startPoint(rectNew.left+rectNew.getWidth()/2,rectNew.top);
		CPoint endPoint(rectNew.left+rectNew.getWidth()/2,rectNew.bottom);
		drawContext->drawLine(startPoint, endPoint);
	}
	else
	{
		drawContext->setFrameColor(m_configSlider.colFrame);
		drawContext->setFillColor(m_configSlider.colHandle);
		drawContext->drawRect(rectNew,kDrawFilled);
		CPoint startPoint(rectNew.left,rectNew.top);
		CPoint endPoint(rectNew.right,rectNew.top);
		drawContext->drawLine(startPoint, endPoint);
	}

	drawMouseOverFrame(drawContext);

	setDirty (false);
}

void CSliderFlat::drawMouseOverFrame(CDrawContext* drawContext)
{
	if(bMouseOn)
	{
		drawContext->setDrawMode(kAntiAliasing);
		drawContext->setLineWidth(1.0);
		CRect lite = size;
		drawContext->drawRect(lite.inset(1,1),kDrawStroked);
	}
}

CMouseEventResult CSliderFlat::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if(style & kNoFrame) // only draggable number
	{
		//if (buttons.isDoubleClick()) doesn't work here
		//{
		//	setValue(getDefaultValue());
		//	setDirty();
		//	return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		//}


		if (!(buttons & kLButton))
			return kMouseEventNotHandled;			

		beginEdit ();				

		curVal = 1.f - getValueNormalized ();

		oldVal  = getMin ()-1;	

		startHPoint = where.x;
		startVPoint = where.y;

		if (checkDefaultValue (buttons))
		{
			endEdit ();
			return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}

		return kMouseEventHandled;		
	}
	else
	{
		return CSlider::onMouseDown(where,buttons);
	}
}

CMouseEventResult CSliderFlat::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	getFrame()->setCursor(kCursorDefault);

	if(style & kNoFrame) // only draggable number
	{
		
		endEdit ();
		return kMouseEventHandled;			
	}
	else
	{
		return CSlider::onMouseUp(where,buttons);
	}
}

CMouseEventResult CSliderFlat::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if(!buttons.getButtonState()){
		if(size.pointInside(where) && !bMouseOn) // for mouseover
		{
			bMouseOn = true;
			setDirty(); // too many calls fixed with && !bMouseOn
		}
	}

	if(style & kNoFrame) // only draggable number
	{
		if (buttons & kLButton)
		{
			oldVal = (value - getMin ()) / (getMax () - getMin ());

			float normValue = getMin (); // starts from zero usually

			float speedV = 0.002f;	float speedH = 0.001f;				

			if (where.y <= startVPoint || where.y >= startVPoint) {
				normValue = curVal + speedV*(float)(where.y - startVPoint)- speedH*(float)(where.x - startHPoint);
			}

			normValue = 1.0f - normValue;

			if (buttons & kZoomModifier)
				normValue = oldVal + ((normValue - oldVal) / zoomFactor);

			setValueNormalized (normValue); // update value

			if (isDirty ())
			{
				valueChanged ();
				invalid ();
			}

			return kMouseEventHandled;
		} //if (buttons & kLButton)
		return kMouseEventNotHandled;

	}
	else
	{
		return CSlider::onMouseMoved(where,buttons);
	}
}
CMouseEventResult CSliderFlat::onMouseEntered (CPoint& where, const CButtonState& buttons)
{
	getFrame()->setCursor(kCursorSizeAll);	

	return kMouseEventHandled;
}

CMouseEventResult CSliderFlat::onMouseExited (CPoint& where, const CButtonState& buttons)
{
	getFrame()->setCursor(kCursorDefault);

	bMouseOn = false; // for mouseover
	setDirty();

	return kMouseEventHandled;
}

bool CSliderFlat::onWheel (const CPoint& where, const float &distance, const CButtonState &buttons)
{	
	return CSlider::onWheel(where, distance*-1, buttons);
}