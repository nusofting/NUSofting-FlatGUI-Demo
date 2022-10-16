//
//  StringsViewer.h
//
//  Copyright 2016 NUSofting.
//

#pragma once

#include "vstgui/vstgui.h"

#include <string>
#include <vector>


namespace Platform
{
  class Logger;
}
class ScaledFontFactory;
class StringsViewerBackgoundView;


/// Encapsulates a view that covers the entire plugin window with a data browser that displays a list of strings.
class StringsViewer : public CViewContainer
{
public:
    StringsViewer(CFrame* frame,
                  const std::string& title,
                  Platform::Logger& logger,
                  ScaledFontFactory* scaledFontFactory);

    virtual CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons);

private:
    CFrame* m_frame;
    Platform::Logger& m_logger;
    StringsViewerBackgoundView* m_background;
    CDataBrowser* m_dataBrowser;
};

