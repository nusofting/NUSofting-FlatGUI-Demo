//
//  Popups.h
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#pragma once

#include "vstgui/vstgui.h"

#include <string>
#include <vector>


/// An extension of VSTGUI's COptionMenu that allows the text drawn for the selected menu item to be different from
/// the text displayed in the menu. This allows the UI to show glyphs that are unavailable in the standard system font
/// typically used in popup menus. (While both Windows and OS X allow menus to contain text drawn in arbitrary fonts,
/// the code involved in doing so is fairly involved, and VSTGUI doesn't natively support that.) In particular, this
/// class supports the use of custom fonts that may have application-specific glyphs in Unicode private use blocks.
class AltTextPopup : public COptionMenu
{
public:
    /// Constructor.
    ///
    /// Uses the same arguments as the VSTGUI superclass.
    AltTextPopup(const CRect& size,
                 IControlListener* listener,
                 int32_t tag,
                 CBitmap* background = 0,
                 CBitmap* bgWhenClick = 0,
                 const int32_t style = 0);

    /// Adds to the menu an item with alternate display text. A restriction of this subclass is that items must be added
    /// to the menu in the exact order that they will appear in the menu.
    ///
    /// @param titleForMenu
    ///     The text that will appear in the popup menu. It should only contain characters that have glyphs defined in
    ///     the standard system popup menu fonts.
    /// @param titleForDisplay
    ///     The text that will appear in control's rectangle when the item being added is selected. It can contain
    ///     whatever characters are supported by the font set for this control.
    /// @param itemFlags
    ///     The same as the argument of the same name in the original VSTGUI versions of this function.
    virtual CMenuItem* addEntry(UTF8StringPtr titleForMenu,
                                UTF8StringPtr titleForDisplay,
                                int32_t itemFlags = CMenuItem::kNoFlags);

    /// Adds to the menu an item without alternate display text. A restriction of this subclass is that items must be
    /// added to the menu in the exact order that they will appear in the menu.
    ///
    /// This is a convenience function for any menu items that can use the same text for the menu and for display.
    ///
    /// @param titleForMenuAndDisplay
    ///     The text that will appear in the popup menu and the display when selected. It should only contain characters
    ///     that have glyphs defined in the standard system popup menu fonts.
    /// @param itemFlags
    ///     The same as the argument of the same name in the original VSTGUI versions of this function.
    virtual CMenuItem* addEntry(UTF8StringPtr titleForMenuAndDisplay,
                                int32_t itemFlags = CMenuItem::kNoFlags);

    /// Overridden member function displays the currently selected value.
    virtual void drawPlatformText(CDrawContext* pContext, IPlatformString* string);

private:
    /// The container of alternate text titles to display for the selected menu item.
    std::vector<std::string> m_displayTitles;
};

