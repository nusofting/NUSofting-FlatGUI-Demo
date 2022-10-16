//-----------------------------------------------------------------------------
// VST Plug-Ins SDK
// VSTGUI: Graphical User Interface Framework for VST plugins
//
// Version 4.3
//
//-----------------------------------------------------------------------------
// VSTGUI LICENSE
// (c) 2015, Steinberg Media Technologies, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "cfontwin32.h"
#include "../../cdrawcontext.h"
#include "win32support.h"

#if WINDOWS

#include "winstring.h"
#include "gdiplusdrawcontext.h"

namespace VSTGUI {

//-----------------------------------------------------------------------------
bool GdiPlusFont::getAllPlatformFontFamilies (std::list<std::string>& fontFamilyNames)
{
	GDIPlusGlobals::enter ();
	Gdiplus::InstalledFontCollection fonts;
	INT numFonts = fonts.GetFamilyCount ();
	if (numFonts > 0)
	{
		Gdiplus::FontFamily* families = ::new Gdiplus::FontFamily[numFonts];
		if (fonts.GetFamilies (numFonts, families, &numFonts) == Gdiplus::Ok)
		{
			WCHAR familyName[LF_FACESIZE];
			for (INT i = 0; i < numFonts; i++)
			{
				families[i].GetFamilyName (familyName);
				UTF8StringHelper str (familyName);
				fontFamilyNames.push_back (std::string (str));
			}
		}
		::delete [] families;
		GDIPlusGlobals::exit ();
		return true;
	}
	GDIPlusGlobals::exit ();
	return false;
}

//-----------------------------------------------------------------------------
static Gdiplus::Graphics* getGraphics (CDrawContext* context)
{
	GdiplusDrawContext* gpdc = dynamic_cast<GdiplusDrawContext*> (context);
	return gpdc ? gpdc->getGraphics () : 0;
}

//-----------------------------------------------------------------------------
static Gdiplus::Brush* getFontBrush (CDrawContext* context)
{
	GdiplusDrawContext* gpdc = dynamic_cast<GdiplusDrawContext*> (context);
	return gpdc ? gpdc->getFontBrush () : 0;
}
//-----------------------------------------------------------------------------
const Gdiplus::StringFormat *GdiPlusFont::stringFormat = 0;

//-----------------------------------------------------------------------------
GdiPlusFont::GdiPlusFont (const char* name, const CCoord& size, const int32_t& style)
: font (0)
, ascent (-1)
, descent (-1)
, leading (-1)
{
	gdiStyle = Gdiplus::FontStyleRegular;
	if (style & kBoldFace)
		gdiStyle |= Gdiplus::FontStyleBold;
	if (style & kItalicFace)
		gdiStyle |= Gdiplus::FontStyleItalic;
	if (style & kUnderlineFace)
		gdiStyle |= Gdiplus::FontStyleUnderline;
	if (style & kStrikethroughFace)
		gdiStyle |= Gdiplus::FontStyleStrikeout;

	WCHAR tempName [200];
	mbstowcs (tempName, name, 200);
	Gdiplus::FontFamily* family = 0;
	GdiPlusCustomFontRegistry *registry = GdiPlusCustomFontRegistry::getInstanceIfFontsRegistered ();
	if (registry && registry->isRegistered (name))
	{
		Gdiplus::PrivateFontCollection &collection = registry->getCollection ();
		family = registry->getFamily (name);
		font = ::new Gdiplus::Font (family, (Gdiplus::REAL)size, gdiStyle, Gdiplus::UnitPixel);
	}
	else
	{
		font = ::new Gdiplus::Font (tempName, (Gdiplus::REAL)size, gdiStyle, Gdiplus::UnitPixel);
		Gdiplus::FontFamily fontFamily;
		if (font->GetFamily (&fontFamily) == Gdiplus::Ok)
		{
			family = fontFamily.Clone ();
		}
	}
	if (font && family)
	{
		double emUnitsToPixels = (double)font->GetSize () / (double)family->GetEmHeight (gdiStyle);
		UINT16 cellAscent = family->GetCellAscent (gdiStyle);
		UINT16 cellDescent = family->GetCellDescent (gdiStyle);
		UINT16 lineSpacing = family->GetLineSpacing (gdiStyle);
		ascent = (double)cellAscent * emUnitsToPixels;
		descent = (double)cellDescent * emUnitsToPixels;
		// There appears to be two conflicting definitions of "leading" as a typographical term.
		// Wikipedia <https://en.wikipedia.org/wiki/Leading> uses the following definition:
		//
		//   In typography, leading /ˈlɛdɪŋ/ refers to the distance between the baselines of successive lines of type. 
		//
		// This also appears to be the interpretation Microsoft uses in GDI+, where they call it less ambiguously "line
		// spacing" (e.g. see https://msdn.microsoft.com/en-us/library/xwf9s90b(v=vs.110).aspx).
		// Core Text appears to define "leading" as the gap between lines.
		// The few places elsewhere in VSTGUI that call IPlatformFont::getLeading() appear to use the Core Text
		// definition, since they sum the ascent, descent and leading to calculate a line / glyph height.
		// Since we need VSTGUI to consistently use one definition or the other, this implementation needs to calculate
		// the line gap via the available font metrics.
		// The nearest approximation I can make is to subtract the ascent and descent from the line spacing. However,
		// some (all?) TTF fonts seem to have lineSpacing == ascent + descent, giving a leading of 0. I can't do
		// much when dealing with such limited (and misleading) font metrics available to GDI+.
		leading = lineSpacing * emUnitsToPixels - ascent - descent;
	}
	if (!stringFormat)
	{
		stringFormat = Gdiplus::StringFormat::GenericTypographic();
	}
}
//-----------------------------------------------------------------------------
GdiPlusFont::~GdiPlusFont ()
{
	if (font)
		::delete font;
}

//-----------------------------------------------------------------------------
void GdiPlusFont::drawString (CDrawContext* context, IPlatformString* string, const CPoint& point, bool antialias, CBaselineTxtAlign baseAlign)
{
	Gdiplus::Graphics* pGraphics = getGraphics (context);
	Gdiplus::Brush* pFontBrush = getFontBrush (context);
	const WinString* winString = dynamic_cast<const WinString*> (string);
	if (pGraphics && font && pFontBrush && winString)
	{
		GdiplusDrawScope drawScope (pGraphics, context->getAbsoluteClipRect (), context->getCurrentTransform ());
		// See discussion below in GdiPlusFont::getStringWidth() about the chosen text rendering hint and the use of
		// StringFormat.
		pGraphics->SetTextRenderingHint (antialias ? Gdiplus::TextRenderingHintAntiAlias : Gdiplus::TextRenderingHintSystemDefault);
		// Adjust the y coordinate of the point to draw at. GDI+ draws text relative to the top-left corner of the
		// cell defining each glyph in the font. The "native" alignment adjustment is the one in the original VSTGUI
		// code, while the "baseline" adjustment makes sure the text is drawn with the font's baseline positioned at
		// the specified point. This option makes the text position compatible with Core Text on the Apple platforms.
		// I'm not sure why anyone would want the text position to vary between the platforms, but keeping the "native"
		// alignment to be compatible with existing code that uses it.
		Gdiplus::REAL adjustmentY = 0.0;
		switch (baseAlign)
		{
			case kAlignNative:
				adjustmentY = 1.f - font->GetHeight (pGraphics->GetDpiY ());
				break;
			case kAlignBaseline:
				adjustmentY = -getAscent();
				break;
		}
		Gdiplus::PointF gdiPoint ((Gdiplus::REAL)point.x, (Gdiplus::REAL)point.y + adjustmentY);
		pGraphics->DrawString(winString->getWideString (), -1, font, gdiPoint, stringFormat, pFontBrush);
	}
}

//-----------------------------------------------------------------------------
CCoord GdiPlusFont::getStringWidth (CDrawContext* context, IPlatformString* string, bool antialias)
{
	CCoord result = 0;
	const WinString* winString = dynamic_cast<const WinString*> (string);
	if (winString)
	{
		Gdiplus::Graphics* pGraphics = context ? getGraphics (context) : 0;
		HDC hdc = 0;
		if (context == 0)
		{
			hdc = CreateCompatibleDC (0);
			pGraphics = ::new Gdiplus::Graphics (hdc);
		}
		if (pGraphics && font)
		{
			Gdiplus::PointF gdiPoint (0., 0.);
			Gdiplus::RectF resultRect;
			// GDI+ defaults to adding extra space before and after the string being measured.
			// Ways to correctly measure strings are in a a section titled "How to Display Adjacent Text" in a legacy
			// (Windows XP) article at: https://support.microsoft.com/en-us/kb/307208
			// The gist of it is:
			//
			// How to Display Adjacent Text
			//
			// Perhaps you want to display two strings side by side so that they appear as one string. You might want do
			// this if you are writing an editor, or are displaying text with a formatting change inside the paragraph.
			//
			// The default action of DrawString works against you when you display adjacent runs: First, the default
			// StringFormat object adds an extra 1/6 em at each end of each output; second, when grid fitted widths are
			// less than designed, the rendered string is allowed to contract from its measured size by up to an em. 
			//
			// To avoid these problems, do the following:
			//   Always pass MeasureString and DrawString a StringFormat object based on the typographic StringFormat
			//   (GenericTypographic).
			// -and-
			//    Set TextRenderingHint graphics to TextRenderingHintAntiAlias.
			//
			// These measures disable the extra 1/6 em added at the run ends, avoid the problems of grid fitting by
			// using anti-aliasing and sub-pixel glyph positioning, and result in perfectly scalable text. The result
			// may be a little gray at smaller sizes. To compensate for this, use the SetTextContrast function to darken
			// the anti-alias text.
			pGraphics->SetTextRenderingHint (antialias ? Gdiplus::TextRenderingHintAntiAlias : Gdiplus::TextRenderingHintSystemDefault);
			pGraphics->MeasureString (winString->getWideString (), -1, font, gdiPoint, stringFormat, &resultRect);
			result = (CCoord)resultRect.Width;
		}
		if (hdc)
		{
			::delete pGraphics;
			DeleteDC (hdc);
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
GdiPlusCustomFontRegistry* GdiPlusCustomFontRegistry::gInstance = 0;

//-----------------------------------------------------------------------------
GdiPlusCustomFontRegistry* GdiPlusCustomFontRegistry::getInstance ()
{
	GDIPlusGlobals::enter ();
	if (!gInstance)
	{
		gInstance = new GdiPlusCustomFontRegistry;
	}
	GDIPlusGlobals::exit ();
	return gInstance;
}

//-----------------------------------------------------------------------------
GdiPlusCustomFontRegistry* GdiPlusCustomFontRegistry::getInstanceIfFontsRegistered ()
{
	GdiPlusCustomFontRegistry* result = 0;
	if (gInstance && !gInstance->registeredFamilies.empty ())
	{
		result = gInstance;
	}
	return result;
}

//-----------------------------------------------------------------------------
GdiPlusCustomFontRegistry::GdiPlusCustomFontRegistry ()
{
	GDIPlusGlobals::enter ();
}

//-----------------------------------------------------------------------------
GdiPlusCustomFontRegistry::~GdiPlusCustomFontRegistry ()
{
	GDIPlusGlobals::exit ();
}
	
//-----------------------------------------------------------------------------
bool GdiPlusCustomFontRegistry::registerFont (UTF8StringPtr name, UTF8StringPtr fileName)
{
	UTF8StringHelper str (fileName);
    Gdiplus::Status res = customFontCollection.AddFontFile (str.getWideString ());
    bool registerSucceeded = res == Gdiplus::Ok;
    if (registerSucceeded)
    {
        // We get and save the font family associated with this font name for two reasons:
        // 1. The family is likely to be reused several times anyway, so this avoids the need to look it up by name
        //    several times over.
        // 2. I've seen strange problems when using the variant of the Gdiplus::Font constructor that takes a
        //    collection argument. The first font seems to be created successfully from the collection, but subsequent
        //    ones fail with an InvalidParameter error. This may be due to the Gdiplus::FontFamily wrapper object
        //    deleting the native family in its destructor. I may be misunderstanding the intended life span of these
        //    wrapper objects, but the GDI+ documentation is silent on such matters. Creating a font family directly
        //	  here seems to avoid this problem and is arguably a neater way to do things anyway.
        UTF8StringHelper nameUtf8 (name);
        Gdiplus::FontFamily* family = new Gdiplus::FontFamily (nameUtf8.getWideString (), &customFontCollection);
        registeredFamilies[std::string (name)] = family;
    }
    return registerSucceeded;
}
	
//-----------------------------------------------------------------------------
bool GdiPlusCustomFontRegistry::isRegistered (UTF8StringPtr name)
{
	return registeredFamilies.count (std::string (name)) != 0;
}

} // namespace

#endif // WINDOWS
