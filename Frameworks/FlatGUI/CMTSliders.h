//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2020
//****************************************************************************************************//

#pragma warning( disable : 4244)// 'initializing' : conversion from 'VSTGUI::CCoord' to 'const int', possible loss of data

#pragma once

#include "FlatGUI/Config.h"
#include "FlatGUI/CSliderFlat.h"

#include "vstgui/vstgui.h"

#ifndef pureTuningCentsOffset
static const int pureTuningCentsOffset[] = {
		0,  // C
		12, // C#
		4,  // D
		-6,  // D# Pythagorean
		-14, // E
		-2, // F
		-10, //F#
		2, // G
		14,  // G#
		-16, // A
		17,	// A# or -2 both Shruti , or -4 Pythagorean
		-12 // B
};
#endif

#ifndef MTlabels
static const char* MTlabels[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#", "B"};
#endif
//------------------------------------------------------------------------------
class CMTSliders : public CView
{
public:
	CMTSliders (const CRect &size, IControlListener* listener, const int32_t tags[12], 
		ScaledFontFactory& fontFactory,  const ConfigSlider& configSlider);
	~CMTSliders()
	{
		if(pSlider) 
		{
			pSlider->forget(); // needed? Since it's attched to "this".
			pSlider = 0; 
		}
	}
	void draw (CDrawContext *pContext);

	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);	
	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);	
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);

	void setValueFromParamMT12(int kParam, float& value)
	{

		centsParam[kParam%12] = value;
		setDirty();
	}
	float getValueFromObjMT12(int kParam)
	{
		return centsParam[kParam%12];
	}
	CControl* getActiveCtrl()
	{
		return pSlider; // the param tag will be set by onMouseDown()
	}
	void setViewSize(CRect& newSize)
	{
		CView::setViewSize(newSize);

		size = newSize;
		w = newSize.getWidth();
		h = newSize.getHeight();
		ratio = 0.00235*w; // for zoom	

		setDirty();
	}

	void setVisible(bool state)
	{
		CView::setVisible(state);

		setDirty();
	}

	void setString32(char* string);

	CLASS_METHODS(CMTSliders , CView)

private:
	static const int noteDisplayed = 12; 
	float centsParam[noteDisplayed];

	CSliderFlat* pSlider;
	CRect rSlider[noteDisplayed];
	CRect rDisplay[noteDisplayed];
	int32_t cents[noteDisplayed]; // display
	int32_t tags_[noteDisplayed];
	int32_t XS;
	int tagEdit;
	const ConfigSlider& m_configSlider;
	bool bMouseOn;

	CRect rShift;
	CRect rPTuning;

	CCoord w;
	CCoord h;
	CCoord ratio;
	CFontDesc* myFont;

	//draw grid and boxes
	void drawBackground(CDrawContext* drawContext)
	{
		drawContext->setDrawMode(kAliasing);
		drawContext->setLineWidth(1.0f);
		drawContext->setFrameColor(m_configSlider.colFrame);
		drawContext->setFillColor(m_configSlider.colBkground1);
		drawContext->setFontColor(m_configSlider.colFont);
		drawContext->setFont(drawContext->getFont(), 29.0*ratio);

		const CCoord displayH = h/6.5;
		const CCoord displayW = w/6.5;

		const CCoord sliderW = 0.4*displayW;
		const CCoord sliderH = 2.5*displayH;

		CCoord shiftX = 0.0;
		CCoord shiftY = 0.0;
		int iEven = 0;
		int iOdd = 0;
		
		// first draw the upper five boxes
		for(int j = 0; j < noteDisplayed; ++j)
		{

			drawContext->setFillColor(m_configSlider.colBkground1);

			rDisplay[j](0.0, 0.0, displayW, displayH);
	
			if(j % 2 == 0) // row even 0 2 4 ... 10
			{	
				shiftX = fabs(iEven*displayW)-1.0;
				shiftY = h-displayH;
				++iEven;
			}
			else  // row odd 1 3 5 ... 11
			{ 
				shiftX = fabs(iOdd*displayW + displayW*0.5) - 1.0;
				shiftY = h-displayH*2.0;
				++iOdd;
			}

			rDisplay[j].offset(size.getTopLeft().offset(1.0f + shiftX, shiftY));
			drawContext->drawRect(rDisplay[j], kDrawFilledAndStroked);

			// draw digits
			cents[j] = static_cast<int>(200.0f*centsParam[j]-100.0f);
			std::string s = std::to_string(cents[j]);			
			drawContext->drawString(s.c_str(), rDisplay[j]);

			// draw tracks
			rSlider[j](0.0,0.0, sliderW, sliderH);
			rSlider[j].offset(rDisplay[j].getCenter().x-0.5*sliderW, size.bottom - displayH*2.0 - sliderH);
			 drawContext->setFillColor(cents[j] == 0 ? m_configSlider.colTrackFront :m_configSlider.colBkground1);	
			drawContext->drawRect(rSlider[j], kDrawFilledAndStroked);
		}
	
		// draw zero line
		drawContext->setFrameColor(m_configSlider.colFrame);
		CPoint XY1_(rSlider[0].left,rSlider[0].getCenter().y);
		CPoint XY2_(rSlider[11].right,rSlider[11].getCenter().y);
		drawContext->drawLine(XY1_,XY2_);
		
		//draw sliders
		drawContext->setDrawMode(kAntiAliasing);
		drawContext->setFrameColor(m_configSlider.colHandle);
		for(int j = 0; j < noteDisplayed; ++j)
		{
			drawContext->setLineWidth(centsParam[j] == 0.5f? 0.5f+ratio : 2.0f+ratio);
			CPoint XY1_(rSlider[j].left+1.0,sliderH-sliderH*centsParam[j]*0.98);		
			CPoint XY2_(rSlider[j].right-1.0,sliderH-sliderH*centsParam[j]*0.98);
			drawContext->drawLine(XY1_.offset(0.0, rSlider[j].top),XY2_.offset(0.0, rSlider[j].top));

			CPoint XYs_(rSlider[j].getCenter().x,sliderH-sliderH*centsParam[j]*0.98+rSlider[j].top);	
			drawContext->drawLine(rSlider[j].getCenter(),XYs_);
		}

		//draw notes name
		for(int j = 0; j < noteDisplayed; ++j)
		{
			VSTGUI::UTF8StringPtr name = MTlabels[j];
			drawContext->drawString(name, rSlider[j].getTopLeft().offset(2.0, -2.0));
		}
	}
	
	
#define Make_Slider_Rect CRect 	rSlider = CRect(0.0, 0.0, w*0.5,h*0.5); \
						 CPoint setAt = this->size.getTopLeft(); \
						setAt.offset(w*0.25,h*0.5); \
						rSlider.offset(setAt.x, setAt.y)\

};