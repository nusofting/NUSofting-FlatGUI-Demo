//
//  StringsViewer.cpp
//
//  Copyright 2016 NUSofting.
//

#include "FlatGUI/StringsViewer.h"
#include "FlatGUI/Controls.h"
#include "FlatGUI//editor_colours.hpp"

#include "FlatGUI/messages_multi_controls.hpp"
#include "vstgui/lib/platform/iplatformfont.h"
#include "Platform/Platform.h"


// Helper class that draws the background of the strings list.
class StringsViewerBackgoundView : public CView
{
public:
    StringsViewerBackgoundView(CFrame* frame, const std::string& title, ScaledFontFactory* scaledFontFactory);
    void calculateBoxes();
    const CRect &getContentBox() { return m_innerBox; }
    const CRect &getOkBox() { return m_okBox; }
    const CRect &getSaveBox() { return m_saveBox; }
    virtual void draw(CDrawContext *pContext);

private:
    CFrame* m_frame;
    const std::string m_title;
    ScaledFontFactory* m_scaledFontFactory;
    CRect m_outerBox;
    CRect m_titleBox;
    CRect m_innerBox;
    CRect m_okBox;
    CRect m_saveBox;
};

StringsViewerBackgoundView::StringsViewerBackgoundView(CFrame* frame,
                                                       const std::string& title,
                                                       ScaledFontFactory* scaledFontFactory)
 : CView(frame->getViewSize()),
   m_frame(frame),
   m_title(title),
   m_scaledFontFactory(scaledFontFactory)
{
    calculateBoxes();
}

void StringsViewerBackgoundView::calculateBoxes()
{
    const CCoord scaleX = size.getWidth()/25;
    const CCoord scaleY = size.getHeight()/25;

    const CCoord  X1 = size.getTopLeft().x;
    const CCoord  Y1 = size.getTopLeft().y;
    const CCoord  X2 = size.getBottomRight().x;
    const CCoord  Y2 = size.getBottomRight().y;

    CRect box( 
        X1 + 2.3*scaleX,
        Y1 + 3.9*scaleY,
        X2 - 2.3*scaleX,
        Y2 - 3.9*scaleY
        );  
    m_outerBox = box;

    const CCoord  X1b = m_outerBox.getTopLeft().x;
    const CCoord  Y1b = m_outerBox.getTopLeft().y; 
    const CCoord  X2b = m_outerBox.getBottomRight().x;
    const CCoord  Y2b = m_outerBox.getBottomRight().y;
    const CCoord  boxWidth = X2b-X1b;

    box.top = Y1b+scaleY;
    box.bottom = box.top+scaleY;
    m_titleBox = box;

    // Make room for two buttons of width 2*scaleX, separated by scaleX / 2
    CRect button(
        X1b + boxWidth*0.5 - 2.5*scaleX,
        Y2b - scaleY,
        X1b + boxWidth*0.5 - 0.5*scaleX,
        Y2b     
        );
    m_okBox = button;
    button.offset(3*scaleX, 0);
    m_saveBox = button;

    m_innerBox = CRect(box.left + scaleX, box.top + scaleY, box.right - scaleX, m_okBox.top - scaleY);
}

void StringsViewerBackgoundView::draw(CDrawContext *pContext)
{
    pContext->setFrameColor(MyColours::sea);
    pContext->setFontColor(MyColours::yellow);
    pContext->setFont(m_scaledFontFactory->getScaledSmallFont());
    pContext->setLineWidth(2);

    pContext->setFillColor(MyColours::azure);
    pContext->drawRect(size,kDrawFilledAndStroked); 

    pContext->setFillColor(MyColours::sea);
    pContext->drawRect(m_outerBox,kDrawFilledAndStroked);

    pContext->drawString(m_title.c_str(), m_titleBox);

    CRect ok = m_okBox;
    pContext->setFillColor(MyColours::red);
    pContext->drawRect(ok,kDrawFilledAndStroked);
    pContext->drawString("OK", ok);

    CRect save = m_saveBox;
    pContext->drawRect(save,kDrawFilledAndStroked);
    pContext->drawString("Save", save);

    setDirty(false);
}


//==============================================================================


/// Helper class that enhances VSTGUI's generic string list data source by measuring the maximum string width.
class StringsViewerDataSource : public GenericStringListDataBrowserSource
{
public:
    StringsViewerDataSource(const std::vector<std::string>& strings, ScaledFontFactory* scaledFontFactory);

    // From GenericStringListDataBrowserSource - reports the width of the widest string in order to correctly size
    // the horizontal scrollbar.
    virtual CCoord dbGetCurrentColumnWidth(int32_t index, CDataBrowser *browser);

private:
    std::vector<std::string> m_strings;
    CCoord m_columnWidth;
};

