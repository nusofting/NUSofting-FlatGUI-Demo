#include "CLightLayer.h"

CLightLayer::CLightLayer(const CRect& size) :CGradientView (size)
{
	CGradientView::setGradientStyle (GradientStyle::kLinearGradient);	
	CGradientView::setRoundRectRadius (0.0);
	//CGradientView::setFrameColor (const CColor& newColor);
	CGradientView::setFrameWidth (0.0);
	CGradientView::setDrawAntialiased (false);	
	//CGradientView::setRadialCenter (const CPoint& center);
	//CGradientView::setRadialRadius (CCoord radius);

	setMouseEnabled(false);
	kTopColor = CColor(0,0,255,128);
	kBottomColor = CColor(0,0,0,128);

	preVar = 0;
	flip = false;
};

CLightLayer::~CLightLayer ()
{

}
void CLightLayer::setColours(double color1Start, double color2Start, const CColor& sTopColor, const CColor& sBottomColor, double angle, bool isAA)
{
	kTopColor = sTopColor;
	kBottomColor = sBottomColor;

	CGradient* m_gradient = CGradient::create (color1Start, color2Start, kTopColor, kBottomColor);
	if(m_gradient) 	CGradientView::setGradient (m_gradient);

	CGradientView::setGradientAngle (angle);
	CGradientView::setGradientStyle (GradientStyle::kLinearGradient);	
	CGradientView::setDrawAntialiased (isAA);	

	//setDirty();
}
//void CLightLayer::draw (CDrawContext *pContext)
//{
//	CDrawContext* drawContext = pContext;
//
//	CGradientView::draw(pContext);
//
//	//drawContext->setFillColor(kTopColor);
//	//drawContext->drawRect(getViewSize(), kDrawFilled);
//
//	setDirty(false);
//}