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

#ifndef __iplatformfont__
#define __iplatformfont__

#include "../vstguifwd.h"
#include <list>

namespace VSTGUI {

//----------------------------
// @brief Text alignment of drawing point to font baseline
//----------------------------
enum CBaselineTxtAlign
{
    kAlignNative = 0,	///< Use the OS's native drawing position (GDI+: top-left, Core Text: font baseline)
    kAlignBaseline		///< Draw the text with the font's baseline set to the requested point
};
    
//-----------------------------------------------------------------------------
// IFontPainter Declaration
//! @brief font paint interface
//-----------------------------------------------------------------------------
class IFontPainter
{
public:
	virtual ~IFontPainter () {}

	virtual void drawString (CDrawContext* context, IPlatformString* string, const CPoint& p, bool antialias = true, CBaselineTxtAlign baseAlign = kAlignBaseline) = 0;
	virtual CCoord getStringWidth (CDrawContext* context, IPlatformString* string, bool antialias = true) = 0;
};

//-----------------------------------------------------------------------------
// IPlatformFont declaration
//! @brief platform font class
///
/// Encapsulation of a platform font. You should never need to call IPlatformFont::create(..), instead use CFontDesc::getPlatformFont().
//-----------------------------------------------------------------------------
class IPlatformFont : public CBaseObject
{
public:
	static IPlatformFont* create (UTF8StringPtr name, const CCoord& size, const int32_t& style);
	static bool getAllPlatformFontFamilies (std::list<std::string>& fontFamilyNames);
	
	virtual double getAscent () const = 0;		///< returns the ascent line offset of the baseline of this font. If not supported returns -1. Typically varies between GDI+ and Core Text due to them using different metrics headers in font files.
	virtual double getDescent () const = 0;		///< returns the descent line offset of the baseline of this font. If not supported returns -1. Typically consistent between platforms.
	virtual double getLeading () const = 0;		///< returns the space between lines for this font. If not supported returns -1. Typically varies between GDI+ and Core Text.
	virtual double getCapHeight () const = 0;	///< returns the height of the highest capital letter for this font. If not supported returns -1. Not available in GDI+.

	virtual IFontPainter* getPainter () = 0;
};

//-----------------------------------------------------------------------------
// ICustomFontRegistry declaration
//! @brief custom font registry class
///
/// Encapsulation of platform-specific facility to register custom fonts.
//-----------------------------------------------------------------------------
class ICustomFontRegistry : public CBaseObject
{
public:
	static ICustomFontRegistry* create ();
	
	/// Register a TTF or OTF font file to use as a custom font.
	/// @param name
	///		The family name of the font, used to identify the font in CFontDesc.
	/// @param fileName
	///		The path to actual TTF or OTF font file to register.
	/// @return
	/// 	Returns true if the font was successfully registered, otherwise returns false.
	/// @note
	///		Typically only TTF files work reliably in GDI+ in Windows.
	virtual bool registerFont (UTF8StringPtr name, UTF8StringPtr fileName) = 0;
};

}

#endif // __iplatformfont__
