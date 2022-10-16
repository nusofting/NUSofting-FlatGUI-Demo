//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2015
//****************************************************************************************************//

#pragma warning( disable : 4244)// 'initializing' : conversion from 'VSTGUI::CCoord' to 'const int', possible loss of data

#pragma once


#include "flat_objects_controls.h"
#include "editor_colours.hpp"
//#include "editor_helpers.inc"
#include <stdio.h>
#include <math.h>
#undef max
#include <algorithm>
#include "fastADSR.h"
#include "LFOxx.h"

#include "FontSizeFactors.hpp"

//-------- custom objects -------

class CSliderFlat : public CSlider
{
public:

	CSliderFlat (const CRect &size, CControlListener* listener, long tag, long iMinPos, long iMaxPos,
	             ScaledFontFactory& fontFactory, const ConfigSlider& configSlider, const long style = kLeft|kHorizontal)
		: CSlider (size, listener, tag,  iMinPos,  iMaxPos, NULL, NULL, CPoint (0, 0), style),
		  m_configSlider(configSlider),
		  m_scaledFontFactory(fontFactory)
	{
		bFreeClick = false;

		if (style & kHorizontal)
		{
			widthOfSlider  = size.width ()-iMaxPos; // handle 
			heightOfSlider = size.height ();
		}
		else
		{
			widthOfSlider  = size.width (); // handle 
			heightOfSlider = size.height ()-iMaxPos;
		}

		widthControl  = size.width (); // background
		heightControl = size.height ();

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
			rangeHandle = 12.0*size.height ();
			oldVal = getValueNormalized ();
			delta = getViewSize ().top + offsetHandle.v;
			heightControl = rangeHandle;
		}

		setOffsetHandle (offsetHandle);

		zoomFactor = 15.2f;

		setWantsFocus (true);

		curVal = getValueNormalized ();

