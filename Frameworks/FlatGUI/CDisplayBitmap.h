#pragma once
#include "vstgui/vstgui.h"
#include "Config.h"

#ifndef VstInt32
typedef int VstInt32;	//< 32 bit integer type
#endif

class CDisplayBitmap : public CControl
{
public:
	CDisplayBitmap(const CRect& size, IControlListener* listener, long tag, CBitmap* background, CBitmap* image);
	~CDisplayBitmap();

	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);

	void idleRun()
	{
		if(osc > 0.0f)
		{
		 osc -= 0.01f;
		}
		else
			osc = 1.0f;

		setDirty();
	}

	void draw (CDrawContext *pContext)
	{
		CRect size = getViewSize ();

		if (getDrawBackground ())
		{
			getDrawBackground ()->draw (pContext, size, CPoint(0.0, 0.0), bMouseOn? osc : 0.9f);
		}

		setDirty(false);
	}

	void doResize(CBitmap* imageZoomed = 0);

	CLASS_METHODS(CDisplayBitmap, CControl)

private:
	CBitmap* image_;
	CView* addWindow_;
	//CRect sizeDisplay;
	bool bMouseOn;
	bool bShowOn;
	float osc;

}; // end class CStereoWavsView
