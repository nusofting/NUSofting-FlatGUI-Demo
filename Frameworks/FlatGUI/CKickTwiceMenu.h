#pragma once

#include "vstgui/vstgui.h"
#include "FlatGUI/Config.h"

class CKickTwiceMenu : public CControl
{
public:
	    CKickTwiceMenu(const CRect& size,
                 IControlListener* listener,
                 int32_t tag,
                 const int32_t style = 0);

	~CKickTwiceMenu();
	
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);	  
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons);
	void draw(CDrawContext *pContext);	
	void ColoursAndConfig(struct ConfigSlider& configSlider );
	void setDisplayFont(CFontRef asFont = 0);
	void setActive(bool var);
	void setFont(CFontRef asFont = 0) { setDisplayFont(asFont); }
	

	COptionMenu* thisMenu;

	// wrapping main COptionMenu methods
	virtual CMenuItem* addEntry (UTF8StringPtr title, int32_t index = -1, int32_t itemFlags = CMenuItem::kNoFlags);	///< add a new entry
	virtual int32_t getCurrentIndex (bool countSeparator = false) const;
	virtual CMenuItem* getEntry (int32_t index) const;															///< get entry at index position
	virtual int32_t getNbEntries () const;																		///< get number of entries
	virtual	bool setCurrent (int32_t index, bool countSeparator = true);										///< set current entry

	// overrides
	virtual void setValue (float val) VSTGUI_OVERRIDE_VMETHOD;
			float getValue ();

	CLASS_METHODS(CKickTwiceMenu, CControl)

private:

	CRect* zone[3];
	bool bMouseOn1,bMouseOn2;
	float step;
	CRect rectOld;
	CColor colorBkground1;
	CColor colorFrame; 
	CColor colorFont;
	CColor colFontInactive;
	CColor colHandle;
	CColor colFrameHandle;
	CColor colTrackBack;
	CColor colTrackFront;
	CCoord heightRatioButtons;
	CCoord heightRatioMenu;
	CRect menuSize;
	bool bActive;

	int32_t style_;
	int32_t indexMax;

	void drawPoints(CDrawContext *pContext, CRect rect, int which);

	void incDec(int posNeg)
	{
		beginEdit ();
		value = oldValue;
		int32_t index = int(value*indexMax+0.5f) + posNeg;
		index = (index < 0)? indexMax : index > indexMax ? 0 : index;
		value = index/float(indexMax);	
		oldValue = value;
		if(listener) listener->valueChanged(this);
		thisMenu->setCurrent(index);
		thisMenu->setDirty();
		endEdit ();
		setDirty();
	}

}; // end class CKickTwiceMenu
