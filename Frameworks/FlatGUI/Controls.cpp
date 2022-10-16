//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2015
//****************************************************************************************************//

#pragma warning( disable : 4244)// 'initializing' : conversion from 'VSTGUI::CCoord' to 'const int', possible loss of data

#include "FlatGUI/Controls.h"
#include "FlatGUI/editor_colours.hpp"
#include "FlatGUI/FontSizeFactors.hpp"

#include <stdio.h>
#include <math.h>
#undef max
#include <algorithm>

#include "vstgui/lib/platform/iplatformfont.h"



//==============================================================================
//
// ScaledFontFactory implementation
//

ScaledFontFactory::ScaledFontFactory(UTF8StringPtr name, const int32_t style)
	: m_smallFont(name, kFontSizeSmall, style),
	m_mediumFont(name, kFontSizeMedium, style),
	m_bigFont(name, kFontSizeBig, style),
	m_scaleY(1.0)
{
}

ScaledFontFactory::~ScaledFontFactory ()
{
}

void ScaledFontFactory::setVerticalScaleFactor(float scaleY_)
{
	m_scaleY = scaleY_;
}

CFontDesc* ScaledFontFactory::getScaledSmallFont()
{
	m_smallFont.setSize(kFontSizeSmall*m_scaleY);
	return &m_smallFont;
}

CFontDesc* ScaledFontFactory::getScaledMediumFont()
{
	m_mediumFont.setSize(kFontSizeMedium*m_scaleY);
	return &m_mediumFont;
}

CFontDesc* ScaledFontFactory::getScaledBigFont()
{
	m_bigFont.setSize(kFontSizeBig*m_scaleY);
	return &m_bigFont;
}


//==============================================================================
//
// CStereoPeaksView implementation
//

CStereoPeaksView::CStereoPeaksView(const CRect& size, const ConfigGraphView& configGraphView, bool bMakeFrame, const CHoriTxtAlign hAlign)
	:CControl (size, 0, 0),
	m_configGraphView(configGraphView)
{
	hAlign_xx = hAlign;
	bDrawFrame = bMakeFrame;
	memset(display,0,sizeof(char)*128);

	setTag(-1);

	decreaseValue = 0.1f;
	value1 = value2 = 0.0f;
	oldValue1 = oldValue2 = 0.0f;
	iMaxValue1 = iMaxValue2 = size.bottom;

	bSingleChannel = false;

	resetPeak();

	myFont = 0;
}

CStereoPeaksView::~CStereoPeaksView()
{
	if(myFont)
		myFont->forget();
}

void CStereoPeaksView::setString128(const char* string)
{		
	strncpy(display,string,128);
}

void CStereoPeaksView::draw (CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;

	CColor alphaedBk = m_configGraphView.colBkground1;
	alphaedBk.alpha = 128;
	drawContext->setFrameColor(m_configGraphView.colFrame);
	drawContext->setFillColor(alphaedBk);
	drawContext->setLineWidth(1);
	drawContext->setFontColor(m_configGraphView.colFont);
	drawContext->setFont(myFont,kFontSizeScaleFactor*(size.getHeight()-2),kBoldFace);

	drawContext->setDrawMode(kNonIntegralMode);

	CRect rectVU1;
	CRect rectVU2; double stepX = size.getWidth()/11.0f;
	rectVU1.left  = size.left;
	rectVU1.right = size.left+stepX*5.0;
	rectVU2.left  = size.left+stepX*6.0;
	rectVU2.right = size.right;

	rectVU2.top  = 	rectVU1.top  = size.top;
	rectVU2.bottom = rectVU1.bottom = size.bottom;

	if(bDrawFrame){
		drawContext->drawRect(rectVU1, kDrawFilled);
		if(!bSingleChannel)drawContext->drawRect(rectVU2, kDrawFilled);
	}

	Clamp(value1); Clamp(value2);

	float newValue1 = oldValue1 - decreaseValue;
	if (newValue1 < value1)
		newValue1 = value1;

	oldValue1 = newValue1;

	float newValue2 = oldValue2 - decreaseValue;
	if (newValue2 < value2)
		newValue2 = value2;

	oldValue2 = newValue2;		

	CRect rectLevel1;
	CRect rectLevel2;

	rectLevel1.left =  rectVU1.left+1;
	rectLevel1.right = rectVU1.right-1;
	rectLevel1.top  = rectVU1.bottom-1 - rectVU1.getHeight()*newValue1;
	rectLevel1.bottom = rectVU1.bottom-1;

	rectLevel2.left =  rectVU2.left+1;
	rectLevel2.right = rectVU2.right-1;
	rectLevel2.top  = rectVU2.bottom-1 - rectVU2.getHeight()*newValue2;
	rectLevel2.bottom = rectVU2.bottom-1;

	drawContext->setFillColor(m_configGraphView.colSignal);

	drawContext->drawRect(rectLevel1, kDrawFilled);
	if(!bSingleChannel)drawContext->drawRect(rectLevel2, kDrawFilled);

	if(rectLevel1.top < size.top)
		rectLevel1.top = size.top+1;
	if(rectLevel2.top < size.top)
		rectLevel2.top = size.top+1;

	if(iMaxValue1 > rectLevel1.top)
		iMaxValue1 = rectLevel1.top+1; 

	if(iMaxValue2 > rectLevel2.top)
		iMaxValue2 = rectLevel2.top+1; 

	if(iMaxValue1 > size.top+10)// 10 is an arbitrary value not exactly 0 dB
		drawContext->setFrameColor(m_configGraphView.colPeak);
	else
		drawContext->setFrameColor(m_configGraphView.colClip);

	CRect Peak;
	Peak.left  = rectLevel1.left;
	Peak.right = rectLevel1.right+1;
	Peak.top  = iMaxValue1;
	Peak.bottom = iMaxValue1+3;

	drawContext->drawRect(Peak,kDrawStroked);		

	if(iMaxValue2 > size.top+10)
		drawContext->setFrameColor(m_configGraphView.colPeak);
	else
		drawContext->setFrameColor(m_configGraphView.colClip);

	Peak.left  = rectLevel2.left;
	Peak.right = rectLevel2.right;
	Peak.top  = iMaxValue2;
	Peak.bottom = iMaxValue2+3;
	if(!bSingleChannel)drawContext->drawRect(Peak,kDrawStroked);

	//if(bDrawFrame)
	//drawContext->drawStringUTF8(display,rectValue, hAlign_xx);

	setDirty(false);
}

