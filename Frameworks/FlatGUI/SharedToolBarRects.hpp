#pragma once

#include "vstgui/vstgui.h"

const CColor cFontClear	= CColor (203, 203, 203, 255);

struct MainToolBarRects
{
	CRect m_rSettings;
	CRect m_rCompare;
	CRect m_rSave;
	CRect m_rBanks;
	CRect m_rPresets;
    CRect m_rColours;
    CRect m_rHelp;
    CRect m_rRandWidget;

	MainToolBarRects():
	m_rSettings(),
	m_rCompare(),
	m_rSave(),
	m_rBanks(),
	m_rPresets(),
	m_rColours(),
	m_rHelp(),
	m_rRandWidget()
	{
	}
};


struct SharedToolBarRects
{
	MainToolBarRects rAllRectsNeeded;

	void drawToolbarFiller(CDrawContext* pContext, const float& scalingX, const float& scalingY, const CCoord& width, CFontDesc* myFont = 0)
	{
		// top bar	

		CRect rTopBkgrnd(CPoint(0,0), CPoint(width,scalingY*(46)));
		pContext->setFillColor(MakeCColor(45, 45, 45, 255));
		pContext->drawRect(rTopBkgrnd,kDrawFilled); // rect for top
		rTopBkgrnd.offset(0.0,scalingY*(46-14));
		rTopBkgrnd.setHeight(scalingY*14);
		pContext->setFillColor(MakeCColor(29, 29, 32, 255));
		pContext->drawRect(rTopBkgrnd,kDrawFilled); // rect for arrows
        
        pContext->setDrawMode(kAntiAliasing);

		const CCoord settingsWidth = width*0.041;
		const CCoord compareWidth = width*0.059;
		const CCoord presetsWidth = width*0.26;		
		const CCoord banksWidth = presetsWidth*0.9;		
		const CCoord GapX = width*0.014;


		rAllRectsNeeded.m_rSettings = CRect(CPoint(0,0),CPoint(settingsWidth,18*scalingY));
		rAllRectsNeeded.m_rSettings.offset(2*scalingX,9*scalingY);

		rAllRectsNeeded.m_rCompare = CRect(CPoint(0,0),CPoint(compareWidth,18*scalingY));
		rAllRectsNeeded.m_rCompare.offset(rAllRectsNeeded.m_rSettings.right+GapX,9*scalingY); //rCompare.makeIntegral();
	
		rAllRectsNeeded.m_rSave = CRect(CPoint(0,0),CPoint(compareWidth,18*scalingY));
		rAllRectsNeeded.m_rSave.offset(rAllRectsNeeded.m_rCompare.right+GapX,9*scalingY); //rSave.makeIntegral();

		rAllRectsNeeded.m_rBanks = CRect(CPoint(0,0),CPoint(banksWidth,26*scalingY));
		rAllRectsNeeded.m_rBanks.offset(rAllRectsNeeded.m_rSave.right+GapX,5*scalingY);

		rAllRectsNeeded.m_rPresets = CRect(CPoint(0,0),CPoint(presetsWidth,26*scalingY));
		rAllRectsNeeded.m_rPresets.offset(rAllRectsNeeded.m_rBanks.right,5*scalingY);


		const CCoord sizeMenuY = rAllRectsNeeded.m_rBanks.getHeight();
		const CCoord topMenuline = rAllRectsNeeded.m_rBanks.top;
		const CCoord fontsSizeSmall = 12*scalingY;

		pContext->setLineWidth(1.0*scalingY);
		pContext->setFont (myFont, fontsSizeSmall, kBoldFace);
		pContext->setFontColor (cFontClear);
		pContext->setFrameColor(MakeCColor(32, 32, 34, 255));
		pContext->drawRect(rAllRectsNeeded.m_rSettings,kDrawStroked);		
		// Shift the right edge of the settings rectangle to be the left edge of the menu stripes.
		// Then the following text is drawn centred in the space to the left of the stripes.
		drawMenuStripes(pContext, rAllRectsNeeded.m_rSettings.right-14*scalingX, topMenuline, sizeMenuY, scalingX, scalingY);
		CRect rString = rAllRectsNeeded.m_rSettings;
		pContext->drawString ("?", rString.offset(-9*scalingX,1*scalingY));	

		pContext->setFillColor(MakeCColor(12, 12, 12, 255));
		pContext->drawRect(rAllRectsNeeded.m_rBanks,kDrawFilled);
		pContext->drawRect(rAllRectsNeeded.m_rPresets,kDrawFilled);
		drawMenuStripes(pContext, rAllRectsNeeded.m_rBanks.right-14*scalingX, topMenuline, sizeMenuY, scalingX, scalingY);
		drawMenuStripes(pContext, rAllRectsNeeded.m_rPresets.right-14*scalingX, topMenuline, sizeMenuY, scalingX, scalingY);

		// gray line between menus
        pContext->setLineWidth(1.0*scalingY);
		pContext->setFrameColor(MakeCColor(111, 111, 111, 255));
		pContext->drawLine(rAllRectsNeeded.m_rBanks.getTopRight(),rAllRectsNeeded.m_rBanks.getBottomRight());

		// tools


		const CCoord yCenterLine = rAllRectsNeeded.m_rPresets.getCenter().y;

		const CCoord xRandWidget = rAllRectsNeeded.m_rPresets.right+GapX*1.1;		

		rAllRectsNeeded.m_rRandWidget = CRect(0.0, 0.0, 82.0f*scalingX, 24.0f*scalingY);
		rAllRectsNeeded.m_rRandWidget.offset(xRandWidget, yCenterLine-12.0f*scalingY);

		const CCoord xHelp = rAllRectsNeeded.m_rRandWidget.right+GapX*2.0;

		CRect rHelp = CRect(0.0, 0.0, floor(18.0f*scalingX), floor(sizeMenuY-2.0)); // launch HTML
		rHelp.offset(xHelp, topMenuline); 
		pContext->drawLine(rHelp.getTopLeft(),rHelp.getTopRight());
		pContext->drawLine(rHelp.getTopRight(),rHelp.getBottomRight());
		const CPoint a = rHelp.getBottomLeft().offset(floor(4.0f*scalingX), 0.0);
		const CPoint b = rHelp.getBottomLeft().offset(0.0, floor(-4.0f*scalingY));
		const CPoint c = CPoint(floor(a.x) , floor(b.y));
		pContext->drawLine(rHelp.getBottomRight(), a);
		pContext->drawLine(rHelp.getTopLeft(), b);
		pContext->drawLine(a, c);
		pContext->drawLine(b, c);
		pContext->drawLine(a, b);
		const CCoord x1 = floor(2.5f*scalingX);
		for(int yn = 4; yn < 22; yn += 3)
		{
			pContext->drawLine(rHelp.getTopLeft().offset(x1, yn*scalingY), rHelp.getTopRight().offset(-x1, yn*scalingY));
		}
		rAllRectsNeeded.m_rHelp = rHelp;

		const CCoord xColours = rAllRectsNeeded.m_rHelp.right+GapX*2.0;

		// Vertically centre within the menu bar, allowing the height to change slightly due to rounding at different
		// scale factors. Also horizontally centre between the left edge of the containing rectangle and the left edge
		// of the menu stripes.
		CCoord vertGap = sizeMenuY * 0.25;
		CCoord sideBar = 5.25; 
		CCoord allColoursWidth = 3.0*sideBar*scalingX+1.0;
		CRect rColours(xColours, topMenuline+vertGap, rHelp.right+GapX+allColoursWidth, topMenuline+sizeMenuY-vertGap);
		rColours.setWidth(sideBar*scalingX);
		pContext->setFillColor(kRedCColor);
		pContext->drawRect(rColours,kDrawFilled);
		rColours.offset(sideBar*scalingX, 0.0); 
		pContext->setFillColor(kGreenCColor);
		pContext->drawRect(rColours,kDrawFilled);
		rColours.offset(sideBar*scalingX-1.0, 0.0);
		pContext->setFillColor(kBlueCColor);
		pContext->drawRect(rColours,kDrawFilled);
		rColours.setWidth(allColoursWidth);
		rColours.offset(-2.0*sideBar*scalingX, 0.0);  
		pContext->setFillColor(MakeCColor(101, 101, 104, 111));
		pContext->drawRect(rColours,kDrawFilledAndStroked);		 
		rAllRectsNeeded.m_rColours = rColours;
	}

	void drawMenuStripes(CDrawContext* pContext, const CCoord& X, const CCoord& Y, CCoord menuHeight, const float& scalingX, const float& scalingY)
	{
		const CCoord relativeStripeHeight = 2;
		const CCoord numStripes = 3;
		// Vertically centre the menu stripes in the height of the menu bar.
		const CCoord heightAllStripes = (2*numStripes-1)*relativeStripeHeight*scalingY;
		const CCoord topMenuStripes = Y+(2.6*scalingY+menuHeight-heightAllStripes)/2.0;
		 CCoord step = floor(relativeStripeHeight*scalingY);
        if(step < 1.0) step = 1.0;

		pContext->setFillColor(cFontClear);

		CRect rStripe(0, 0, 10.0*scalingX, step);
		rStripe.offset(X,topMenuStripes);
        if(rStripe.getHeight() < 2.0) rStripe.setHeight(2.0);
		for (size_t i = 0; i < numStripes; ++i)
		{
			pContext->drawRect(rStripe,kDrawFilled);
			rStripe.offset(0,2.0*step);
		}
	}
};
