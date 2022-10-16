//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2019
//****************************************************************************************************//

#include "CKickTwiceMenu.h"

#undef max
#include <algorithm>

CKickTwiceMenu::CKickTwiceMenu(const CRect& size,
							   IControlListener* listener,
							   int32_t tag,
							   const int32_t style)
							   :CControl (size, listener, tag, 0),
							   style_(style)
							 
{
	/*
	v menu active in the middle zone[2]
	|¯¯¯|¯¯|¯¯¯|   3/3
	|   |__|   |
	|___|  |___|
	^      ^  buttons zone[0], zone[1] 


	*/

	heightRatioButtons = 1.2/4.0;
	heightRatioMenu = 1.0 - heightRatioButtons;

	const CCoord fullWidth = size.getWidth();
	const CCoord fullHeight = size.getHeight();
	zone[0] = new CRect(0.0, 0.0, (1.0/3.0)*fullWidth, fullHeight);
	zone[0]->moveTo(size.getTopLeft());
	zone[1] = new CRect(*zone[0]);
	zone[1]->moveTo(size.getTopLeft().offset((2.0/3.0)*fullWidth, 0.0));

	zone[2] = new CRect((1.0/3.0)*fullWidth, 0.0, (2.0/3.0)*fullWidth, fullHeight);
	zone[2]->moveTo(size.getTopLeft().offset((1.0/3.0)*fullWidth, 0.0));

	menuSize = size;
	menuSize.setHeight(heightRatioMenu*size.getHeight());
	menuSize.setWidth(size.getWidth());

	thisMenu = new COptionMenu(menuSize, nullptr, -1, 0, 0, this->style_);

	if(thisMenu)
	{
		thisMenu->setMouseableArea(*zone[2]);

		CFontRef fontID = thisMenu->getFont();
		if(fontID){
			fontID->setSize(0.55*menuSize.getHeight());
			fontID->setStyle(CTxtFace::kNormalFace);
			thisMenu->setFont(fontID);
		}
		thisMenu->setWantsFocus(true);
		thisMenu->setMouseableArea(menuSize); // seems useless
		//thisMenu->attached(this);  // not working
		//addDependency(thisMenu); // needed? No, it seems
		thisMenu->setWantsIdle(true);
	} 

	bMouseOn1=bMouseOn2 =false;
	step = 1.0f;
	rectOld = size;

	ConfigSlider configSlider;
	ColoursAndConfig(configSlider);

	setWantsFocus(true);
	setWantsIdle(true);

	bActive = true;
}

CKickTwiceMenu::~CKickTwiceMenu()
{
	if(zone[0]){
		delete zone[0]; 
		zone[0] = 0;
	}

	if(zone[1]){
		delete zone[1]; 
		zone[1] = 0;
	}

	if(zone[2]){
		delete zone[2]; 
		zone[2] = 0;
	}

	if(thisMenu)
		thisMenu->forget(); // needed?
}