void CStereoPeaksView::setValues(float& var1, float& var2)
{
	value1 = sqrtf(var1);
	if(!bSingleChannel)value2 = sqrtf(var2);
	setDirty();
}
CMouseEventResult CStereoPeaksView::onMouseDown (CPoint& where, const CButtonState& buttons)
{	
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	if(buttons.isLeftButton() && size.pointInside(where))
	{
		resetPeak();			  
		return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	else
		return kMouseEventNotHandled;
}

void CStereoPeaksView::resetPeak()
{
	iMaxValue1 = iMaxValue2 = size.bottom-2;
	setDirty();
}

void CStereoPeaksView::setDisplayFont(CFontRef asFont)
{
	setDirty ();
	if (myFont)
		myFont->forget ();

	if(asFont)
	{myFont = asFont; myFont->remember ();}
	else
	{
		myFont = new CFontDesc ("Arial", 0,  0); // needs delete
		myFont->remember ();
	}
}

void CStereoPeaksView::Clamp(float& value)
{
	if (value > 1.0f)
		value = 1.0f;
	else if (value < 0.0f)
		value = 0.0f;
}


//==============================================================================
//
// CKnobFlatSwitch implementation
//

CKnobFlatSwitch::CKnobFlatSwitch (const CRect &size, IControlListener* listener, long tag, const ConfigKnob& configKnob, int steps)
	: CControl (size, listener, tag),
	m_configKnob(configKnob)
{
	inset = 3;
	startAngle = 0.0f;
	halfAngle = 0.0f;
	aCoef = 0.0f;
	bCoef = 0.0f;

	radius = (float)(size.right - size.left) / 2.f;

	rangeAngle = 1.f;
	setStartAngle ((float)(5.f * kPI / 4.f));
	setRangeAngle ((float)(-3.f * kPI / 2.f));
	zoomFactor = 2.6f;

	setWantsFocus (true);

	memset(display,0,sizeof(display));	

	rectOld = size;

	this->steps = steps;

	if(steps == 2){ // to set the switch up visually
		setStartAngle (getStartAngle ()+getRangeAngle ()/float(steps+1));
		setRangeAngle (getRangeAngle ()/float(steps+1));

		halfCurveSwitch =  204/(steps+2);
	}
	else
		halfCurveSwitch =  104/(steps+2);

	myFont = 0;

	setDisplayFont();

	offset = CPoint(0,0);
	setConfig(configKnob.bDrawCirle2, configKnob.iCursOffsetRel);

	bMouseOn = false;

	bActive = true;
}
CKnobFlatSwitch::~CKnobFlatSwitch ()
{
	if(myFont)// && 0 == strcmp(myFont->getName(), "Arial")
		myFont->forget();
}

void CKnobFlatSwitch::setString32(char* string)
{
		if(strlen(string) > 0)
			strncpy(display,string,32);
		else
			strncpy(display,"",1);
}

void CKnobFlatSwitch::draw (CDrawContext *pContext)
{
	if(rectOld != size)
	{
		CRect newSize(size);
		radius = (float)(newSize.right - newSize.left) / 2.f;
		iCursOffset = newSize.getWidth()/m_configKnob.iCursOffsetRel;
	}

	rectOld = size;

	CRect n(size); n.inset(1,1);
	pContext->setLineWidth(2);
	pContext->setFrameColor(m_configKnob.colFrame);
	pContext->setFillColor(m_configKnob.colBkground1);
	pContext->setDrawMode(kAntiAliasing);
	/*	pContext->drawEllipse(n,kDrawFilledAndStroked);
	if(bDrawCirle2){
	pContext->setFillColor(m_configKnob.colBkground2);
	n.inset(iCursOffset,iCursOffset);
	pContext->drawEllipse(n,kDrawFilledAndStroked);
	}

	drawString (pContext);
	drawHandle (pContext);	*/
	setDirty (false);	
}

void CKnobFlatSwitch::drawHandle (CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;			

	CPoint origin (size.getWidth () / 2, size.getHeight () / 2);
	CPoint where;

	if(steps > 1){

		quantizeValue();

		if(bActive)
			drawContext->setFrameColor (m_configKnob.colShadowHandle);
		else
			drawContext->setFrameColor (m_configKnob.colInactive);

		for(int j=int(value*512.0f)-halfCurveSwitch; j< int(value*512.0f)+halfCurveSwitch; ++j) // draws the curve
		{
			drawEdgeLines(drawContext,where,origin,j/512.0f);
		}

		drawContext->setFrameColor (m_configKnob.colFrame);

		for(int j=0; j< steps; ++j) // draws the stops
		{
			drawEdgeLines(drawContext,where,origin,float(j)/float(steps-1));
		}
	}else{ // continuous mode

		if(bActive)
			drawContext->setFrameColor (m_configKnob.colShadowHandle);
		else
			drawContext->setFrameColor (m_configKnob.colInactive);

		for(int j=0; j< int(value*512.0f)-1; ++j) // draws the curve
		{
			drawEdgeLines(drawContext,where,origin,j/512.0f);
		}
	}

	if(bActive)
		drawContext->setFrameColor (m_configKnob.colHandle);
	else
		drawContext->setFrameColor (m_configKnob.colInactive);

	drawEdgeLines(drawContext,where,origin,value); // draws the handle
}
void CKnobFlatSwitch::valueToPointOffset (CPoint &point1, CPoint &point2, float value) const
{
	float alpha = (value - bCoef) / aCoef;
	float k1 = cosf (alpha);
	float k2 = sinf (alpha);
	point1.x = (CCoord)(radius + k1 * (radius - inset) + 0.5f);
	point1.y = (CCoord)(radius - k2 * (radius - inset) + 0.5f);
	point2.x = (CCoord)(radius + k1 * (radius - iCursOffset) + 0.5f);
	point2.y = (CCoord)(radius - k2 * (radius - iCursOffset) + 0.5f);
}

void CKnobFlatSwitch::drawString (CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;

	CRect rectValue(size);
	rectValue.top = size.top+size.getHeight()/3;		
	rectValue.offset(size.getWidth()/5,2);
	rectValue.setWidth(size.getWidth()-2*size.getWidth()/5);
	rectValue.setHeight(size.getHeight()/3);
	drawContext->setFontColor(m_configKnob.colFont);
	drawContext->setFont(myFont,kFontSizeScaleFactor*(rectValue.getHeight()/2-3),kBoldFace);
	//if(steps > 1){
	//	quantizeValue();
	//	sprintf(display,"step %d",  getValueQuantized()+1); 
	//}else{
	//	sprintf(display,"%1.2f",  value);
	//}


		if(sizeof(display) > 0)
			drawContext->drawString(display,rectValue,kCenterText);


	//drawContext->drawRect(rectValue);
}

void CKnobFlatSwitch::setActive(bool var)
{
	bActive = var;

	//setMouseEnabled(true);

	setDirty();
}

//void CKnobFlatSwitch::setValueQuantized(int var)
//{
//	if(steps > 1)
//		value = var/float(steps-1);
//	else
//		value = float(var);		
//}

//int CKnobFlatSwitch::getValueQuantized()
//{
//	if(steps > 1)
//		return int(value*(steps-1) + 0.5f);
//	else
//		return int(value + 0.5f);// as if steps = 2
//}	

CMouseEventResult CKnobFlatSwitch::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	bMouseOn = true;

	beginEdit ();

	if(steps > 1)
	{
		if (buttons & kLButton)
		{
			if(value < 1.0f)
				value += 1.0f/float((steps-1));
			else
				value = 0.0f;
		}
		if (buttons & kRButton)
		{
			if(value > 0.0f)
				value -= 1.0f/float((steps-1));
			else
				value = 1.0f;
		}

		getListener()->valueChanged(this);

		endEdit ();
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	else
	{
		if (checkDefaultValue (buttons))
		{
			endEdit ();
			return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}

		firstPoint = where;
		lastPoint (-1, -1);
		startValue = oldValue;

		modeLinear = false;
		fEntryState = value;
		range = 200.f;
		coef = (vmax - vmin) / range;
		oldButton = buttons;

		long mode    = kCircularMode;
		long newMode = getFrame ()->getKnobMode ();
		if (kLinearMode == newMode)
		{
			if (!(buttons & kAlt))
				mode = newMode;
		}
		else if (buttons & kAlt) 
			mode = kLinearMode;

		if (mode == kLinearMode && (buttons & kLButton))
		{
			if (buttons & kShift)
				range *= zoomFactor;
			lastPoint = where;
			modeLinear = true;
			coef = (vmax - vmin) / range;
		}
		else
		{
			CPoint where2 (where);
			where2.offset (-size.left, -size.top);
			startValue = valueFromPoint (where2);
		}

		return onMouseMoved (where, buttons);
	}
}//CMouseEventResult onMouseDown (CPoint& where, const long& buttons)

CMouseEventResult CKnobFlatSwitch::onMouseUp (CPoint& where, const CButtonState& buttons)
{
	bMouseOn = false;
	setDirty();

	endEdit ();
	return kMouseEventHandled;
}
CMouseEventResult CKnobFlatSwitch::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if(!buttons.getButtonState()){
		if(size.pointInside(where) && !bMouseOn) // for mouseover
		{
			bMouseOn = true;
			setDirty(); // too many calls fixed with && !bMouseOn
		}
	}

	if (buttons & kLButton)
	{
		float middle = (vmax - vmin) * 0.5f; // used only with Alt+Mouse

		if (where != lastPoint)
		{
			lastPoint = where;
			if (modeLinear)
			{
				CCoord diff = (firstPoint.y - where.y) + (where.x - firstPoint.x);
				if (buttons != oldButton)
				{
					range = 200.f;
					if (buttons & kShift)
						range *= zoomFactor;

					float coef2 = (vmax - vmin) / range;
					fEntryState += diff * (coef - coef2);
					coef = coef2;
					oldButton = buttons;
				}
				value = fEntryState + diff * coef;
				bounceValue ();
			}
			else
			{
				where.offset (-size.left, -size.top);
				value = valueFromPoint (where);
				if (startValue - value > middle)
					value = vmax;
				else if (value - startValue > middle)
					value = vmin;
				else
					startValue = value;
			}
			if (value != oldValue && listener)
				listener->valueChanged (this);
			//if (isDirty ()) // VSTGUI uses this
			//	invalid ();

			setDirty(); // but uses this now to try if it's better

			// Deprecated / does nothing in VSTGUI 4.3: doIdleStuff(); // needed to allow linked external dispaly to update
		}
	}
	return kMouseEventHandled;
}

CMouseEventResult CKnobFlatSwitch::onMouseExited (CPoint& where, const CButtonState& buttons)
{
	bMouseOn = false; // for mouseover
	setDirty();

	return kMouseEventHandled;
}

bool CKnobFlatSwitch::onWheel (const CPoint& where, const float &distance, const CButtonState &buttons)
{
	if (!getMouseEnabled ())
		return false;

	float v = getValueNormalized ();
	if (buttons & kZoomModifier)
		v += 0.1f * distance * wheelInc*-1;
	else
		v += distance * wheelInc*-1;
	setValueNormalized (v);

	if (isDirty ())
	{
		invalid ();

		// begin of edit parameter
		beginEdit ();

		valueChanged ();

		// end of edit parameter
		endEdit ();
	}
	return true;
}

//------------------------------------------------------------------------
void CKnobFlatSwitch::setStartAngle (float val)
{
	startAngle = val;
	compute ();
}

//------------------------------------------------------------------------
void CKnobFlatSwitch::setRangeAngle (float val)
{
	rangeAngle = val;
	compute ();
}

//------------------------------------------------------------------------
void CKnobFlatSwitch::compute ()
{
	aCoef = (vmax - vmin) / rangeAngle;
	bCoef = vmin - aCoef * startAngle;
	halfAngle = ((float)k2PI - fabsf (rangeAngle)) * 0.5f;
	setDirty ();
}

