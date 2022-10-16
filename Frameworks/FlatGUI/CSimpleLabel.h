#pragma once
#include "vstgui/vstgui.h"
#include "FlatGUI/Config.h"

#undef min
#undef max
#include <algorithm>

class CPopup : public CView
{
public:

	CPopup(const CRect& size): CView (size)
	{ }

	void setDisplay(std::string& string) { display = string; };

	void draw (CDrawContext *pContext)
	{			
		pContext->setFontColor(kBlackCColor);
		pContext->setFillColor(kWhiteCColor);
		pContext->drawRect(size, kDrawFilled);
		pContext->drawString(display.c_str(), size);

		setDirty(false);
	}

	CLASS_METHODS(CPopup, CView)

private:

	std::string display;
};

class CSimpleLabel : public CParamDisplay
{
public:

	CSimpleLabel(const CRect& size, bool bMakeFrame = false, const CHoriTxtAlign hAlign = kCenterText);
	~CSimpleLabel();

	void setString128(const char* string);
	void draw (CDrawContext *pContext);
	void ColoursAndConfig(struct ConfigLabel& configLabel, uint8_t alpha = 255);
	void setFontColourOnly(const CColor& colFontsin);
	void setDisplayFont(CFontDesc* scaledFontFactory);
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseEntered (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons);
	void setValint(int val);

	void usePopUp(bool var) { bPopUpMode = var;}

	CLASS_METHODS(CSimpleLabel, CParamDisplay)

private:

	std::string display;
	bool bDrawFrame;
	CHoriTxtAlign hAlign_xx;
	CColor colorBkground1; // currently unused in CSimpleLabel
	CColor colorFrame; // used if bDrawFrame
	CColor colorFont;

	CFontDesc* m_scaledFontFactory;

	bool bflag;
	bool bMouseOn;
	bool bPopUpMode;
	int prevValint;

	CPopup* addWindow_;

}; // end class CSimpleLabel 
