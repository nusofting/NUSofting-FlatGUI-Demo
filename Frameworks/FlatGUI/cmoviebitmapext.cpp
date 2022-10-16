#include "cmoviebitmapext.h"


//------------------------------------------------------------------------
// CMovieBitmapExt
//------------------------------------------------------------------------
/**
* CMovieBitmapExt constructor.
* @param size the size of this view
* @param listener the listener
* @param tag the control tag
* @param background bitmap
* @param offset
*/
//------------------------------------------------------------------------
CMovieBitmapExt::CMovieBitmapExt (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, 
								  CBitmap* background, ConfigSlider sliderColours_, const CPoint &offset_)
								  : CControl (size, listener, tag, background), sliderColours(sliderColours_), offset (offset_), thisBk(background)
{

	bkCol = CColor(115,115,115,255);
	fmCol = CColor(0,0,0,255);
	wvCol = CColor(255,255,255,255);

	setNumSubPixmaps (subPixmaps);
	setHeightOfOneImage (heightOfOneImage); // as in PNG

	oldSize = size;
	curKey = 0;
	clipsScaled = 0;	

	if(thisBk)
	{
		const double scaleFactor = makeScaleFactor(oldSize);

		IPlatformBitmap* tempBMP = thisBk->getPlatformBitmap();
		if(tempBMP)
		{
			tempBMP->setScaleFactor(scaleFactor); 

			clipsScaled = new CBitmap(tempBMP);

			setHeightOfOneImage(clipsScaled->getHeight());
		}
		else
			clipsScaled = thisBk; // never happens?
	}
	else
	{
		thisBk = 0; clipsScaled = 0;
	}	

}
//------------------------------------------------------------------------
CMovieBitmapExt::~CMovieBitmapExt ()
{
	if(clipsScaled) { clipsScaled->forget(); clipsScaled = 0;}
}
//------------------------------------------------------------------------
void CMovieBitmapExt::draw (CDrawContext *pContext)
{
	if(clipsScaled)
	{	
		CPoint where (offset.x, offset.y);

		const double n = double(getNumSubPixmaps());	
		where.y += (heightOfOneImage/n) * curKey;

		pContext->drawBitmap (clipsScaled, getViewSize(), where, 0.5f);		
	}

	setDirty (false);
}
//------------------------------------------------------------------------
CMouseEventResult CMovieBitmapExt::onMouseDown (CPoint& where, const CButtonState& buttons)
{
	if (buttons & kLButton | kRButton)
	{   
		int32_t indexMax = getNumSubPixmaps() - 1;
		curKey = int32_t(value*indexMax + 0.5f); 
		if (buttons & kLButton) ++curKey; else --curKey;
		// wrap
		curKey = curKey < 0 ? indexMax : curKey > indexMax ? 0 : curKey;

		value = float(curKey)/float(indexMax);

		bounceValue();
		invalid ();
		valueChanged ();

		return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
	}
	return kMouseEventNotHandled;
}
//------------------------------------------------------------------------
void CMovieBitmapExt::ColoursAndConfig(struct ConfigSlider& configSlider)
{
	return;
/*
	SharedPointer<BitmapFilter::IFilter> setColorFilter = owned (BitmapFilter::Factory::getInstance().createFilter(BitmapFilter::Standard::kReplaceColor));
	if (setColorFilter)
	{
		setColorFilter->setProperty (BitmapFilter::Standard::Property::kInputBitmap, clipsScaled);
		setColorFilter->setProperty (BitmapFilter::Standard::Property::kInputColor,  wvCol);				 
		setColorFilter->setProperty (BitmapFilter::Standard::Property::kOutputColor,  sliderColours.colFont);
		wvCol = sliderColours.colFont;
		setColorFilter->setProperty (BitmapFilter::Standard::Property::kOutputBitmap, clipsScaled);
		setColorFilter->run (true);
		setColorFilter->setProperty (BitmapFilter::Standard::Property::kInputColor,  bkCol);				 
		setColorFilter->setProperty (BitmapFilter::Standard::Property::kOutputColor,  sliderColours.colTrackBack);
		bkCol = sliderColours.colTrackBack;
		setColorFilter->setProperty (BitmapFilter::Standard::Property::kOutputBitmap, clipsScaled);
		setColorFilter->run (true);
		setColorFilter->setProperty (BitmapFilter::Standard::Property::kInputColor,  fmCol);				 
		setColorFilter->setProperty (BitmapFilter::Standard::Property::kOutputColor,  sliderColours.colHandle);
		fmCol = sliderColours.colHandle;
		setColorFilter->setProperty (BitmapFilter::Standard::Property::kOutputBitmap, clipsScaled);
		setColorFilter->run (true);
	}
	//SharedPointer<BitmapFilter::IFilter> setBlurFilter = owned (BitmapFilter::Factory::getInstance().createFilter(BitmapFilter::Standard::kBoxBlur));
	//if (setColorFilter)
	//{
	//	setBlurFilter->setProperty (BitmapFilter::Standard::Property::kInputBitmap, clipsScaled);
	//	setBlurFilter->setProperty (BitmapFilter::Standard::Property::kRadius, 2);
	//	setBlurFilter->run (true);
	//}	

	setDirty();
	*/
}
//------------------------------------------------------------------------
void CMovieBitmapExt::setViewSize(CRect& newSize)
{	
	oldSize = newSize;		

	//if(clipsScaled)
	//{	
	//	const double scaleFactor = makeScaleFactor(newSize);
	//	IPlatformBitmap* tempBMP = clipsScaled->getPlatformBitmap();

	//	if(tempBMP)
	//	{
	//		tempBMP->setScaleFactor(scaleFactor); 
	//		clipsScaled = new CBitmap(tempBMP);

	//		//setHeightOfOneImage(clipsScaled->getHeight());
	//	}
	//}

	CControl::setViewSize(newSize);
	invalid();	
}