//------------------------------------------------------------------------
void CKnobFlatSwitch::valueToPoint (CPoint &point) const
{
	float alpha = (value - bCoef) / aCoef;
	point.x = (CCoord)(radius + cosf (alpha) * (radius - inset) + 0.5f);
	point.y = (CCoord)(radius - sinf (alpha) * (radius - inset) + 0.5f);
}
//------------------------------------------------------------------------
void CKnobFlatSwitch::zeroToPoint (CPoint &point) const
{
	float alpha = (0.0 - bCoef) / aCoef;
	point.x = (CCoord)(radius + cosf (alpha) * (radius - inset) + 0.5f);
	point.y = (CCoord)(radius - sinf (alpha) * (radius - inset) + 0.5f);
}

//------------------------------------------------------------------------
void CKnobFlatSwitch::pointToValue (CPoint &point, float value, CCoord radius) const
{
	float alpha = (value - bCoef) / aCoef;
	point.x = (CCoord)(radius + cosf (alpha) * (radius - inset) + 0.5f);
	point.y = (CCoord)(radius - sinf (alpha) * (radius - inset) + 0.5f);
}

//------------------------------------------------------------------------
float CKnobFlatSwitch::valueFromPoint (CPoint &point) const // used only with Alt+Mouse
{
	float v;
	float alpha = (float)atan2 (radius - point.y, point.x - radius);
	if (alpha < 0.f)
		alpha += (float)k2PI;

	float alpha2 = alpha - startAngle;
	if (rangeAngle < 0)
	{
		alpha2 -= rangeAngle;
		float alpha3 = alpha2;
		if (alpha3 < 0.f)
			alpha3 += (float)k2PI;
		else if (alpha3 > k2PI)
			alpha3 -= (float)k2PI;
		if (alpha3 > halfAngle - rangeAngle)
			v = vmax;
		else if (alpha3 > -rangeAngle)
			v = vmin;
		else
		{
			if (alpha2 > halfAngle - rangeAngle)
				alpha2 -= (float)k2PI;
			else if (alpha2 < -halfAngle)
				alpha2 += (float)k2PI;
			v = aCoef * alpha2 + vmax;
		}
	}
	else
	{
		float alpha3 = alpha2;
		if (alpha3 < 0.f)
			alpha3 += (float)k2PI;
		else if (alpha3 > k2PI)
			alpha3 -= (float)k2PI;
		if (alpha3 > rangeAngle + halfAngle)
			v = vmin;
		else if (alpha3 > rangeAngle)
			v = vmax;
		else
		{
			if (alpha2 > rangeAngle + halfAngle)
				alpha2 -= (float)k2PI;
			else if (alpha2 < -halfAngle)
				alpha2 += (float)k2PI;
			v = aCoef * alpha2 + vmin;
		}
	}

	return v;
}

void CKnobFlatSwitch::setConfig(bool drawCircle2, CCoord cursOffsetRel)
{
	this->bDrawCirle2 = drawCircle2;
	this->iCursOffsetRel = cursOffsetRel;
	iCursOffset = (float)size.getWidth()/(float)iCursOffsetRel;
	if(iCursOffset < 1) iCursOffset = 1;

	setDirty();
}

void CKnobFlatSwitch::setDisplayFont(CFontRef asFont)
{
	setDirty ();
	if (myFont)
		myFont->forget ();

	if(asFont)
	{myFont = asFont; myFont->remember ();}
	else
	{
		myFont = new CFontDesc ("Arial", 0,  0); // needs delete
		myFont->remember ();
	}
}

void CKnobFlatSwitch::quantizeValue()
{	
	value =	float(int((steps-1)*value + 0.5f)/float(steps-1));
}

void CKnobFlatSwitch::drawEdgeLines(CDrawContext* pContext,CPoint &point1, CPoint &point2, float value) const
{
	valueToPointOffset(point1,point2,value);

	point1.offset (size.left-1, size.top-1);
	point2.offset (size.left-1, size.top-1);

	pContext->drawLine (point1, point2);
}	


//==============================================================================
//
// CKnobFlatBranis implementation
//

CKnobFlatBranis::CKnobFlatBranis(const CRect &size, IControlListener* listener, long tag, const ConfigKnob& configKnob, int steps)
	: CKnobFlatSwitch (size, listener, tag, configKnob, steps)
{
	const CCoord inscale = 1.0/14.0;  // 1/14
	inset = size.getWidth()*inscale;

	iCursOffset = size.getWidth()/iCursOffsetRel;
	iCursLength = 0.8*size.getWidth()/4.0 + 0.5;
	radius = 0.5f*(size.right - size.left);


	if(sizeof(display) > 0)
		strncpy(display," ",1);

	bMakeCurve = true;
	bShowValue = true;	
	bReverse = false;

	value = 0.0f;
	CControl::setDirty (true);	
}

void CKnobFlatBranis::draw (CDrawContext *pContext)
{
    CDrawContext* drawContext = pContext;
    
	CKnobFlatSwitch::draw(drawContext);
    
    drawContext->setDrawMode(kAntiAliasing | kNonIntegralMode);

	const CCoord inscale = 1.0/14.0;  // 1/14

//    if(rectOld != CControl::size)
//    {
//        CRect newSize(CControl::size);
//        radius = 0.5f*(newSize.right - newSize.left);
//        iCursOffset = newSize.getWidth()/m_configKnob.iCursOffsetRel;
//        iCursLength = 0.8*newSize.getWidth()/4.0 + 0.5;
//        inset = (newSize.getWidth()*inscale);
//        drawBarsLines(pContext, false); // update
//    }
//
//    rectOld = CControl::size;

	CRect n(CControl::size); n.inset(CControl::size.getWidth()*0.0714f,CControl::size.getWidth()*0.0714f);
	drawContext->setLineWidth(CControl::size.getWidth()/10.0); //n.inset(1.0,1.0); //n.inset(inset+0.5,inset+0.5);
	drawContext->setFrameColor(m_configKnob.colFrame);
	drawContext->setFillColor(m_configKnob.colBkground1);
	drawContext->drawEllipse(n,kDrawFilledAndStroked);

	drawHandle (drawContext);
	if(bShowValue && (bMouseOn || steps > 1) && (steps > 1 || CControl::size.getWidth() >= 32.0))
		drawString (drawContext);

	CControl::setDirty (false);		
}

void CKnobFlatBranis::drawHandle (CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;

	CPoint origin (CControl::size.getWidth () / 2, CControl::size.getHeight () / 2);
	CPoint where;

	//const float wih = CControl::size.getWidth();
	const float rez = 512.0f;
	const CCoord handleWidth = CControl::getFrame()->getViewSize().getWidth()/233;

	if(steps > 1){ // switch mode

		quantizeValue();		

		drawContext->setFrameColor (m_configKnob.colFrame);
		pContext->setLineWidth(handleWidth);

		for(int j=0; j< steps; ++j) // draws the stops
		{
			drawEdgeLines(drawContext,where,origin,float(j)/float(steps-1), iCursLength+12);
		}

		drawBarsLines(drawContext, false); // draws the curve

	}else{ // continuous mode

		pContext->setLineWidth(handleWidth);
		if(bMakeCurve)
			drawBarsLines(drawContext, steps == -1); // - and + values when true	
	}

	pContext->setLineWidth(handleWidth);
	if(bActive)
		drawContext->setFrameColor (m_configKnob.colHandle);
	else
		drawContext->setFrameColor (m_configKnob.colInactive);

	drawEdgeLines(drawContext,where,origin,CControl::value, iCursLength+1); // draws the handle
}

void CKnobFlatBranis::valueToPointOffset (CPoint &point1, CPoint &point2, float value, CCoord iCursLength) const
{
	int handleOffset = 0;
	if(iCursLength) handleOffset = static_cast<int>(iCursLength/9.0 + 0.5); // style change
	float alpha = (value - bCoef) / aCoef; 
	float k1 = cosf (alpha);
	float k2 = sinf (alpha);
	point1.x = (CCoord)(radius + k1 * (radius - inset - handleOffset) + 0.5f);
	point1.y = (CCoord)(radius - k2 * (radius - inset - handleOffset) + 0.5f);
	point2.x = (CCoord)(radius + k1 * (radius - iCursOffset-iCursLength) + 0.5f);
	point2.y = (CCoord)(radius - k2 * (radius - iCursOffset-iCursLength) + 0.5f);
}

void CKnobFlatBranis::drawString (CDrawContext *pContext)
{
	if(0==strcmp(display, " ")) return;

	CDrawContext* drawContext = pContext;

	CRect rectValue(CControl::size);
	if(CControl::size.getHeight() < 52.0)
	{rectValue.inset(1.0,CControl::size.getHeight()*0.38);}
	else
	{rectValue.inset(2.0,CControl::size.getHeight()*0.41);}
	if(steps == 1 || steps == -1){
		drawContext->setFillColor(MakeCColor(0,0,0,111));
		drawContext->drawRect(rectValue,kDrawFilled);
	}
	drawContext->setFontColor(m_configKnob.colFont);
	drawContext->setFont(myFont,rectValue.getHeight(),kBoldFace);

	//if(steps > 1){
	//	quantizeValue();
	//	sprintf(display,"step %d",  getValueQuantized()+1); 
	//}else{
	//	sprintf(display,"%1.2f",  value);
	//}


	if(sizeof(display) > 0)
    {
	if(drawContext->getStringWidth(display) >= size.getWidth()-1.0)
	{drawContext->drawString(display,rectValue,kLeftText);}
	else
	{drawContext->drawString(display,rectValue,kCenterText);}
    }

	//drawContext->drawRect(rectValue);	
}

