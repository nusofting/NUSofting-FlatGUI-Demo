#pragma once
#include "vstgui/vstgui.h"
#include "FlatGUI/Config.h"

class CKickTwiceZoom : public CControl
{
public:
	CKickTwiceZoom(CRect& size, IControlListener* pListener, long tag);

	~CKickTwiceZoom();

	// Already provided by CControl: long getTag();

	void draw(CDrawContext *pContext);	 

	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);	  
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);	  
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);

	void setStepFix(float value)
	{
		step = value;
	}

	void ColoursAndConfig(struct ConfigKick& configKick );


	//void setDisplayFont(CFontRef asFont = 0);


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

	void drawMinusAndPlus(CDrawContext *pContext, CRect rect, int which);

}; // end class CKickTwiceZoom
