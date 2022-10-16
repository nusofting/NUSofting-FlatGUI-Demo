#include "CStereoWavsView.h"

CStereoWavsView::CStereoWavsView(const CRect& size, const ConfigGraphView& configGraphView, IControlListener* listener, long tag, VstInt32 sizeDataFromEffect)
:CControl (size, listener, tag),
m_configGraphView(configGraphView)
{
	waveArray = 0;		
	if(sizeDataFromEffect) waveArray = new float[sizeDataFromEffect];
	atSample = 0;
	sizeData = sizeDataFromEffect;
	scale = 0.5f; 
	rangeHandle = 0;
	bMouseOn = false;
	nextY = 0;
	oldPos = 0;
	ia = ib = 0;

	if(waveArray)
		for(int j = 0; j < sizeData; j++)
		{
			waveArray[j] = 2.0f*j/float(sizeData-1) - 1.0f;
		}
}

CStereoWavsView::~CStereoWavsView()
{
	delete[] waveArray;
}

CMouseEventResult CStereoWavsView::onMouseDown (CPoint& where, const CButtonState& buttons)
{	
	if (!(buttons & kLButton))
		return kMouseEventNotHandled;

	if(buttons.isLeftButton() && rSlider.pointInside(where))
	{
		bMouseOn = true;
		return  onMouseMoved(where,buttons);
	}
	return kMouseEventNotHandled;
}

CMouseEventResult CStereoWavsView::onMouseMoved (CPoint& where, const CButtonState& buttons)
{
	if(!buttons.getButtonState()){
		if(rSlider.pointInside(where)) // for mouseover
		{
			bMouseOn = true;
			setDirty();
		}
	}

	if (buttons & kLButton)
	{
		scale = (float)(where.x - rSlider.left) / (float)rangeHandle;
		Clamp(scale);

	}
	return kMouseEventHandled;
}

CMouseEventResult CStereoWavsView::onMouseExited (CPoint& where, const CButtonState& buttons)
{
	bMouseOn = false; // for mouseover
	setDirty();

	return kMouseEventHandled;
}

//virtual CMouseEventResult CStereoWavsView::onMouseUp (CPoint& where, const long& buttons)
//{	
//	bMouseOn = false; // for mouseover
//	setDirty();

//	return kMouseEventHandled;
//}