		memset(display,0,sizeof(display));	

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
	}

	virtual	void setString32(char* string)
	{
		if(strlen(string) > 0)
			strncpy(display,string,32);
		else
			strncpy(display,"",1);
	}

	void draw (CDrawContext *pContext)
	{
		if(rectOld != size)
		{
			CRect newSize(size);

			iMaxPos = (newSize.right - newSize.left)-32*scaleGUIx;

			widthOfSlider  = newSize.width ()-iMaxPos; // handle 
			heightOfSlider = newSize.height ();

			widthControl  = newSize.width (); // background
			heightControl = newSize.height ();

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
				delta = getViewSize ().top + offsetHandle.v;
				heightControl =  rangeHandle;
			}
		}

		rectOld = size;

		CDrawContext* drawContext = pContext;

		float fValue;
		if (style & kLeft || style & kTop)
			fValue = value;
		else 
			fValue = 1.f - value;


		if(style & kNoFrame) // only draggable number
		{	
			drawContext->setLineWidth(1);
			drawContext->setDrawMode(kAliasing);	
			drawContext->setFrameColor(m_configSlider.colFrame);
			drawContext->setFillColor((bSwapDisplayColours)? m_configSlider.colFont : m_configSlider.colBkground1);
			drawContext->drawRect(size,kDrawFilledAndStroked);
			drawContext->setFontColor((bActive)? ((bSwapDisplayColours)? m_configSlider.colBkground1 : m_configSlider.colFont)  : m_configSlider.colFontInactive);
			drawContext->setFont(m_scaledFontFactory.getScaledSmallFont(),0,kBoldFace);

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
			drawContext->setFont(m_scaledFontFactory.getScaledSmallFont(),0,kBoldFace);
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

			rectNew.top    = offsetHandle.v;
			rectNew.bottom = rectNew.top + heightOfSlider;	

			rectNew.left   = offsetHandle.h + (int)(fValue * rangeHandle);
			rectNew.left   = (rectNew.left < minTmp) ? minTmp : rectNew.left;

			rectNew.right  = rectNew.left + widthOfSlider;
			//rectNew.right  = (rectNew.right > maxTmp) ? maxTmp : rectNew.right;

			//minTmp = offsetHandle.h + minPos;
			//maxTmp = minTmp + rangeHandle + widthOfSlider;
		}
		else
		{
			rectNew.left   = offsetHandle.h;
			rectNew.right  = rectNew.left + widthOfSlider;	

			rectNew.top    = offsetHandle.v + (int)(fValue * rangeHandle);
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
			drawContext->moveTo(CPoint(rectNew.left+rectNew.width()/2,rectNew.top));
			drawContext->lineTo(CPoint(rectNew.left+rectNew.width()/2,rectNew.bottom));
		}
		else
		{
			drawContext->setFrameColor(m_configSlider.colFrame);
			drawContext->setFillColor(m_configSlider.colHandle);
			drawContext->drawRect(rectNew,kDrawFilled);
			drawContext->moveTo(CPoint(rectNew.left,rectNew.top));
			drawContext->lineTo(CPoint(rectNew.right,rectNew.top));
		}


		setDirty (false);
	}

	void setScaleGUIx(float var)
	{
		scaleGUIx = var;
	}

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons)
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

			getFrame()->setCursor(kCursorSizeAll);			

			curVal = 1.f - getValueNormalized ();

			oldVal  = getMin ()-1;	

			startHPoint = where.h;
			startVPoint = where.v;

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

	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons)
	{
		if(style & kNoFrame) // only draggable number
		{
			getFrame()->setCursor(kCursorDefault);
			endEdit ();
			return kMouseEventHandled;			
		}
		else
		{
			return CSlider::onMouseUp(where,buttons);
		}
	}

	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons)
	{

		if(style & kNoFrame) // only draggable number
		{
			if (buttons & kLButton)
			{
				oldVal = (value - getMin ()) / (getMax () - getMin ());

				float normValue = getMin (); // starts from zero usually

				float speedV = 0.001f;	float speedH = 0.001f;				

				if (where.v <= startVPoint || where.v >= startVPoint) {
					normValue = curVal + speedV*(float)(where.v - startVPoint)- speedH*(float)(where.h - startHPoint);
				}

				normValue = 1.f - normValue;

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

	virtual void setActive(bool var)
	{
		bActive = var;
		if(bActive)
			setMouseEnabled(true);
		else
			setMouseEnabled(false);

		setDirty();
	}

	virtual void reverseColours(bool var)
	{
		bSwapDisplayColours = var;
	}

	virtual void addLayer(bool var)
	{
		bUseSliderLayerFlat = var;
	}

	virtual void addSideLabels(bool var, const char* lb1, const char* lb2)
	{
		if(strlen(lb1) > 1) strncpy(sideLeft,lb1,31);
		if(strlen(lb2) > 1) strncpy(sideRight,lb2,31);

		bUseSideLabels = var;
	}

	

	CLASS_METHODS(CSliderFlat, CSlider)

private:

	char display[32];

	float	scaleGUIx;

	CCoord iMaxPos, iMinPos;

	CRect rectOld;

	const ConfigSlider& m_configSlider;

	float oldVal;
	float curVal;
	float delta;

	bool bActive;
	CCoord startHPoint,startVPoint;
	ScaledFontFactory& m_scaledFontFactory;
	bool bSwapDisplayColours;
	bool bUseSliderLayerFlat;
	bool bUseSideLabels;
	char sideLeft[32];
	char sideRight[32];
	CCoord glyphsVGap;



};// end class CSliderFlat

//==============================================================================
//
// ScaledFontFactory implementation
//

#if !defined(USING_CPP_AS_INCLUDE)

ScaledFontFactory::ScaledFontFactory(UTF8StringPtr name, const int32_t style)
: m_smallFont(name, kFontSizeSmall, style),
  m_mediumFont(name, kFontSizeMedium, style),
  m_bigFont(name, kFontSizeBig, style),
  m_scaleY(1.0)
{
	 //setName (UTF8StringPtr newName);			///< set the name of the font
	 //setSize (CCoord newSize);				///< set the height of the font
	 //setStyle (int32_t newStyle);				///< set the style of the font @sa CTxtFace
	

	 //IPlatformFont* getPlatformFont ();
	 //IFontPainter* getFontPainter ();
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


#endif // defined(USING_CPP_AS_INCLUDE)

//==============================================================================
//
// CSimpleLabel implementation
//

#if !defined(USING_CPP_AS_INCLUDE)

CSimpleLabel::CSimpleLabel(const CRect& size, bool bMakeFrame, const CHoriTxtAlign hAlign)
:CParamDisplay (size)
{
	hAlign_xx = hAlign;
	bDrawFrame = bMakeFrame;
	memset(display,0,sizeof(display));

	setTag(-1);

	ConfigLabel configLabel;
	ColoursAndConfig(configLabel);

	myFont = 0;

	setDisplayFont();

	bflag = false;
	bMouseOn = false;

	pxs = 2;
	inCountChar = -1;
}

CSimpleLabel::~CSimpleLabel()
{
	if(myFont)
		myFont->forget();
}

void CSimpleLabel::setString128(const char* string)
{		
	strncpy(display,string,128);
	CParamDisplay::setDirty();
}

void CSimpleLabel::draw (CDrawContext *pContext)
{

	CDrawContext* drawContext = pContext;
	drawContext->setFont(myFont,kFontSizeScaleFactor*(size.height()),kBoldFace);

	float stringWidth = 0.0f; 

	if(display)
		stringWidth = drawContext->getStringWidth(display);

	//getStringPxs(drawContext, size, &myFont, display, stringWidth);

	if(inCountChar < int(strlen(display)-1) || inCountChar > int(strlen(display)+1))// should use monospaced fonts to avoid this
	{
		pxs = 2; // reset before rescaling
		inCountChar = strlen(display);
		while(stringWidth > (float)size.width()-2.0f) // good for Eugene Filter only?
		{		
			myFont->setSize(size.height()-(pxs++)); 
			IFontPainter* painter = myFont->getPlatformFont()->getPainter();
			if (painter)
			{
				stringWidth = painter->getStringWidth (drawContext, CString(display), true);
			}
		}
	}

	drawContext->setFontColor(colorFont);
	drawContext->setFont(myFont,kFontSizeScaleFactor*(size.height()-pxs),kBoldFace);

	CRect rectValue (size);
	if(bDrawFrame)
	{	
		if(bMouseOn)
			drawContext->setFrameColor(colorFont);
		else
			drawContext->setFrameColor(colorFrame);
		drawContext->setLineWidth(1);
		drawContext->drawRect(rectValue, kDrawStroked);
	}
	drawContext->drawString(display,rectValue, hAlign_xx);
	setDirty(false);
}
void CSimpleLabel::ColoursAndConfig(struct ConfigLabel& configLabel )
{
	colorBkground1 = configLabel.colBkground1;
	colorFrame = configLabel.colFrame;
	colorFont = configLabel.colFont;
	setDirty();
}

void CSimpleLabel::setFontColourOnly(const CColor& colFontsin)
{
	colorFont = colFontsin;
	setDirty();
}

void CSimpleLabel::setDisplayFont(CFontRef asFont, const char* fontName)
{
	setDirty ();
	if (myFont)
		myFont->forget ();

	if(asFont)
	{myFont = asFont; myFont->remember ();}
	else
	{
		myFont = new CFontDesc (fontName, 0,  0); // needs delete
		myFont->remember ();
	}
}
CMouseEventResult CSimpleLabel::onMouseDown (CPoint& where, const CButtonState& buttons)
{	
	if(size.pointInside(where) && ((buttons & kLButton) || (buttons & kRButton)))
	{ 
		CControlListener* listener = getListener();
		if(listener) listener->valueChanged(this);
		bflag = true;
		return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}

	return kMouseEventNotHandled;
}

CMouseEventResult CSimpleLabel::onMouseEntered (CPoint& where, const CButtonState& buttons) 
{
	bMouseOn = true;
	setDirty();
	return  kMouseEventHandled;
}

CMouseEventResult CSimpleLabel::onMouseExited (CPoint& where, const CButtonState& buttons)
{
	bMouseOn = false;
	setDirty();
	return  kMouseEventHandled;
}

void CSimpleLabel::setFontSize()
{
	pxs = 2;
	inCountChar = -1;
	setDirty();
}

#endif // defined(USING_CPP_AS_INCLUDE)

class CKickTwiceZoom : public CControl
{
public:
	CKickTwiceZoom(CRect& size, CControlListener* pListener, long tag) :
	  CControl(size, pListener, tag, 0)
	  {	
		  zone[0] = new CRect(size.left,size.top,size.right-size.getWidth()/2,size.bottom);
		  zone[1] = new CRect(size.left+size.getWidth()/2,size.top,size.right,size.bottom);
		  fEntryState1 = false;
		  fEntryState2 = false;
		  step = 1.0f;
		  rectOld = size;

		  ConfigKick configKick;
		  ColoursAndConfig(configKick);

		  myFont = new CFontDesc ("Arial", 0,  0); // needs delete

		  setDisplayFont();
	  }

	  ~CKickTwiceZoom()
	  {
		  if(zone[0]){
			  delete zone[0]; 
			  zone[0] = 0;
		  }

		  if(zone[1]){
			  delete zone[1]; 
			  zone[1] = 0;
		  }

		  if(myFont)
			  myFont->forget();
	  }

	  long getTag(){ return CControl::getTag(); }

	  void draw(CDrawContext *pContext)
	  {
		  if(rectOld != size)
		  {
			  *zone[0] = CRect(size.left,size.top,size.right-size.getWidth()/2,size.bottom);
			  *zone[1] = CRect(size.left+size.getWidth()/2,size.top,size.right,size.bottom);			
		  }

		  rectOld = size;

		  pContext->setDrawMode(kAntiAliasing);
		  pContext->setLineWidth(1);
		  pContext->setFrameColor(colorFrame);

		  pContext->setFillColor((fEntryState1)?  colorBkground1 : colorBkground2);
		  pContext->drawRect(*zone[0],kDrawFilledAndStroked);

		  pContext->setFillColor((fEntryState2)?  colorBkground1 : colorBkground2);
		  pContext->drawRect(*zone[1],kDrawFilledAndStroked);

		  //pContext->setFontColor(colorFont);
		  //pContext->setFont(myFont,kFontSizeScaleFactor*(size.height()-3),kBoldFace);
		  //pContext->drawString("-1",*zone[0]);
		  //pContext->drawString("+1",*zone[1]);

		  pContext->setLineWidth(std::max(1.0,0.1*size.height()));
		  pContext->setFrameColor(colorFont);
		  drawMinusAndPlus(pContext, *zone[0], 1);
		  drawMinusAndPlus(pContext, *zone[1], 2);

		  setDirty(false);
	  }

	  void drawMinusAndPlus(CDrawContext *pContext, CRect rect, int which)
	  {		
		  CCoord width = rect.width();
		  CCoord height = rect.height();
		  CPoint x1 = CPoint(rect.left+width*0.1, rect.top+height*0.5); 
		  CPoint x2 = CPoint(rect.left+width*0.5, rect.top+height*0.5);	
		  if(which == 1 || which == 2)
		  {		 
			  pContext->moveTo(x1);
			  pContext->lineTo(x2);
			  CPoint x3 = CPoint(rect.left+width*0.665, rect.top+height*0.3); 
			  CPoint x4 = CPoint(rect.left+width*0.753, rect.top+height*0.2); 
			  CPoint x5 = CPoint(rect.left+width*0.753, rect.top+height*0.8);
			  colorFont.alpha = 53;
			  pContext->setFrameColor(colorFont);
			  pContext->moveTo(x3);
			  pContext->lineTo(x4);
			  pContext->lineTo(x5);
			  colorFont.alpha = 255;
			  pContext->setFrameColor(colorFont);
		  }
		  if(which == 2)
		  {
			  CPoint y1 = CPoint(x1.h+width*0.21, rect.top+height*0.2);
			  CPoint y2 = CPoint(x1.h+width*0.21, rect.top+height*0.8);
			  pContext->moveTo(y1);
			  pContext->lineTo(y2);
		  }
	  }

	  virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons)
	  {	
		  value = oldValue;

		  if (!(buttons & kLButton))
			  return kMouseEventNotHandled;
		  //fEntryState = value;
		  beginEdit ();

		  if(zone[0]->pointInside(where))
		  {
			  fEntryState1 = true;
			  setDirty();  
			  return  kMouseEventHandled;
		  }
		  else if(zone[1]->pointInside(where))
		  {
			  fEntryState2 = true;
			  setDirty(); 
			  return  kMouseEventHandled;
		  }
		  return kMouseEventNotHandled;
	  }
	  virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons)
	  { 
		  if(fEntryState1)
		  {
			  value -= step;
		  }
		  else if(fEntryState2)
		  {
			  value += step;
		  }
		  bounceValue ();
		  if (listener)
			  listener->valueChanged (this);

		  oldValue = value;		

		  fEntryState1 = false;
		  fEntryState2 = false;
		  endEdit ();
		  setDirty();
		  return  kMouseEventHandled;
	  }
	  virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons)
	  {	
		  if(fEntryState1 == true && !zone[0]->pointInside(where))
		  {
			  fEntryState1 = false;
			  endEdit ();
			  setDirty();
		  }
		  if(fEntryState2 == true && !zone[1]->pointInside(where))
		  {
			  fEntryState2 = false;
			  endEdit ();
			  setDirty();
		  }
		  return  kMouseEventHandled;
	  } 

	  void setStepFix(float value)
	  {
		  step = value;
	  }

	  void ColoursAndConfig(struct ConfigKick& configKick )
	  {
		  colorBkground1 = configKick.colBkground1;
		  colorBkground2 = configKick.colBkground2;
		  colorFrame = configKick.colFrame;
		  colorFont = configKick.colFont;
		  setDirty();
	  }

	  void setDisplayFont(CFontRef asFont = 0)
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

	  CLASS_METHODS(CKickTwiceZoom, CControl)

private:

	CRect* zone[2];
	bool fEntryState1,fEntryState2;	
	float step;
	CRect rectOld;
	CColor colorBkground1;
	CColor colorBkground2;
	CColor colorFrame; 
	CColor colorFont;

	CFontRef myFont;

}; // end class CKickTwiceZoom

