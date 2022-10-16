#pragma once
#include "vstgui/vstgui.h"
#include "vstgui/lib/platform/iplatformbitmap.h"
#include "FlatGUI/Config.h"

//-----------------------------------------------------------------------------
// CMovieBitmapExt Declaration
//! @brief a bitmap view that displays different bitmaps according to its current value
/// @ingroup views
//-----------------------------------------------------------------------------
class CMovieBitmapExt : public CControl, public IMultiBitmapControl
{
public:
	CMovieBitmapExt (const CRect& size, IControlListener* listener, int32_t tag, int32_t subPixmaps, CCoord heightOfOneImage, CBitmap* background, ConfigSlider sliderColours_, const CPoint& offset_ = CPoint (0, 0));

	virtual void draw (CDrawContext*) VSTGUI_OVERRIDE_VMETHOD;
	virtual void setValue (float value) VSTGUI_OVERRIDE_VMETHOD 
	{
		CControl::setValue(value); 

		int32_t indexMax = getNumSubPixmaps() - 1;
		curKey = int32_t(value*indexMax + 0.5f); 
		curKey = curKey < 0 ? 0 : curKey > indexMax ? indexMax : curKey;		

		invalid ();
		
	}

	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons) VSTGUI_OVERRIDE_VMETHOD;
	void setViewSize(CRect& newSize);

	void ColoursAndConfig(struct ConfigSlider& configSlider);

	void setNumSubPixmaps (int32_t numSubPixmaps) VSTGUI_OVERRIDE_VMETHOD { IMultiBitmapControl::setNumSubPixmaps (numSubPixmaps); invalid (); }

	CLASS_METHODS(CMovieBitmapExt, CControl)
protected:
	~CMovieBitmapExt ();
	CPoint	offset;
	CBitmap* clipsScaled;
	CBitmap* thisBk;
	CColor bkCol;
	CColor fmCol;
	CColor wvCol;
	ConfigSlider sliderColours;
	CRect oldSize;
	int32_t curKey;

	//void texturePixels(const CRect& rect, const CPoint& offset = CPoint(0.0,0.0))
	//{	
	//	CColor cBackground = kBlueCColor;
	//	int32_t  width = int32_t(rect.getWidth()+0.5);
	//	int32_t  height = int32_t(rect.getHeight()+0.5);

	//	IPlatformBitmapPixelAccess* cBMPpx = clipsScaled->getPlatformBitmap()->lockPixels(false);
	//	if(cBMPpx)
	//	{
	//		uint8_t* pPxLine = cBMPpx->getAddress(); // temporary buffer
	//		int32_t iPxStride = cBMPpx->getBytesPerRow(); // iPxStride/4 = width // colours kBGRA	
	//		const int32_t bytes = iPxStride * height;
	//		int32_t line = 0;

	//		float x = 0.0f;

	//		for (int32_t i = 0; i < bytes - 4; i += 4) // all pxs
	//		{	
	//			if(i%4 == 0) x++; 				

	//			if(i > line*iPxStride-4)
	//			{
	//				line++; // next line
	//				x = 0.0f;
	//			}

	//			reverse(pPxLine[i]);    // B
	//			reverse(pPxLine[i+1]);  // G
	//			reverse(pPxLine[i+2]);  // R
	//			pPxLine[i+3] = 255;     // A
	//		}

	//		cBMPpx->forget(); // Unlock the bits. // temporary buffer copied back to bitmap
	//	}//if(cBMPpx)
	//}

	//void reverse(uint8_t&  byteCol)
	//{
	//	byteCol = byteCol > 128? byteCol - 127: byteCol + 127;
	//}

	double makeScaleFactor(CRect& curSize) const
	{
		const CCoord bw = thisBk->getWidth();
		return  1.11*bw/curSize.getWidth(); // for VSTGUI fix
	}
};