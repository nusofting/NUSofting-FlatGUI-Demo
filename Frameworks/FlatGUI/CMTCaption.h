//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2020
//****************************************************************************************************//

#pragma warning( disable : 4244)// 'initializing' : conversion from 'VSTGUI::CCoord' to 'const int', possible loss of data

#pragma once

#include "FlatGUI/Config.h"
#include "vstgui/vstgui.h"

//------------------------------------------------------------------------------
class CMTCaption : public CView
{
public:
	CMTCaption (const CRect &size, ScaledFontFactory& fontFactory,  const ConfigSlider& configSlider);
	~CMTCaption()
	{

	}
	void draw (CDrawContext *pContext);

	void setViewSize(CRect& newSize)
	{
		CView::setViewSize(newSize);
		size = newSize;

		setDirty();
	}

	void setVisible(bool state)
	{
		CView::setVisible(state);

		setDirty();
	}

	void setMsgs(int type)
	{
		if(type == 0)
		{
			msg1 = "octave microtuning:";
			msg2 = "click [MT] to open.";
		}
		else
		{
			msg1 = "MTS-ESP is";
			msg2 = "connected.";
		}

		setDirty();
	}



	CLASS_METHODS(CMTCaption , CView)

private:

	const ConfigSlider& m_configSlider;
	bool bMouseOn;

	const char* msg1;
	const char* msg2;
	
};
