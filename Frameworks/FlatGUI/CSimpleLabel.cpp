//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2015
//****************************************************************************************************//

#include "FlatGUI/CSimpleLabel.h"
#include "FlatGUI/FontSizeFactors.hpp"

CSimpleLabel::CSimpleLabel(const CRect& size, bool bMakeFrame, const CHoriTxtAlign hAlign)
:CParamDisplay (size)
{
	hAlign_xx = hAlign;
	bDrawFrame = bMakeFrame;

	display.reserve(128);

	setTag(-1);

	ConfigLabel configLabel;
	ColoursAndConfig(configLabel);

	bflag = false;
	bMouseOn = false;
	bPopUpMode = false;
	m_scaledFontFactory  = 0;
	prevValint = 0;
	addWindow_ = 0;

	CRect sizeDisplay(0.0, 0.0, size.getWidth()*10.0, size.getHeight()*0.5);
	sizeDisplay.offset(size.left-size.getWidth()*10.0, size.bottom);								
	addWindow_ = new CPopup(sizeDisplay);	
}

CSimpleLabel::~CSimpleLabel()
{
	//if(addWindow_)  addWindow_->forget(); NO!
	addWindow_ = 0;
}

void CSimpleLabel::setString128(const char* string)
{	
	if(strlen(string) > 0 &&  strlen(string) < 128)
	{
		display.assign(string);
		if(addWindow_) addWindow_->setDisplay(display);
	}

	setDirty();
}

void CSimpleLabel::setValint(int val)
{
	if(val != prevValint)
	{
	char string[128] = {0};
	sprintf(string, "%d", val);
	setString128(string);
	prevValint = val;
	}
}

void CSimpleLabel::draw (CDrawContext *pContext)
{
	CDrawContext* drawContext = pContext;	
     drawContext->setDrawMode(kAntiAliasing | kNonIntegralMode);

	CRect sizeNew = getViewSize();

	CCoord minLineWidth = sizeNew.getHeight()*0.01;
	if(minLineWidth < 1.5) minLineWidth = 1.5;
	
	CRect rectValue (sizeNew);
	rectValue.inset(minLineWidth*0.5, minLineWidth*0.5);

	CRect rectclip (sizeNew);
	drawContext->setClipRect(rectclip.extend(2.0,0.0));

	if(bDrawFrame)
	{	
		if(bMouseOn)
			drawContext->setFrameColor(colorFont);
		else
			drawContext->setFrameColor(colorFrame);

		drawContext->setLineWidth(minLineWidth);
		//drawContext->drawRect(rectValue, kDrawStroked);

		drawContext->drawLine(rectValue.getTopLeft(), rectValue.getTopRight());
		drawContext->drawLine(rectValue.getBottomLeft().offset(0.0,-1.0), rectValue.getBottomRight().offset(0.0,-1.0));
	}

	if(display.size() > 0 && m_scaledFontFactory)
	{	
		
		pContext->setFont(m_scaledFontFactory);
		pContext->setFontColor(colorFont);
#if MAC
		CCoord fontSize  = 1.0*m_scaledFontFactory->getSize();
#else
		CCoord fontSize  = 0.9*m_scaledFontFactory->getSize();
#endif
		pContext->setFont(m_scaledFontFactory, fontSize, kBoldFace);
		
		if(!bPopUpMode) 
		{
			drawContext->drawString(display.c_str(), rectValue, hAlign_xx);
		}
		else if(addWindow_)
		{
			CRect sizeDisplay(0.0, 0.0, sizeNew.getWidth()*10.0, sizeNew.getHeight()*0.5);
			sizeDisplay.offset(sizeNew.left-size.getWidth()*10.0, sizeNew.bottom);								
			addWindow_->setViewSize(sizeDisplay);
		}
	}

	setDirty(false);
}
void CSimpleLabel::ColoursAndConfig(struct ConfigLabel& configLabel, uint8_t alpha)
{
	colorBkground1 = configLabel.colBkground1;
	colorFrame = configLabel.colFrame;
	colorFont = configLabel.colFont;

	colorFrame.alpha = alpha;
	colorFont.alpha = alpha;	

	setDirty();
}

void CSimpleLabel::setFontColourOnly(const CColor& colFontsin)
{
	colorFont = colFontsin;
	setDirty();
}

void CSimpleLabel::setDisplayFont(CFontDesc* scaledFontFactory)
{
	m_scaledFontFactory = scaledFontFactory;
}

CMouseEventResult CSimpleLabel::onMouseDown (CPoint& where, const CButtonState& buttons)
{	
	if(size.pointInside(where) && ((buttons & kLButton) || (buttons & kRButton)))
	{ 
		IControlListener* listener = getListener();
		if(listener) listener->valueChanged(this);
		bflag = true;
		return  kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	} 

	return kMouseEventNotHandled;
}

CMouseEventResult CSimpleLabel::onMouseEntered (CPoint& where, const CButtonState& buttons) 
{
	if(size.pointInside(where) && !((buttons & kLButton) || (buttons & kRButton)))
	{
		if(bPopUpMode && addWindow_)
		{
			if(!addWindow_->isAttached()) getFrame()->addView(addWindow_);
			addWindow_->setVisible(true);
		}
	}	

	bMouseOn = true;
	setDirty();
	return  kMouseEventHandled;
}

CMouseEventResult CSimpleLabel::onMouseExited (CPoint& where, const CButtonState& buttons)
{
	if(bPopUpMode && addWindow_) addWindow_->setVisible(false);

	bMouseOn = false;
	setDirty();
	return  kMouseEventHandled;
}
