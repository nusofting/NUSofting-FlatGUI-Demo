//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2015
//****************************************************************************************************//

#include "FlatGUI/CSliderDuos.h"
#include "FlatGUI/editor_colours.hpp"
#include "FlatGUI/FontSizeFactors.hpp"


CSliderDuos::CSliderDuos (const CRect &size, IControlListener* listener, long tag1, long tag2, long tagN,
						  ScaledFontFactory& fontFactory, const ConfigSlider& configSlider, const long style):
						  CControl(size, listener, tagN),
						  m_configSlider(configSlider),
						  m_scaledFontFactory(fontFactory),
						  tag1_(tag1), tag2_(tag2)
{

	initRects();

	//zoomFactor = 15.2f;

	setWantsFocus (false);

	curVal1 = oldVal1 = 1.0f/3.0f;
	curVal2 = oldVal2 = 2.0f/3.0f;
	curValM = 0.5f*(curVal2 - curVal1)+curVal1;
	delta1 = delta2 = delta3 = 0.0f;

	bMouseOverMiddle = false;	
	bSwapDisplayColours = false;
	bActive = true;

	memset(sideLeft,0,sizeof(sideLeft));
	memset(sideRight,0,sizeof(sideRight));

	glyphsVGap = 1.0;// good for Windows

	oldButton = 0;

	use1 = use2 = use3 = false;

	memset(display1,0,sizeof(char)*32);	
	memset(display2,0,sizeof(char)*32);	
}

void CSliderDuos::draw (CDrawContext *pContext)
{
	initRects();

	CDrawContext* drawContext = pContext;

	// (re)draw background
	CRect rect (0, 0, widthControl, heightControl);
	rect.offset (size.left+rDisplay.getWidth (), size.top);
	drawContext->setFrameColor(MakeCColor(200, 200, 200, 255));
	drawContext->setFillColor(MakeCColor(140, 12, 12, 255));
	drawContext->drawRect(rect,kDrawFilledAndStroked);

	// calc new coords of slider
	// two values
		drawContext->setFrameColor(MakeCColor(5, 5, 5, 255));
		drawContext->setLineWidth(4.0*0.01f*heightControl);

		rHandle1.offset(curVal1*rangeHandle, 0.0);
		rHandle2.offset(curVal2*rangeHandle, 0.0);

		rectValue = CRect(size);
		rectValue.left = rHandle1.right;
		rectValue.right = rHandle2.left;		
		drawContext->setFillColor(MakeCColor(12, 140, 12, 255));
		drawContext->drawRect(rectValue,kDrawFilled);

		const CCoord xc = rectValue.getCenter().x;
		const CCoord yc = rectValue.getCenter().y+0.5;
		const CCoord sc = heightControl*0.29;
		rMiddle.left = xc-sc; rMiddle.right = xc+sc;
		rMiddle.top = yc-sc; rMiddle.bottom = yc+sc;
		drawContext->setFillColor(MakeCColor(200, 200, 4, bMouseOverMiddle? 255 : 16));	
		drawContext->drawRect(rMiddle,kDrawFilledAndStroked);
		
		drawContext->setFillColor(MakeCColor(200, 200, 200, 255));		
		drawContext->drawRect(rHandle1,kDrawFilledAndStroked);
		drawContext->drawRect(rHandle2,kDrawFilledAndStroked);
		drawContext->drawRect(rDisplay,kDrawFilledAndStroked);	
		drawContext->setFontColor(MakeCColor(5, 5, 5, 255));
		drawContext->setFont(m_scaledFontFactory.getScaledSmallFont(),0,kBoldFace);
		//sprintf(sideLeft,"%1.4f",curVal1);
		drawContext->drawString(display1,rDisplay);		
		rDisplay.offset(widthControl,0.0);
		drawContext->drawRect(rDisplay,kDrawFilledAndStroked);
		//sprintf(sideRight,"%1.4f",curVal2);
		drawContext->drawString(display2,rDisplay);

	setDirty (false);
}

void CSliderDuos::setString32A(char* string)
{
	if(strlen(string) > 0)
		strncpy(display1,string,32);
	else
		strncpy(display1,"",1);

	setDirty();
}

void CSliderDuos::setString32B(char* string)
{
	if(strlen(string) > 0)
		strncpy(display2,string,32);
	else
		strncpy(display2,"",1);

	setDirty();
}

