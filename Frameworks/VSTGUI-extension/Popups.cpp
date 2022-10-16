//
//  Popups.cpp
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "Popups.h"


//==============================================================================
//
// AltTextPopup implementation
//

AltTextPopup::AltTextPopup(const CRect& size,
                           IControlListener* listener,
                           int32_t tag,
                           CBitmap* background,
                           CBitmap* bgWhenClick,
                           const int32_t style)
 : COptionMenu(size, listener, tag, background, bgWhenClick, style)
{
}

CMenuItem* AltTextPopup::addEntry(UTF8StringPtr titleForMenu,
                                  UTF8StringPtr titleForDisplay,
                                  int32_t itemFlags)
{
    CMenuItem* newItem = COptionMenu::addEntry(titleForMenu, m_displayTitles.size(), itemFlags);
    m_displayTitles.push_back(titleForDisplay);
    return newItem;
}

CMenuItem* AltTextPopup::addEntry(UTF8StringPtr titleForMenuAndDisplay,
                                  int32_t itemFlags)
{
    return addEntry(titleForMenuAndDisplay, titleForMenuAndDisplay, itemFlags);
}

void AltTextPopup::drawPlatformText(CDrawContext* pContext, IPlatformString* string)
{
    int32_t selectedIndex = getCurrentIndex();
    COptionMenu::drawPlatformText(pContext, CString(m_displayTitles[selectedIndex].c_str()).getPlatformString());
}

