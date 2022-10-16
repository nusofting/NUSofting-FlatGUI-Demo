#pragma once
#include "vstgui/vstgui.h"

class CLightLayer  : public CGradientView 
{
public:
	CLightLayer(const CRect& size);
	~CLightLayer ();
	//void draw (CDrawContext *pContext);
	void setColours(double color1Start, double color2Start, const CColor& sTopColor, const CColor& sBottomColor, double angle, bool isAA = false);

	void setViewSize (const CRect& newSize)
	{
		if (size != newSize)
		{
			size = newSize;

			setDirty();
		}
	}
	void setMouseableArea (const CRect& rect) // called after setViewSize ()
	{ 
		setMouseEnabled(false);
	}
	CMessageResult notify(CBaseObject* sender, IdStringPtr message) 
	{
		if(message == CVSTGUITimer::kMsgTimer)
		{	
			if(flip) preVar -= 5;
			else preVar += 5;

			if(preVar >= 255) flip = true;
			if(preVar <= 0) flip = false;	

			//double grads =  90.0*rand()/(RAND_MAX+1.0);
		    setColours( -0.5, 1.0, CColor(255,preVar,0,111),  CColor(0,0,preVar,140), 0.0);
	
			return kMessageNotified;
		}
		else
			return kMessageUnknown;

		//setDirty();		not needed
	}


	CLASS_METHODS(CLightLayer, CGradientView)

private:

	CColor kTopColor, kBottomColor;

	int preVar;
	bool flip;

};