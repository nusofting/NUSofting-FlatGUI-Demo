/*-----------------------------------------------------------------------------

© 2016 nusofting.com - Luigi Felici (aka Liqih)

-----------------------------------------------------------------------------*/

#define SHADOWS // set fake 3D or flat keys
#include "Keyboard.h"

//------------------------------------------------------------------------

Keyboard::Keyboard(const CRect &size, IControlListener *listener, long tag,
				   int aNNotes, int aFirstNote) // aFirstNote from 0 to 6
				   : CControl(size, listener, tag, 0)
{
	reset(); // keys up and redraw
	NNotes = aNNotes; // only not diesis
	FirstNote = aFirstNote;
	indexRect = 0;
	NZones = int(0.5f+5.0f*NNotes/7.0f)+NNotes-2; // all keys

	setViewSize (size, false); // initialize variables

	cFrameColour  = MakeCColor(16, 16, 16, 255);
	cClearColour1 = MakeCColor(203, 203, 203, 255); 
	cClearColour2 = luminosity(cClearColour1,0.83f);
	cClearColour3 = luminosity(cClearColour1,0.21f);
	cDarkColour1  = MakeCColor(111, 111, 111, 255); 
	cDarkColour2  = luminosity(cDarkColour1,0.75f);
	cDarkColour3  = luminosity(cDarkColour1,0.54f);   
	cDarkColour4  = luminosity(cDarkColour1,0.21f);   

		value = -1;// ?
}

//------------------------------------------------------------------------
Keyboard::~Keyboard()
{	
}

//------------------------------------------------------------------------
void Keyboard::reset(void) 
{
	for (int j = 0; j < 128; ++j) 
	{
		Keys[j] = -1;
	}
	
	if (KeyPresed != -1)
	{
		noteOff(KeyPresed);
	}
	KeyPresed = -1;
	
	setDirty();
}

//------------------------------------------------------------------------
void Keyboard::noteOn(int Note)
{
	Keys[Note&127] = 1;
	setDirty();
}

//------------------------------------------------------------------------
void Keyboard::noteOff(int Note)
{
	Keys[Note&127] = -1;
	setDirty();
}
//------------------------------------------------------------------------
int Keyboard::getKeyPressed()
{
	return KeyPresed;
}
//------------------------------------------------------------------------
void Keyboard::buildZones (const CRect& rect)
{
	if(indexRect <= NZones)
	{
		rs[indexRect++] = CRect(rect);
	}
}
//------------------------------------------------------------------------
void Keyboard::draw(CDrawContext *pContext)
{
	COffscreenContext* oc = COffscreenContext::create (getFrame(), size.getWidth(), size.getHeight());
	oc->beginDraw();	

	oc->setFrameColor(cFrameColour);
	oc->setLineWidth(1.0f);	
	oc->setDrawMode(kAntiAliasing);

	CRect rKeyW = CRect(0.0,0.0,witdhW,heightW);
	rKeyW.offset(1.0,1.0);

	CRect rKeyB = CRect(0.0,0.0,witdhB,heightB);
	rKeyB.offset(1.0,1.0);

	const int wKey[7] = {kUpC,kUpD,kUpE,kUpC,kUpD,kUpD,kUpE}; //do re mi fa sol la si // C D E F G A B
	int index = FirstNote; // 0 to 6 .. C to B
	int noteNum = 0;
	bool playing = false;
	for (int kk = 0; kk < NNotes-1; ++kk) 
	{
		playing = Keys[noteNum] == 1;
		if(kk == 0 && (index == 2 || index == 6)) // E or B first
		{
			drawKey(oc, rKeyW, playing, kUpN);			
		}
		else if(kk == 0 && (index == 1 ||index == 4 ||index == 5)) // D or G or A first
		{
			drawKey(oc, rKeyW, playing, kUpC);
		}
		else
		{
			drawKey(oc, rKeyW, playing, wKey[index]);
		}
		++noteNum;

		if(wKey[index] == kUpC || wKey[index] == kUpD)
		{
			playing = Keys[noteNum] == 1;
			rKeyB.moveTo(rKeyW.right-witdhBhalf ,0.0);
			drawKey(oc, rKeyB, playing, kUpS);		
			++noteNum;
		}

		rKeyW.offset(witdhW,0.0);		
		index++;
		if(index > 6) index = 0;
	}
	playing = Keys[noteNum] == 1;
	if(index == 0 || index == 3)
		drawKey(oc, rKeyW, playing, kUpN);
	else if(index == 1 ||index == 2 ||index == 4 || index == 5 || index == 6)
		drawKey(oc, rKeyW, playing, kUpE);

	CRect rFull = CRect(0.0, 0.0, size.getWidth(), size.getHeight());
	oc->drawRect(rFull,kDrawStroked);

#ifdef _DEBUG
	oc->setFontColor(MakeCColor(255, 255, 255, 255));
	oc->setFont(kNormalFontVeryBig);
	char num[128] = {0};
	sprintf(num,"%d",KeyPresed);
	oc->drawString(num,rFull);
#endif

	oc->endDraw();

	oc->copyFrom(pContext,size);

	oc->forget();
	//Do not call setDirty(false); it prevents correct redraws in 32-bit OS X (Carbon)
}

