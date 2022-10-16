//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2020
//****************************************************************************************************//

#pragma warning( disable : 4244)// 'initializing' : conversion from 'VSTGUI::CCoord' to 'const int', possible loss of data

#pragma once

#include "FlatGUI/Config.h"

#include "vstgui/vstgui.h"


//------------------------------------------------------------------------------
class CKSDisplay : public CControl
{
public:
	CKSDisplay (const CRect &size, IControlListener* listener, int32_t tag, const ConfigGraphView& configGraphView);
	void draw (CDrawContext *pContext);
	void setCurve(float fCenterNoteParm, float fKeytrkParm, int mode);

	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);	  
	virtual CMouseEventResult onMouseEntered (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons);

	void setViewSizeExt (const CRect& newSize)
	{
		if (size != newSize)
		{
			invalid ();
			CRect oldSize = size;
			size = newSize;

			setMouseableArea(size);

			setDirty ();
		}
		setVisible(true);
	}

	CLASS_METHODS(CKSDisplay, CControl)

private:
	static const int noteDisplayed = 100; // with +20 offset from MIDI note zero
	float levels[128];
	float fCenterNoteParm_;
	int XS;
	const ConfigGraphView& m_configGraphView;
	bool bMouseOn;
};