StringsViewerDataSource::StringsViewerDataSource(const std::vector<std::string>& strings,
                                                 ScaledFontFactory* scaledFontFactory)
 : GenericStringListDataBrowserSource(&m_strings),
   m_strings(strings),
   m_columnWidth(0)
{
    CFontDesc* font = scaledFontFactory->getScaledSmallFont();
    IFontPainter* fp = font->getFontPainter();
    for (size_t i = 0; i < m_strings.size(); ++i)
    {
        CCoord stringWidth = fp->getStringWidth(0, CString(m_strings[i].c_str()).getPlatformString());
        m_columnWidth = std::max<>(m_columnWidth, stringWidth);
    }
    setupUI(kTransparentCColor, MyColours::yellow, kGreyCColor, MyColours::sea, MyColours::sea, font);
}

CCoord StringsViewerDataSource::dbGetCurrentColumnWidth(int32_t index, CDataBrowser *browser)
{
    // Only 1 column in the strings viewer. Add a few pixels worth of slop so that there is always room for a small
    // space at the end of the widest string.
    return m_columnWidth + 5;
}

//==============================================================================

StringsViewer::StringsViewer(CFrame* frame,
                             const std::string& title,
                             Platform::Logger& logger,
                             ScaledFontFactory* scaledFontFactory)
 : CViewContainer(frame->getViewSize()),
   m_frame(frame),
   m_logger(logger)
{
    m_background = new StringsViewerBackgoundView(frame, title, scaledFontFactory);
    StringsViewerDataSource* dataSrc = new StringsViewerDataSource(logger.getLines(), scaledFontFactory);
    addView(m_background);
    CRect size(frame->getViewSize());
    CRect contentBox(m_background->getContentBox());
    static const CCoord kScrollbarWidth = 10;
    int32_t style = CDataBrowser::kVerticalScrollbar | CDataBrowser::kDontDrawFrame;
    if ((contentBox.getWidth() - kScrollbarWidth) < dataSrc->dbGetCurrentColumnWidth(0, 0))
    {
        // Only add a horizontal scrollbar when the maximum string width (calculated above)
        // is bigger than the viewable size of the content box that will display the strings
        // (viewable size means the area that will display the string, so exclude the vertical scrollbar).
        style |= CDataBrowser::kHorizontalScrollbar;
    }
    m_dataBrowser = new CDataBrowser(contentBox, dataSrc, style, kScrollbarWidth);
    addView(m_dataBrowser);
    dataSrc->forget();
    setTransparency(true);
    CColor scrollbarColour = MyColours::red;
    CScrollbar* vertSB = m_dataBrowser->getVerticalScrollbar();
    CScrollbar* horzSB = m_dataBrowser->getHorizontalScrollbar();
    vertSB->setScrollerColor(scrollbarColour);
    if (horzSB)
    {
        horzSB->setScrollerColor(scrollbarColour);
    }
}

CMouseEventResult StringsViewer::onMouseDown(CPoint& where, const CButtonState& buttons)
{
    if (buttons & kLButton)
    {
        if (!m_dataBrowser->hitTest(where, buttons))
        {
            CRect okBox(m_background->getOkBox());
            CRect saveBox(m_background->getSaveBox());
            if (saveBox.pointInside(where))
            {
                std::string result;
                CNewFileSelector *fileSelector = CNewFileSelector::create(m_frame, CNewFileSelector::kSelectSaveFile);
                fileSelector->setTitle("Save Log");
                fileSelector->setDefaultSaveName(Platform::buildMessagesFileName().c_str());
                CFileExtension txtExtension("Text file", "txt");
                fileSelector->setDefaultExtension(txtExtension);
                if (fileSelector->runModal() && fileSelector->getNumSelectedFiles() > 0)
                {
                    result = fileSelector->getSelectedFile(0);
                }
                fileSelector->forget();
                if (!result.empty())
                {
                    if (!m_logger.saveFile(result))
                    {
                        const char* lines[] = { "Could not save to selected file name" };
                        MessAgeBox(m_frame, "Error", lines, 1);
                    }
                }
            }
            else if (okBox.pointInside(where))
            {
                // Close view when clicked inside OK button
                looseFocus();
                m_frame->setModalView(NULL);
            }
            return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
        }
    }
    // The data browser needs to receive mouse clicks in order for the scrollbars to work properly, so make sure the
    // CViewContainer superclass also performs its standard mouse event handling.
    return CViewContainer::onMouseDown(where, buttons);
}
    
