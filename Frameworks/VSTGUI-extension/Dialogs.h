//
//  Dialogs.h
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#pragma once

#include "vstgui/vstgui.h"

#include <string>
#include <vector>


class CSimpleLabel;

/// Simple class for obtaining a single piece of text from the user via a modal view.
class GetTextDialog : public CViewContainer, public IControlListener
{
public:
    GetTextDialog(const CRect& size, const char* actionName);
    ~GetTextDialog() { }
    virtual void valueChanged(CControl* pControl);
    void handleCancelPressed(float value);
    void handleActionPressed(float value);
    void handleTextChanged();
    void setText(const char* prompt, const char* initialText);
    void setTextListener(CBaseObject* listener) { m_textListener = listener; }
    const char* getText();

private:
    CBaseObject* m_textListener;
    CSimpleLabel* m_prompt;
    CTextEdit* m_text;
    CTextButton* m_actionButton;
    CTextButton* m_cancelButton;
    std::string m_originalText;
    bool m_textUpdated;
};


/// Simple class for a button that, when clicked, opens a modal dialog to
// obtaining a single piece of text from the user.
class TextDialogButton : public CSplashScreen
{
public:
    TextDialogButton(const CRect& size, IControlListener* listener, int32_t tag, const char* buttonText, GetTextDialog* splashView);
    ~TextDialogButton() { }
    void setText(const char* prompt, const char* initialText);
    const char* getText();
    void draw(CDrawContext*);
    CMessageResult notify(CBaseObject* sender, IdStringPtr message);
	virtual CMouseEventResult onMouseEntered (CPoint& where, const CButtonState& buttons);
	virtual CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons);
	void setViewSize(const CRect& size);
	void setDisplayArea(const CRect& rect); // for the splashView

	CLASS_METHODS(TextDialogButton, CSplashScreen)

private:
    GetTextDialog* m_dialog;
    CSimpleLabel* m_buttonText;
};


//==============================================================================

/// Abstract base class defining an interface for notifying another object of the
/// results of the dialog.
/// We can't use the VSTGUI listener interface for this purpose because dialogs
/// are CView / CViewContainer objects, and not CControl objects.
class DialogResultHandler
{
public:
    virtual ~DialogResultHandler() { }
    virtual void cancelPressed() = 0;
    virtual void actionPressed() = 0;
};


/// Class for presenting a modal window with a prompt, a text entry field, a
/// cancel button and multiple (currently at most 2) action buttons.
/// @todo This could be made much more general, but the sizing logic in VSTGUI
/// is a bit tricky and so I'm making a simple, limited implementation first.
class MultiActionDialog : public CViewContainer, public IControlListener
{
public:
    MultiActionDialog(const CRect& size, const char* actionName1, const char* actionName2 = 0);
    virtual ~MultiActionDialog() { }
//    void addAction(const char* actionName);
    void show(const char* prompt, const char* initialText);
    void setActionVisibility(size_t actionNum, bool isVisible);
    virtual void valueChanged(CControl* pControl);
    virtual bool handleActionPressed(size_t actionNum, float value) = 0;
    virtual void handleTextChanged() { }
    virtual void handleShow() { }
    void notifyCancel();
    void notifyActionResult();
    void setDialogResultHandler(DialogResultHandler* handler) { m_resultHandler = handler; }
    const char* getText();

private:
    DialogResultHandler* m_resultHandler;
    CSimpleLabel* m_prompt;
    CTextEdit* m_text;
    std::vector<CTextButton*> m_actionButtons;
    CTextButton* m_cancelButton;
};


/// Simple class for a button that, when clicked, opens a modal dialog to prompt
// the user for a choice of actions.
class MultiActionDialogButton : public CSplashScreen, public DialogResultHandler
{
public:
    MultiActionDialogButton(const CRect& size,
                            IControlListener* listener,
                            int32_t tag,
                            const char* buttonText,
                            MultiActionDialog* splashView);
    ~MultiActionDialogButton() { }
    void setText(const char* prompt, const char* initialText);
    /// Dialog result handlers
    /// @{
    virtual void cancelPressed();
    virtual void actionPressed();
    /// @}

    const char* getText();

    void draw(CDrawContext*);
    virtual CMouseEventResult onMouseEntered (CPoint& where, const CButtonState& buttons);
    virtual CMouseEventResult onMouseExited (CPoint& where, const CButtonState& buttons);
    virtual void setViewSize(const CRect& size, bool doInvalid = true); // Must match function signature in CView
    void setDisplayArea(const CRect& rect); // for the splashView
	void setDisplayFont(CFontDesc*);

    CLASS_METHODS(MultiActionDialogButton, CSplashScreen)

private:
    MultiActionDialog* m_dialog;
    CSimpleLabel* m_buttonText;
};

