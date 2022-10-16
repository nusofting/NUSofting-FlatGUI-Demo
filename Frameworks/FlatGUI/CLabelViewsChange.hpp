// temporary classes, to be unified

#pragma once

class CLabelViewsChange: public CView
{
public:

	CLabelViewsChange(const CRect& size):CView(size)
	{
		bKT = true;
		myFont = 0;
		setDisplayFont();
	}
	~CLabelViewsChange()
	{
		if(myFont)
			myFont->forget();
	}
	void setDisplayColour(const CColor& colour)
	{
		colorFont = colour;
		setDirty();
	}
	void setDisplayFont(CFontRef asFont = 0)
	{
		setDirty ();
		if (myFont)
			myFont->forget ();

		if(asFont)
		{myFont = asFont; myFont->remember ();}
		else 
		{myFont = 0;}
	}

	void draw(CDrawContext *pContext)
	{
		const CCoord third = size.getHeight()/3.0;

		pContext->setFontColor(colorFont);
		myFont->setStyle(kBoldFace);
		pContext->setFont(myFont); 

		if(bKT)
		{
			CRect rKeytrk = CRect(size);
			rKeytrk.bottom = size.top + third;
			pContext->drawString("keytrk", rKeytrk, kLeftText);
			rKeytrk.top += third; 		rKeytrk.bottom += third;
			pContext->drawString("rate", rKeytrk, kLeftText);
			rKeytrk.top += third; 		rKeytrk.bottom += third;
			pContext->drawString("osc1", rKeytrk, kLeftText);
		}
		else
		{
			CRect rKeytrk = CRect(size);
			rKeytrk.top += third; 	rKeytrk.bottom -= third;
			pContext->drawString("width", rKeytrk, kLeftText);
		}
		setDirty(false);
	}

	void setKT(bool var)
	{
		bKT = var;
		setDirty();
	}

private:
	bool bKT;
	CColor colorFont;
	CFontRef myFont;
};


class CLabelViewsADSR: public CView
{
public:

	CLabelViewsADSR(const CRect& size):CView(size)
	{
		bADSR = true;
		myFont = 0;
		setDisplayFont();
	}
	~CLabelViewsADSR()
	{
		if(myFont)
			myFont->forget();
	}
	void setDisplayColour(const CColor& colour)
	{
		colorFont = colour;
		setDirty();
	}
	void setDisplayFont(CFontRef asFont = 0)
	{
		setDirty ();
		if (myFont)
			myFont->forget ();

		if(asFont)
		{myFont = asFont; myFont->remember ();}
		else 
		{myFont = 0;}
	}

	void draw(CDrawContext *pContext)
	{
		pContext->setFontColor(colorFont);
		myFont->setStyle(kBoldFace);
		pContext->setFont(myFont); 

		if(bADSR)
		{
			pContext->drawString("LP-HP", size, kLeftText);
		}
		else
		{
			pContext->drawString("BP pitch", size, kLeftText);
		}
		setDirty(false);
	}

	void setADSR(bool var)
	{
		bADSR = var;
		setDirty();
	}

private:
	bool bADSR;
	CColor colorFont;
	CFontRef myFont;
};