class CStereoPeaksView : public CControl
{
public:

	CStereoPeaksView(const CRect& size, const ConfigGraphView& configGraphView, bool bMakeFrame = true, const CHoriTxtAlign hAlign = kCenterText)
		:CControl (size, 0, 0),
		m_configGraphView(configGraphView)
	{
		hAlign_xx = hAlign;
		bDrawFrame = bMakeFrame;
		memset(display,0,sizeof(display));

		setTag(-1);

		decreaseValue = 0.1f;
		value1 = value2 = 0.0f;
		oldValue1 = oldValue2 = 0.0f;
		iMaxValue1 = iMaxValue2 = size.bottom;

		resetPeak();

		myFont = 0;
	}

	~CStereoPeaksView()
	{
		if(myFont)
			myFont->forget();
	}

	void setString128(const char* string)
	{		
		strncpy(display,string,128);
	}

	void draw (CDrawContext *pContext)
	{
		CDrawContext* drawContext = pContext;

		drawContext->setFrameColor(m_configGraphView.colFrame);
		drawContext->setFillColor(m_configGraphView.colBkground1);
		drawContext->setLineWidth(1);
		drawContext->setFontColor(m_configGraphView.colFont);
		drawContext->setFont(myFont,kFontSizeScaleFactor*(size.height()-2),kBoldFace);

		CRect rectVU1;
		CRect rectVU2;
		rectVU1.left  = size.left;
		rectVU1.right = size.right-size.width()/2 -3;
		rectVU1.top  = size.top;
		rectVU1.bottom = size.bottom;

		rectVU2.left  = size.right-rectVU1.width();
		rectVU2.right = size.right;
		rectVU2.top  = size.top;
		rectVU2.bottom = size.bottom;

		if(bDrawFrame){
			drawContext->drawRect(rectVU1, kDrawFilledAndStroked);
			drawContext->drawRect(rectVU2, kDrawFilledAndStroked);
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
		rectLevel1.top  = rectVU1.bottom-1 - rectVU1.height()*newValue1;
		rectLevel1.bottom = rectVU1.bottom-1;

		rectLevel2.left =  rectVU2.left+1;
		rectLevel2.right = rectVU2.right-1;
		rectLevel2.top  = rectVU2.bottom-1 - rectVU2.height()*newValue2;
		rectLevel2.bottom = rectVU2.bottom-1;

		drawContext->setFillColor(m_configGraphView.colSignal);

		drawContext->drawRect(rectLevel1, kDrawFilled);
		drawContext->drawRect(rectLevel2, kDrawFilled);

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
		Peak.right = rectLevel1.right;
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
		drawContext->drawRect(Peak,kDrawStroked);

		//if(bDrawFrame)
		//drawContext->drawStringUTF8(display,rectValue, hAlign_xx);

		setDirty(false);
	}

	void setValues(float& var1, float& var2)
	{
		value1 = var1;
		value2 = var2;
		setDirty();
	}
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons)
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

	void resetPeak()
	{
		iMaxValue1 = iMaxValue2 = size.bottom-2;
		setDirty();
	}

	void setDisplayFont(CFontRef asFont = 0)
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

	CLASS_METHODS(CStereoPeaksView, CControl)

private:

	char display[128];
	bool bDrawFrame;
	CHoriTxtAlign hAlign_xx;
	float decreaseValue;
	float value1,value2;
	float oldValue1,oldValue2;
	CCoord iMaxValue1,iMaxValue2;

	void Clamp(float& value)
	{
		if (value > 1.0f)
			value = 1.0f;
		else if (value < 0.0f)
			value = 0.0f;
	}

	const ConfigGraphView& m_configGraphView;

	CFontRef myFont;

}; // end class CStereoPeaksView 

class CKnobFlatSwitch : public CControl
{
public:

	CKnobFlatSwitch (const CRect &size, CControlListener* listener, long tag, const ConfigKnob& configKnob, int steps = 0)
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

		this->iMaxPos = iMaxPos;
		this->iMinPos = iMinPos;

		rectOld = size;

		this->steps = steps;

		if(steps == 2){
			setStartAngle (getStartAngle ()+getRangeAngle ()/float(steps+1));
			setRangeAngle (getRangeAngle ()/float(steps+1));
		}

		halfCurveSwitch =(steps == 3)? 140/steps : (steps == 4)? 188/steps : 255/steps;

		myFont = 0;

		setDisplayFont();

		offset = CPoint(0,0);
		setConfig(configKnob.bDrawCirle2, configKnob.iCursOffsetRel);

		bMouseOn = false;

		bActive = true;
	}
	virtual	~CKnobFlatSwitch ()
	{
		if(myFont)// && 0 == strcmp(myFont->getName(), "Arial")
			myFont->forget();
	}

	virtual	void setString32(char* string)
	{
		if(strlen(string) > 0)
			strncpy(display,string,32);
		else
			strncpy(display,"",1);
	}

	virtual	void draw (CDrawContext *pContext)
	{
		if(rectOld != size)
		{
			CRect newSize(size);
			radius = (float)(newSize.right - newSize.left) / 2.f;
			iCursOffset = newSize.width()/iCursOffsetRel;
		}

		rectOld = size;

		CRect n(size); n.inset(1,1);
		pContext->setLineWidth(2);
		pContext->setFrameColor(m_configKnob.colFrame);
		pContext->setFillColor(m_configKnob.colBkground1);
		pContext->setDrawMode(kAntialias);
		//pContext->drawEllipse(n,kDrawFilledAndStroked);
		//if(bDrawCirle2){
		//	pContext->setFillColor(m_configKnob.colBkground2);
		//	n.inset(iCursOffset,iCursOffset);
		//	pContext->drawEllipse(n,kDrawFilledAndStroked);
		//}

		//drawString (pContext);
		//drawHandle (pContext);
		setDirty (false);		
	}

	virtual	void drawHandle (CDrawContext *pContext)
	{
		CDrawContext* drawContext = pContext;			

		CPoint origin (size.width () / 2, size.height () / 2);
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
	virtual	void valueToPointOffset (CPoint &point1, CPoint &point2, float value) const
	{
		float alpha = (value - bCoef) / aCoef;
		float k1 = cosf (alpha);
		float k2 = sinf (alpha);
		point1.h = (CCoord)(radius + k1 * (radius - inset) + 0.5f);
		point1.v = (CCoord)(radius - k2 * (radius - inset) + 0.5f);
		point2.h = (CCoord)(radius + k1 * (radius - iCursOffset) + 0.5f);
		point2.v = (CCoord)(radius - k2 * (radius - iCursOffset) + 0.5f);
	}

	virtual	void drawString (CDrawContext *pContext)
	{
		CDrawContext* drawContext = pContext;

		CRect rectValue(size);
		rectValue.top = size.top+size.height()/3;		
		rectValue.offset(size.width()/5,2);
		rectValue.setWidth(size.width()-2*size.width()/5);
		rectValue.setHeight(size.height()/3);
		drawContext->setFontColor(m_configKnob.colFont);
		drawContext->setFont(myFont,kFontSizeScaleFactor*(rectValue.height()/2-3),kBoldFace);
		//if(steps > 1){
		//	quantizeValue();
		//	sprintf(display,"step %d",  getValueQuantized()+1); 
		//}else{
		//	sprintf(display,"%1.2f",  value);
		//}
		drawContext->drawString(display,rectValue,kCenterText);

		//drawContext->drawRect(rectValue);
	}

	virtual void setActive(bool var)
	{
		bActive = var;
		if(bActive)
			setMouseEnabled(true);
		else
			setMouseEnabled(false);

		setDirty();
	}

	//void setValueQuantized(int var)
	//{
	//	if(steps > 1)
	//		value = var/float(steps-1);
	//	else
	//		value = float(var);		
	//}

	//int getValueQuantized()
	//{
	//	if(steps > 1)
	//		return int(value*(steps-1) + 0.5f);
	//	else
	//		return int(value + 0.5f);// as if steps = 2
	//}	

	virtual	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons)
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

	virtual	CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons)
	{
		bMouseOn = false;
		setDirty();

		endEdit ();
		return kMouseEventHandled;
	}
	virtual	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons)
	{
		if(!buttons.getButtonState()){
			if(where.isInside(size) && !bMouseOn) // for mouseover
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
					CCoord diff = (firstPoint.v - where.v) + (where.h - firstPoint.h);
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

				doIdleStuff(); // needed to allow linked external dispaly to update
			}
		}
		return kMouseEventHandled;
	}

	virtual	CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons)
	{
		bMouseOn = false; // for mouseover
		setDirty();

		return kMouseEventHandled;
	}

	//------------------------------------------------------------------------
	virtual	void setStartAngle (float val)
	{
		startAngle = val;
		compute ();
	}

	//------------------------------------------------------------------------
	virtual	void setRangeAngle (float val)
	{
		rangeAngle = val;
		compute ();
	}

	virtual	float getStartAngle () const { return startAngle; }


	virtual	float getRangeAngle () const { return rangeAngle; }

	//------------------------------------------------------------------------
	virtual	void compute ()
	{
		aCoef = (vmax - vmin) / rangeAngle;
		bCoef = vmin - aCoef * startAngle;
		halfAngle = ((float)k2PI - fabsf (rangeAngle)) * 0.5f;
		setDirty ();
	}

	//------------------------------------------------------------------------
	virtual	void valueToPoint (CPoint &point) const
	{
		float alpha = (value - bCoef) / aCoef;
		point.h = (CCoord)(radius + cosf (alpha) * (radius - inset) + 0.5f);
		point.v = (CCoord)(radius - sinf (alpha) * (radius - inset) + 0.5f);
	}

	//------------------------------------------------------------------------
	virtual	float valueFromPoint (CPoint &point) const // used only with Alt+Mouse
	{
		float v;
		float alpha = (float)atan2 (radius - point.v, point.h - radius);
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

	void setConfig(bool drawCircle2, CCoord cursOffsetRel)
	{
		this->bDrawCirle2 = drawCircle2;
		this->iCursOffsetRel = cursOffsetRel;
		iCursOffset = (float)size.width()/(float)iCursOffsetRel;
		if(iCursOffset < 1) iCursOffset = 1;

		setDirty();
	}

	void setDisplayFont(CFontRef asFont = 0)
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


	CLASS_METHODS(CKnobFlatSwitch, CControl)

private:
	void quantizeValue()
	{	
		value =	float(int((steps-1)*value + 0.5f)/float(steps-1));
	}

	void drawEdgeLines(CDrawContext* pContext,CPoint &point1, CPoint &point2, float value) const
	{
		valueToPointOffset(point1,point2,value);

		point1.offset (size.left-1, size.top-1);
		point2.offset (size.left-1, size.top-1);

		pContext->moveTo (point1);
		pContext->lineTo (point2);
	}	

protected:

	bool bDrawCirle2;

	int halfCurveSwitch;

	const ConfigKnob& m_configKnob;

	char display[32];

	CCoord iMaxPos, iMinPos;
	CCoord iCursOffset; CCoord iCursOffsetRel;


	CRect rectOld;

	int steps;

	CFontRef myFont;

	CPoint offset;

	long     inset;
	float    startAngle, rangeAngle, halfAngle;
	float    aCoef, bCoef;
	float    radius;
	float    zoomFactor;
	bool	 bMouseOn;
	bool	bActive;

private:
	CPoint firstPoint;
	CPoint lastPoint;
	float  startValue;
	float  fEntryState;
	float  range;
	float  coef;
	CButtonState   oldButton;
	bool   modeLinear;

};// end class CKnobFlatSwitch

