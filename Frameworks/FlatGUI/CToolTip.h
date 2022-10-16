#pragma once
#include "vstgui/vstgui.h"

class CToolTip  : public CParamDisplay 
{
public:
	CToolTip(const CRect& size, bool bMakeFrame, const CHoriTxtAlign hAlign);

	~CToolTip ();

	void setColours(const CColor& sFrameCColor, const CColor& sFontCColor2)
	{
		kFrameCColor = sFrameCColor;
		kFontCColor2 = sFontCColor2;
		setDirty();
	}

	void setString128(const char sToolTipN[5][128])
	{		
		strncpy(display1,sToolTipN[0],128);
		strncpy(display2,sToolTipN[1],128);
		strncpy(display3,sToolTipN[2],128);
		strncpy(display4,sToolTipN[3],128);
		strncpy(display5,sToolTipN[4],128);
	}

	void setDisplayFont(CFontRef asFont = 0, const char* fontName = "Arial");


	void draw (CDrawContext *pContext)
	{
		if(!myFont) return;

		CDrawContext* drawContext = pContext;

		if(bShowLarge)
		{
			const double lineHeight = size.getHeight()/5.5;
			if(bDrawFrame)
				drawContext->setFrameColor(kFrameCColor);

			kFontCColor2.alpha = 255;
			drawContext->setFontColor(kFontCColor2);
			drawContext->setFont(myFont,lineHeight*0.57,kBoldFace);

			CRect rectValue (size);
			if(bDrawFrame){
				kFrameCColor.alpha = 128;
				drawContext->setFillColor(kFrameCColor);
				drawContext->drawRect(rectValue, kDrawFilled);
			}
			rectValue.setHeight(lineHeight);
			drawContext->drawString(display1,rectValue, hAlign_xx);
			rectValue.offset(0,lineHeight);
			drawContext->drawString(display2,rectValue, hAlign_xx);
			rectValue.offset(0,lineHeight);
			drawContext->drawString(display3,rectValue, hAlign_xx);
			rectValue.offset(0,lineHeight);
			drawContext->drawString(display4,rectValue, hAlign_xx);
			rectValue.offset(0,lineHeight);
			drawContext->drawString(display5,rectValue, hAlign_xx);

			if(bDrawFrame)
			{
				const CCoord switchSide = size.getWidth()/12.0f;
				CRect fakeSwitch = CRect(size.getBottomRight(), CPoint(switchSide,switchSide));
				fakeSwitch.offset(-switchSide, -switchSide);

				drawContext->drawRect(fakeSwitch, kDrawStroked);
				drawContext->drawString("X",fakeSwitch.offset(0.0,1.0), kCenterText);
			}

		}
		else
		{
			drawContext->drawString("-",rStoredSize, kCenterText);

			kFontCColor2.alpha = 111;
			drawContext->setFillColor(kFontCColor2);
			drawContext->drawRect(rStoredSize, kDrawFilled);
		}

		setDirty(false);
	}

	void setViewSize (const CRect& newSize)
	{
		if (size != newSize)
		{
			size = newSize;

			const CCoord switchSide = size.getWidth()/12.0f;
			rStoredSize = CRect(size.getBottomLeft(), CPoint(switchSide,switchSide));
			rStoredSize.offset(0.0, -switchSide);

			setDirty();
		}
	}

	void setMouseableArea (const CRect& rect) // called after setViewSize ()
	{ 
		if(bShowLarge)
			CView::setMouseableArea(rect); 
		else
			CView::setMouseableArea(rStoredSize);
	}

	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);

	bool getStateView() { return bShowLarge; }
	void setStateView(bool var) { bShowLarge = var;  setDirty(); }

	/// Custom message sent to objects that register themselves as dependencies.
	/// Sent when the user changes the state of the view.
	static IdStringPtr kStateViewChanged;

	CLASS_METHODS(CToolTip, CParamDisplay)

private:

	char display1[128];
	char display2[128];
	char display3[128];
	char display4[128];
	char display5[128];
	bool bDrawFrame;
	CHoriTxtAlign hAlign_xx;

	CColor kFrameCColor, kFontCColor2;

	CFontRef myFont;
	bool bShowLarge;
	CRect rStoredSize;
};
