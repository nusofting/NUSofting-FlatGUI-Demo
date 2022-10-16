//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2015
//****************************************************************************************************//

#include "FlatGUI/CKickTwiceZoom.h"

#undef max
#include <algorithm>

CKickTwiceZoom::CKickTwiceZoom(CRect& size, IControlListener* pListener, long tag) :
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

	//myFont = new CFontDesc ("Arial", 0,  0); // needs delete
}
CKickTwiceZoom::~CKickTwiceZoom()
{
	if(zone[0]){
		delete zone[0]; 
		zone[0] = 0;
	}

	if(zone[1]){
		delete zone[1]; 
		zone[1] = 0;
	}

	//if(myFont)
	//	myFont->forget();
}
void CKickTwiceZoom::draw(CDrawContext *pContext)
{
	if(rectOld != size)
	{
		*zone[0] = CRect(size.left,size.top,size.right-size.getWidth()/2,size.bottom);
		*zone[1] = CRect(size.left+size.getWidth()/2,size.top,size.right,size.bottom);			
	}

	rectOld = size;

	pContext->setDrawMode(kAntiAliasing | kNonIntegralMode);
	pContext->setLineWidth(1);
	pContext->setFrameColor(colorFrame);

	pContext->setFillColor((fEntryState1)?  colorBkground1 : colorBkground2);
	pContext->drawRect(*zone[0],kDrawFilledAndStroked);

	pContext->setFillColor((fEntryState2)?  colorBkground1 : colorBkground2);
	pContext->drawRect(*zone[1],kDrawFilledAndStroked);

	pContext->setLineWidth(std::max(1.0,0.09*size.getHeight()));
	
	drawMinusAndPlus(pContext, *zone[0], 1);
	drawMinusAndPlus(pContext, *zone[1], 2);

	setDirty(false);
}
void CKickTwiceZoom::drawMinusAndPlus(CDrawContext *pContext, CRect rect, int which)
{		
	colorFont.alpha = 255;
	pContext->setFrameColor(colorFont);

	CCoord width = rect.getWidth();
	CCoord height = rect.getHeight();
	CRect circle = rect;
	circle.setWidth(height);
	circle.offset(0.5*(width-height),0.0); // since width > height
	circle.inset(0.16*height,0.16*height);

	CCoord middleY = circle.getCenter().y;
	CCoord middleX = circle.getCenter().x-0.5;
	CPoint x1 = CPoint(circle.left+height*0.16-0.5, middleY);
	CPoint x2 = CPoint(circle.right-height*0.16-0.5,middleY);

	if(which == 1 || which == 2)
	{		 
		pContext->drawLine(x1, x2); // minus
	}
	if(which == 2)
	{
		CPoint y1 = CPoint(middleX, circle.top+height*0.14);
		CPoint y2 = CPoint(middleX, circle.bottom-height*0.14);
		pContext->drawLine(y1, y2);
	}

	colorFont.alpha = 128;
	pContext->setFrameColor(colorFont);
	pContext->drawEllipse(circle);

	CPoint yc = rect.getBottomRight();
	pContext->drawLine(yc.offset(-0.26*width,-0.26*width), rect.getBottomRight().offset(0.0,-0.1*width));	
}
CMouseEventResult CKickTwiceZoom::onMouseDown (CPoint& where, const CButtonState& buttons)
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
CMouseEventResult CKickTwiceZoom::onMouseUp (CPoint& where, const CButtonState& buttons)
{ 
	// value = oldValue;

	if(fEntryState1)
	{
		value -= step;
	}
	else if(fEntryState2)
	{
		value += step;
	}
	if(value < getMin()) value = getMin();
	else if (value > getMax()) value = getMax();

	if (listener)
		listener->valueChanged (this);

	// oldValue = value;

	
	fEntryState1 = false;
	fEntryState2 = false;
	endEdit (); // possible bug here
	setDirty();
	return  kMouseEventHandled;
}
CMouseEventResult CKickTwiceZoom::onMouseMoved (CPoint& where, const CButtonState& buttons)
{	
	if(fEntryState1 == true && !zone[0]->pointInside(where)  )
	{
		fEntryState1 = false;
		return onMouseUp (where, buttons);
	}
	if(fEntryState2 == true && !zone[1]->pointInside(where))
	{
		fEntryState2 = false;
		return onMouseUp (where, buttons);
	}

	if(isEditing())	endEdit ();

	setDirty();

	return  kMouseEventHandled;
} 
void CKickTwiceZoom::ColoursAndConfig(struct ConfigKick& configKick )
{
	colorBkground1 = configKick.colBkground1;
	colorBkground2 = configKick.colBkground2;
	colorFrame = configKick.colFrame;
	colorFont = configKick.colFont;
	setDirty();
}