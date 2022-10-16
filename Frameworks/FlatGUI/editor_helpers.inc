// include this after the others vstgui to have Types def

static void getStringPxs(CDrawContext *pContext, CRect& size, CFontRef MyFont, const char* display, float& stringWidth)
{
	CDrawContext* drawContext = pContext;
	int chars = strlen(display);

#if WINDOWS
	WCHAR tempdisplay [200];
	mbstowcs (tempdisplay, display, 200);
	Gdiplus::Font* WinFont = static_cast<Gdiplus::Font*>(MyFont->getPlatformFont());
	Gdiplus::Graphics* g = drawContext->getGraphics();
	Gdiplus::RectF INRect = Gdiplus::RectF (size.x,size.y,size.getWidth(),size.getHeight());
	Gdiplus::RectF OUTRect = Gdiplus::RectF (size.x,size.y,size.getWidth(),size.getHeight());
	g->MeasureString(tempdisplay, chars, WinFont, INRect, &OUTRect); 
	stringWidth = OUTRect.Width;		
#else
	CFontRef oldFont = pContext->getFont();
	oldFont->remember();
	pContext->setFont(MyFont,size.getHeight());
	stringWidth = pContext->getStringWidth(display);
	pContext->setFont(oldFont);
	oldFont->forget();
#endif

}