class CKnobFlatBranis : public CKnobFlatSwitch
{
public:

	CKnobFlatBranis(const CRect &size, CControlListener* listener, long tag, const ConfigKnob& configKnob, int steps = 0)
		: CKnobFlatSwitch (size, listener, tag, configKnob, steps)
	{
		const CCoord inscale = 1.0/15.0;
		inset = size.width()*inscale; 

		iCursLength = size.width()/4 + 0.5;

		strncpy(display,"",1);

		bMakeCurve = true;
		bShowValue = true;	

		fModsValue = 0.0f;
	}

	void draw (CDrawContext *pContext)
	{
		const CCoord inscale = 1.0/15.0;

		if(rectOld != CControl::size)
		{
			CRect newSize(CControl::size);
			radius = (float)(newSize.right - newSize.left) / 2.0f;
			iCursOffset = newSize.width()/iCursOffsetRel;
			iCursLength = newSize.width()/4 + 0.5;
			inset = (newSize.width()*inscale);
		}

		rectOld = CControl::size;		

		CKnobFlatSwitch::draw(pContext); // mostly for pContext->setDrawMode(kAntialias);?

		pContext->setLineWidth(CControl::size.width()/10 + 0.5);
		pContext->setFrameColor(m_configKnob.colFrame);
		pContext->setFillColor(m_configKnob.colBkground1);
		CRect n(CControl::size); n.inset(inset+0.5,inset+0.5);
		pContext->drawEllipse(n,kDrawFilledAndStroked);

		drawHandle (pContext);
		if(bShowValue && (bMouseOn || steps > 1) && CControl::size.width() >= 28.0)
			drawString (pContext);

		//drawModsCircle(pContext);

		CControl::setDirty (false);		
	}

	void drawHandle (CDrawContext *pContext)
	{
		CDrawContext* drawContext = pContext;			

		CPoint origin (CControl::size.width () / 2, CControl::size.height () / 2);
		CPoint where;

		pContext->setLineWidth(1.4);

		if(steps > 1){

			quantizeValue();

			if(bActive)
				drawContext->setFrameColor (m_configKnob.colShadowHandle);
			else
				drawContext->setFrameColor (m_configKnob.colInactive);

			for(int j=int(CControl::value*512.0f)-halfCurveSwitch; j< int(CControl::value*512.0f)+halfCurveSwitch; ++j) // draws the curve
			{
				drawEdgeLines(drawContext,where,origin,j/512.0f, iCursLength/5);
			}

			drawContext->setFrameColor (m_configKnob.colFrame);
			pContext->setLineWidth(CControl::getFrame()->getViewSize().getWidth()/233);

			for(int j=0; j< steps; ++j) // draws the stops
			{
				drawEdgeLines(drawContext,where,origin,float(j)/float(steps-1),iCursLength);
			}
		}else{ // continuous mode

			if(bActive)
				drawContext->setFrameColor (m_configKnob.colShadowHandle);
			else
				drawContext->setFrameColor (m_configKnob.colInactive);

			if( steps == -1) // - and + values
			{
				int start = int(CControl::value*200.0f); 
				int end   =  int(0.5f*200.0f)-1;
				if(CControl::value > 0.5f)
				{
					start =  int(0.5f*200.0f); 
					end   =  int(CControl::value*200.0f)-1;
				}
				if(bMakeCurve)
					for(int j= start; j< end; ++j) // draws the curve
					{
						drawEdgeLines(drawContext,where,origin,j/200.0f, iCursLength*0.16f);
					}
			}
			else
			{
				float rez = 512.0f;
				if(CControl::size.width() < 52) rez = 128.0f;
				if(bMakeCurve)
					for(int j=0; j< int(CControl::value*rez)-1; ++j) // draws the curve
					{
						drawEdgeLines(drawContext,where,origin,j/rez, iCursLength*0.16f);
					}
			}
		}

		pContext->setLineWidth(CControl::getFrame()->getViewSize().getWidth()/233);
		if(bActive)
			drawContext->setFrameColor (m_configKnob.colHandle);
		else
			drawContext->setFrameColor (m_configKnob.colInactive);

		drawEdgeLines(drawContext,where,origin,CControl::value,iCursLength); // draws the handle
	}

	void valueToPointOffset (CPoint &point1, CPoint &point2, float value, CCoord iCursLength) const
	{
		CKnobFlatSwitch::valueToPointOffset(point1, point2, value);

		int handleOffset = 0;
		if(iCursLength) handleOffset = (CCoord)(iCursLength/5.0f + 0.5f);
		float alpha = (value - bCoef) / aCoef; 
		float k1 = cosf (alpha);
		float k2 = sinf (alpha);
		point1.h = (CCoord)(radius + k1 * (radius - inset - handleOffset) + 0.5f);
		point1.v = (CCoord)(radius - k2 * (radius - inset - handleOffset) + 0.5f);
		point2.h = (CCoord)(radius + k1 * (radius - iCursOffset-iCursLength) + 0.5f);
		point2.v = (CCoord)(radius - k2 * (radius - iCursOffset-iCursLength) + 0.5f);
	}

