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

#ifndef __vstkeycode__
#define __vstkeycode__

#ifndef __aeffectx__
// define VstKeyCode for non VST useage

//---Structure and enum used for keyUp/keyDown-----
struct VstKeyCode
{
	int32_t character;
	unsigned char virt;     // see enum VstVirtualKey
	unsigned char modifier; // see enum VstModifierKey
};

//---Used by member virt of VstKeyCode-------------
enum VstVirtualKey 
{
	VKEY_BACK = 1, 
	VKEY_TAB, 
	VKEY_CLEAR, 
	VKEY_RETURN, 
	VKEY_PAUSE, 
	VKEY_ESCAPE, 
	VKEY_SPACE, 
	VKEY_NEXT, 
	VKEY_END, 
	VKEY_HOME, 

	VKEY_LEFT, 
	VKEY_UP, 
	VKEY_RIGHT, 
	VKEY_DOWN, 
	VKEY_PAGEUP, 
	VKEY_PAGEDOWN, 

	VKEY_SELECT, 
	VKEY_PRINT, 
	VKEY_ENTER, 
	VKEY_SNAPSHOT, 
	VKEY_INSERT, 
	VKEY_DELETE, 
	VKEY_HELP, 
	VKEY_NUMPAD0, 
	VKEY_NUMPAD1, 
	VKEY_NUMPAD2, 
	VKEY_NUMPAD3, 
	VKEY_NUMPAD4, 
	VKEY_NUMPAD5, 
	VKEY_NUMPAD6, 
	VKEY_NUMPAD7, 
	VKEY_NUMPAD8, 
	VKEY_NUMPAD9, 
	VKEY_MULTIPLY, 
	VKEY_ADD, 
	VKEY_SEPARATOR, 
	VKEY_SUBTRACT, 
	VKEY_DECIMAL, 
	VKEY_DIVIDE, 
	VKEY_F1, 
	VKEY_F2, 
	VKEY_F3, 
	VKEY_F4, 
	VKEY_F5, 
	VKEY_F6, 
	VKEY_F7, 
	VKEY_F8, 
	VKEY_F9, 
	VKEY_F10, 
	VKEY_F11, 
	VKEY_F12, 
	VKEY_NUMLOCK, 
	VKEY_SCROLL,

	VKEY_SHIFT,
	VKEY_CONTROL,
	VKEY_ALT,

	VKEY_EQUALS
};

//---Used by member modifier of VstKeyCode---------
enum VstModifierKey
{
	MODIFIER_SHIFT     = 1<<0, // Shift
	MODIFIER_ALTERNATE = 1<<1, // Alt (also known as Option on Mac)
	MODIFIER_COMMAND   = 1<<2, // Command on Mac, unless swapped with Ctrl
	MODIFIER_CONTROL   = 1<<3  // Ctrl on PC and Mac, unless swapped with Command
};
#endif

#endif
