#pragma once

#include "vstgui/vstgui.h"
#include "vstgui/lib/platform/iplatformbitmap.h"

#include "FlatGUI/AppearanceController.h"
#include "FlatGUI/FontSizeFactors.hpp"
#include "FlatGUI/CreateScaledBitmap.hpp"
#include "FlatGUI/SharedToolBarRects.hpp"
//#include "FlatGUI/graph_fractal_functions_.hpp"


#include <algorithm>

static const float refWidth =  752.0f;
static const float refHeight = 464.0f;

static const float HRatio = 1.0f/refWidth;
static const float VRatio = 1.0f/refHeight;

 const float pi__ = 3.14159265359f;

class BFRBackgroundBitmap: public CBitmap
{
public:
	BFRBackgroundBitmap(CCoord width, CCoord height, AppearanceController& appearance, const char* fontName = "Thaoma")
		:CBitmap (width,height)
	{
		cBackground = appearance.getColourById(CLR_MAIN_BACKGROUND);
		cLinesColour = appearance.getColourById(CLR_HIGHLIGHT_LINE);
		cKnobLink = appearance.getColourById(CLR_KNOB_RIM);
		cFontClear = MakeCColor(203, 203, 203, 255); // @todo Toolbar lines and ?
		cFontLabels = appearance.getColourById(CLR_LABEL_FONT);
		cLinesPlain = appearance.getColourById(CLR_PLAIN_LINE);
		cControls = appearance.getColourById(CLR_CONTROL_ACTIVE);
		cTooltips = appearance.getColourById(CLR_TOOLTIP_FONT);
		decoType = appearance.getDecorationType();		

		myFont = 0;
		myFont = new CFontDesc(fontName, 12);

		const bool reverse = cBackground.getLightness() < 128;
		CCoord sizePic = 4400/width;
		pic1 = createScaledBitmap("Docs+PNGs/bends.png",  sizePic, reverse);
		pic2 = createScaledBitmap("Docs+PNGs/depths.png", sizePic, reverse);
		picB = createScaledBitmap("Docs+PNGs/flat.png",   sizePic, reverse);
		picD = createScaledBitmap("Docs+PNGs/sharp.png",  sizePic, reverse);
		
		if(decoType == DECO_BITMAP1)
			tile = createScaledBitmap("Docs+PNGs/base_demoLibVST.png", 1400/width);
		else
			tile = 0;

		bitmap = createScaledBitmap("Docs+PNGs/nusofting_logo.png", 3290/width);

		memset(rCtrls,0,sizeof(rCtrls)); // ?

		pNoiseTable = NULL;

		tileNotUsed = true;
	}

	~BFRBackgroundBitmap()
	{
		if(myFont){
			myFont->forget();
		}

		forgetCBitmap(bitmap);
		forgetCBitmap(pic1);
		forgetCBitmap(pic2);
		forgetCBitmap(picB);
		forgetCBitmap(picD);
		forgetCBitmap(tile);
	}