	void drawString (CDrawContext *pContext)
	{
		if(0==strcmp(display, " ")) return;

		CDrawContext* drawContext = pContext;

		CRect rectValue(CControl::size);
		if(CControl::size.height() < 52.0)
		{rectValue.inset(1.0,CControl::size.height()*0.38);}
		else
		{rectValue.inset(2.0,CControl::size.height()*0.41);}
		if(steps == 1 || steps == -1){
			drawContext->setFillColor(MakeCColor(0,0,0,111));
			drawContext->drawRect(rectValue,kDrawFilled);
		}
		drawContext->setFontColor(m_configKnob.colFont);
		drawContext->setFont(myFont,rectValue.height(),kBoldFace);

		//if(steps > 1){
		//	quantizeValue();
		//	sprintf(display,"step %d",  getValueQuantized()+1); 
		//}else{
		//	sprintf(display,"%1.2f",  value);
		//}
		if(drawContext->getStringWidth(display) >= size.width()-1.0)
		{drawContext->drawString(display,rectValue,kLeftText);}
		else
		{drawContext->drawString(display,rectValue,kCenterText);}

		//drawContext->drawRect(rectValue);	
	}

	void getModsValue(float mod)
	{
		fModsValue = mod > value? value : mod;
	}

	//void setValueQuantized(int var)
	//{
	//	if(steps > 1)
	//		value = var/float(steps-1);
	//	else
	//		value = float(var);		
	//}

	//int getValueQuantized()
	//{
	//	if(steps > 1)
	//		return int(value*(steps-1) + 0.5f);
	//	else
	//		return int(value + 0.5f);// as if steps = 2
	//}	

	void DoCurve(bool var)
	{
		bMakeCurve = var;
	}
	void SeeValue(bool var)
	{
		bShowValue = var;
	}

	CLASS_METHODS(CKnobFlatBranis, CKnobFlatSwitch)

private:

	CCoord iCursLength;

	void quantizeValue()
	{	
		CControl::value =	float(int((steps-1)*CControl::value + 0.5f)/float(steps-1));
	}

	void drawEdgeLines(CDrawContext* pContext,CPoint &point1, CPoint &point2, float value, CCoord iCursLength = 0) const
	{
		valueToPointOffset(point1,point2,value,iCursLength);

		point1.offset (CControl::size.left-1, CControl::size.top-1);
		point2.offset (CControl::size.left-1, CControl::size.top-1);

		// hack for small knobs
		//if(point2.h == point1.h) point2.h = point1.h+1;
		//if(point2.v == point1.v) point2.v = point1.v+1;

		pContext->moveTo (point1);
		pContext->lineTo (point2);
	}

	void drawModsCircle (CDrawContext *pContext)
	{
		CPoint origin (CControl::size.width () / 2, CControl::size.height () / 2);
		CPoint point1 (CControl::size.width (),  CControl::size.height ());

		CCoord radius = CControl::size.width ()/2;
		CRect spot(0.0,0.0,fModsValue*radius, fModsValue*radius);
		spot.centerInside(CControl::size);

		pContext->setFillColor(kRedCColor);
		pContext->drawEllipse(spot, kDrawFilled);

		//float alpha = (fModsValue - bCoef) / aCoef; 
		//float k1 = cosf (alpha);
		//float k2 = sinf (alpha);
		//float inset = 0.2f*CControl::size.getWidth();
		//point1.h = (CCoord)(radius + k1 * (radius - inset) + 0.5f);
		//point1.v = (CCoord)(radius - k2 * (radius - inset) + 0.5f);
		//point2.h = (CCoord)(radius + k1 * (radius - inset*2.0) + 0.5f);
		//point2.v = (CCoord)(radius - k2 * (radius - inset*2.0) + 0.5f);
		//CRect spot(point2,point1);
		//drawContext->setFillColor(kRedCColor);
		//drawContext->drawEllipse(spot, kDrawFilled);
	}

	bool bMakeCurve;
	bool bShowValue;
	float fModsValue;

};// end class CKnobFlatBranis

class CEnvDisplay : public CView
{
public:
	CEnvDisplay(const CRect &size, const ConfigGraphView& configGraphView): CView(size), m_configGraphView(configGraphView)
	{
		env1.Reset();
		env1.setSR(size.width()*0.25f);
		env1.setAttackTime(0.5f); // seconds
		env1.setDecayTime(0.5f); // seconds
		env1.setSustainLevel(0.5f);
		env1.setReleaseTime(0.5f); // seconds
		XS = size.width(); 
	}

	void draw (CDrawContext *pContext)
	{
		CDrawContext* drawContext = pContext;
		drawContext->setFillColor(m_configGraphView.colBkground1);
		drawContext->setFrameColor(m_configGraphView.colFrame);
		drawContext->drawRect(size,kDrawFilledAndStroked);
		drawContext->moveTo(size.getBottomLeft().offset(2,-5));		
		drawContext->setDrawMode(kAntialias);
		drawContext->setFrameColor(m_configGraphView.colSignal);
		drawContext->setLineWidth(1);
		const int w = size.width();

		//env1.Reset();
		env1.keyOn(127);
		for(int x = 2; x < w; ++x)
		{			
			float level = env1.tick();
			const int h = size.height()-5;
			CPoint Y2 = CPoint(x, h-env1.tick()*h + 1);
			Y2.offset(size.left, size.top);

			if(x == w/2+10) 
			{
				//drawContext->moveTo(CPoint(x+size.left, size.bottom));
				//drawContext->lineTo(CPoint(x+size.left, size.top));
				//drawContext->moveTo(Y2);
				env1.keyOff(); // 10 of sustain
			}

			drawContext->lineTo(Y2);

			if (level < 0.0001f)
			{break;}
		}

		setDirty(false);
	}

	void attack(float var)
	{
		env1.setAttackTime(var+0.05f); // seconds , offset for the draw
		setDirty();
	}
	void decay(float var)
	{
		env1.setDecayTime(var); // seconds
		setDirty();
	}
	void sustain(float var)
	{
		env1.setSustainLevel(var+0.001f);
		setDirty();
	}
	void release(float var)
	{
		env1.setReleaseTime(var); // seconds
		setDirty();
	}

	CLASS_METHODS(CEnvDisplay, CView)

private:

	fastADSR env1;
	int XS;
	const ConfigGraphView& m_configGraphView;

};


class CSliderSpot : public CSlider
{
public:

	CSliderSpot (const CRect &size, CControlListener* listener, long tag, long iMinPos, long iMaxPos, const long style = kLeft|kHorizontal)
		: CSlider (size, listener, tag,  iMinPos,  iMaxPos, NULL, NULL, CPoint (0, 0), style)
	{
		bFreeClick = false;

		if (style & kHorizontal)
		{
			widthOfSlider  = size.width ()-iMaxPos; // handle 
			heightOfSlider = size.height ();
		}
		else
		{
			widthOfSlider  = size.width (); // handle 
			heightOfSlider = size.height ()-iMaxPos;
		}

		widthControl  = size.width (); // background
		heightControl = size.height ();

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

	virtual	~CSliderSpot()
	{
		if(myFont)// && 0 == strcmp(myFont->getName(), "Arial")
			myFont->forget();
	}

	void setString32(const char* string)
	{
		strncpy(display,string,32);
	}

	void draw (CDrawContext *pContext)
	{
		if(rectOld != size)
		{
			CRect newSize(size);

			iMaxPos = (newSize.right - newSize.left)-46*scaleGUIx; // 46 matches the number in bool myEditor::open(void* ptr), ugly so far

			widthOfSlider  = newSize.width ()-iMaxPos; // handle 
			heightOfSlider = newSize.height ();

			widthControl  = newSize.width (); // background
			heightControl = newSize.height ();

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
			rectNew.top    = offsetHandle.v;
			rectNew.bottom = rectNew.top + heightOfSlider;	

			rectNew.left   = offsetHandle.h + (int)(fValue * rangeHandle);
			rectNew.left   = (rectNew.left < minTmp) ? minTmp : rectNew.left;

			rectNew.right  = rectNew.left + widthOfSlider;
		}
		else
		{
			rectNew.left   = offsetHandle.h-1;
			rectNew.right  = rectNew.left + widthOfSlider + 1;	

			rectNew.top    = offsetHandle.v + (int)(fValue * rangeHandle);

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
			drawContext->setLineWidth(rectNew.width()/7);
			drawContext->moveTo(CPoint(rectNew.left+rectNew.width()/2,rectNew.top+rectNew.height()/4));
			drawContext->lineTo(CPoint(rectNew.left+rectNew.width()/2,rectNew.bottom-rectNew.height()/4));
		}
		else
		{
			drawContext->setLineWidth(1);
			drawContext->setFrameColor(colorFrameHandle);
			drawContext->setFillColor(colorHandle);	
			drawContext->drawRect(rectNew,kDrawFilledAndStroked);
			drawContext->moveTo(CPoint(rectNew.left,rectNew.top));
			drawContext->lineTo(CPoint(rectNew.right,rectNew.top));
		}

		if(bMouseOn)
		{
			drawString (pContext);
		}

		setDirty (false);
	}

	void drawString (CDrawContext *pContext)
	{
		CDrawContext* drawContext = pContext;

		if (style & kHorizontal)
		{
			const int iVlabel = int(size.height()*0.45833333333333333333f); //(1/2-1/24)

			drawContext->setFillColor(colorBkground1);
			drawContext->setFontColor(colorFont);
			drawContext->setFont(myFont,kFontSizeScaleFactor*iVlabel,kBoldFace); // must be before etStringWidthUTF8(()
			CRect rectValue(size);
			rectValue.top = size.bottom-iVlabel; 
			rectValue.setHeight(iVlabel);
			rectValue.setWidth(drawContext->getStringWidth(display)+3);			

			const int iHspace = size.width()/19;

			if(value > 0.5f){
				rectValue.offset(iHspace,1);
				drawContext->drawRect(rectValue,kDrawFilled);
				drawContext->drawString(display,rectValue,kLeftText);
			}else{
				rectValue.offset(size.right-(3*iHspace+rectValue.width()),1);
				drawContext->drawRect(rectValue,kDrawFilled);
				drawContext->drawString(display,rectValue,kRightText);
			}	
		}
	}

	void ColoursAndConfig(struct ConfigSlider& configSlider )
	{
		colorBkground1 = configSlider.colBkground1;
		colorFrame = configSlider.colFrame;
		colorHandle = configSlider.colHandle;
		colorFrameHandle = configSlider.colFrameHandle;
		colorFont = configSlider.colFont;
		setDirty();
	}

	void setScaleGUIx(float var)
	{
		scaleGUIx = var;
	}

	void setDisplayFont(CFontRef asFont = 0)
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

	CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons)
	{
		if(!buttons.getButtonState()){
			if(where.isInside(size)) // for mouseover
			{
				bMouseOn = true;
				setDirty();
			}
		}
		else
			bMouseOn = true;

		return CSlider::onMouseMoved (where, buttons);
	}	