//------------------------------------------------------------------------
CMouseEventResult Keyboard::onMouseDown(CPoint &where, const CButtonState &buttons)
{
	if (buttons.isLeftButton() && size.pointInside(where))
	{
		where.y = where.y - size.top;
		where.x = where.x - size.left;

		int newKey = KeyPresed;
		for (int indexZone = 0; indexZone<=indexRect; ++indexZone) {
			if(indexZone >= 128) return kMouseEventHandled;
			if (rs[indexZone].pointInside(where)) { // not implemented yet with the whole key extremis 

				if(rs[indexZone].getHeight() > heightB && indexZone <= 126)// not a diesis zone
				{
					if(indexZone > 0)
					{
						if(!rs[indexZone-1].pointInside(where) && !rs[indexZone+1].pointInside(where))
							newKey = indexZone;
					}
					else
					{
						if(!rs[indexZone+1].pointInside(where))
							newKey = indexZone;
					}
				}
				else // diesis zone , black keys
				{
					newKey = indexZone;
				}

				if (newKey != KeyPresed)
				{
					if (KeyPresed != -1)
					{
						noteOff(KeyPresed);
					}
					KeyPresed = newKey;
					noteOn(KeyPresed);
					value = (float)(KeyPresed)/128;
					invalidRect(rs[indexZone]);
					valueChanged ();
				}
			}
		}

	}	
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult Keyboard::onMouseUp(CPoint &where, const CButtonState &buttons)
{
	//if (buttons.isLeftButton())
	//{
		where.y = where.y - size.top;
		where.x = where.x - size.left;
		value = -1;
		if (KeyPresed != -1)
		{
			noteOff(KeyPresed);
		}
		KeyPresed = -1;
		setDirty();
		valueChanged ();
		
	//}
	return kMouseEventHandled;
}

//------------------------------------------------------------------------
CMouseEventResult Keyboard::onMouseMoved(CPoint &where, const CButtonState &buttons)
{
	return onMouseDown(where, buttons);
}

//------------------------------------------------------------------------
void Keyboard::setViewSize (const CRect& newSize, bool invalid)
{
	CView::setViewSize(newSize, invalid);

	witdhW = (size.getWidth() - 2.0)/NNotes;
	heightW = size.getHeight()-2.0;
	witdhB = witdhW/1.52;
	heightB = heightW/1.52;
	witdhBhalf = witdhB*0.5;
	witdhBside = witdhB*0.21;
	ShadowHeightTop = heightW/52.0;
	ShadowHeightBottom = heightW/23.0;

	setMouseableArea(newSize);
	indexRect = 0; // rebuild zones
}
//------------------------------------------------------------------------
void Keyboard::bottomShadow (COffscreenContext* oc, const CRect& rect)
{
	CRect rShadowBottom = CRect(rect);
	rShadowBottom.setHeight(ShadowHeightBottom);
	rShadowBottom.offset(0.0,heightW-ShadowHeightBottom);
	oc->setFillColor(cClearColour3);
	oc->drawRect(rShadowBottom,kDrawFilled);
}
//------------------------------------------------------------------------
#ifdef SHADOWS
void Keyboard::drawKey (COffscreenContext* oc, const CRect& rect, bool playing, int iType)
{
	if(iType < 0) return;

	if(playing)
	{
		switch(iType)
		{
		case kUpC: { iType = kDownC; } break;
		case kUpD: { iType = kDownD; } break;
		case kUpE: { iType = kDownE; } break;
		case kUpN: { iType = kDownN; } break;
		case kUpS: { iType = kDownS; } break;			
		}
	}

	oc->setFillColor(cClearColour1);

	const CCoord BottomX = rect.getBottomRight().x;
	const CCoord TopY = rect.getTopLeft().y;
	const CCoord LeftX = rect.getBottomLeft().x;
	const CCoord MidX2 = rect.getBottomRight().x-witdhBhalf;
	const CCoord MidX1 = rect.getBottomLeft().x+witdhBhalf;	

	switch(iType)
	{
	case kUpC:
		{
			CGraphicsPath* path = oc->createGraphicsPath ();
			path->beginSubpath (rect.getBottomLeft());
			path->addLine(rect.getBottomRight());
			path->addLine(CPoint(BottomX, TopY+heightB));
			path->addLine(CPoint(MidX2, TopY+heightB));
			path->addLine(CPoint(MidX2, TopY));
			path->addLine(rect.getTopLeft());
			path->closeSubpath();

			oc->drawGraphicsPath(path, CDrawContext::kPathFilledEvenOdd);
			oc->drawGraphicsPath(path, CDrawContext::kPathStroked);
			bottomShadow(oc,rect);

			path->forget();
			buildZones (rect);
		}
		break;
	case kUpD:
		{
			CGraphicsPath* path = oc->createGraphicsPath ();
			path->beginSubpath (rect.getBottomLeft());
			path->addLine(rect.getBottomRight());
			path->addLine(CPoint(BottomX, TopY+heightB));
			path->addLine(CPoint(MidX2, TopY+heightB));
			path->addLine(CPoint(MidX2, TopY));
			path->addLine(CPoint(MidX1, TopY));
			path->addLine(CPoint(MidX1, TopY+heightB));
			path->addLine(CPoint(LeftX, TopY+heightB));
			path->closeSubpath();

			oc->drawGraphicsPath(path, CDrawContext::kPathFilledEvenOdd);
			oc->drawGraphicsPath(path, CDrawContext::kPathStroked);
			bottomShadow(oc,rect);

			path->forget();
			buildZones (rect);
		}
		break;
	case kUpE:
		{
			CGraphicsPath* path = oc->createGraphicsPath ();
			path->beginSubpath (rect.getBottomLeft());
			path->addLine(rect.getBottomRight());
			path->addLine(rect.getTopRight());
			path->addLine(CPoint(MidX1, TopY));
			path->addLine(CPoint(MidX1, TopY+heightB));
			path->addLine(CPoint(LeftX, TopY+heightB));
			path->closeSubpath();

			oc->drawGraphicsPath(path, CDrawContext::kPathFilledEvenOdd);
			oc->drawGraphicsPath(path, CDrawContext::kPathStroked);
			bottomShadow(oc,rect);

			path->forget();
			buildZones (rect);
		}
		break;
	case kUpN:
		{			
			oc->drawRect(rect,kDrawFilledAndStroked);
			bottomShadow(oc,rect);
			buildZones (rect);
		}
		break;
	case kUpS:
		{		
			oc->setFillColor(cDarkColour3);
			oc->drawRect(rect,kDrawFilledAndStroked);
			CRect rLight = CRect(0.0, 0.0, witdhB-witdhBside*2.0, heightB-ShadowHeightBottom);
			rLight.offset(rect.left+witdhBside,0.0);
			oc->setFillColor(cDarkColour1);
			oc->drawRect(rLight,kDrawFilled);
			oc->setFillColor(cDarkColour4);
			CGraphicsPath* path = oc->createGraphicsPath ();
			path->beginSubpath (rect.getBottomLeft());
			path->addLine(rect.getBottomRight());
			path->addLine(rect.getBottomRight().offset(-witdhBside,-ShadowHeightBottom));
			path->addLine(rect.getBottomLeft().offset(witdhBside,-ShadowHeightBottom));
			path->closeSubpath();
			oc->drawGraphicsPath(path, CDrawContext::kPathFilledEvenOdd);
			
			buildZones (rect);
		}
		break;
	case kDownC:
		{
			oc->setFillColor(cClearColour2);
			CGraphicsPath* path = oc->createGraphicsPath ();
			path->beginSubpath (rect.getBottomLeft());
			path->addLine(rect.getBottomRight());
			path->addLine(CPoint(BottomX, TopY+heightB+ShadowHeightTop));
			path->addLine(CPoint(MidX2, TopY+heightB+ShadowHeightTop));
			path->addLine(CPoint(MidX2, TopY+ShadowHeightTop));
			path->addLine(CPoint(rect.getTopLeft().x, rect.getTopLeft().y+ShadowHeightTop));
			path->closeSubpath();

			oc->drawGraphicsPath(path, CDrawContext::kPathFilledEvenOdd);
			oc->drawGraphicsPath(path, CDrawContext::kPathStroked);

			path->forget();
		}
		break;
	case kDownD:
		{
			oc->setFillColor(cClearColour2);
			CGraphicsPath* path = oc->createGraphicsPath ();
			path->beginSubpath (rect.getBottomLeft());
			path->addLine(rect.getBottomRight());
			path->addLine(CPoint(BottomX, TopY+heightB+ShadowHeightTop));
			path->addLine(CPoint(MidX2, TopY+heightB+ShadowHeightTop));
			path->addLine(CPoint(MidX2, TopY+ShadowHeightTop));
			path->addLine(CPoint(MidX1, TopY+ShadowHeightTop));
			path->addLine(CPoint(MidX1, TopY+heightB+ShadowHeightTop));
			path->addLine(CPoint(LeftX, TopY+heightB+ShadowHeightTop));
			path->closeSubpath();

			oc->drawGraphicsPath(path, CDrawContext::kPathFilledEvenOdd);
			oc->drawGraphicsPath(path, CDrawContext::kPathStroked);

			path->forget();
		}
		break;
	case kDownE: //  _|  |
		{        // |____|
			oc->setFillColor(cClearColour2);
			CGraphicsPath* path = oc->createGraphicsPath ();
			path->beginSubpath (rect.getBottomLeft());
			path->addLine(rect.getBottomRight());
			path->addLine(CPoint(rect.getTopRight().x, rect.getTopRight().y+ShadowHeightTop));
			path->addLine(CPoint(MidX1, TopY+ShadowHeightTop));
			path->addLine(CPoint(MidX1, TopY+heightB+ShadowHeightTop));
			path->addLine(CPoint(LeftX, TopY+heightB+ShadowHeightTop));
			path->closeSubpath();

			oc->drawGraphicsPath(path, CDrawContext::kPathFilledEvenOdd);
			oc->drawGraphicsPath(path, CDrawContext::kPathStroked);

			path->forget();
		}
		break;
	case kDownN:
		{
			CRect rShadowTop = CRect(rect);
			rShadowTop.setHeight(ShadowHeightTop);
			oc->setFillColor(cClearColour3);
			oc->drawRect(rShadowTop,kDrawFilled);
			CRect rShadowMid = CRect(rect);
			rShadowMid .setHeight(rect.getHeight()-ShadowHeightTop);
			rShadowMid .offset(0.0, ShadowHeightTop);
			rShadowMid.inset(1.0,0.0);
			oc->setFillColor(cClearColour2);
			oc->drawRect(rShadowMid, kDrawFilled);
		}
		break;
	case kDownS:
		{
			oc->setFillColor(cDarkColour3);
			oc->drawRect(rect,kDrawFilledAndStroked);
			CRect rLight = CRect(0.0, 0.0, witdhB-witdhBside*2.0, heightB-ShadowHeightTop*0.5);
			rLight.offset(rect.left+witdhBside,0.0);
			oc->setFillColor(cDarkColour2);
			oc->drawRect(rLight,kDrawFilled);
			CRect rShadowTop = CRect(rect);
			rShadowTop.setHeight(ShadowHeightTop);
			oc->setFillColor(cDarkColour4);
			oc->drawRect(rShadowTop,kDrawFilled);
		}
		break;
	}
};
//------------------------------------------------------------------------
#else // flat keys
void Keyboard::drawKey (COffscreenContext* oc, const CRect& rect, bool playing, int iType)
{
	if(iType < 0) return;

	if(playing)
	{
		switch(iType)
		{
		case kUpC: { iType = kDownC; } break;
		case kUpD: { iType = kDownD; } break;
		case kUpE: { iType = kDownE; } break;
		case kUpN: { iType = kDownN; } break;
		case kUpS: { iType = kDownS; } break;			
		}
	}

	oc->setFillColor(cClearColour1);

	const CCoord BottomX = rect.getBottomRight().x;
	const CCoord TopY = rect.getTopLeft().y;
	const CCoord LeftX = rect.getBottomLeft().x;
	const CCoord MidX2 = rect.getBottomRight().x-witdhBhalf;
	const CCoord MidX1 = rect.getBottomLeft().x+witdhBhalf;

	switch(iType)
	{
	case kUpC:
		{
			CGraphicsPath* path = oc->createGraphicsPath ();
			path->beginSubpath (rect.getBottomLeft());
			path->addLine(rect.getBottomRight());
			path->addLine(CPoint(BottomX, TopY+heightB));
			path->addLine(CPoint(MidX2, TopY+heightB));
			path->addLine(CPoint(MidX2, TopY));
			path->addLine(rect.getTopLeft());
			path->closeSubpath();

			oc->drawGraphicsPath(path, CDrawContext::kPathFilledEvenOdd);
			oc->drawGraphicsPath(path, CDrawContext::kPathStroked);

			path->forget();
			buildZones (rect);
		}
		break;
	case kUpD:
		{
			CGraphicsPath* path = oc->createGraphicsPath ();
			path->beginSubpath (rect.getBottomLeft());
			path->addLine(rect.getBottomRight());
			path->addLine(CPoint(BottomX, TopY+heightB));
			path->addLine(CPoint(MidX2, TopY+heightB));
			path->addLine(CPoint(MidX2, TopY));
			path->addLine(CPoint(MidX1, TopY));
			path->addLine(CPoint(MidX1, TopY+heightB));
			path->addLine(CPoint(LeftX, TopY+heightB));
			path->closeSubpath();

			oc->drawGraphicsPath(path, CDrawContext::kPathFilledEvenOdd);
			oc->drawGraphicsPath(path, CDrawContext::kPathStroked);

			path->forget();
			buildZones (rect);
		}
		break;
	case kUpE:
		{
			CGraphicsPath* path = oc->createGraphicsPath ();
			path->beginSubpath (rect.getBottomLeft());
			path->addLine(rect.getBottomRight());
			path->addLine(rect.getTopRight());
			path->addLine(CPoint(MidX1, TopY));
			path->addLine(CPoint(MidX1, TopY+heightB));
			path->addLine(CPoint(LeftX, TopY+heightB));
			path->closeSubpath();

			oc->drawGraphicsPath(path, CDrawContext::kPathFilledEvenOdd);
			oc->drawGraphicsPath(path, CDrawContext::kPathStroked);

			path->forget();
			buildZones (rect);
		}
		break;
	case kUpN:
		{
			oc->drawRect(rect,kDrawFilledAndStroked);
			buildZones (rect);
		}
		break;
	case kUpS:
		{
			oc->setFillColor(cDarkColour2);
			oc->drawRect(rect,kDrawFilledAndStroked);
			buildZones (rect);
		}
		break;
	case kDownC:
		{
			oc->setFillColor(cClearColour2);
			CGraphicsPath* path = oc->createGraphicsPath ();
			path->beginSubpath (rect.getBottomLeft());
			path->addLine(rect.getBottomRight());
			path->addLine(CPoint(BottomX, TopY+heightB));
			path->addLine(CPoint(MidX2, TopY+heightB));
			path->addLine(CPoint(MidX2, TopY));
			path->addLine(rect.getTopLeft());
			path->closeSubpath();

			oc->drawGraphicsPath(path, CDrawContext::kPathFilledEvenOdd);
			oc->drawGraphicsPath(path, CDrawContext::kPathStroked);

			path->forget();
		}
		break;
	case kDownD:
		{
			oc->setFillColor(cClearColour2);
			CGraphicsPath* path = oc->createGraphicsPath ();
			path->beginSubpath (rect.getBottomLeft());
			path->addLine(rect.getBottomRight());
			path->addLine(CPoint(BottomX, TopY+heightB));
			path->addLine(CPoint(MidX2, TopY+heightB));
			path->addLine(CPoint(MidX2, TopY));
			path->addLine(CPoint(MidX1, TopY));
			path->addLine(CPoint(MidX1, TopY+heightB));
			path->addLine(CPoint(LeftX, TopY+heightB));
			path->closeSubpath();

			oc->drawGraphicsPath(path, CDrawContext::kPathFilledEvenOdd);
			oc->drawGraphicsPath(path, CDrawContext::kPathStroked);

			path->forget();
		}
		break;
	case kDownE:
		{
			oc->setFillColor(cClearColour2);
			CGraphicsPath* path = oc->createGraphicsPath ();
			path->beginSubpath (rect.getBottomLeft());
			path->addLine(rect.getBottomRight());
			path->addLine(rect.getTopRight());
			path->addLine(CPoint(MidX1, TopY));
			path->addLine(CPoint(MidX1, TopY+heightB));
			path->addLine(CPoint(LeftX, TopY+heightB));
			path->closeSubpath();

			oc->drawGraphicsPath(path, CDrawContext::kPathFilledEvenOdd);
			oc->drawGraphicsPath(path, CDrawContext::kPathStroked);

			path->forget();
		}
		break;
	case kDownN:
		{
			oc->setFillColor(cClearColour2);
			oc->drawRect(rect,kDrawFilledAndStroked);
		}
		break;
	case kDownS:
		{
			oc->setFillColor(cDarkColour3);
			oc->drawRect(rect,kDrawFilledAndStroked);
		}
		break;
	}
};
#endif