	void draw (CDrawContext* context, const CRect& rect, const CPoint& offset, float alpha)
	{	
		if(!context) return;

		CDrawContext* pContext = context;

		if(!myFont) return;

		const CCoord width  = rect.getWidth();
		const CCoord height = rect.getHeight();

		const float scalingX = width*HRatio;
		const float scalingY = height*VRatio;

		pContext->setFillColor(cBackground);
		pContext->drawRect(rect,kDrawFilled);

		if(decoType == DECO_BITMAP1 && tile)
		{

			CRect rLogo(CPoint(0,0), CPoint(width, height));
			pContext->drawBitmap(tile,rLogo);
			tileNotUsed = false;

		}

		// draw borders as some wood when not tile

		const uint8_t shadow = 32;
		CRect rBorder = CRect(0,46.0f*scalingY,9.0f*scalingX, refHeight*scalingY);
		pContext->setFillColor(MakeCColor(120,86,45,246));
		if(tileNotUsed)pContext->drawRect(rBorder,kDrawFilled);

		pContext->setLineWidth(1.6*scalingX);
		pContext->setFrameColor(MakeCColor(109+shadow,86+shadow,45+shadow,246));
		pContext->drawLine(rBorder.getTopLeft(),rBorder.getBottomLeft());
		pContext->setFrameColor(MakeCColor(109-shadow,86-shadow,45-shadow,246));
		if(tileNotUsed)pContext->drawLine(rBorder.getTopRight(),rBorder.getBottomRight());

		rBorder.offset(refWidth*scalingX-9.0f*scalingX,0.0);
		if(tileNotUsed)pContext->drawRect(rBorder,kDrawFilled);

		pContext->setFrameColor(MakeCColor(109+shadow,86+shadow,45+shadow,246));
		if(tileNotUsed)pContext->drawLine(rBorder.getTopLeft(),rBorder.getBottomLeft());
		pContext->setFrameColor(MakeCColor(109-shadow,86-shadow,45-shadow,246));
		if(tileNotUsed)pContext->drawLine(rBorder.getTopRight(),rBorder.getBottomRight());

		CRect rSpring = CRect(0,0, width-2.2f*rBorder.getWidth(), 54.0f*scalingY);
		rSpring.offset(1.1f*rBorder.getWidth(), height-47.0f*scalingY);
		texturePixels(pContext, rSpring, CPoint(0.0, 0.0), rSpring.getWidth(), rSpring.getHeight());

		// first titles and lines <<<<<<<<<<
		pContext->setFont(myFont,rSpring.getHeight()/2.5,kBoldFace);
		pContext->setFontColor (cFontLabels);
		pContext->drawString("E c h o b i s",rSpring.inset(9.0f*scalingX,9.0f*scalingY),kLeftText);

		rSpring.offset(198.0f*scalingX,0.0); 
		rSpring.setHeight(kFontSizeSmall*scalingY);
		rSpring.offset(-198.0f*scalingX, 35.0f*scalingY); 
		pContext->setFont(myFont,kFontSizeSmall*scalingY,kBoldFace);
		pContext->setFontColor (cFontLabels);
		//pContext->drawString("Supporting your rights to repair and modifiy the hardware you own. See \"repair.org\"",rSpring,kLeftText);

		//*********

		if(bitmap)
		{
			CRect rLogo(CPoint(0,0), CPoint(bitmap->getWidth(), bitmap->getHeight()));
			rLogo.offset(width-bitmap->getWidth()-26.0f*scalingX, height-bitmap->getHeight()-10.0f*scalingY);
			pContext->drawBitmap(bitmap,rLogo);
		}	
		
		//*********

		sharedToolBarRects.drawToolbarFiller(pContext, scalingX, scalingY, width, myFont);

		//*********

		CRect rSliderSize = CRect(0,0,24.0f*scalingX, 302.0f*scalingY);
		CCoord ySliderTop = 73.0f*scalingY;
		CCoord xSliderGap = 42.0f*scalingX;
		rCtrls[kPre_Gain] = rSliderSize;
		rCtrls[kPre_Gain].offset(24.0f*scalingX, ySliderTop);
		rCtrls[kMixChannels] = rSliderSize;
		rCtrls[kMixChannels].offset(36.0f*scalingX+xSliderGap*1.1, ySliderTop);
		rSliderSize = CRect(0,0,22.0f*scalingX, 248.0f*scalingY);
		rCtrls[kLevel1] = rSliderSize;
		rCtrls[kLevel1].offset(600.0f*scalingX, ySliderTop);
		rCtrls[kLevel2] = rSliderSize;
		rCtrls[kLevel2].offset(600.0f*scalingX+xSliderGap*1.1, ySliderTop);		
		rCtrls[kDry_Gain] = rSliderSize;
		rCtrls[kDry_Gain].offset(600.0f*scalingX+xSliderGap*2.1, ySliderTop);

		CRect rKnob68 = CRect(0,0,68.0f*scalingX, 68.0f*scalingY);
		CRect rKnob52 = CRect(0,0,52.0f*scalingX, 52.0f*scalingY);
		CRect rKnob44 = CRect(0,0,44.0f*scalingX, 44.0f*scalingY);


		const CCoord shiftX = 9.0f*scalingX;

		rCtrls[kDelayTime1] = rKnob68;
		rCtrls[kDelayTime1].offset(165.0f*scalingX, ySliderTop);
		rCtrls[kFeedback1] = rKnob68;
		rCtrls[kFeedback1].offset(486.0f*scalingX, ySliderTop);	

		rLabelTime1 = CRect(0,0, rCtrls[kDelayTime1].getWidth(), 16.0f*scalingY); 
		rLabelTime1.offset(rCtrls[kDelayTime1].left, rCtrls[kDelayTime1].bottom+5.0f*scalingY);
		rLabelTime2 = rLabelTime1 ;
		rLabelTime2.offset(0, 181.0f*scalingY);

		rAmpMod1 = CRect(0,0, 22.0f*scalingX, 12.0f*scalingY);
		rAmpMod1.offset(rLabelTime1.left+60.0f*scalingX+shiftX, rLabelTime1.top-16.5f*scalingY);
		rAmpMod2 = rAmpMod1;
		rAmpMod2.offset(0, 181.0f*scalingY);

		CCoord yLineTop = rCtrls[kDelayTime1].getCenter().y-rKnob52.getHeight()*0.5;
		CCoord xLineGap = 24.0f*scalingX;

		rCtrls[kTimeMod1] = rKnob44;
		rCtrls[kTimeMod1].offset(rCtrls[kDelayTime1].right+xLineGap+shiftX, rCtrls[kDelayTime1].top-14.0f*scalingY);
		rCtrls[kDepth1] = rKnob44;
		rCtrls[kDepth1].offset(rCtrls[kDelayTime1].right+xLineGap+shiftX, rCtrls[kTimeMod1].bottom+6.0f*scalingY);
		rCtrls[kDiv1] = rKnob44;
		rCtrls[kDiv1].offset(rCtrls[kDelayTime1].right+xLineGap+shiftX, rCtrls[kDepth1].bottom+6.0f*scalingY);

		rCtrls[kPitch1] = rKnob52;
		rCtrls[kPitch1].offset(rCtrls[kTimeMod1].right+xLineGap+2.0f*scalingX, yLineTop);
		rCtrls[kLP1] = rKnob52;
		rCtrls[kLP1].offset(rCtrls[kPitch1].right+xLineGap-2.0f*scalingX, yLineTop);

		yLineTop = 146.0f*scalingY;

		rCtrls[kRate1] = rKnob52;
		rCtrls[kRate1].offset(rCtrls[kDepth1].right+xLineGap+2.0f*scalingX, yLineTop);
		rCtrls[kHP1] = rKnob52;
		rCtrls[kHP1].offset(rCtrls[kRate1].right+xLineGap-2.0f*scalingX, yLineTop);

		yLineTop = 181.0f*scalingY; // delay 2

		rCtrls[kDelayTime2] = CRect(rCtrls[kDelayTime1]);
		rCtrls[kDelayTime2].offset(0, yLineTop);
		rCtrls[kFeedback2] = CRect(rCtrls[kFeedback1]);
		rCtrls[kFeedback2].offset(0, yLineTop);

		rCtrls[kTimeMod2] = rKnob44;
		rCtrls[kTimeMod2].offset(rCtrls[kDelayTime2].right+xLineGap+shiftX, rCtrls[kDelayTime2].top-14.0f*scalingY);
		rCtrls[kDepth2] = rKnob44;
		rCtrls[kDepth2].offset(rCtrls[kDelayTime2].right+xLineGap+shiftX, rCtrls[kTimeMod2].bottom+6.0f*scalingY);
		rCtrls[kDiv2] = rKnob44;
		rCtrls[kDiv2].offset(rCtrls[kDelayTime2].right+xLineGap+shiftX, rCtrls[kDepth2].bottom+6.0f*scalingY);

		rCtrls[kPitch2] = CRect(rCtrls[kPitch1]);
		rCtrls[kPitch2].offset(0, yLineTop);
		rCtrls[kRate2] = CRect(rCtrls[kRate1]);
		rCtrls[kRate2].offset(0, yLineTop);
		rCtrls[kHP2] = CRect(rCtrls[kHP1]);
		rCtrls[kHP2].offset(0, yLineTop);
		rCtrls[kLP2] = CRect(rCtrls[kLP1]);
		rCtrls[kLP2].offset(0, yLineTop);


		//knob link
		pContext->setDrawMode(kAntiAliasing);
		pContext->setFrameColor(cKnobLink);
		pContext->setLineWidth(5.0f*scalingX);			
		pContext->drawLine(rCtrls[kDelayTime1].getCenter(),rCtrls[kFeedback1].getCenter());
		pContext->drawLine(rCtrls[kDelayTime2].getCenter(),rCtrls[kFeedback2].getCenter());

		//inner rects	
		CColor cHere = cBackground;
		cHere.red += 5;
		cHere.green += 5;
		cHere.blue += 5;
	
		pContext->setFillColor(cHere);
		pContext->setLineWidth(2.0f*scalingX);
		CRect rModN = CRect(rCtrls[kTimeMod1].left, rCtrls[kTimeMod1].top, rCtrls[kDiv1].right, rCtrls[kDiv1].bottom);
		rModN.extend(10.0f*scalingX,10.0f*scalingY);
		rModN.offset(0.0f,4.0f*scalingY);
		drawRoundedRect(pContext,  rModN, 	CDrawContext::kPathFilled);	
		drawRoundedRect(pContext,  rModN, 	CDrawContext::kPathStroked);
		rModN.offset(0.0f,181.0f*scalingY);
		drawRoundedRect(pContext,  rModN, 	CDrawContext::kPathFilled);	
		drawRoundedRect(pContext,  rModN, 	CDrawContext::kPathStroked);
		rModN.offset(74.0f*scalingX,-181.0f*scalingY);
		drawRoundedRect(pContext,  rModN, 	CDrawContext::kPathFilled);	
		drawRoundedRect(pContext,  rModN, 	CDrawContext::kPathStroked);
		rModN.offset(0.0f,181.0f*scalingY);
		drawRoundedRect(pContext,  rModN, 	CDrawContext::kPathFilled);	
		drawRoundedRect(pContext,  rModN, 	CDrawContext::kPathStroked);
		rModN.offset(74.0f*scalingX,-181.0f*scalingY);
		drawRoundedRect(pContext,  rModN, 	CDrawContext::kPathFilled);	
		drawRoundedRect(pContext,  rModN, 	CDrawContext::kPathStroked);
		rModN.offset(0.0f,181.0f*scalingY);
		drawRoundedRect(pContext,  rModN, 	CDrawContext::kPathFilled);	
		drawRoundedRect(pContext,  rModN, 	CDrawContext::kPathStroked);


		pContext->setLineWidth(4.0f*scalingX);
		pContext->drawLine(rCtrls[kTimeMod1].getCenter(),rCtrls[kDiv1].getCenter());
		pContext->drawLine(rCtrls[kTimeMod2].getCenter(),rCtrls[kDiv2].getCenter());
		pContext->drawLine(rCtrls[kPitch1].getCenter(),rCtrls[kRate1].getCenter());
		pContext->drawLine(rCtrls[kPitch2].getCenter(),rCtrls[kRate2].getCenter());	
		pContext->drawLine(rCtrls[kLP2].getCenter(),rCtrls[kHP2].getCenter());
		pContext->drawLine(rCtrls[kLP1].getCenter(),rCtrls[kHP1].getCenter());


		if(pic1 && pic2 && picB && picD)
		{
			const float alpha = 0.986f;
			CRect rPicN(CPoint(0,0), CPoint(pic1->getWidth(), pic1->getHeight()));
			rPicN.offset(rCtrls[kTimeMod1].left-10.0f*scalingX, rCtrls[kTimeMod1].top);
			pContext->drawBitmap(pic1, rPicN, CPoint(0,0), alpha); 
			rPicN.offset(0.0f, 181.0f*scalingY);
			pContext->drawBitmap(pic1, rPicN, CPoint(0,0), alpha); 

			rPicN = CRect(CPoint(0,0), CPoint(pic1->getWidth(), pic1->getHeight()));
			rPicN.offset(rCtrls[kDepth1].left-10.0f*scalingX, rCtrls[kDepth1].top);
			pContext->drawBitmap(pic2, rPicN, CPoint(0,0), alpha);  
			rPicN.offset(0.0f, 181.0f*scalingY);
			pContext->drawBitmap(pic2, rPicN, CPoint(0,0), alpha); 

			rPicN = CRect(CPoint(0,0), CPoint(pic1->getWidth(), pic1->getHeight()));
			rPicN.offset(rCtrls[kPitch1].left-12.0f*scalingX, rCtrls[kPitch1].top);
			pContext->drawBitmap(picB, rPicN, CPoint(0,0), alpha); 
			rPicN.offset(rCtrls[kPitch1].getWidth()+6.0f*scalingX, 0.0f);
			pContext->drawBitmap(picD, rPicN, CPoint(0,0), alpha); 

			rPicN = CRect(CPoint(0,0), CPoint(pic1->getWidth(), pic1->getHeight()));
			rPicN.offset(rCtrls[kPitch2].left-12.0f*scalingX, rCtrls[kPitch2].top);
			pContext->drawBitmap(picB, rPicN, CPoint(0,0), alpha);  
			rPicN.offset(rCtrls[kPitch2].getWidth()+6.0f*scalingX, 0.0f);
			pContext->drawBitmap(picD, rPicN, CPoint(0,0), alpha); 
		}

		//**************************??



		CRect rSwitch = CRect(0,0,19.0f*scalingX, 19.0f*scalingY);

		const CCoord ySyncLine1 = rCtrls[kDelayTime1].bottom+2.0f*rSwitch.getHeight();
		const CCoord ySyncLine2 = rCtrls[kDelayTime2].bottom+2.0f*rSwitch.getHeight();
		rCtrls[kSync1] = rSwitch;
		rCtrls[kSync1].offset(rCtrls[kDelayTime1].getCenter().x-rSwitch.getWidth()*0.5, ySyncLine1);
		rCtrls[kSync2] = rSwitch;
		rCtrls[kSync2].offset(rCtrls[kDelayTime2].getCenter().x-rSwitch.getWidth()*0.5, ySyncLine2);

		rCtrls[kTriplets1] = rCtrls[kSync1];
		rCtrls[kTriplets1].offset(25.0f*scalingX, 0);

		rCtrls[kTriplets2] = rCtrls[kSync2];
		rCtrls[kTriplets2].offset(25.0f*scalingX, 0);		

		rCtrls[kPhase1] = rSwitch;
		rCtrls[kPhase1].offset(rCtrls[kFeedback1].getCenter().x-rSwitch.getWidth()*0.5, ySyncLine1);
		rCtrls[kPhase2] = rSwitch;
		rCtrls[kPhase2].offset(rCtrls[kFeedback2].getCenter().x-rSwitch.getWidth()*0.5, ySyncLine2);		

		rCtrls[kActive1] = CRect(0,0,48.0f*scalingX, 18.0f*scalingY);
		rCtrls[kActive1].offset(rCtrls[kMixChannels].right+20.0f*scalingX, 53.0f*scalingY);
		rCtrls[kActive2] = rCtrls[kActive1];
		rCtrls[kActive2].offset(0.0f, 181.0f*scalingY);

		pContext->setLineWidth(1.6f*scalingY);
		pContext->setFrameColor(cLinesColour);
		pContext->drawLine(rCtrls[kActive1].getBottomLeft().offset(2.0f*scalingX,2.0f*scalingY),rCtrls[kActive1].getBottomRight().offset(0.0f,2.0f*scalingY));
		pContext->drawLine(rCtrls[kActive2].getBottomLeft().offset(2.0f*scalingX,2.0f*scalingY),rCtrls[kActive2].getBottomRight().offset(0.0f,2.0f*scalingY));

		rKickClearBypass = rCtrls[kActive2];
		rKickClearBypass.setHeight(20.0f*scalingY);
		rKickClearBypass.offset(0.0f, 162.0f*scalingY);

		rCtrls[kLinkFeeds] = CRect(0,0,25.0f*scalingX, 14.0f*scalingY);
		rCtrls[kLinkFeeds].offset(rCtrls[kPhase2].left-4.0f*scalingX, rCtrls[kHP2].bottom+22.0f*scalingY);

		rCtrls[kLinkLevels] = CRect(0,0,25.0f*scalingX, 14.0f*scalingY);
		rCtrls[kLinkLevels].offset(rCtrls[kLevel1].right, rCtrls[kHP2].bottom+22.0f*scalingY);

		rCtrls[kLinkLPHPs] = CRect(0,0,25.0f*scalingX, 14.0f*scalingY);
		rCtrls[kLinkLPHPs].offset(rCtrls[kHP2].getCenter().x-12.5f*scalingX, rCtrls[kHP2].bottom+22.0f*scalingY);


		CRect rKnob35 = CRect(0,0,35.0f*scalingX, 35.0f*scalingY);

		rCtrls[kPan1] = rKnob35;
		rCtrls[kPan1].offset(rCtrls[kLevel1].getCenter().x-rKnob35.getWidth()*0.5, rCtrls[kSync2].bottom - rKnob35.getHeight());
		rCtrls[kPan2] = rKnob35;
		rCtrls[kPan2].offset(rCtrls[kLevel2].getCenter().x-rKnob35.getWidth()*0.5, rCtrls[kSync2].bottom - rKnob35.getHeight());

		rCtrls[kCrossfeed] = CRect(0,0,151.0f*scalingX, 22.0f*scalingY);
		rCtrls[kCrossfeed].offset(rCtrls[kDepth2].getCenter().x, 438.0f*scalingY);

		rCtrls[kTapeComp] = rSwitch;
		rCtrls[kTapeComp].offset(rCtrls[kPhase2].left,rCtrls[kCrossfeed].top);
		rCtrls[kDrift] = rSwitch;
		rCtrls[kDrift].offset(rCtrls[kSync1].left,rCtrls[kCrossfeed].top);

		// labels

		const CCoord verticalGap = 5.0f*scalingY;
		const CCoord smallHorizontalGap = 3.5f*scalingX;
		const CCoord mediumHorizontalGap = 7.0f*scalingX;

		pContext->setFont(myFont,kFontSizeMedium*scalingY,kNormalFace);
		pContext->setFontColor (cFontLabels);

		drawAlignedStringAbove(pContext, "input", kPre_Gain, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringAbove(pContext, "mix", kMixChannels, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringAbove(pContext, "time", kDelayTime1, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringBelow(pContext, "sync", kSync1, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringBelow(pContext, "rate", kRate1, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringBelow(pContext, "div", kDiv1, verticalGap*0.05, kFontSizeMedium*scalingY);
		drawAlignedStringAbove(pContext, "feedback", kFeedback1, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringAbove(pContext, "LP", kLP1, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringBelow(pContext, "HP", kHP1, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringAbove(pContext, "grain", kPitch1, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringBelow(pContext, "phase", kPhase1, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringBelow(pContext, "phase", kPhase2, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringAbove(pContext, "level 1", kLevel1, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringBelow(pContext, "pan 1", kPan1, verticalGap, kFontSizeMedium*scalingY);
		drawAlignedStringBelow(pContext, "pan 2", kPan2, verticalGap, kFontSizeMedium*scalingY);
		drawAlignedStringAbove(pContext, "time", kDelayTime2, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringBelow(pContext, "sync", kSync2, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringBelow(pContext, "rate", kRate2, verticalGap, kFontSizeMedium*scalingY);
		drawAlignedStringBelow(pContext, "div", kDiv2, verticalGap*0.05 , kFontSizeMedium*scalingY);
		drawAlignedStringAbove(pContext, "feedback", kFeedback2, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringAbove(pContext, "LP", kLP2, verticalGap, kFontSizeMedium*scalingY);
		drawAlignedStringBelow(pContext, "HP", kHP2, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringAbove(pContext, "grain", kPitch2, verticalGap, kFontSizeMedium*scalingY);
		drawAlignedStringAbove(pContext, "level 2", kLevel2, verticalGap , kFontSizeMedium*scalingY);
		drawAlignedStringAbove(pContext, "dry", kDry_Gain, verticalGap , kFontSizeMedium*scalingY);

		//pContext->setFontColor (cControls);
		drawAlignedStringAbove(pContext, "crossfeed 1><2", kCrossfeed, verticalGap*0.5, kFontSizeMedium*scalingY);
		drawAlignedStringAbove(pContext, "comp", kTapeComp, verticalGap*0.84, kFontSizeMedium*scalingY);
		drawAlignedStringAbove(pContext, "ana drift", kDrift, verticalGap*0.84, kFontSizeMedium*scalingY);

		pContext->setLineWidth(1.0);
		pContext->setFrameColor(cLinesColour);
		const CCoord leftX = rCtrls[kDelayTime1].left - 42.0f*scalingX;
		const CCoord rightX = rCtrls[kFeedback1].right + 28.0f*scalingX;
		const CCoord LineLevel = rCtrls[kHP1].bottom + 27.0f*scalingY;
		pContext->drawLine(CPoint(leftX, LineLevel),CPoint(rightX, LineLevel));
		pContext->drawLine(CPoint(leftX, 48.0f*scalingY),CPoint(leftX, height-47.0f*scalingY));
		pContext->drawLine(CPoint(rightX, 48.0f*scalingY),CPoint(rightX, height-47.0f*scalingY));


		pContext->setFont(myFont,12.0f*scalingY,kNormalFace);
		pContext->setFontColor (cTooltips);
		CRect rSliderLabel = CRect(0,0,18.0f*scalingX,12.0f*scalingY);
		rSliderLabel.offset(rCtrls[kPre_Gain].left-5.0f*scalingX,138.0f*scalingY);
		pContext->drawString("-", rSliderLabel, kLeftText);
		rSliderLabel.offset(24.0f*scalingX,0);
		pContext->drawString("0 dB", rSliderLabel, kLeftText);
		rSliderLabel = CRect(0,0,18.0f*scalingX,12.0f*scalingY);
		rSliderLabel.offset(rCtrls[kMixChannels].left-5.0f*scalingX,rCtrls[kMixChannels].getCenter().y-6.0f*scalingY);
		pContext->drawString("=", rSliderLabel, kLeftText);
		rSliderLabel.offset(0,-rCtrls[kMixChannels].getHeight()*0.5+10.0f*scalingY);
		pContext->drawString("1", rSliderLabel, kLeftText);
		rSliderLabel.offset(0,rCtrls[kMixChannels].getHeight()*0.93);
		pContext->drawString("2", rSliderLabel, kLeftText);
		rSliderLabel.offset(-52.0f*scalingX, 18.0f*scalingY);
		pContext->drawString("no fx", rSliderLabel, kLeftText);

		rSliderLabel = CRect(0,0,18.0f*scalingX,12.0f*scalingY);
		rSliderLabel.offset(rCtrls[kLevel1].left-5.0f*scalingY,rCtrls[kLevel1].top+5.0f*scalingY);
		pContext->drawString("-", rSliderLabel, kLeftText);
		rSliderLabel.offset(xSliderGap*1.1,0);
		pContext->drawString("-", rSliderLabel, kLeftText);
		rSliderLabel.offset(xSliderGap,0);
		pContext->drawString("-", rSliderLabel, kLeftText);
		rSliderLabel.offset(22.0f*scalingX,0);
		pContext->drawString("0 dB", rSliderLabel, kLeftText);

		pContext->setFont(myFont,kFontSizeSmall*scalingY*1.2,kBoldFace);
		cTooltips.alpha = 144;
		pContext->setFrameColor (cTooltips);
		pContext->setFontColor (cTooltips);


		CRect rinfoLabel(rCtrls[kPre_Gain].left,rCtrls[kPre_Gain].bottom+24.0f*scalingY, 			
		rCtrls[kMixChannels].right, rCtrls[kMixChannels].bottom+24.0f*scalingY);

		rinfoLabel.inset(4.0f*scalingX, 0.0f);
		rinfoLabel.offset(2.0f*scalingX, -2.0f);

		cFontLabels.alpha = 144;
		pContext->setFontColor(cFontLabels);
		pContext->setFont(myFont,kFontSizeSmall*scalingY,kBoldFace);

		char numberDisplay[128] = "v. ";

#if DEMO		
		strcat(numberDisplay, VstSynthVersionNum);
		strcat(numberDisplay, " DEMO"); 
		pContext->drawString (numberDisplay, rinfoLabel);
		rinfoLabel.offset(0.0, 14.0f*scalingY);
		pContext->drawRect(rinfoLabel, kDrawFilled);
		strcpy(numberDisplay, "[not saving state!]");
		pContext->drawString (numberDisplay, rinfoLabel);
#else
		rinfoLabel.offset(0.0, 14.0f*scalingY);
		std::string extraInfoDisplay(Platform::buildVersionString(""));
		if (extraInfoDisplay.length() > 2)
		{
			pContext->drawRect(rinfoLabel, kDrawFilled);
			pContext->drawString (extraInfoDisplay.c_str(), rinfoLabel);
			rinfoLabel.offset(0.0, -14.0f*scalingY);
		}
		strcat(numberDisplay, VstSynthVersionNum);
		pContext->drawString (numberDisplay, rinfoLabel);
#endif
	}

	void getAllrCtrls(CRect* editorKnobsr)
	{
		for(int j = 0; j < kNumParams; ++j)
		{
			if(rCtrls[j].getWidth() > 0.0)
			{
                //rCtrls[j].makeIntegral();
				editorKnobsr[j] = CRect(rCtrls[j]);	
			}
		}
	}


	CRect rLabelTime1;
	CRect rLabelTime2;

	CRect rAmpMod1;
	CRect rAmpMod2;

	CRect rKickClearBypass;

	SharedToolBarRects sharedToolBarRects;	

private:
	CColor cBackground, cLinesColour, cLinesPlain, cKnobLink, cFontClear, cFontLabels, cControls, cTooltips;
	CFontDesc* myFont;
	bool tileNotUsed;

		/// Draw a string (typically a label) aligned with two parameter control rectangles, one to be horizontally
	/// centred around and one to be vertically centred with.
	///
	/// @param pContext
	///		The graphics context to draw the string in. The font and text colour needs to be set in this context before
	///		calling this function.
	/// @param str
	///		The string to align and draw.
	/// @param horizControlId
	///		The parameter ID enum value for the control around which the string will be horizontally centred.
	/// @param vertControlId
	///		The parameter ID enum value for the control around which the string will be vertically aligned.
	///		The type of alignment depends on the value of the gap parameter.
	/// @param gap
	///		The vertical spacing gap used for vertical alignment.
	///		If this is 0, the string is vertically centred relative to the rectangle for the control specified by 
	///		vertControlId.
	///		If this is non-zero, the string is positioned above the top of the rectangle for the control specified by 
	///		vertControlId plus the gap amount.
	/// @param height
	///		If the gap is non-zero, the height of the string's rectangle must be specified in this parameter.
	void drawDoubleAlignedString(CDrawContext* pContext, const char *str, int horizControlId, int vertControlId, CCoord gap = 0, CCoord height = 0)
	{
		CRect labelRect = rCtrls[horizControlId];
		if (gap == 0)
		{
			labelRect.top = rCtrls[vertControlId].top;
			labelRect.bottom = rCtrls[vertControlId].bottom;
		}
		else
		{
			labelRect.bottom = rCtrls[vertControlId].top - gap;
			labelRect.top = labelRect.bottom - height;
		}

		pContext->drawString(str, labelRect);
		//pContext->drawRect (labelRect);
	}

	/// Draw a string horizontally centred above a parameter control rectangle.
	///
	/// @param pContext
	///		The graphics context to draw the string in. The font and text colour needs to be set in this context before
	///		calling this function.
	/// @param str
	///		The string to align and draw.
	/// @param horizControlId
	///		The parameter ID enum value for the control around which the string will be horizontally centred.
	/// @param gap
	///		The vertical spacing gap between the top of the control and the bottom of the rectangle containing the string.
	/// @param height
	///		The height of the rectangle containing the string.
	void drawAlignedStringAbove(CDrawContext* pContext, const char *str, int horizControlId, CCoord gap, CCoord height)
	{
		CRect labelRect = rCtrls[horizControlId];
		labelRect.setHeight(height);
		labelRect.offset(0.0, -labelRect.getHeight() - gap);
		pContext->drawString(str, labelRect);
		//pContext->drawRect (labelRect);
	}

	/// Draw a string horizontally aligned below a parameter control rectangle.
	///
	/// @param pContext
	///		The graphics context to draw the string in. The font and text colour needs to be set in this context before
	///		calling this function.
	/// @param str
	///		The string to align and draw.
	/// @param horizControlId
	///		The parameter ID enum value for the control around which the string will be horizontally aligned.
	/// @param gap
	///		The vertical spacing gap between the bottom of the control and the top of the rectangle containing the string.
	/// @param height
	///		The height of the rectangle containing the string.
	/// @param alignment
	///		The VSTGUI horizontal alignment style to use for aligning the string with the control rectangle above.
	void drawAlignedStringBelow(CDrawContext* pContext, const char *str, int horizControlId, CCoord gap, CCoord height, CHoriTxtAlign alignment = kCenterText)
	{
		CRect labelRect = rCtrls[horizControlId];
		labelRect.offset(0.0, labelRect.getHeight() + gap);
		labelRect.setHeight(height);
		pContext->drawString(str, labelRect, alignment);
		//pContext->drawRect (labelRect);
	}

	/// Draw a string vertically centred and right-aligned before a parameter control rectangle.
	///
	/// @param pContext
	///		The graphics context to draw the string in. The font and text colour needs to be set in this context before
	///		calling this function.
	/// @param str
	///		The string to align and draw.
	/// @param horizControlId
	///		The parameter ID enum value for the control for which the string will be aligned.
	/// @param gap
	///		The horizontal spacing gap between the left edge of the control and the right end of the string.
	void drawAlignedStringBefore(CDrawContext* pContext, const char *str, int vertControlId, CCoord gap)
	{
		CRect labelRect = rCtrls[vertControlId];
		labelRect.offset(-labelRect.getWidth(), 0.0);
		labelRect.right = rCtrls[vertControlId].left - gap;
		pContext->drawString(str, labelRect, kRightText);
		//pContext->drawRect (labelRect);
	}
	void drawAlignedStringAfter(CDrawContext* pContext, const char *str, int vertControlId, CCoord gap)
	{
		CRect labelRect = rCtrls[vertControlId];
		labelRect.offset(rCtrls[vertControlId].right, 0.0);
		labelRect.left = rCtrls[vertControlId].right + gap;
		pContext->drawString(str, labelRect, kLeftText);
		//pContext->drawRect (labelRect);
	}

	/// Draw a string horizontally centred between two parameter control rectangles, optionally offset in height.
	/// The two rectangles are assumed to be vertically aligned and the same height.
	///
	/// @param pContext
	///		The graphics context to draw the string in. The font and text colour needs to be set in this context before
	///		calling this function.
	/// @param str
	///		The string to align and draw.
	/// @param beforeControlId
	///		The parameter ID enum value for the control to the left of the string.
	/// @param afterControlId
	///		The parameter ID enum value for the control to the right of the string.
	/// @param gap
	///		The vertical spacing gap used for vertical alignment.
	///		If this is 0, the string is vertically centred relative to the rectangle for the control specified by 
	///		beforeControlId.
	///		If this is non-zero, the string is positioned above the top of the rectangle for the control specified by 
	///		beforeControlId plus the gap amount.
	/// @param height
	///		If the gap is non-zero, the height of the string's rectangle must be specified in this parameter.
	CRect drawAlignedStringBetween(CDrawContext* pContext, const char *str, int beforeControlId, int afterControlId, CCoord gap = 0, CCoord height = 0)
	{
		CRect labelRect = rCtrls[beforeControlId];
		labelRect.left = rCtrls[beforeControlId].right;
		labelRect.right = rCtrls[afterControlId].left;
		if (gap != 0)
		{
			labelRect.bottom = rCtrls[beforeControlId].top - gap;
			labelRect.top = labelRect.bottom - height;
		}
		pContext->setFont (myFont, height, kNormalFace);
		pContext->drawString(str, labelRect);
		//pContext->drawRect (labelRect);
		return labelRect;
	}

	/// Draw a left-aligned string followed by a horizontal line. The string is vertically centred around the line.
	///
	/// @param pContext
	///		The graphics context to draw the string in. The font and text colour needs to be set in this context before
	///		calling this function.
	/// @param str
	///		The string to align and draw.
	/// @param stringStartX
	///		The left position of the string.
	/// @param lineEndX
	///		The right position of the line.
	/// @param gap
	///		The horizontal gap to leave between the string and the line.
	/// @param lineY
	///		The vertical position of the line to vertically centre around.
	void drawAlignedStringAndLine(CDrawContext* pContext, const char *str,
		CCoord stringStartX, CCoord lineEndX, CCoord gap, CCoord lineY)
	{
		// We don't need to know the width, because VSTGUI is no longer clipping the text to the rectangle.
		// Also, set the height of the rectangle to zero, because we want to vertically centre around a line.
		CRect labelRect(0, 0, 10.0, 0.0);
		labelRect.offset(stringStartX, lineY);
		pContext->drawString(str, labelRect, kLeftText);
		//pContext->drawRect (labelRect);
		CCoord strWidth = pContext->getStringWidth(str);
		pContext->drawLine(CPoint(stringStartX + strWidth + gap, lineY),CPoint(lineEndX, lineY));
	}

	void drawRoundedRect(CDrawContext* pContext,  CRect& rRounded, CDrawContext::PathDrawMode style = CDrawContext::kPathFilledEvenOdd)
	{
		CGraphicsPath* path = pContext->createGraphicsPath ();
		if (path)
		{
			path->addRoundRect(rRounded,16.0);
			pContext->drawGraphicsPath(path, style);

			path->forget();
		}		
	}

	void drawArrowRect(CDrawContext* pContext,  CRect& rArrow, const int way = 1)
	{
		CGraphicsPath* path = pContext->createGraphicsPath ();
		if (path)
		{
			if(way == 1){
				const CPoint p1 = rArrow.getTopLeft().offset(0.0,rArrow.getHeight()/2.0);
				const CPoint p2 = rArrow.getTopLeft().offset(rArrow.getWidth()/4.0,0.0);
				const CPoint p3 = CPoint(p2.x,p2.y + rArrow.getHeight()/3.0);
				const CPoint p4 = rArrow.getTopRight().offset(0.0,rArrow.getHeight()/3.0);
				const CPoint p5 = rArrow.getBottomRight().offset(0.0,-rArrow.getHeight()/3.0);
				const CPoint p6 = CPoint(p3.x, p3.y + rArrow.getHeight()/3.0);
				const CPoint p7 = CPoint(p2.x,p2.y + rArrow.getHeight());
				path->beginSubpath(p1); path->addLine(p2); path->addLine(p3);
				path->addLine(p4); path->addLine(p5); path->addLine(p6);
				path->addLine(p7);
			}else{
				const CPoint p1 = rArrow.getTopRight().offset(0.0,rArrow.getHeight()/2.0);
				const CPoint p2 = rArrow.getTopRight().offset(-rArrow.getWidth()/4.0,0.0);
				const CPoint p3 = CPoint(p2.x,p2.y + rArrow.getHeight()/3.0);
				const CPoint p4 = rArrow.getTopLeft().offset(0.0,rArrow.getHeight()/3.0);
				const CPoint p5 = rArrow.getBottomLeft().offset(0.0,-rArrow.getHeight()/3.0);
				const CPoint p6 = CPoint(p3.x, p3.y + rArrow.getHeight()/3.0);
				const CPoint p7 = CPoint(p2.x,p2.y + rArrow.getHeight());
				path->beginSubpath(p1); path->addLine(p2); path->addLine(p3);
				path->addLine(p4); path->addLine(p5); path->addLine(p6);
				path->addLine(p7);
			}
			path->closeSubpath();
			pContext->drawGraphicsPath(path, CDrawContext::kPathFilledEvenOdd);

			path->forget();
		}

	}

	CRect rCtrls[kNumParams];

	CBitmap* bitmap;
	CBitmap* tile;
	CBitmap* pic1;
	CBitmap* pic2;
	CBitmap* picB;
	CBitmap* picD;

	void forgetCBitmap( CBitmap* ptr)
	{
		if(ptr) ptr->forget();
	}


	void texturePixels(CDrawContext* pContext, const CRect& rect, const CPoint& offset, const CCoord width, const CCoord height)
	{
        
		float _width = floor(width);

		IPlatformBitmapPixelAccess* cBMPpx = getPlatformBitmap()->lockPixels(false);
		if(cBMPpx)
		{
			uint8_t* pPxLine = cBMPpx->getAddress(); // temporary buffer
			int32_t iPxStride = cBMPpx->getBytesPerRow(); // iPxStride/4 = width // colours kBGRA	
			const int32_t bytes = iPxStride * int(height);
			int32_t line = 0;

			float x = 0.0f;

			for (int32_t i = 0; i < bytes - 4; i += 4) // all pxs
			{	
				if(i%4 == 0) x++; 				
				const float y = float(line);

				const float speed2 =  2.0f*pi__/(iPxStride/4);
				const float speed1 =  2.0f*pi__/(int(height));

				uint8_t mix1 = uint8_t(32 + 32 * cos(speed1*y)* sin(speed2*x));
				uint8_t mix2 = uint8_t(32 + 32 * sin(speed1*y)* cos(speed2*x));

				if(i > line*iPxStride-4)
				{
					line++;
					x = 0.0f;
				}

				if(line%2 == 0 )
				{
					pPxLine[i] = std::max<int>(cBackground.red-16,16); // B
					pPxLine[i+1] = std::max<int>(cBackground.green-mix2,mix2); // G
					pPxLine[i+2] = std::max<int>(cBackground.blue-mix1,mix1); // R
					pPxLine[i+3] = 255; // A
				}
			}
			cBMPpx->forget(); // Unlock the bits. // temporary buffer copied back to bitmap

			if(pContext) CBitmap::draw(pContext,rect,offset);
		}
		//else
		//{
		//	MessageBox(0,0,"IPlatformBitmapPixelAccess == 0",0);
		//}		
	}

	float** pNoiseTable;

	void makeNoiseTable(size_t dim, const CCoord width, const CCoord height)
	{
		const size_t xx = size_t(width)/dim;
		const size_t yy = size_t(height)/dim;

		pNoiseTable = (float**)malloc(yy*sizeof(float*)); // rows

		for(size_t i=0; i<yy; ++i)
			pNoiseTable[i] = (float*)malloc(xx*sizeof(float)); // each row

		for(size_t y = 0; y < yy; ++y)
		{
			if(pNoiseTable[y])
			{
				for( size_t x = 0; x < xx; ++x)			
					pNoiseTable[y][x] = rand()/(double(RAND_MAX)+1.0);
			}
		}
	}

	void clearNoiseTable()
	{
		if(pNoiseTable == NULL){
			free((void*)pNoiseTable);
		}
	}

	uint8_t scaleLightness(const uint8_t& cc, float var)
	{
		return  static_cast<uint8_t>(float(cc)*var);
	}


	DecorationId decoType;

};


//float noise = rand()/(double(RAND_MAX)+1.0);

//fBuffer = fBuffer + 0.5f*(noise-fBuffer);	

//if(i%4 == 0) x++; 				
//const float y = float(line);


//const float speed2 =  2.0f*pi/(iPxStride/4);
//const float speed1 =  2.0f*pi/(int(height));

//uint8_t mix1 = uint8_t(32 + 32 * cos(speed1*y)* sin(speed2*x));
//uint8_t mix2 = uint8_t(32 + 32 * sin(speed1*y)* cos(speed2*x));


//if(i > line*iPxStride-4)
//{
//	line++;
//	x = 0.0f;
//}

//if(line%3 == 0 )
//{
//	pPxLine[i] = std::max<int>(cBackground.red-16,16); // B
//	pPxLine[i+1] = std::max<int>(cBackground.green-mix2,mix2); // G
//	pPxLine[i+2] = std::max<int>(cBackground.blue-mix1,mix1); // R
//	pPxLine[i+3] = 255; // A
//}


//CColor cMain = CColor(cBackground);
//unsigned char Hue, Sat, Value;
//RGBtoHSV(Hue, Sat, Value, cBackground);
//cMain.fromHSV(360.0*Hue/255.0,Sat/255.0,1.6*Value/255.0);
//pContext->setFillColor(cMain);
//pContext->drawRect(rect,kDrawFilled);


			//const CCoord tileWidth  = tile->getWidth();
			//const CCoord tileHeight = tile->getHeight();
			//CRect rTile(0.0, 0.0, tileWidth, tileHeight);
			//CCoord y = 0; CCoord x = 0;
			//while(y < 1.0+height/tileHeight)
			//{		
			//	while(x < 1.0+width/tileWidth)
			//	{					
			//		pContext->drawBitmap(tile,rTile, CPoint(0,0), 1.0f);
			//		x += 1.0; 
			//		rTile.moveTo(x*tileWidth, y*tileHeight);
			//	}
			//	y += 1.0; 
			//	x = 0.0;
			//	rTile.moveTo(0.0, y*tileHeight);
			//}