	CMouseEventResult onMouseUp(CPoint& where, const CButtonState& buttons)
	{
		bMouseOn = false; // for mouseover
		setDirty();

		return CSlider::onMouseUp(where, buttons);
	}

	CMouseEventResult onMouseExited (CPoint& where,const CButtonState& buttons)
	{
		bMouseOn = false; // for mouseover
		setDirty();

		return kMouseEventHandled;
	}

	CLASS_METHODS(CSliderSpot, CSlider)

private:

	char display[32];

	float	scaleGUIx;

	CCoord iMaxPos, iMinPos;

	CRect rectOld;

	CColor colorBkground1, colorFrame, colorFont, colorHandle, colorFrameHandle;

	CFontRef myFont;
	bool bMouseOn;
};// end class CSliderSpot

class CSimpleOnOff : public CControl
{
public:
	CSimpleOnOff(CRect& size, CControlListener* pListener, long tag, const ConfigSwitch& configSwitch) :
	  CControl(size, pListener, tag, 0),
	  m_configSwitch(configSwitch)
	  {	
		  bActive = true;
		  bAddDisplay = false;
	  }

	  long getTag(){ return CControl::getTag(); }

	  void draw(CDrawContext *pContext)
	  {
		  pContext->setLineWidth(1);
		  pContext->setFrameColor(m_configSwitch.colFrame);

		  pContext->setFillColor((value && bActive)?  m_configSwitch.colBkground1 : m_configSwitch.colBkground2);
		  size.makeIntegral();
		  pContext->drawRect(size,kDrawFilledAndStroked);
		  if(bAddDisplay)
		  {
			  CRect rLabel = CRect(size);
			  rLabel.offset(0.0,1.0); 
			  rLabel.inset(2.2,2.5);
			  pContext->setFont(kNormalFontSmaller,rLabel.getHeight(),kBoldFace);
			  pContext->setFontColor((value && bActive)? m_configSwitch.colFont1 : m_configSwitch.colFont2);
			  pContext->drawString(sString,rLabel);
		  }

		  setDirty(false);
	  }

	  virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons)
	  {	
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
			  setDirty();  
			  return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		  }

		  endEdit ();
		  return kMouseEventNotHandled;
	  }

	  virtual void setActive(bool var)
	  {
		  bActive = var;
		  if(bActive)
			  setMouseEnabled(true);
		  else
			  setMouseEnabled(false);

		  setDirty();
	  }

	  virtual void setString32(const char* inLabel = 0)
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


	  CLASS_METHODS(CSimpleOnOff, CControl)

private:

	float step;
	CRect rectOld;
	bool bActive;
	const ConfigSwitch& m_configSwitch;
	bool bAddDisplay;
	char sString[32];

}; // end class CSimpleOnOff

class CMultiSwitch : public CControl // horizontal switch max 12 positions
{
public:
	CMultiSwitch(CRect& size, CControlListener* pListener, long tag, int pos, ScaledFontFactory& fontFactory, const ConfigSwitch& configSwitch) :
	  CControl(size, pListener, tag, 0),
	  m_scaledFontFactory(fontFactory),
	  m_configSwitch(configSwitch)
	  {	
		  this->pos = pos;
		  if(pos > 12)
			  this->pos = 12;

		  for(int j = 0; j < 12; ++j)
		  {
			  labels[j] = new char[32];
		  }	

		  bActive = true;

		  setLabelsGapV(0.0);

		  bSwitchBkground = false;
		  bMomentarySwitch = false;
	  }

	  ~CMultiSwitch()
	  {
		  for(int j = 0; j < 12; ++j)
		  {			  
			  delete labels[j];
		  }	
	  }
	  void setMomentary(const bool value)
	  {
		  bMomentarySwitch = value;
	  }

	  void setLabels(const char** Labels)
	  {
		  for(int j = 0; j < pos; ++j)
		  {
			  strncpy(labels[j],Labels[j],32);
		  }		  		  
	  }

	  long getTag(){ return CControl::getTag(); }

	  void draw(CDrawContext *pContext)
	  {
		  CColor cSwitchBkground;
		   if(bSwitchBkground)
		  {			  
			  cSwitchBkground.red = m_configSwitch.colBkground2.red;
			  cSwitchBkground.green = m_configSwitch.colBkground2.green;
			  cSwitchBkground.blue = m_configSwitch.colBkground2.blue;
			  setValues(0.5f,0.5f,-0.16f,cSwitchBkground);			  
		  }

		  pContext->setDrawMode(kCopyMode);
		  pContext->setLineWidth(1.5f);
		  pContext->setFont(m_scaledFontFactory.getScaledSmallFont(),0,kBoldFace);
		  pContext->setFillColor(m_configSwitch.colBkground2);
		  pContext->setFrameColor(m_configSwitch.colFrame);
		  CCoord celSize = size.width()/pos;
		  CRect rStep = CRect(0, 0, celSize, size.height());
		  rStep.offset(size.left+1, size.top);	
		  for(int j = 0; j < pos; ++j)
		  {
			  if(bActive)
			  {
				  if(pos > 1)
				  {
					  if(j == int(value*(pos-1)+0.5f)) {
						  pContext->setFont(m_scaledFontFactory.getScaledMediumFont(),0,kBoldFace);
						  pContext->setFontColor(m_configSwitch.colFont2);
						   if(bSwitchBkground) pContext->setFillColor(m_configSwitch.colBkground2);
					  }else{
						  pContext->setFont(m_scaledFontFactory.getScaledSmallFont(),0,kBoldFace);
						  pContext->setFontColor(m_configSwitch.colFont1);
						   if(bSwitchBkground) pContext->setFillColor(cSwitchBkground);
					  }
				  }
				  else
				  {
					  pContext->setFont((value)? m_scaledFontFactory.getScaledMediumFont() : m_scaledFontFactory.getScaledSmallFont(),0,kBoldFace);
					  pContext->setFontColor((value)? m_configSwitch.colFont2 : m_configSwitch.colFont1);
					  if(bSwitchBkground) pContext->setFillColor((value)?m_configSwitch.colBkground2: cSwitchBkground);
				  }
			  }
			  else
				  pContext->setFontColor(m_configSwitch.colFrame);

			  if(bMomentarySwitch) 
			  {
				  if(bSwitchBkground) pContext->setFillColor(cSwitchBkground);
				  pContext->setFontColor(m_configSwitch.colFont1);
			  }

			  pContext->drawRect(rStep.inset(1,0),kDrawFilled); rStep.inset(-1,0); //restore width
			   rStep.offset(0.0, labelsGapV);
			  pContext->drawString(labels[j], rStep);
			   rStep.offset(0.0, -labelsGapV);

			  pContext->moveTo(rStep.getTopLeft());
			  pContext->lineTo(rStep.getBottomLeft());

			  rStep.offset(celSize+j/pos,0);
		  }

		  pContext->moveTo(size.getTopRight().offset(-1,0));
		  pContext->lineTo(size.getBottomRight().offset(-1,0));

		  setDirty(false);
	  }

