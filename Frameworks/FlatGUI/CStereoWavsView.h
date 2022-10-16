#pragma once
#include "vstgui/vstgui.h"
#include "Config.h"

#ifndef VstInt32
typedef int VstInt32;	//< 32 bit integer type
#endif

class CStereoWavsView : public CControl
{
public:
	CStereoWavsView(const CRect& size, const ConfigGraphView& configGraphView, IControlListener* listener = 0, long tag = 0, VstInt32 sizeDataFromEffect = 0);
	~CStereoWavsView();

	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons);

	void draw (CDrawContext *pContext)
	{
		if(!waveArray) return;

		CDrawContext* drawContext = pContext;

		drawContext->setFrameColor(m_configGraphView.colFrame);
		drawContext->setFillColor(m_configGraphView.colBkground1);
		drawContext->setLineWidth(1);
		drawContext->setDrawMode(kAntiAliasing);

		drawContext->drawRect(size,kDrawFilled);

		const CCoord width = size.getWidth();
		const CCoord height = size.getHeight();
		const CCoord plus1 = height*0.98; 
		const CCoord minus1 = height-plus1;


		drawContext->drawLine(size.getTopLeft().offset(0,height*0.5),size.getTopRight().offset(0,height*0.5));

		drawContext->setFrameColor(m_configGraphView.colClip);

		drawContext->drawLine(size.getTopLeft().offset(0,plus1), size.getTopRight().offset(0,plus1));
		drawContext->drawLine(size.getTopLeft().offset(0,minus1),size.getTopRight().offset(0,minus1));

		drawContext->setFrameColor(m_configGraphView.colSignal);

		CPoint* arrayPoints = new CPoint[static_cast<size_t>(width)];

		for(int x = 0; x < width; x++)
		{
			unsigned int j = int(float(scale*scale*x)/float(width-1) * float(sizeData) + 0.5f);
			const float range = 0.5f*waveArray[(atSample+j)&(sizeData-1)]+0.5f;
			const CCoord y = range*(minus1-plus1+2)+plus1-1; // +2 and -1 so that max volts is 1 px below the red line 

			//if(scale < 0.5f) // interpolate
			//{
			// nextY = nextY + 0.5*(y - nextY);
			//}

			CPoint XY = CPoint(x,y);
			XY.offset(size.getTopLeft().x, size.getTopLeft().y);

			if(x >= 1)
			{
				unsigned int j = int(float(scale*scale*(x-1))/float(width-1) * float(sizeData) + 0.5f);
				const float range = 0.5f*waveArray[(atSample+j)&(sizeData-1)]+0.5f;
				const CCoord y = range*(minus1-plus1+2)+plus1-1; // +2 and -1 so that max volts is 1 px below the red line 

				CPoint prevXY = CPoint(x-1,y);
				prevXY.offset(size.getTopLeft().x, size.getTopLeft().y);

				drawContext->drawLine(XY,prevXY);
			}
			else
			{
				drawContext->drawPoint(XY,m_configGraphView.colSignal);
			}
		}

		delete[] arrayPoints;


		rangeHandle = 0.32*width;
		rSlider = CRect(0,0,rangeHandle,0.05*height);
		rSlider.offset(size.getTopLeft().x+rangeHandle, size.getBottomRight().y-0.05*height-minus1);
		drawContext->setFillColor(m_configGraphView.colBkground2);
		drawContext->drawRect(rSlider,kDrawFilled);
		

		CColor cSlider = m_configGraphView.colHandle; cSlider.alpha = 111;
		if(bMouseOn)
		{	
			cSlider.alpha = 255;
		}

		CRect rSliderMove(rSlider);
		rSliderMove.right = rSliderMove.left + scale*rangeHandle + 1.0;
		drawContext->setFillColor(cSlider);
		drawContext->drawRect(rSliderMove,kDrawFilled);

		setDirty(false);
	}

	void inputValues(const float* prt = 0, VstInt32 pos = 0)
	{
		if(!waveArray) return;
		if(!prt) return;
		//if(pos >= sizeData) pos = pos-sizeData;
		//else if (pos < 0) pos = sizeData+pos;


		for(int j = pos; j <= sizeData-1; ++j) // old samples
		{
			waveArray[j-pos] = prt[j];  // could read filledData[] while audio is writing in it
		}
		for(int j = 0; j < pos; ++j) //new samples
		{
			int i1 = j+(sizeData-1-pos);  if(i1 < 0 || i1 > sizeData) i1 = 0;
			waveArray[i1] = prt[j]; 
		}

		//for(int j = sizeData-1; j >= 0; --j)
		//{
		//	waveArray[j] = prt[(sizeData-1) - j]; // reverse left to right copy
		//}

		atSample = 0; int j = 0;
		while(atSample == 0 && j < sizeData-2) // find zero point
		{
			const float x1 = waveArray[j]; 
			const float x2 = waveArray[j+1];
			const float x3 = waveArray[j+2];

			if(x1 < 0.0f && x3 > 0.0f && x2 < x3 && x2 > x1) 
			{
				atSample = j;
			}

			++j;
		}

			setDirty();
	}


	CLASS_METHODS(CStereoWavsView, CControl)

private:

	float value1,value2;
	float oldValue1,oldValue2;
	CCoord iMaxValue1,iMaxValue2;
	float* waveArray;
	VstInt32 atSample;
	VstInt32 sizeData;
	float scale;
	CRect rSlider;
	CCoord rangeHandle;
	bool bMouseOn;
	CCoord nextY;
	VstInt32 oldPos;
	int ia,ib;

	void Clamp(float& value)
	{
		if (value > 1.0f)
			value = 1.0f;
		else if (value < 0.0f)
			value = 0.0f;
	}

	const ConfigGraphView& m_configGraphView;

}; // end class CStereoWavsView
