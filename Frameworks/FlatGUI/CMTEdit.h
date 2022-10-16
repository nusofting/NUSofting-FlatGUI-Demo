#pragma once
#include "vstgui/vstgui.h"
#include "Config.h"
#include "Controls.h"

typedef int VstInt32;				///< 32 bit integer type

class CMTEdit : public CViewContainer, public IControlListener, public IDependency
{
public:
	CMTEdit(CFrame* frame, const CRect& size);
	~CMTEdit();

	virtual CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseMoved (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseUp (CPoint& where, const CButtonState& buttons);
	CMessageResult notify (CBaseObject* sender, IdStringPtr message);
	void runFileSelector (bool load);

	virtual void drawBackgroundRect (CDrawContext *pContext, const CRect& _updateRect)
	{
		const CCoord width = size.getWidth();
		const CCoord height = size.getHeight();
		const CCoord fontSize = height/25.0;
		const CCoord fontSizeWas = pContext->getFont()->getSize();

		pContext->setFillColor(MakeCColor(98,98,98,234));
		pContext->drawRect(size, kDrawFilled);	
		CRect rTitle = CRect(0,0,width/3.0,fontSize);
		rTitle.offset(size.left+width/3.0,size.top+4.0);
		pContext->getFont()->setSize(fontSize);
		pContext->drawString("MicroTuning", rTitle);

		rClose = CRect(0,0,width/5.0,fontSize);
		rClose.offset(size.right-width/5.0-4.0,size.top+4.0);
		pContext->setFillColor(MakeCColor(HL1,102,HL1,200));
		pContext->drawRect(rClose, kDrawFilledAndStroked);
		pContext->getFont()->setSize(fontSize/1.2);
		pContext->drawString("close", rClose);

		rMTEditLoad = CRect(rClose);
		rMTEditLoad.offset(0.0,fontSize+4.0);
		rMTEditSave = CRect(rMTEditLoad);
		rMTEditSave.offset(0.0,fontSize+4.0);

		pContext->setFillColor(MakeCColor(102,HL2,HL2,200));
		pContext->drawRect(rMTEditLoad, kDrawFilledAndStroked);
		pContext->drawString("load", rMTEditLoad);
		pContext->setFillColor(MakeCColor(102,HL3,HL3,200));
		pContext->drawRect(rMTEditSave, kDrawFilledAndStroked);
		pContext->drawString("save", rMTEditSave);

		pContext->getFont()->setSize(fontSizeWas);

		//rMouseableArea = CRect(rClose.getTopLeft(),rMTEditSave.getBottomRight());
		//this->setMouseableArea(rMouseableArea);

		CRect rDisplay = CRect(0,0,width/2.0,height/3.0);
		rDisplay.offset(width/4.0,height/3.0);
		char mouse[512] = {0};
		sprintf(mouse, "X %1.2f Y %1.2f", whereOld.x,whereOld.y);
		pContext->drawRect(rDisplay, kDrawStroked);
		pContext->drawString(scaleName.c_str(), rDisplay);
		

		setDirty(false);
	}

	virtual void valueChanged(VSTGUI::CControl* pControl)
	{
		if (pControl)
		{
			long tag = pControl->getTag();
			float value = pControl->getValue();
		}
	}

	bool showCView(CFrame* pOwner = NULL)
	{
		if(pOwner)
		{
			pOwner->setModalView(this);
			return true;
		}
		else
			return false;
	}

	void setSize(const CRect& NewSize, bool doInvalid = true) // Must match function signature in CView
	{
		size = CRect(NewSize);
		CViewContainer::setViewSize(size, doInvalid);
	}

	float* getValuesTune()
	{
		return valuesTun;
	}
	const char* getScaleName()
	{
		return scaleName.c_str();
	}

	/// Custom message sent to objects that register themselves as dependencies.
	/// Sent when the user changes the state of the view.
	static IdStringPtr kSetTunNewFile;

	CLASS_METHODS(CMTEdit, CView)

private:

	CFrame* m_frame;
	CRect rClose;
	bool bMouseOn;

	CRect rMTEditLoad;
	CRect rMTEditSave;
	const ConfigSlider* m_configSlider;
	const ConfigSwitch* m_configSwitch;
	ScaledFontFactory* m_scaledFontFactory;

	char HL1,HL2,HL3;
	CCoord x, y;

	CPoint whereOld;

	CSliderNano* pBox1;
	CSimpleOnOff* pSwitch1;	

	CRect rMouseableArea;
	CRect rBox1;
	CRect rBox2;

	float valuesTun[128];
	std::string scaleName;

	bool bLoadNoSave;

}; // end class CMTEdit