	  virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons)
	  {	
		  value = oldValue;

		  if (!(buttons & kLButton))
			  return kMouseEventNotHandled;

		  beginEdit ();

		  CCoord celSize = size.width()/pos;
		  CRect rStep = CRect(0, 0, celSize, size.height());
		  rStep.offset(size.left, size.top);

		  if(size.pointInside(where))
		  {
			  if(pos > 1)
			  {
				  const float step = float(size.width())/float(pos);
				  const float X = float(where.x-size.left-step*0.5f)/ float(size.width());

				  value = int(X*pos+0.5f)/float(pos-1);
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
			  setDirty(); 
			  return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		  }

		  endEdit ();
		  return kMouseEventNotHandled;
	  }

	  virtual void setActive(bool var)
	  {
		  bActive = var;
		  if(bActive)
			  setMouseEnabled(true);
		  else
			  setMouseEnabled(false);

		  setDirty();
	  }

	  void setLabelsGapV(CCoord var)
	  {
		  labelsGapV = var;
	  }

	  virtual void setSwitchBkground(bool var)
	  {
		  bSwitchBkground = var;
	  }

	  CLASS_METHODS(CMultiSwitch, CControl)

private:

	float step;
	CRect rectOld;
	int pos;
	char* labels[12];
	bool bActive;
	ScaledFontFactory& m_scaledFontFactory;
	const ConfigSwitch& m_configSwitch;
	CCoord labelsGapV;
	bool bSwitchBkground;
	bool bMomentarySwitch;

}; // end class CMultiSwitch

class CLFODisplay : public CView
{
public:
	CLFODisplay(const CRect &size): CView(size)
	{
		XS = size.width(); 
		lfo1.setSR(size.width());
		lfo1.setPureRate(2.0f);
		setMouseEnabled (false);
		bSmoothed = false;
	}

	void draw (CDrawContext *pContext)
	{		
		CDrawContext* drawContext = pContext;
		drawContext->setFrameColor(MakeCColor(212,212,212,255));
		drawContext->setLineWidth(1);	
		drawContext->setDrawMode(kAntialias);		

		const int w = int(size.width());

		for(int x = 0; x < w; ++x)
		{			
			float level = 0.5f*lfo1.tick()+0.5;
			double h = size.height()-2.0;
			CPoint Y1 = CPoint(x, h - 0.89f*level*h);
			Y1.offset(size.left, size.top);
			if(x == 0)
				drawContext->moveTo(Y1);
			else
				drawContext->lineTo(Y1);
		}

		if(bSmoothed){ // do we need such overloaded code here?!
			drawContext->setFillColor(MakeCColor(203,203,203,111));
			drawContext->drawRect(size,kDrawFilled);
			CColor oldColor = drawContext->getFontColor(); // get
			CFontRef tempFont = drawContext->getFont(); // get
			const double oldSize = tempFont->getSize(); // get
			tempFont->setSize(size.height()-5.0); // set
			drawContext->setFontColor(kBlackCColor); // set
			drawContext->drawString("smoothed",size);
			drawContext->setFontColor(oldColor); // restore
			tempFont->setSize(oldSize); // restore
		}

		setDirty(false);
	}

	void setShape(int var)
	{	
		bSmoothed = false;

		if(var == 7) {var = 5; bSmoothed = true;}
		else if(var == 8) {var = 6; bSmoothed = true;}

		lfo1.setWaveform(var);
		if(var == 9 || var == 6)
			lfo1.setPureRate(10.0f);
		else
			lfo1.setPureRate(2.0f);	

		lfo1.gainForDisplay();

		lfo1.sync();

		setDirty();
	}

	CLASS_METHODS(CLFODisplay, CView)

private:

	LFOx lfo1;
	int XS;
	bool bSmoothed;

};

class CSliderNano : public CSlider
{
public:

	CSliderNano (const CRect &size, CControlListener* listener, long tag, long iMinPos, long iMaxPos,
	             const ConfigSlider& configSlider, const long style = kLeft|kHorizontal)
		: CSlider (size, listener, tag,  iMinPos,  iMaxPos, NULL, NULL, CPoint (0, 0), style),
		  m_configSlider(configSlider)
	{
		bFreeClick = true;

		if (style & kHorizontal)
		{				
			widthOfSlider =	heightOfSlider = size.height ();  // handle 
		}
		else
		{				
			widthOfSlider =	heightOfSlider = size.width ();  // handle 
		}

		zoomFactor = 10.2f;

		//setWantsFocus (true);

		//scaleGUIx = 1.0f;


		//this->iMaxPos = iMaxPos;
		//this->iMinPos = iMinPos;

		rectOld = size;

		bSquareView = false;
		bActive = true;
	}

	void draw (CDrawContext *pContext)
	{
		if(rectOld != size)
		{
			CRect newSize(size);

			if (style & kHorizontal)
			{				
				widthOfSlider =	heightOfSlider = newSize.getHeight();
				CSlider::minPos = 0.0;
			}
			else
			{				
				widthOfSlider =	heightOfSlider = newSize.getWidth();
				CSlider::minPos = newSize.getHeight();
			}			
		}

		rectOld = size;

		CDrawContext* drawContext = pContext;

		// (re)draw background
		CRect rect (0, 0, widthControl, heightControl/8);
		rect.offset (size.left, size.top+heightControl/2-heightControl/16);

		if (style & kVertical)
		{
			rect = CRect(0, 0, widthControl/8, heightControl);
			rect.offset (size.left+widthControl/2-widthControl/16, size.top);
		}


		const CCoord frcWidth = rect.getWidth();
		if (frcWidth < 2 ) rect.setWidth( 1 );	// fix of resize
		else if(frcWidth < 3 ) rect.setWidth( 2 ); 

		const CCoord frcHeight = rect.getHeight();
		if (frcHeight < 2 ) rect.setHeight( 1 ); // fix of resize
		else if(frcHeight < 3 ) rect.setHeight( 2 ); 

		drawContext->setDrawMode(kAliasing);
		drawContext->setFrameColor(m_configSlider.colFrame);
		drawContext->setFillColor(m_configSlider.colTrackFront);
		drawContext->drawRect(rect,kDrawFilled);


		float normValue = getValueNormalized ();
		if (style & kRight || style & kBottom)
			normValue = 1.f - normValue;

		// calc new coords of slider
		CRect rectNew;
		if (style & kHorizontal)
		{
			rectNew.top    = offsetHandle.v; // which is zero in CSliderNano
			rectNew.bottom = rectNew.top + heightOfSlider;	

			rectNew.left   = offsetHandle.h + floor (normValue * rangeHandle);
			rectNew.left   = (rectNew.left < minTmp) ? minTmp : rectNew.left;

			rectNew.right  = rectNew.left + widthOfSlider;
			//rectNew.right  = (rectNew.right > maxTmp) ? maxTmp : rectNew.right;
		}
		else
		{
			rectNew.left   = offsetHandle.h; // which is zero in CSliderNano
			rectNew.right  = rectNew.left + widthOfSlider;	

			rectNew.top    = offsetHandle.v + floor (normValue * rangeHandle);
			//rectNew.top    = (rectNew.top < minTmp) ? minTmp : rectNew.top;

			rectNew.bottom = rectNew.top + heightOfSlider;
			//rectNew.bottom = (rectNew.bottom > maxTmp) ? maxTmp : rectNew.bottom;
		}
		rectNew.offset (getViewSize ().left, getViewSize ().top);


		CRect rectBar (rect); 

		if (style & kHorizontal)
		{		
			rectBar.right += 1; 
			rectBar.left = rectNew.left;
		}
		else
		{
			rectBar.bottom = rectNew.bottom;			
		}
		drawContext->setFillColor(m_configSlider.colTrackBack);
		drawContext->drawRect(rectBar,kDrawFilled);		

		drawContext->setDrawMode(kAntiAliasing);

		// draw slider handle at new position		

		CColor handleColOff = MakeCColor(128,128,128,255);

		if(bActive)
			drawContext->setFillColor(m_configSlider.colHandle);
		else
			drawContext->setFillColor(handleColOff);

		if(bSquareView && CSlider::getValue() < 0.01f)
			drawContext->setFillColor(handleColOff);

		drawContext->setFrameColor(m_configSlider.colTrackFront);
		drawContext->setLineWidth(1.0);

		if(bSquareView)
			drawContext->drawRect(rectNew.inset(1,1),kDrawFilledAndStroked);
		else
			drawContext->drawEllipse(rectNew.inset(1,1),kDrawFilledAndStroked);

		setDirty (false);
	}

	void setScaleGUIx(float var)
	{
		//scaleGUIx = var;
	}

	virtual void setActive(bool var)
	{
		bActive = var;
		if(bActive)
			setMouseEnabled(true);
		else
			setMouseEnabled(false);

		setDirty();
	}

	void setSquareHandle(bool mode)
	{
		bSquareView = mode;
	}

	CLASS_METHODS(CSliderNano, CSlider)

private:

	//float	scaleGUIx;

	//CCoord iMaxPos, iMinPos;

	CRect rectOld;

	const ConfigSlider& m_configSlider;

	bool bSquareView;
	bool bActive;

};// end class CSliderNano


