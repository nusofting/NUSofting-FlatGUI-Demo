#include "FlatGUI/CDisplayBitmap.h"

CDisplayBitmap::CDisplayBitmap(const CRect& size, IControlListener* listener, long tag, CBitmap* background, CBitmap* image)
	:CControl (size, listener, tag, background)
{
	bMouseOn = false;
	bShowOn = false;

	osc = 0.0f;

	if(image)  
	{
		image_ = image;
	}	
	else
	{
		image_ = 0;
	}

	addWindow_ = 0;
}

CDisplayBitmap::~CDisplayBitmap()
{
	 image_ = 0; //?
	 addWindow_ = 0;
}

CMouseEventResult CDisplayBitmap::onMouseDown (CPoint& where, const CButtonState& buttons)
{	
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	if(buttons.isLeftButton() && getViewSize().pointInside(where))
	{
		bMouseOn = !bMouseOn;

		if(bMouseOn)
		{
			beginEdit();

			if(image_)
			{
				CRect sizeDisplay(0.0, 0.0, image_->getWidth(), image_->getHeight());
				sizeDisplay.offset(size.right, size.top*0.23);								
				addWindow_ = new CView(sizeDisplay);
				addWindow_->setBackground(image_);
				getFrame()->addView(addWindow_);
			}
		}
		else
		{
			endEdit();
			if(addWindow_) getFrame()->removeView(addWindow_);
		}	

		setDirty();
		return  kMouseMoveEventHandledButDontNeedMoreEvents;
	}
    
    return kMouseEventNotHandled;
}

//------------------------------------------------------------------------
CMouseEventResult CDisplayBitmap::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if (isEditing())
	{

		return kMouseMoveEventHandledButDontNeedMoreEvents;		
	}
	
	return kMouseEventNotHandled;
}
//------------------------------------------------------------------------
void CDisplayBitmap::doResize(CBitmap* imageZoomed)
{		
	if(imageZoomed)
	{
		this->image_->forget();
		image_ = imageZoomed;

		CPoint where = getViewSize().getCenter();
		if(bMouseOn) onMouseDown (where, kLButton);

		this->invalid ();
	}	
}
