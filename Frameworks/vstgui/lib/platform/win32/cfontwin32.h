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

#ifndef __cfontwin32__
#define __cfontwin32__

#include "../iplatformfont.h"

#if WINDOWS

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>

#include <map>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class GdiPlusFont : public IPlatformFont, public IFontPainter
{
public:
	GdiPlusFont (const char* name, const CCoord& size, const int32_t& style);

	Gdiplus::Font* getFont () const { return font; }

	static bool getAllPlatformFontFamilies (std::list<std::string>& fontFamilyNames);
protected:
	~GdiPlusFont ();
	
	double getAscent () const VSTGUI_OVERRIDE_VMETHOD { return ascent; }
	double getDescent () const VSTGUI_OVERRIDE_VMETHOD { return descent; }
	double getLeading () const VSTGUI_OVERRIDE_VMETHOD { return leading; }
	double getCapHeight () const VSTGUI_OVERRIDE_VMETHOD { return -1; } // Not available in GDI+, sadly

	IFontPainter* getPainter () VSTGUI_OVERRIDE_VMETHOD { return this; }

	void drawString (CDrawContext* context, IPlatformString* string, const CPoint& p, bool antialias = true, CBaselineTxtAlign baseAlign = kAlignBaseline) VSTGUI_OVERRIDE_VMETHOD;
	CCoord getStringWidth (CDrawContext* context, IPlatformString* string, bool antialias = true) VSTGUI_OVERRIDE_VMETHOD;

	Gdiplus::Font* font;
	INT gdiStyle;

private:
	double ascent;
	double descent;
	double leading;
	static const Gdiplus::StringFormat *stringFormat;
};

//-----------------------------------------------------------------------------
class GdiPlusCustomFontRegistry : public ICustomFontRegistry
{
public:
	/// Return the instance, creating it if necessary.
	static GdiPlusCustomFontRegistry* getInstance ();

	/// Return the instance only if the registry has been created and at least
	/// one custom font has been registered, otherwise return a null pointer.
	static GdiPlusCustomFontRegistry* getInstanceIfFontsRegistered ();

	virtual bool registerFont (UTF8StringPtr name, UTF8StringPtr fileName);
	bool isRegistered (UTF8StringPtr name);
	Gdiplus::PrivateFontCollection& getCollection () { return customFontCollection; }
	Gdiplus::FontFamily* getFamily (UTF8StringPtr name) { return registeredFamilies[name]; }

private:
	GdiPlusCustomFontRegistry ();
	~GdiPlusCustomFontRegistry ();
	
	static GdiPlusCustomFontRegistry* gInstance;
	Gdiplus::PrivateFontCollection customFontCollection;
	std::map<std::string, Gdiplus::FontFamily*> registeredFamilies;
};

} // namespace

#endif // WINDOWS

#endif
