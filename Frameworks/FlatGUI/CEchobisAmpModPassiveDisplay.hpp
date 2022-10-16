// used as info in Echobis

#pragma once





class CEchobisAmpModPassiveDisplay: public CView
{
public:


	CEchobisAmpModPassiveDisplay(const CRect& size, bool isAmpGraph):CView(size)
	{
		CView::setMouseEnabled(false);
		fDepth = 1.0f;
		biVar = 0.5f;
		oneOVERdiv = 1.0f;
		colorLine = kWhiteCColor;
		bAmpMod = isAmpGraph;
	}
	void setDisplayColour(const CColor& colour)
	{
		colorLine = colour;
		setDirty();
	}
	void draw(CDrawContext *pContext)
	{
		const float pi__ = 3.14159265359f;

		CView::setMouseEnabled(false);
		pContext->setDrawMode(kAntiAliasing | kNonIntegralMode);		
		CRect drawRect = CRect(size);
		drawRect = drawRect.inset(1.0, 1.0);

		int w = int(drawRect.getWidth()+0.5);
		int h = int(drawRect.getHeight()+0.5);

		if(bAmpMod)
		{
			colorLine.alpha = uint8_t(255.0f*fDepth+0.5f);
			pContext->setFrameColor(colorLine);
			pContext->setLineWidth(0.1*size.getHeight());

			const float windowDepth2 = fDepth*0.5f*0.95;
			const float windowDepth1 = 1.0f-windowDepth2;

			std::vector<CPoint> thePs;
		

			for(int i = 0; i < w-1; ++i)
			{
				double sub = ((drawRect.getWidth()*oneOVERdiv)-1.0);
				if(sub < 1.0E-23)  sub = 1.0E-23; // fix quiet NaN

				const double window1 = cosf(2.0*pi__*double(i)/sub);
				const double window2 = cosf(2.0*pi__*(i+1.0)/sub);

				const float ampEnvTick1 = windowDepth1 - windowDepth2*window1;
				const float ampEnvTick2 = windowDepth1 - windowDepth2*window2;

				const double y1  = drawRect.bottom - ampEnvTick1*h;
				const double y2  = drawRect.bottom - ampEnvTick2*h;
				const double x1 = i+drawRect.left;
				const double x2 = x1+1.0;
				CPoint p1 = CPoint(x1, y1);
				CPoint p2 = CPoint(x2, y2);
				pContext->drawLine(p1, p2);

				thePs.push_back(p1);		
			}

			colorLine.alpha = uint8_t(128.0f*fDepth+0.5f);
			pContext->setFrameColor(colorLine);
			pContext->setLineWidth(0.056*size.getHeight());
			pContext->drawLine(drawRect.getBottomLeft(), drawRect.getBottomRight());

			for(double i = 1; i < w; i += 2.0)
			{
				pContext->drawLine(CPoint(i+drawRect.left, drawRect.bottom), thePs.at(i-1));
			}
		}
		else // time mod 
		{
			colorLine.alpha = uint8_t(fabs(255.0f*biVar+0.5f));
			pContext->setFrameColor(colorLine);
			pContext->setLineWidth(0.1*size.getHeight());

			const double ym = h * 0.5;

			for(int i = 0; i < w-1; ++i)
			{
				double sub = ((drawRect.getWidth()*oneOVERdiv)-1.0);
				if(sub < 1.0E-23)  sub = 1.0E-23; // fix quiet NaN

				const double window1 = 1.0-cosf(2.0*pi__*double(i)/sub);
				const double window2 = 1.0-cosf(2.0*pi__*(i+1.0)/sub);

				const double y1  = drawRect.top + ym - biVar*window1*ym; 
				const double y2  = drawRect.top + ym - biVar*window2*ym;
				const double x1 = i+drawRect.left;
				const double x2 = x1+1.0;
				CPoint p1 = CPoint(x1, y1);
				CPoint p2 = CPoint(x2, y2);		
				pContext->drawLine(p1, p2);
			}

			colorLine.alpha = uint8_t(fabs((48.0f+2.0f*255.0f)*biVar));
			pContext->setFrameColor(colorLine);	
			pContext->setFillColor(colorLine);

			CRect rNote(0.0, 0.0, 0.3*w, 0.4*h);
			pContext->drawEllipse(rNote.offset(drawRect.left, drawRect.top+ym-0.2*h), kDrawFilled);
			CPoint p1 = CPoint(rNote.right-0.1*size.getHeight(), rNote.getCenter().y);
			CPoint p2 = CPoint(rNote.right-0.1*size.getHeight(), rNote.getCenter().y-ym-1.0);
			pContext->drawLine(p1, p2);

			pContext->setLineWidth(0.056*size.getHeight());
			pContext->drawLine(drawRect.getBottomLeft().offset(0.0, - ym), drawRect.getBottomRight().offset(0.0, - ym));
		}

		setDirty(false);
	}

	void setDepth(float var, float div)
	{
		fDepth = var;
		biVar = 0.9*var-0.45f;
		oneOVERdiv = 1.0/div;
		setDirty();
	}

private:

	float fDepth;
	float biVar;
	float oneOVERdiv;
	bool bAmpMod;
	CColor colorLine;

	 
};
