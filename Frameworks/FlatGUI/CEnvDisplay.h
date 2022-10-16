//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2015
//****************************************************************************************************//

#pragma warning( disable : 4244)// 'initializing' : conversion from 'VSTGUI::CCoord' to 'const int', possible loss of data

#pragma once

#include "FlatGUI/Config.h"

#include "vstgui/vstgui.h"

#include "../../DashLibrary/fastADSR.h"

//------------------------------------------------------------------------------
class CEnvDisplay : public CView
{
public:
	CEnvDisplay(const CRect &size, const ConfigGraphView& configGraphView);
	void draw (CDrawContext *pContext);
	void attack(float var);
	void decay(float var);
	void sustain(float var);
	void release(float var);
	void ratio(float var);
	void setBackLayer(bool flag)
	{
		bBackLayer = flag;
	}
	void setNumber(int val)
	{
		iNumber = val;
	}
	

	CLASS_METHODS(CEnvDisplay, CView)

private:

	fastADSR env1;
	int XS;
	const ConfigGraphView& m_configGraphView;
	bool bBackLayer;
	int iNumber;

};


