#include "CToolTip.h"

CToolTip::CToolTip(const CRect& size, bool bMakeFrame = false, const CHoriTxtAlign hAlign = kCenterText)
:CParamDisplay (size)
{
	hAlign_xx = hAlign;
	bDrawFrame = bMakeFrame;
	memset(display1,0,sizeof(display1));
	memset(display2,0,sizeof(display2));
	memset(display3,0,sizeof(display3));
	memset(display4,0,sizeof(display4));
	memset(display5,0,sizeof(display5));

	kFrameCColor (127, 127, 201, 255); 
	kFontCColor2 (255, 201, 100, 255);

	myFont = 0 ; //fontID->remember ();

	//setDisplayFont(); // needed to initialize  myFont 

	setTag(-1);

	bShowLarge = true;

	const CCoord switchSide = size.getWidth()/12.0f;
	rStoredSize = CRect(size.getBottomLeft(), CPoint(switchSide,switchSide));
	rStoredSize.offset(0.0, -switchSide);
};

CToolTip::~CToolTip ()
{
	if(myFont)
		myFont->forget();	
}

CMouseEventResult CToolTip::onMouseDown (CPoint& where, const CButtonState& buttons)
{	
	if(!bDrawFrame)
		return kMouseEventNotHandled;

	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	if(buttons.isLeftButton()&& size.pointInside(where))
	{
		if(bShowLarge )
		{ bShowLarge = false; CView::setMouseableArea(rStoredSize); }
		else
		{ bShowLarge = true; CView::setMouseableArea(size); }

		setDirty();
		changed(kStateViewChanged);
		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	return kMouseEventNotHandled;
}

void CToolTip::setDisplayFont(CFontRef asFont, const char* fontName)
{
	if (myFont)
		myFont->forget ();

	if(asFont)
	{
		myFont = asFont; myFont->remember ();
		CParamDisplay::setFont(asFont); 
	}
	else
	{
		myFont = new CFontDesc (fontName, 0,  0); // needs delete
		myFont->remember ();

		drawStyleChanged (); // same as setDirty (); !!
	}
}

IdStringPtr CToolTip::kStateViewChanged = "kStateViewChanged";