//void CKnobFlatBranis::setValueQuantized(int var)
//{
//	if(steps > 1)
//		value = var/float(steps-1);
//	else
//		value = float(var);		
//}

//int CKnobFlatBranis::getValueQuantized()
//{
//	if(steps > 1)
//		return int(value*(steps-1) + 0.5f);
//	else
//		return int(value + 0.5f);// as if steps = 2
//}	

void CKnobFlatBranis::DoCurve(bool var, bool rev)
{
	bMakeCurve = var;
	bReverse = rev;
}
void CKnobFlatBranis::SeeValue(bool var)
{
	bShowValue = var;
}

void CKnobFlatBranis::quantizeValue()
{	
	CControl::value = float(int((steps-1)*CControl::value + 0.5f)/float(steps-1));
}

void CKnobFlatBranis::drawEdgeLines(CDrawContext* pContext,CPoint &point1, CPoint &point2, float value, CCoord iCursLength) const
{
	valueToPointOffset(point1,point2,value,iCursLength);

	point1.offset (CControl::size.left-1, CControl::size.top-1);
	point2.offset (CControl::size.left-1, CControl::size.top-1);


	CLineStyle cStyleLine(CLineStyle::kLineCapRound, CLineStyle::kLineJoinBevel);
	pContext->setLineStyle(cStyleLine);

	pContext->drawLine (point1, point2);
}

void CKnobFlatBranis::drawBarsLines(CDrawContext* pContext, bool bLeftRight) const
{	
	if(bActive)
	{
		pContext->setFrameColor (m_configKnob.colShadowHandle);
	}
	else
	{
		pContext->setFrameColor (m_configKnob.colInactive);
	}


	pContext->setLineStyle(CLineStyle::kLineCapButt);
	
	const CCoord wihControl = getViewSize().getWidth();
	CCoord wihLine = 0.04*wihControl;
	pContext->setLineWidth(wihLine < 1.0 ? 1.0 : wihLine);

	const double rez = wihControl*2.5;
	
	
#if MAC
    const CCoord inCrv  = wihControl*0.446;
    const CCoord adjustHor = wihControl*0.041;
	const CCoord adjustVer = wihControl*0.057;
#else
    const CCoord inCrv  = wihControl*0.459;
    const CCoord adjustHor = wihControl*0.026;
    const CCoord adjustVer = wihControl*0.026;
#endif
    
	const int maxVal = int(rez+0.5);

	CPoint point1, point2;

	int start = !bLeftRight? 0 : int(CControl::value*rez); 
	int end   = !bLeftRight? int(CControl::value*rez) : int(0.5f*rez)-1;

	if(bLeftRight && CControl::value > 0.5f)
	{
		start =  int(0.5*rez); 
		end   =  int(CControl::value*rez)-1;
	}
	else if(steps > 1)
	{
		start =  int(CControl::value*rez)-halfCurveSwitch; 
		end   =  int(CControl::value*rez)+halfCurveSwitch; 
	}
	else if(bReverse)
	{
		start = int(CControl::value*maxVal+0.5f); 
		end   = int(1.0f*rez+0.5f);  
	}

	CGraphicsPath* path = pContext->createGraphicsPath ();
	if (path)
	{
        CRect newSize = getViewSize();
		pointToValue(point1, start/rez, inCrv);
		point1.offset (newSize.left+adjustHor, newSize.top+adjustVer);
		path->beginSubpath(point1);

		for(int j = start; j < end; j += 3) // draws the curve
		{			
			pointToValue(point2, (j+1.0)/rez, inCrv);
			point2.offset (newSize.left+adjustHor, newSize.top+adjustVer);

			path->addLine(point2);
		}

		pContext->drawGraphicsPath(path, CDrawContext::kPathStroked);

		path->forget();
	}	
}

//==============================================================================
//
// CSliderSpot implementation
//

CSliderSpot::CSliderSpot (const CRect &size, IControlListener* listener, long tag, long iMinPos, long iMaxPos, const long style)
	: CSlider (size, listener, tag,  iMinPos,  iMaxPos, NULL, NULL, CPoint (0, 0), style)
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

	setOffsetHandle (offsetHandle);

	zoomFactor = 10.2f;

	setWantsFocus (true);

	memset(display,0,sizeof(display));	

	scaleGUIx = 1.0f;

	this->iMaxPos = iMaxPos;
	this->iMinPos = iMinPos;

	rectOld = size;

	ConfigSlider configSlider;
	ColoursAndConfig(configSlider);

	myFont = 0;

	setDisplayFont();

	bMouseOn = false;
}

CSliderSpot::~CSliderSpot()
{
	if(myFont)// && 0 == strcmp(myFont->getName(), "Arial")
		myFont->forget();
}

void CSliderSpot::setString32(const char* string)
{
	strncpy(display,string,32);
}

void CSliderSpot::draw (CDrawContext *pContext)
{
	if(rectOld != size)
	{
		CRect newSize(size);

		iMaxPos = (newSize.right - newSize.left)-46*scaleGUIx; // 46 matches the number in bool myEditor::open(void* ptr), ugly so far

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
	}

	rectOld = size;

	CDrawContext* drawContext = pContext;

	float fValue;
	if (style & kLeft || style & kTop)
		fValue = value;
	else 
		fValue = 1.f - value;


	// (re)draw background
	CRect rect (0, 0, widthControl, heightControl/12);
	rect.offset (size.left, size.top+heightControl/2-heightControl/24);

	drawContext->setFrameColor(colorFrame);
	drawContext->setFillColor(colorBkground1);
	drawContext->drawRect(rect,kDrawFilled);

	// calc new coords of slider
	CRect  rectNew;
	if (style & kHorizontal)
	{	
		rectNew.top    = offsetHandle.y;
		rectNew.bottom = rectNew.top + heightOfSlider;	

		rectNew.left   = offsetHandle.x + (int)(fValue * rangeHandle);
		rectNew.left   = (rectNew.left < minTmp) ? minTmp : rectNew.left;

		rectNew.right  = rectNew.left + widthOfSlider;
	}
	else
	{
		rectNew.left   = offsetHandle.x-1;
		rectNew.right  = rectNew.left + widthOfSlider + 1;	

		rectNew.top    = offsetHandle.y + (int)(fValue * rangeHandle);

		rectNew.bottom = rectNew.top + heightOfSlider;
	}
	rectNew.offset (size.left, size.top);

	CRect rectBar (rect); rectBar.left -= 1; 

	if (style & kHorizontal)
	{			
		rectBar.right = rectNew.left;
	}
	else
	{
		rectBar.top = rectNew.bottom;
	}
	drawContext->setFillColor(colorFrame);
	drawContext->drawRect(rectBar,kDrawFilled);

	// draw slider handle at new position		

	if (style & kHorizontal)
	{
		drawContext->setLineWidth(1);
		drawContext->setFrameColor(colorFrameHandle);
		drawContext->setFillColor(colorHandle);	
		drawContext->drawRect(rectNew,kDrawFilledAndStroked);
		drawContext->setLineWidth(rectNew.getWidth()/7);
		CPoint startPoint(rectNew.left+rectNew.getWidth()/2,rectNew.top+rectNew.getHeight()/4);
		CPoint endPoint(rectNew.left+rectNew.getWidth()/2,rectNew.bottom-rectNew.getHeight()/4);
		drawContext->drawLine(startPoint, endPoint);
	}
	else
	{
		drawContext->setLineWidth(1);
		drawContext->setFrameColor(colorFrameHandle);
		drawContext->setFillColor(colorHandle);	
		drawContext->drawRect(rectNew,kDrawFilledAndStroked);
		CPoint startPoint(rectNew.left,rectNew.top);
		CPoint endPoint(rectNew.right,rectNew.top);
		drawContext->drawLine(startPoint, endPoint);
	}

	if(bMouseOn)
	{
		drawString (pContext);
	}

	setDirty (false);
}

void CSliderSpot::drawString (CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;

	if (style & kHorizontal)
	{
		const int iVlabel = int(size.getHeight()*0.45833333333333333333f); //(1/2-1/24)

		drawContext->setFillColor(colorBkground1);
		drawContext->setFontColor(colorFont);
		drawContext->setFont(myFont,kFontSizeScaleFactor*iVlabel,kBoldFace); // must be before etStringWidthUTF8(()
		CRect rectValue(size);
		rectValue.top = size.bottom-iVlabel; 
		rectValue.setHeight(iVlabel);
		rectValue.setWidth(drawContext->getStringWidth(display)+3);			

		const int iHspace = size.getWidth()/19;

		if(value > 0.5f){
			rectValue.offset(iHspace,1);
			drawContext->drawRect(rectValue,kDrawFilled);
			drawContext->drawString(display,rectValue,kLeftText);
		}else{
			rectValue.offset(size.right-(3*iHspace+rectValue.getWidth()),1);
			drawContext->drawRect(rectValue,kDrawFilled);
			drawContext->drawString(display,rectValue,kRightText);
		}	
	}
}