void CKickTwiceMenu::draw(CDrawContext *pContext)
{
	if(!thisMenu) return;

	if(rectOld != size)
	{
		const CCoord fullWidth = size.getWidth();
		const CCoord fullHeight = size.getHeight();
		zone[0] = new CRect(0.0, 0.0, (1.0/3.0)*fullWidth, fullHeight);
		zone[0]->moveTo(size.getTopLeft());
		zone[1] = new CRect(*zone[0]);
		zone[1]->moveTo(size.getTopLeft().offset((2.0/3.0)*fullWidth, 0.0));

		zone[2] = new CRect((1.0/3.0)*fullWidth, 0.0, (2.0/3.0)*fullWidth, fullHeight);
		zone[2]->moveTo(size.getTopLeft().offset((1.0/3.0)*fullWidth, 0.0));

	    menuSize = size;
		menuSize.setHeight(heightRatioMenu*size.getHeight());
		menuSize.setWidth(size.getWidth());
		thisMenu->setViewSize(menuSize);
		CFontRef fontID = thisMenu->getFont();
		fontID->setSize(0.55*menuSize.getHeight());
		thisMenu->setFont(fontID);
		thisMenu->setMouseableArea(*zone[2]);
	}

	rectOld = size;	

	if(bActive)
	{
		colTrackBack.alpha = 163;
		colorBkground1.alpha = 255;
		colorFont.alpha = 255;
		thisMenu->setBackColor(colTrackBack);
		thisMenu->setFontColor(colorFont);
	}
	else
	{
		colTrackBack.alpha = 100;
		colorBkground1.alpha = 100;
		colorFont.alpha = 100;
		thisMenu->setBackColor(colTrackBack);
		thisMenu->setFontColor(colorFont);
	}	

	thisMenu->setFrameColor(kTransparentCColor);

	pContext->setFillColor(bMouseOn1? colFrameHandle : colorBkground1);
	pContext->drawRect(*zone[0],kDrawFilled);

	pContext->setFillColor(bMouseOn2? colFrameHandle : colorBkground1);
	pContext->drawRect(*zone[1],kDrawFilled);

	
	pContext->setDrawMode(kAntiAliasing);
	pContext->setLineWidth(std::max(1.0,0.1*zone[1]->getHeight()));
	pContext->setFrameColor(bMouseOn1? colorFont : colFrameHandle);
	drawPoints(pContext, *zone[0], 1);
	pContext->setFrameColor(bMouseOn2? colorFont : colFrameHandle);
	drawPoints(pContext, *zone[1], 2);

	thisMenu->draw(pContext);

	const CCoord swh = zone[2]->getWidth();
	const CCoord sht = zone[2]->getHeight();
	const CCoord gap = sht/9.0;

	CRect side = CRect(0,0, swh, sht*heightRatioButtons);
	side.moveTo(zone[2]->getTopLeft().offset(0, sht*heightRatioMenu));
	side.inset(2.0, sht/8.0); side.makeIntegral();
	pContext->setFillColor(colorFrame);
	side.offset(0, -gap);	side.makeIntegral();	
	pContext->drawRect(side, kDrawFilled);
	side.offset(0, gap); side.makeIntegral();
	pContext->drawRect(side, kDrawFilled);
	side.offset(0, gap); side.makeIntegral();
	pContext->drawRect(side, kDrawFilled);
	setDirty(false);
}

void CKickTwiceMenu::drawPoints(CDrawContext *pContext, CRect rect, int which)
{	
	CCoord shift = 0.009;
	CCoord width = getFrame()->getWidth(); //  why this is needed to work??? Mistery
	CCoord height = rect.getHeight();
	if(which == 1)
	{	
		CPoint x1 = CPoint(rect.left+width*shift, rect.top+height*0.2); 
		CPoint x2 = CPoint(rect.left+width*0.005, rect.top+height*0.5);	
		CPoint x3 = CPoint(rect.left+width*shift, rect.top+height*0.8);

		for(int i = 0; i < 3; ++i)
		{
			pContext->drawLine(x1, x2);
			pContext->drawLine(x2, x3);
			x1.offset(width*shift,0);
			x2.offset(width*shift,0);
			x3.offset(width*shift,0);
		}
	}
	if(which == 2)
	{
		CPoint x1 = CPoint(rect.right-width*shift, rect.top+height*0.2); 
		CPoint x2 = CPoint(rect.right-width*0.005, rect.top+height*0.5);	
		CPoint x3 = CPoint(rect.right-width*shift, rect.top+height*0.8);

		for(int i = 0; i < 3; ++i)
		{
			pContext->drawLine(x1, x2);
			pContext->drawLine(x2, x3);
			x1.offset(-width*shift,0);
			x2.offset(-width*shift,0);
			x3.offset(-width*shift,0);
		}
	}
}
// wrapping main COptionMenu methods
CMenuItem* CKickTwiceMenu::addEntry (UTF8StringPtr title, int32_t index, int32_t itemFlags)
{
	return thisMenu->addEntry (title,index, itemFlags);
}
int32_t CKickTwiceMenu::getCurrentIndex (bool countSeparator) const
{
	return thisMenu->getCurrentIndex (countSeparator);
}
CMenuItem* CKickTwiceMenu::getEntry (int32_t index) const
{
	return thisMenu->getEntry (index);
}
int32_t CKickTwiceMenu::getNbEntries () const
{
	return thisMenu->getNbEntries (); // max index + 1 == size
}
bool CKickTwiceMenu::setCurrent (int32_t index, bool countSeparator)
{
	return thisMenu->setCurrent (index, countSeparator);
}
void CKickTwiceMenu::setValue (float val)
{
	indexMax = thisMenu->getNbEntries() - 1;
	thisMenu->setValue (float(int(val*indexMax+0.5f)));
	thisMenu->setDirty();
	CControl::setValue(val);
}
float CKickTwiceMenu::getValue ()
{
	value = CControl::getValue(); // ?
	indexMax = thisMenu->getNbEntries() - 1;
	return  float(thisMenu->getCurrentIndex())/float(indexMax);
}