class CFilterPlot : public CControl
{
public:
	CFilterPlot(const CRect &size): CControl(size,0,-100000,0)
	{
		setMouseEnabled (false);
		iType = 0;
	}

	void draw (CDrawContext *pContext)
	{		
		CRect sime(size); 
		sime.inset(0.23*size.width() ,0.23*size.height());
		sime.offset(-0.23*size.width(),0.0);


		CDrawContext* drawContext = pContext;
		drawContext->setFrameColor(MakeCColor(101,101,102,255));
		drawContext->setLineWidth(1);	
		drawContext->moveTo(sime.getTopLeft());
		drawContext->lineTo(sime.getBottomLeft());
		drawContext->lineTo(sime.getBottomRight());
		drawContext->setDrawMode(kAntialias);
		drawContext->setFrameColor(MakeCColor(197,197,197,255));
		drawContext->setLineWidth(2);

		int XS = int(sime.width());

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

			double h = sime.height()-2.0;
			CPoint Y1 = CPoint(x, h - 0.89f*level*h);
			Y1.offset(sime.left, sime.top);
			if(x == 0)
				drawContext->moveTo(Y1);
			else
				drawContext->lineTo(Y1);
		}

		setDirty(false);
	}

	void setType(int var)
	{	
		iType = var;
		setDirty();
	}

	CLASS_METHODS(CFilterPlot, CControl)

private:

	int iType;
}; // end class CFilterPlot



class CKickPreviousNext : public CControl
{
public:
	CKickPreviousNext(CRect& size, CControlListener* pListener, long tag) :
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

	  ~CKickPreviousNext()
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

	  long getTag(){ return CControl::getTag(); }

	  void draw(CDrawContext *pContext)
	  {
		  if(rectOld != size)
		  {
			  *zone[0] = CRect(size.left,size.top,size.right-size.getWidth()/2,size.bottom);
			  *zone[1] = CRect(size.left+size.getWidth()/2,size.top,size.right,size.bottom);			
		  }

		  rectOld = size;

		  pContext->setDrawMode(kAntiAliasing);
		  pContext->setFrameColor(colorFont);

		  pContext->setFillColor(bMouseOn1? colorBkground1 : colorBkground2);
		  pContext->drawRect(*zone[0],kDrawFilled);

		  pContext->setFillColor(bMouseOn2? colorBkground1 : colorBkground2);
		  pContext->drawRect(*zone[1],kDrawFilled);

		  pContext->setLineWidth(std::max(1.0,0.1*size.height()));
		  drawPoints(pContext, *zone[0], 1);
		  drawPoints(pContext, *zone[1], 2);

		  setDirty(false);
	  }

	  void drawPoints(CDrawContext *pContext, CRect rect, int which)
	  {	
		  CCoord width = getFrame()->getWidth();
		  CCoord height = rect.height();
		  if(which == 1)
		  {	
			  CPoint x1 = CPoint(rect.left+width*0.014, rect.top+height*0.2); 
			  CPoint x2 = CPoint(rect.left+width*0.005, rect.top+height*0.5);	
			  CPoint x3 = CPoint(rect.left+width*0.014, rect.top+height*0.8);

			  for(int i = 0; i < 3; ++i)
			  {
				  pContext->moveTo(x1);
				  pContext->lineTo(x2);
				  pContext->lineTo(x3);
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
				  pContext->moveTo(x1);
				  pContext->lineTo(x2);
				  pContext->lineTo(x3);
				  x1.offset(-width*0.014,0);
				  x2.offset(-width*0.014,0);
				  x3.offset(-width*0.014,0);
			  }
		  }
	  }

	  virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons)
	  {	
		  if (!(buttons & kLButton))
			  return kMouseEventNotHandled;

		  beginEdit ();

		  value = oldValue;

		  if(zone[0]->pointInside(where))
		  {
			  value -= step;
		  }
		  else if(zone[1]->pointInside(where))
		  {
			  value += step;
		  }
		  if(listener) 
			  listener->valueChanged(this);
		  setDirty();  
		  endEdit ();
		  oldValue = value;
		  return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	  }

	  virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons) 
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

	  virtual CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons)
	  {
		  bMouseOn1 = false;
		  bMouseOn2 = false;

		  setDirty();
		  return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	  }

	  void ColoursAndConfig(struct ConfigKick& configKick )
	  {
		  colorBkground1 = configKick.colBkground1;
		  colorBkground2 = configKick.colBkground2;
		  colorFrame = configKick.colFrame;
		  colorFont = configKick.colFont;
		  setDirty();
	  }

	  CLASS_METHODS(CKickPreviousNext, CControl)

private:

	CRect* zone[2];
	float step;
	CRect rectOld;
	CColor colorBkground1;
	CColor colorBkground2;
	CColor colorFrame; 
	CColor colorFont;
	bool bMouseOn1,bMouseOn2;

}; // end class CKickPreviousNext

class CPlotDisplay : public CView
{
public:
	CPlotDisplay(const CRect &size, const ConfigGraphView& configGraphView): CView(size), m_configGraphView(configGraphView)
	{
		for (int i = 0; i < 512; ++i)
		{
			arrayPlotEQ[i] = float(i)/511.0f;
		}
	}

	void draw (CDrawContext *pContext)
	{
		CDrawContext* drawContext = pContext;
		drawContext->setFrameColor(m_configGraphView.colFrame);
		drawContext->setDrawMode(kAntialias);
		drawContext->setFillColor(m_configGraphView.colBkground1);
		drawContext->drawRect(size,kDrawFilled);
		drawContext->setLineWidth(1);
		drawContext->moveTo(size.getTopLeft());
		drawContext->lineTo(size.getBottomLeft().offset(0,-1));
		drawContext->lineTo(size.getBottomRight().offset(0,-1));
		drawContext->setFrameColor(m_configGraphView.colSignal);

		const int w = size.width();

		for(int x = 1; x < w-1; ++x)
		{	
			float level = arrayPlotEQ[int(511.0f*float(x)/float(w) + 0.5f)];
			const int h = size.height()-5;
			CPoint Y2 = CPoint(x, h-level*h + 1);
			Y2.offset(size.left, size.top);
			if(x == 1)
				drawContext->moveTo(Y2);
			else
				drawContext->lineTo(Y2);
		}

		setDirty(false);
	}

	void setPlotEQ(float* inArray)
	{
		memcpy(arrayPlotEQ, inArray, sizeof(float)*512);
		setDirty();
	}

	CLASS_METHODS(CPlotDisplay, CView)

private:

	float arrayPlotEQ[512];
	const ConfigGraphView& m_configGraphView;

};


class CGain2OnOff : public CControl
{
public:
	CGain2OnOff(CRect& size, CControlListener* pListener, long tag, const ConfigSwitch& configSwitch) :
	  CControl(size, pListener, tag, 0),
	  m_configSwitch(configSwitch)
	  {	
		  bActive = true;
	  }

	  long getTag(){ return CControl::getTag(); }

	  void draw(CDrawContext *pContext)
	  {
		  pContext->setLineWidth(1);
		  pContext->setFrameColor(m_configSwitch.colFrame);

		  pContext->setFillColor((value && bActive)?  m_configSwitch.colBkground1 : m_configSwitch.colBkground2);

		  if(value < 0.5f || value > 0.5f)
		  {
			  pContext->drawRect(size,kDrawFilledAndStroked);
		  }
		  else
		  {
			  pContext->setFillColor(m_configSwitch.colBkground1);
			  CRect rHalf = CRect(size);
			  rHalf.setWidth(size.getWidth()*0.5);
			  pContext->drawRect(rHalf,kDrawFilledAndStroked);
			  pContext->setFillColor(m_configSwitch.colBkground2);
			  rHalf.offset(size.getWidth()*0.5,0.0);
			  pContext->drawRect(rHalf,kDrawFilledAndStroked);
		  }

		  setDirty(false);
	  }

	  virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons)
	  {	
		  value = oldValue;

		  if (!(buttons & kLButton))
			  return kMouseEventNotHandled;

		  beginEdit ();

		  if(size.pointInside(where))
		  {
			  if(value > 0.5f)
				  value = 0.0f;
			  else if(value)
				  value = 1.0f;
			  else
				  value = 0.5f;

			  if (value != oldValue && listener)
				  listener->valueChanged (this);

			  bounceValue ();
			  endEdit ();
			  setDirty();  
			  return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		  }

		  endEdit ();
		  return kMouseEventNotHandled;
	  }

	  virtual void setActive(bool var)
	  {
		  bActive = var;
		  if(bActive)
			  setMouseEnabled(true);
		  else
			  setMouseEnabled(false);

		  setDirty();
	  }


	  CLASS_METHODS(CGain2OnOff, CControl)

private:

	float step;
	CRect rectOld;
	bool bActive;
	const ConfigSwitch& m_configSwitch;

}; // end class CGain2OnOff