void CSliderSpot::ColoursAndConfig(struct ConfigSlider& configSlider )
{
	colorBkground1 = configSlider.colBkground1;
	colorFrame = configSlider.colFrame;
	colorHandle = configSlider.colHandle;
	colorFrameHandle = configSlider.colFrameHandle;
	colorFont = configSlider.colFont;
	setDirty();
}

void CSliderSpot::setScaleGUIx(float var)
{
	scaleGUIx = var;
}

void CSliderSpot::setDisplayFont(CFontRef asFont)
{
	setDirty ();
	if (myFont)
		myFont->forget ();

	if(asFont)
	{myFont = asFont; myFont->remember ();}
	else
	{
		myFont = new CFontDesc ("Arial", 0,  0); // needs delete
		myFont->remember ();
	}
}

CMouseEventResult CSliderSpot::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if(!buttons.getButtonState()){
		if(size.pointInside(where)) // for mouseover
		{
			bMouseOn = true;
			setDirty();
		}
	}
	else
		bMouseOn = true;

	return CSlider::onMouseMoved (where, buttons);
}	

CMouseEventResult CSliderSpot::onMouseUp(CPoint& where, const CButtonState& buttons)
{
	bMouseOn = false; // for mouseover
	setDirty();

	return CSlider::onMouseUp(where, buttons);
}

CMouseEventResult CSliderSpot::onMouseExited (CPoint& where,const CButtonState& buttons)
{
	bMouseOn = false; // for mouseover
	setDirty();

	return kMouseEventHandled;
}


//==============================================================================
//
// CSimpleOnOff implementation
//

CSimpleOnOff::CSimpleOnOff(CRect& size, IControlListener* pListener, long tag, const ConfigSwitch& configSwitch) :
	CControl(size, pListener, tag, 0),
	m_configSwitch(configSwitch)
{	
	step = 1.0f;
	CRect rectOld;
	memset(sString,0, sizeof(char)*32);
	bActive = true;
	bAddDisplay = false;
	bDoing3D = false;

}

void CSimpleOnOff::draw(CDrawContext *pContext)
{
    CCoord thickness = std::max(2.0, size.getWidth()*0.05);
	if(!bAddDisplay) size.setHeight(size.getWidth());
	CRect rSmall = CRect(size);

	if(bDoing3D)
	{	
		if( value < 0.5f) thickness = std::max(2.0, size.getWidth()*0.1);
		else thickness = std::max(2.0, size.getWidth()*0.05);

		rSmall.inset(size.getWidth()*0.05, size.getWidth()*0.05);
	}
	
	CRect rSquareView = rSmall;
	rSquareView.inset(thickness,thickness);

	CColor onCol = m_configSwitch.colBkground1;
	onCol.alpha = bActive? 255 : 111;

	const bool bOn = value >= 0.5f;

	pContext->setDrawMode(kAntiAliasing|kNonIntegralMode);
	pContext->setFillColor(m_configSwitch.colFrame);
	pContext->drawRect(rSmall,kDrawFilled);
	pContext->setFillColor((bOn)?  onCol : m_configSwitch.colBkground2);
	pContext->drawRect(rSquareView,kDrawFilled);

	pContext->setFrameColor(kGreyCColor);
	pContext->setFillColor(kGreyCColor);
	if(!bAddDisplay)
	{
		CRect dot = CRect(rSquareView); 
		CCoord inner = rSquareView.getWidth()*0.32;
		if(bOn) 
		{   pContext->setFillColor(kWhiteCColor);
			inner = rSquareView.getWidth()*0.23;
		}

		dot.inset(inner,inner);

		pContext->setLineWidth(0.5*thickness);
		pContext->drawRect(dot,kDrawFilledAndStroked);
	}
	else
	{
		CRect rLabel = CRect(rSmall);
		rLabel.offset(0.0,1.0); 
		rLabel.inset(2.2,2.5);
		pContext->setFont(kNormalFontSmaller, 0.8*rLabel.getHeight(),kBoldFace);
		pContext->setFontColor((bOn && bActive)? m_configSwitch.colFont1 : m_configSwitch.colFont2);
		pContext->drawString(sString,rLabel.offset(0.2*thickness, 0.5*thickness));
	}

	setDirty(false);
}