CMouseEventResult CSliderDuos::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	use1 = rHandle1.pointInside (where);
	use2 = rHandle2.pointInside (where);
	use3 = rectValue.pointInside (where);
	if (use3 || use1 && (buttons & kLButton))
	{
		CCoord result;
		// style always kHorizontal
		result = getViewSize ().left;

		CCoord actualPos1;

		actualPos1 = result + (int32_t)(getValue1() * rangeHandle);

		result += where.x - actualPos1;
		delta1 = float(result); 
	}
	if (use3 || use2 && (buttons & kLButton))
	{
		CCoord result;
		// style always kHorizontal
		result = getViewSize ().left;

		CCoord actualPos2;

		actualPos2 = result + (int32_t)(getValue2() * rangeHandle);

		result += where.x - actualPos2;
		delta2 = float(result);		
	}

	if ((use1 || use2 || use3) && (buttons & kLButton))
	{
		oldButton = buttons.getButtonState();

		if (checkDefaultValue (buttons, where))
		{
			return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}

		beginEdit ();
		if (buttons & kZoomModifier)
			return kMouseEventHandled;

		return onMouseMoved (where, buttons);
	}
	else
		return kMouseEventNotHandled;
}

CMouseEventResult CSliderDuos::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	use1 = use2 = use3 = false;
	oldButton = 0;
	endEdit ();
	setDirty();
	return kMouseEventHandled;
}

CMouseEventResult CSliderDuos::onMouseMoved (CPoint& where, const CButtonState& _buttons)
{
	//bMouseOverMiddle = rectValue.pointInside (where);
	

	if (CControl::isEditing ())
	{
		CButtonState buttons (_buttons);

		if (buttons & kLButton)
		{	
			if ((oldButton != buttons.getButtonState()) /*&& (buttons & kZoomModifier)*/)
			{
				oldButton = buttons.getButtonState();
			}

			if (use3 || use1)
			{
				oldVal1 = curVal1;

				float normValue1;
				// style always kHorizontal
				normValue1 = (float)(where.x - delta1) / (float)rangeHandle;

				if (buttons & kZoomModifier)
					normValue1 = oldVal1 + ((normValue1 - oldVal1) / 15.2f);

				bounceValue(normValue1);

				if(normValue1 < curVal2-0.14f)
					setValue1(normValue1); // move the handle
			}
			if (use3 || use2)
			{
				oldVal2 = curVal2;

				float normValue2;
				// style always kHorizontal
				normValue2 = (float)(where.x - delta2) / (float)rangeHandle;

				if (buttons & kZoomModifier)
					normValue2 = oldVal2 + ((normValue2 - oldVal2) / 15.2f);

				bounceValue(normValue2);

				if(normValue2 > curVal1+0.14f)
					setValue2(normValue2); // move the handle
			}

			setDirty ();
		}
		return kMouseEventHandled;
	}
	return kMouseEventNotHandled;
}
CMouseEventResult CSliderDuos::onMouseEntered (CPoint& where, const CButtonState& _buttons)
{
	bMouseOverMiddle = rMiddle.pointInside (where);
	invalidRect(rMiddle);

	return kMouseMoveEventHandledButDontNeedMoreEvents;
}
CMouseEventResult CSliderDuos::onMouseExited (CPoint& where, const CButtonState& _buttons)
{
	bMouseOverMiddle = false;
	setDirty();

	return kMouseMoveEventHandledButDontNeedMoreEvents;
}

void CSliderDuos::initRects()
{

		rHandle1 = CRect(0.0, 0.0, size.getHeight () , size.getHeight ());
		rHandle2 = CRect(rHandle1);
		rDisplay = CRect(0.0, 0.0,size.getWidth ()/10.0, size.getHeight ());

		widthControl  = size.getWidth () - rDisplay.getWidth (); 
		heightControl = size.getHeight ();
		rangeHandle =  widthControl - rHandle1.getWidth()*2.0; 
		
		rHandle1.offset(rDisplay.getWidth(), 0.0);
		rHandle2.offset(rHandle1.getWidth(), 0.0);
		widthOfSlider = rangeHandle;

	rHandle1.offset(size.left, size.top);
	rHandle2.offset(size.left, size.top);
	rDisplay.offset(size.left, size.top);
}


bool CSliderDuos::checkDefaultValue (CButtonState button, CPoint& where)
{
#if TARGET_OS_IPHONE
	if (button.isDoubleClick ())
#else
	if (button.isLeftButton () && (button.getModifierState () == kDefaultValueModifier || button.isDoubleClick()))
#endif
	{
		use1 = rHandle1.pointInside (where);
		use2 = rHandle2.pointInside (where);
		use3 = rectValue.pointInside (where);

		// begin of edit parameter
		beginEdit ();
		if (use1)
		{
			setValue1(0.0f);
		}
		else if(use2)
		{
			setValue2(1.0f);
		}
		else if (use3)
		{
			setValue1(0.0f);
			setValue2(1.0f);
		}
		// end of edit parameter
		endEdit ();

		use1 = use2 = use3 = false;

		setDirty ();

		return true;
	}
	return false;
}