CMouseEventResult CKickTwiceMenu::onMouseDown (CPoint& where, const CButtonState& buttons)
{	
	CMouseEventResult result = kMouseEventNotHandled;
	if(!thisMenu)  { return result; }
	indexMax = thisMenu->getNbEntries() - 1;

	if (buttons & (kLButton|kRButton|kApple))
	{
		CRect menuSize = size;
		menuSize.setHeight(heightRatioMenu*size.getHeight());
		if (zone[2]->pointInside(where)) // case menu:
		{
			getFrame ()->setFocusView (thisMenu);
			getFrame ()->setModalView(thisMenu);
			if (thisMenu->popup())
			{	
				value = thisMenu->getCurrentIndex()/float(indexMax);
				value = (value < 0.0f)? 0.0f : value > 1.0 ? 1.0f : value;
				beginEdit ();
				if(listener) listener->valueChanged(this);
				CControl::setValue(value);
				endEdit ();
				setDirty();
				

				result = kMouseDownEventHandledButDontNeedMovedOrUpEvents;	
			}	
			getFrame ()->setModalView(0);
		}
		else if(zone[0]->pointInside(where)) // case buttons:
		{			
			incDec(-1);
			result = kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}
		else if(zone[1]->pointInside(where)) // case buttons:
		{
			incDec(1);
			result = kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}
	}//if (buttons & (kLButton|kRButton|kApple)
	return result;
}

CMouseEventResult CKickTwiceMenu::onMouseMoved (CPoint& where, const CButtonState& buttons)
{	
	if(zone[0]->pointInside(where))
	{
		bMouseOn1 = true;
		bMouseOn2 = false;
		setDirty();
		return  kMouseMoveEventHandledButDontNeedMoreEvents;
	}
	else if(zone[1]->pointInside(where))
	{
		bMouseOn2 = true;
		bMouseOn1 = false;
		setDirty();
		return  kMouseMoveEventHandledButDontNeedMoreEvents;
	}		
	return  kMouseEventNotHandled;
} 

CMouseEventResult CKickTwiceMenu::onMouseExited (CPoint& where, const CButtonState& buttons)
{
	bMouseOn1 = false;
	bMouseOn2 = false;

	setDirty();
	return  kMouseMoveEventHandledButDontNeedMoreEvents;
}

void CKickTwiceMenu::ColoursAndConfig(struct ConfigSlider& configSlider )
{
	colorBkground1 = configSlider.colBkground1;
	colHandle = configSlider.colHandle;
	colorFrame = configSlider.colFrame;
	colFontInactive = configSlider.colFontInactive;
	colorFont = configSlider.colFont;
	colFrameHandle = configSlider.colFrameHandle;
	colTrackBack = configSlider.colTrackBack;
	colTrackFront = configSlider.colTrackFront;

	

	setDirty();
}

void CKickTwiceMenu::setDisplayFont(CFontRef asFont)
{
	if(thisMenu) thisMenu->setFont(asFont); 
}
void CKickTwiceMenu::setActive(bool var)
{
	bActive = var;

	setDirty();
}