CMouseEventResult CSimpleOnOff::onMouseDown (CPoint& where, const CButtonState& buttons)
{	

	//MessageBoxA(0,"on prova switch","mouse",0);

	value = oldValue;

	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	beginEdit ();

	if(size.pointInside(where))
	{
		if(value)
			value = 0.0f;
		else
			value = 1.0f;

		if (value != oldValue && listener)
			listener->valueChanged (this);

		bounceValue ();
		endEdit ();
		invalidRect(size);
		setDirty();
		return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	endEdit ();
	return kMouseEventNotHandled;
}

void CSimpleOnOff::setActive(bool var)
{
	bActive = var;
	//if(bActive)
	//	setMouseEnabled(true);
	//else
	//	setMouseEnabled(false);

	setDirty();
}

void CSimpleOnOff::setString32(const char* inLabel)
{
	if(strlen(inLabel)> 0)
	{
		bAddDisplay = true;
		strcpy(sString,inLabel);
	}
	else
		bAddDisplay = false;


	setDirty();
}


//==============================================================================
//
// CMultiSwitch implementation
//

CMultiSwitch::CMultiSwitch(CRect& size, IControlListener* pListener, long tag, int pos, ScaledFontFactory& fontFactory, const ConfigSwitch& configSwitch) :
	CControl(size, pListener, tag, 0),
	m_scaledFontFactory(fontFactory),
	m_configSwitch(configSwitch)
{	
	this->pos = pos;
	if(pos > 12)
		this->pos = 12;
	
	if(pos < 1)
		this->pos = 1;

	for(int j = 0; j < 12; ++j)
	{
		labels[j] = new char[32];
	}	

	bActive = true;

	setLabelsGapV(0.0);

	bSwitchBkground = false;

	bHorizontal = true;

	bMomentarySwitch = false;

	bLinkDraw = false;

	iAlpha = 255;
}

CMultiSwitch::~CMultiSwitch()
{
	for(int j = 0; j < 12; ++j)
	{			
		delete labels[j];
	}	
}

void CMultiSwitch::setMomentary(const bool value)
{
	bMomentarySwitch = value;
}

void CMultiSwitch::setHorizontal(bool mode)
{
	bHorizontal = mode;
}

void CMultiSwitch::setLabels(const char** Labels)
{
	for(int j = 0; j < pos; ++j)
	{
		strncpy(labels[j],Labels[j],32);
	}				
}

void CMultiSwitch::draw(CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;

    CRect newSize = CRect(size);
    newSize.setHeight(size.getHeight()-1.0);
    //drawContext->setClipRect(newSize);
	drawContext->setFont(m_scaledFontFactory.getScaledSmallFont());
    
    drawContext->setDrawMode(kAntiAliasing);

	CColor cSwitchBkground;
	if(bSwitchBkground)
	{
		int r= m_configSwitch.colBkground2.red-16;
		int g = m_configSwitch.colBkground2.green-16;
		int b =  m_configSwitch.colBkground2.blue-16;
		cSwitchBkground.red = r < 3 ? 3 : r;
		cSwitchBkground.green  = g < 3 ? 3 : g;
		cSwitchBkground.blue = b < 3 ? 3 : b;
		cSwitchBkground.alpha = iAlpha;
	}

	drawContext->setDrawMode(kAliasing);
	drawContext->setLineWidth(1);
	drawContext->setFillColor(m_configSwitch.colBkground2);
	drawContext->setFrameColor(m_configSwitch.colFrame);

	if(bHorizontal)
	{
		CCoord celSize = size.getWidth()/pos;
		CRect rStep = CRect(0, 0, celSize, size.getHeight());
		rStep.offset(size.left+1, size.top);	
		for(int j = 0; j < pos; ++j)
		{
			if(bActive)
			{
				if(pos > 1)
				{
					if(j == int(value*(pos-1)+0.5f)) {
						drawContext->setFont(m_scaledFontFactory.getScaledMediumFont());
						drawContext->setFontColor(m_configSwitch.colFont2);
						if(bSwitchBkground) drawContext->setFillColor(m_configSwitch.colBkground2);
					}else{
						drawContext->setFont(m_scaledFontFactory.getScaledSmallFont());
						drawContext->setFontColor(m_configSwitch.colFont1);
						if(bSwitchBkground) drawContext->setFillColor(cSwitchBkground);
					}
				}
				else
				{
					if(value >= 0.5f) {
						drawContext->setFont(m_scaledFontFactory.getScaledMediumFont());
						drawContext->setFontColor(m_configSwitch.colFont2);
						if(bSwitchBkground) drawContext->setFillColor(m_configSwitch.colBkground2);
					}else{
							drawContext->setFont(m_scaledFontFactory.getScaledSmallFont());
						drawContext->setFontColor(m_configSwitch.colFont1);
						if(bSwitchBkground) drawContext->setFillColor(cSwitchBkground);
					}
				}
			}
			else
				drawContext->setFontColor(MakeCColor(128,128,128,255));

			if(bMomentarySwitch) 
			{
				if(bSwitchBkground) drawContext->setFillColor(cSwitchBkground);
				drawContext->setFontColor(m_configSwitch.colFont1);
			}

			drawContext->drawRect(rStep.inset(1,0),kDrawFilled); rStep.inset(-1,0); //restore width
			rStep.offset(0.0, labelsGapV);
			if(!bLinkDraw) 
			{
				drawContext->drawString(labels[j], rStep);
			}
			rStep.offset(0.0, -labelsGapV);

			drawContext->drawLine(rStep.getTopLeft(), rStep.getBottomLeft());

			rStep.offset(celSize+j/pos,0);
		}

		drawContext->drawLine(size.getTopRight().offset(-1,0), size.getBottomRight().offset(-1,0));


		if(bLinkDraw) 
		{
			
			drawContext->setDrawMode(kAntiAliasing);
			drawContext->setLineWidth(0.05*size.getWidth());
			drawContext->setFrameColor((value >= 0.5f)? m_configSwitch.colFont2 : m_configSwitch.colFont1);
			CRect square = size;
			square.setWidth(size.getHeight());
			const CCoord gap = 0.1*size.getWidth();
			drawContext->drawEllipse(square.inset(gap,gap).offset(gap*1.5,0));
			drawContext->drawEllipse(square.offset(gap*2.0,0));
		}
			
	}
	else // !bHorizontal , use only pos > 1 not for Peti SA
	{
		CCoord celSize = size.getHeight()/pos;
		CRect rStep = CRect(0, 0, size.getWidth(), celSize);
		rStep.offset(size.left+1, size.top);

		for(int j = 0; j < pos; ++j)
		{
			if(bActive)
			{
				if(j == int(value*(pos-1)+0.5f)) {
					drawContext->setFont(m_scaledFontFactory.getScaledMediumFont());
					drawContext->setFontColor(m_configSwitch.colFont2);
					if(bSwitchBkground) drawContext->setFillColor(m_configSwitch.colBkground2);
				}else{
					drawContext->setFont(m_scaledFontFactory.getScaledSmallFont());
					drawContext->setFontColor(m_configSwitch.colFont1);
					if(bSwitchBkground) drawContext->setFillColor(cSwitchBkground);
				}
			}
			else
				drawContext->setFontColor(MakeCColor(128,128,128,255));

			drawContext->drawRect(rStep.inset(0,0),kDrawFilled); 
			rStep.offset(0.0, labelsGapV);
			drawContext->drawString(labels[j], rStep);
			rStep.offset(0.0, -labelsGapV);

			drawContext->drawLine(rStep.getTopLeft(), rStep.getTopRight());

			rStep.offset(0,celSize+j/pos);
		}
	}

   // drawContext->resetClipRect();
	setDirty(false);
}

CMouseEventResult CMultiSwitch::onMouseDown (CPoint& where, const CButtonState& buttons)
{	
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	CCoord celSize = (bHorizontal)? size.getWidth()/pos : size.getHeight()/pos ;
	CRect rStep = (bHorizontal)? CRect(0, 0, celSize, size.getHeight()): CRect(0, 0, size.getWidth(), celSize);

	rStep.offset(size.left, size.top); 

	if(size.pointInside(where))
	{
		value = oldValue;

		beginEdit ();

		if(pos > 1)
		{
			if(bHorizontal)
			{
				const float step = float(size.getWidth())/float(pos);
				const float X = float(where.x-size.left-step*0.5f)/ float(size.getWidth());

				value = int(X*pos+0.5f)/float(pos-1);
			}
			else
			{
				const float step = float(size.getHeight())/float(pos);
				const float Y = float(where.y-size.top-step*0.5f)/ float(size.getHeight());

				value = int(Y*pos+0.5f)/float(pos-1);
			}
		}
		else
		{
			if(value)
				value = 0.0f;
			else
				value = 1.0f;
		}

		if (notSameFloat(value,oldValue) && listener)
			listener->valueChanged (this);

		// bounceValue ();
		endEdit ();
		if (isDirty ())
		{	
			invalid (); // does setDirty (false); invalidRect (size);
		}
		return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	return kMouseEventNotHandled;
}

void CMultiSwitch::setActive(bool var)
{
	bActive = var;
	//if(bActive)
	//	setMouseEnabled(true);
	//else
	//	setMouseEnabled(false);

	setDirty();
}

void CMultiSwitch::setLabelsGapV(CCoord var)
{
	labelsGapV = var;
}

void CMultiSwitch::setSwitchBkground(bool var)
{
	bSwitchBkground = var;
}

void CMultiSwitch::setBkgdAlpha(int var)
{
	iAlpha = var;
}


//==============================================================================
//
// CLFODisplay implementation
//

CLFODisplay::CLFODisplay(const CRect &size): CView(size)
{

	XS = size.getWidth(); 
	//lfo1->setSR(size.getWidth());
	//lfo1->setPureRate(2.0f);
	setMouseEnabled (false);
	bSmoothed = false;
	
}

void CLFODisplay::draw (CDrawContext *pContext)
{		
	CDrawContext* drawContext = pContext;
	drawContext->setFrameColor(MakeCColor(212,212,212,255));
	drawContext->setLineWidth(1);	
	drawContext->setDrawMode(kAntiAliasing);		

	const int w = int(size.getWidth());

	CPoint prevY1;
	for(int x = 0; x < w; ++x)
	{			
		float level = 0.0f;// get the LFO value 0.5f*lfo1.tick()+0.5;
		double h = size.getHeight()-2.0;
		CPoint Y1 = CPoint(x, h - 0.89f*level*h);
		Y1.offset(size.left, size.top);
		if(x > 0)
			drawContext->drawLine(prevY1, Y1);
		prevY1 = Y1;
	}

	if(bSmoothed){ // do we need such overloaded code here?!
		drawContext->setFillColor(MakeCColor(203,203,203,111));
		drawContext->drawRect(size,kDrawFilled);
		CColor oldColor = drawContext->getFontColor(); // get
		CFontRef tempFont = drawContext->getFont(); // get
		const double oldSize = tempFont->getSize(); // get
		tempFont->setSize(size.getHeight()-5.0); // set
		drawContext->setFontColor(kBlackCColor); // set
		drawContext->drawString("smoothed",size);
		drawContext->setFontColor(oldColor); // restore
		tempFont->setSize(oldSize); // restore
	}

	setDirty(false);
}

void CLFODisplay::setShape(int var)
{	
	bSmoothed = false;

	if(var == 7) {var = 5; bSmoothed = true;}
	else if(var == 8) {var = 6; bSmoothed = true;}

	//lfo1.setWaveform(var);
	//if(var == 9 || var == 6)
	//	lfo1.setPureRate(10.0f);
	//else
	//	lfo1.setPureRate(2.0f);	

	//lfo1.gainForDisplay();

	//lfo1.sync();

	setDirty();
}


//==============================================================================
//
// CSliderNano implementation
//

CSliderNano::CSliderNano (const CRect &size, IControlListener* listener, long tag,  const ConfigSlider& configSlider, const long style)
						  : CSlider (size, listener, tag,  0, (style & kHorizontal)? size.getWidth()-size.getHeight() : size.getHeight()-size.getWidth(), NULL, NULL, CPoint (0, 0), style),
						  m_configSlider(configSlider)
{
	
	setMode(kFreeClickMode);

	if (style & kHorizontal)
	{				
		widthOfSlider =	heightOfSlider = size.getHeight ();  // handle 
		rangeHandle = size.getWidth()-size.getHeight();
	}
	else
	{				
		widthOfSlider =	heightOfSlider = size.getWidth ();  // handle 
		rangeHandle = size.getHeight()-size.getWidth();
	}

	zoomFactor = 10.2f;

	rectOld = size;

	bSquareView = false;
	bActive = true;
	bDrawLittleShadows = true;
	bUpDownDir = false;
}

void CSliderNano::draw (CDrawContext *pContext)
{
	size.makeIntegral();

	if(rectOld != size)
	{
		CRect newSize(size);

		if (style & kHorizontal)
		{				
			widthOfSlider =	heightOfSlider = newSize.getHeight();
			CSlider::minPos = 0.0;
			rangeHandle = newSize.getWidth()-newSize.getHeight();
		}
		else
		{				
			widthOfSlider =	heightOfSlider = newSize.getWidth();
			CSlider::minPos = newSize.getHeight();
			rangeHandle = newSize.getHeight()-newSize.getWidth();	
		}	
	}

		rectOld = size;

	CDrawContext* drawContext = pContext;

	// (re)draw background

	CCoord mintrackHor = heightControl/8.0;
	CCoord mintrackVer = widthControl/8;
	if( mintrackHor < 2.0)  mintrackHor = 2.0;
	if( mintrackVer < 2.0)  mintrackVer = 2.0;

	CRect rect (0, 0, widthControl, mintrackHor);
	rect.offset (size.left, size.top+heightControl/2.0-heightControl/16.0);

	if (style & kVertical)
	{
		rect = CRect(0, 0, mintrackVer, heightControl);
		rect.offset (size.left+widthControl/2-widthControl/16, size.top);
	}

	const CCoord frcWidth = rect.getWidth();
	if (frcWidth < 2 ) rect.setWidth( 1 );	// fix of resize
	else if(frcWidth < 3 ) rect.setWidth( 2 ); 

	const CCoord frcHeight = rect.getHeight();
	if (frcHeight < 2 ) rect.setHeight( 1 ); // fix of resize
	else if(frcHeight < 3 ) rect.setHeight( 2 ); 

	drawContext->setDrawMode(kAntiAliasing|kNonIntegralMode);
	drawContext->setFrameColor(m_configSlider.colFrame);
	drawContext->setFillColor(bUpDownDir? m_configSlider.colTrackBack : m_configSlider.colTrackFront);
	drawContext->drawRect(rect, kDrawFilled);


	float normValue = getValueNormalized ();
	if (style & kRight || style & kBottom)
		normValue = 1.0f - normValue;

	// calc new coords of slider
	CRect rectNew;
	if (style & kHorizontal)
	{
		rectNew.top    = offsetHandle.y; // which is zero in CSliderNano
		rectNew.bottom = rectNew.top + heightOfSlider;	

		rectNew.left   = offsetHandle.x + floor (normValue * rangeHandle);
		rectNew.left   = (rectNew.left < minTmp) ? minTmp : rectNew.left;

		rectNew.right  = rectNew.left + widthOfSlider;
		//rectNew.right  = (rectNew.right > maxTmp) ? maxTmp : rectNew.right;
	}
	else
	{
		rectNew.left   = offsetHandle.x; // which is zero in CSliderNano
		rectNew.right  = rectNew.left + widthOfSlider;	

		rectNew.top    = offsetHandle.y + floor (normValue * rangeHandle);
		//rectNew.top    = (rectNew.top < minTmp) ? minTmp : rectNew.top;

		rectNew.bottom = rectNew.top + heightOfSlider;
		//rectNew.bottom = (rectNew.bottom > maxTmp) ? maxTmp : rectNew.bottom;
	}
	rectNew.offset (getViewSize ().left, getViewSize ().top);

	CRect rectBar (rect); 

	if (style & kHorizontal){		

		if(bUpDownDir) // actually is Left/Right since kHorizontal
		{
			rectBar.left = rectNew.left+0.5*size.getHeight();	
			rectBar.right = rect.getCenter().x; 
			drawContext->setFillColor(m_configSlider.colTrackFront);
		}
		else
		{
		rectBar.right += 1; 
		rectBar.left = rectNew.left+0.5*size.getHeight();
		drawContext->setFillColor(m_configSlider.colTrackBack);
		}
	}
	else
	{
		if(bUpDownDir)
		{
			rectBar.bottom = rectNew.bottom-0.5*size.getWidth();	
			rectBar.top = rect.getCenter().y; 
			drawContext->setFillColor(m_configSlider.colTrackFront);
		}
		else
		{
			CColor shaded = m_configSlider.colTrackBack;
			shaded.alpha = 128;
			rectBar.bottom = rectNew.bottom-0.5f*size.getWidth();
			drawContext->setFillColor(bActive? m_configSlider.colTrackBack : shaded);
		}
	}

	drawContext->drawRect(rectBar,kDrawFilled);		

	// draw slider handle at new position		

	CColor handleColOff = MakeCColor(128,128,128,255);

	if(bActive)
		drawContext->setFillColor(m_configSlider.colHandle);	
	else
		drawContext->setFillColor(handleColOff);

	if(bSquareView)
	{
		if((!bUpDownDir  && CSlider::getValue() < 0.01f) || (bUpDownDir && CSlider::getValue() == 0.5f))
			drawContext->setFillColor(handleColOff);
	}

    CCoord thickness = std::max(2.0, size.getWidth()*0.05);

	if (style & kHorizontal)
	{
        thickness = std::max(2.0, size.getHeight()*0.05);
	}

	drawContext->setFrameColor(m_configSlider.colFrameHandle);
	drawContext->setLineWidth(thickness);

	CRect rHandle = rectNew.inset(0.1f*widthOfSlider, 0.1f*widthOfSlider);
	CRect rShadow = rHandle;
	const CCoord radius = rShadow.getWidth()*0.5;
	rShadow.offset(0.1f*radius, 0.4f*radius);
	CGraphicsPath*  fb_cGraphicsPath = pContext->createGraphicsPath();
	if(bSquareView)
		fb_cGraphicsPath->addRect(rShadow);
	else
		fb_cGraphicsPath->addEllipse(rShadow);

	CColor cGrad2 = kBlackCColor;
	cGrad2.alpha = 0;	


	if(bSquareView)
	{
		if(bDrawLittleShadows && bActive)
		{
			CGradient* gradient = fb_cGraphicsPath->createGradient (0.8, 0.93, kBlackCColor , cGrad2);
			drawContext->fillLinearGradient(fb_cGraphicsPath, *gradient, rShadow.getTopLeft(), rShadow.getBottomLeft());
		}
		drawContext->drawRect(rHandle,kDrawFilledAndStroked);
	}
	else
	{
		if(bDrawLittleShadows && bActive)
		{
			CGradient* gradient = fb_cGraphicsPath->createGradient (0.6, 1.0, kBlackCColor , cGrad2);
			drawContext->fillRadialGradient(fb_cGraphicsPath, *gradient, rShadow.getCenter(), radius);
		}
		drawContext->drawEllipse(rHandle,kDrawFilledAndStroked);
	}

	setDirty (false);
}

void CSliderNano::setScaleGUIx(float var)
{
	//scaleGUIx = var;
}

void CSliderNano::setActive(bool var)
{
	bActive = var;
	//if(bActive)
	//	setMouseEnabled(true);
	//else
	//	setMouseEnabled(false);

	this->invalidRect(size);

	setDirty();
}


//==============================================================================
//
// CFilterPlot implementation
//

CFilterPlot::CFilterPlot(const CRect &size): CControl(size,0,-100000,0)
{
	setMouseEnabled (false);
	iType = 0;
}

void CFilterPlot::draw (CDrawContext *pContext)
{		
	CRect sime(size); 
	sime.inset(0.23*size.getWidth() ,0.23*size.getHeight());
	sime.offset(-0.23*size.getWidth(),0.0);


	CDrawContext* drawContext = pContext;
	drawContext->setFrameColor(MakeCColor(101,101,102,255));
	drawContext->setLineWidth(1);	
	drawContext->drawLine(sime.getTopLeft(), sime.getBottomLeft());
	drawContext->drawLine(sime.getBottomLeft(), sime.getBottomRight());
	drawContext->setDrawMode(kAntiAliasing);
	drawContext->setFrameColor(MakeCColor(197,197,197,255));
	drawContext->setLineWidth(2);

	int XS = int(sime.getWidth());

	CPoint prevY1;
	for(int x = 0; x < XS; x += 5)
	{	
		float level = 0.0f;
		double a = 1.0; 
		double s = 1.0;

		switch(iType)
		{
		case 0:{
			a = 5.3; // a: Order
			s=1.6*x/XS; // LP 
			   }break;
		case 1:
			if(x < XS/2.3){
				a = 3.0; // a: Order
				s= XS/(3.7*x); // HP
			}else{
				a = 5.3; // a: Order
				s=1.6*x/XS; // LP 
			}
			break;
		case 2:
			if(x < XS/2.3){
				a = 5.0; // a: Order
				s= XS/(3.3*x); // HP
			}else{
				a = 5.3; // a: Order
				s=1.6*x/XS; // LP 
			}
			break;
		case 3:{
			a = 11.0; // a: Order
			s=1.793*x/XS; // LP 
			   }break;
		}
		double H_sq=1.0/(1.0+pow(s,(a*2.0))); 
		level = sqrt(H_sq);

		double h = sime.getHeight()-2.0;
		CPoint Y1 = CPoint(x, h - 0.89f*level*h);
		Y1.offset(sime.left, sime.top);
		if(x > 0)
			drawContext->drawLine(prevY1, Y1);
		prevY1 = Y1;
	}

	setDirty(false);
}

void CFilterPlot::setType(int var)
{	
	iType = var;
	setDirty();
}


//==============================================================================
//
// CKickPreviousNext implementation
//

CKickPreviousNext::CKickPreviousNext(CRect& size, IControlListener* pListener, long tag) :
	CControl(size, pListener, tag, 0)
{	
	zone[0] = new CRect(size.left,size.top,size.right-size.getWidth()/2,size.bottom);
	zone[1] = new CRect(size.left+size.getWidth()/2,size.top,size.right,size.bottom);
	step = 1.0f;
	rectOld = size;

	ConfigKick configKick;
	ColoursAndConfig(configKick);

	bMouseOn1 = false;
	bMouseOn2 = false;
}

CKickPreviousNext::~CKickPreviousNext()
{
	if(zone[0]){
		delete zone[0]; 
		zone[0] = 0;
	}

	if(zone[1]){
		delete zone[1]; 
		zone[1] = 0;
	}
}

void CKickPreviousNext::draw(CDrawContext *pContext)
{
	if(rectOld != size)
	{
		*zone[0] = CRect(size.left,size.top,size.right-size.getWidth()/2,size.bottom);
		*zone[1] = CRect(size.left+size.getWidth()/2,size.top,size.right,size.bottom);			
	}

	rectOld = size;

	pContext->setDrawMode(kAntiAliasing);
	pContext->setFrameColor(colorFont);

	CColor modColBk2 = colorBkground2;
	modColBk2.red += 10;
	modColBk2.green += 10;
	modColBk2.blue += 10;

	pContext->setFillColor(bMouseOn1? colorBkground1 : modColBk2);
	pContext->drawRect(*zone[0],  kDrawFilled);

	pContext->setFillColor(bMouseOn2? colorBkground1 : modColBk2);
	pContext->drawRect(*zone[1], kDrawFilled);

	pContext->setLineWidth(std::max(1.0, 0.1*size.getHeight()));
	drawPoints(pContext, *zone[0], 1);
	drawPoints(pContext, *zone[1], 2);

	setDirty(false);
}

void CKickPreviousNext::drawPoints(CDrawContext *pContext, CRect rect, int which)
{	
	CCoord width = getFrame()->getWidth();
	CCoord height = rect.getHeight();
	if(which == 1)
	{	
		CPoint x1 = CPoint(rect.left+width*0.014, rect.top+height*0.2); 
		CPoint x2 = CPoint(rect.left+width*0.005, rect.top+height*0.5);	
		CPoint x3 = CPoint(rect.left+width*0.014, rect.top+height*0.8);

		for(int i = 0; i < 3; ++i)
		{
			pContext->drawLine(x1, x2);
			pContext->drawLine(x2, x3);
			x1.offset(width*0.014,0);
			x2.offset(width*0.014,0);
			x3.offset(width*0.014,0);
		}
	}
	if(which == 2)
	{
		CPoint x1 = CPoint(rect.right-width*0.014, rect.top+height*0.2); 
		CPoint x2 = CPoint(rect.right-width*0.005, rect.top+height*0.5);	
		CPoint x3 = CPoint(rect.right-width*0.014, rect.top+height*0.8);

		for(int i = 0; i < 3; ++i)
		{
			pContext->drawLine(x1, x2);
			pContext->drawLine(x2, x3);
			x1.offset(-width*0.014,0);
			x2.offset(-width*0.014,0);
			x3.offset(-width*0.014,0);
		}
	}
}

CMouseEventResult CKickPreviousNext::onMouseDown (CPoint& where, const CButtonState& buttons)
{	
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	beginEdit ();

	value = oldValue;

	if(zone[0]->pointInside(where))
	{
		value -= step;
		if(value < 0.0f) value = 0.0f;
	}
	else if(zone[1]->pointInside(where))
	{
		value += step;
		if(value > CControl::getMax()) value = CControl::getMax();
	}
	if(listener) 
		listener->valueChanged(this);
	setDirty();
	endEdit ();
	oldValue = value;
	return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

CMouseEventResult CKickPreviousNext::onMouseMoved (CPoint& where, const CButtonState& buttons) 
{
	if(zone[0]->pointInside(where))
	{
		bMouseOn1 = true;
		bMouseOn2 = false;
		setDirty();
		return  kMouseEventHandled;
	}
	else if(zone[1]->pointInside(where))
	{
		bMouseOn2 = true;
		bMouseOn1 = false;
		setDirty();
		return  kMouseEventHandled;
	}		
	return  kMouseEventNotHandled;
}

CMouseEventResult CKickPreviousNext::onMouseExited (CPoint& where, const CButtonState& buttons)
{
	bMouseOn1 = false;
	bMouseOn2 = false;

	setDirty();
	return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
}

void CKickPreviousNext::ColoursAndConfig(struct ConfigKick& configKick )
{
	colorBkground1 = configKick.colBkground1;
	colorBkground2 = configKick.colBkground2;
	colorFrame = configKick.colFrame;
	colorFont = configKick.colFont;
	setDirty();
}

//==============================================================================
//
// CPlotDisplay implementation
//

CPlotDisplay::CPlotDisplay(const CRect &size, const ConfigGraphView& configGraphView): CView(size), m_configGraphView(configGraphView)
{
	for (int i = 0; i < 512; ++i)
	{
		arrayPlotEQ[i] = float(i)/511.0f;
	}
}

void CPlotDisplay::draw (CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;
	CColor locCol = m_configGraphView.colFrame;
	locCol.alpha = 55;
	drawContext->setFrameColor(locCol);
	drawContext->setDrawMode(kAntiAliasing | kNonIntegralMode);
	drawContext->setLineWidth(1);
	drawContext->setFillColor(m_configGraphView.colBkground1);
	drawContext->drawRect(size,kDrawFilledAndStroked);
	//drawContext->drawLine(size.getTopLeft(), size.getBottomLeft().offset(0,-1));
	//drawContext->drawLine(size.getBottomLeft().offset(0,-1), size.getBottomRight().offset(0,-1));

	drawContext->setFrameColor(m_configGraphView.colSignal);

	const int w = size.getWidth();

	CPoint prevY2;
	for(int x = 1; x < w-1; ++x)
	{	
		float level = arrayPlotEQ[int(511.0f*float(x)/float(w) + 0.5f)];
		const int h = size.getHeight()-5;
		CPoint Y2 = CPoint(x, h-level*h + 1);
		Y2.offset(size.left, size.top);
		if(x > 1)
			drawContext->drawLine(prevY2, Y2);
		prevY2 = Y2;
	}

	setDirty(false);
}

void CPlotDisplay::setPlotEQ(const float* inArray)
{
	if(inArray)
	{
		memcpy(arrayPlotEQ, inArray, sizeof(float)*512);
		setDirty();
	}
}


//==============================================================================
//
// CGain2OnOff implementation
//

CGain2OnOff::CGain2OnOff(CRect& size, IControlListener* pListener, long tag, const ConfigSwitch& configSwitch) :
	CControl(size, pListener, tag, 0),
	m_configSwitch(configSwitch)
{	
	bActive = true;
}

void CGain2OnOff::draw(CDrawContext *pContext)
{		
	size.setHeight(size.getWidth());

	CColor onCol = m_configSwitch.colBkground1;
	onCol.alpha = bActive? 255 : 111;	

	CColor offCol = m_configSwitch.colBkground2;
	offCol = brightnessColour(offCol,  4.0f);

	pContext->setDrawMode(kAntiAliasing|kNonIntegralMode);
	pContext->setFillColor(m_configSwitch.colFrame);
	pContext->drawRect(size, kDrawFilled); // frame on GUI

	CCoord thickness = std::max(2.0, size.getWidth()*0.05);		
	CRect rSquareView = size;
	rSquareView.inset(thickness,thickness); // inner rect

	CRect rHalf = rSquareView;
	rHalf.setHeight(rSquareView.getHeight()*0.5);

	CRect bar2 = rHalf; 
	bar2.inset(rHalf.getWidth()*0.32,rHalf.getHeight()*0.23);
	CRect bar1 = bar2;
	bar1.offset(0.0, rHalf.getHeight()); // bottom
	pContext->setLineWidth(0.5*thickness);

	if(value >= 0.75f) // as 1.0f DSP value
	{	
		pContext->setFillColor(onCol);
		pContext->drawRect(rSquareView,kDrawFilled);		

		pContext->setFrameColor(kGreyCColor);
		pContext->setFillColor(kWhiteCColor);
		pContext->drawRect(bar1.inset(-1.0,-1.0),kDrawFilledAndStroked);
		pContext->drawRect(bar2.inset(-1.0,-1.0),kDrawFilledAndStroked);
	}
	if(value < 0.75f && value > 0.25f) // as 0.5 DSP value
	{	
		pContext->setFillColor(onCol);
		pContext->drawRect(rHalf.offset(0.0,rHalf.getHeight()) ,kDrawFilled);

		pContext->setFrameColor(kGreyCColor);
		pContext->setFillColor(kWhiteCColor); // on
		pContext->drawRect(bar1.inset(-1.0,-1.0),kDrawFilledAndStroked);
		pContext->setFillColor(offCol); // off
		pContext->drawRect(bar2,kDrawFilledAndStroked);
	}
	if(value <= 0.25f && value >= 0.0f) //as 0.0 DSP value
	{	
		pContext->setFillColor(m_configSwitch.colBkground2);
		pContext->drawRect(rSquareView,kDrawFilled);

		pContext->setFrameColor(offCol);
		pContext->setFillColor(offCol);
		pContext->drawRect(bar1,kDrawFilledAndStroked);
		pContext->drawRect(bar2,kDrawFilledAndStroked);
	}
	
	setDirty(false);
}

CMouseEventResult CGain2OnOff::onMouseDown (CPoint& where, const CButtonState& buttons)
{	
	if((buttons & kLButton) && size.pointInside(where))
	{
		beginEdit ();

		value = oldValue;

		value += 0.5f;
		if(value > 1.0f) value = 0.0f;	

		oldValue = oldValue;

		if (listener)
			listener->valueChanged (this);	

		endEdit ();
		invalidRect(getViewSize());
		//setDirty(); sets oldValue
		return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	else

	return kMouseEventNotHandled;
}

void CGain2OnOff::setActive(bool var)
{
	bActive = var;

	setDirty();
}
