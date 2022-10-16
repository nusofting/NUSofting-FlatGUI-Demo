//
//  Dialogs.h
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "Dialogs.h"

#include "FlatGUI/Controls.h"

//==============================================================================
//
// GetTextDialog implementation
//
// @todo Make this dialog look professional. This is current a proof of functionality only.
// Currently it divides the space up proportionally between the prompt, the
// text edit control and the buttons. This means the prompt and the buttons are
// giant sized. 
//
// Added attempt to improve look

GetTextDialog::GetTextDialog(const CRect &size, const char* actionName)
 : CViewContainer(size),
   m_textListener(0),
   m_textUpdated(false)
{

	const CCoord scale = 0.38;
	const CCoord width = scale*size.getWidth();
	const CCoord height = scale*size.getHeight();
    const CCoord kIndent = 0.05*width;
    const CCoord kDialogWidth = width;
    const CCoord kDialogHeight = height;
    const CCoord kControlHeight = kDialogHeight / 3;

    const CCoord leftMargin = kIndent;
    const CCoord rightMargin = kDialogWidth - kIndent;
    const CCoord buttonTopMargin = 2 * kControlHeight + kIndent;
    const CCoord buttonBottomMargin = kDialogHeight - kIndent;

	CRect rBackground(0, 0, kDialogWidth ,kDialogHeight);
	rBackground.offset(scale*width, scale*height);
	CViewContainer* m_Background = new CViewContainer(rBackground);
	m_Background->setBackgroundColor(MakeCColor(53, 53, 53, 255));
	addView(m_Background);

    CRect promptRect(leftMargin, kIndent, rightMargin, kControlHeight - kIndent);
    m_prompt = new CSimpleLabel(promptRect.offset(scale*width,2.0*scale*height));
    addView(m_prompt);

    CRect textRect(leftMargin, kControlHeight + kIndent, rightMargin, 2 * kControlHeight - kIndent);
    m_text = new CTextEdit(textRect.offset(scale*width,2.0*scale*height), this, -1, "");
	m_text->setBackColor(MakeCColor(188, 188, 188, 255));
	m_text->setFontColor(MakeCColor(35, 35, 35, 255));
	m_text->setStyle(VSTGUI::kNoFrame);
    addView(m_text);

    CRect actionButtonRect(leftMargin, buttonTopMargin, (kDialogWidth - kIndent) * 0.5, buttonBottomMargin);
    m_actionButton = new CTextButton(actionButtonRect.offset(scale*width,scale*height), this, -1, actionName);
    addView(m_actionButton);
	CFontRef fondId = m_actionButton->getFont();
	fondId->setSize(actionButtonRect.getHeight()*0.689);
	m_actionButton->setFont(fondId);

    CRect cancelButtonRect((kDialogWidth + kIndent) * 0.5, buttonTopMargin, rightMargin, buttonBottomMargin);
    m_cancelButton = new CTextButton(cancelButtonRect.offset(scale*width,scale*height), this, -1, "Cancel");
    m_cancelButton->setFont(fondId);
	addView(m_cancelButton);

	const CColor  azure = MakeCColor(40, 120, 255, 111);
	setBackgroundColor(azure);

    //setAlphaValue(0.75);
}

void GetTextDialog::valueChanged(CControl* pControl)
{
    if (pControl == m_cancelButton)
    {
        handleCancelPressed(pControl->getValue());
    }
    else if (pControl == m_actionButton)
    {
        handleActionPressed(pControl->getValue());
    }
    else if (pControl == m_text)
    {
        handleTextChanged();
    }
}

void GetTextDialog::handleCancelPressed(float value)
{
    // VSTGUI buttons have a strange design where you get 2 value change events
    // when the button is pressed: firstly the value is changed to 1, then back
    // to 0. We are only interested in the final event, to indicate the end of
    // the button press.
    if (value == 0.0)
    {
        if (m_textListener)
        {
            m_textListener->notify(this, 0);
        }
    }
}

void GetTextDialog::handleActionPressed(float value)
{
    // VSTGUI buttons have a strange design where you get 2 value change events
    // when the button is pressed: firstly the value is changed to 1, then back
    // to 0. We are only interested in the final event, to indicate the end of
    // the button press.
    if (value == 0.0)
    {
        if (m_textListener)
        {
            m_textListener->notify(this, m_textUpdated ? m_text->getText() : 0);
        }
    }
}

void GetTextDialog::handleTextChanged()
{
    m_textUpdated = strcmp(m_originalText.c_str(), m_text->getText()) != 0;
}

void GetTextDialog::setText(const char* prompt, const char* initialText)
{
    m_prompt->setString128(prompt);
    m_text->setText(initialText);
    m_originalText = initialText ? initialText : "";
    m_textUpdated = false;
}

const char* GetTextDialog::getText()
{
    return m_text->getText();
}


//==============================================================================
//
// TextDialogButton implementation
//

TextDialogButton::TextDialogButton(const CRect& size, IControlListener* listener, int32_t tag, const char* buttonText, GetTextDialog* splashView)
 : CSplashScreen(size, listener, tag, splashView),
   m_dialog(splashView)
{
    m_buttonText = new CSimpleLabel(size, true);
    m_buttonText->setString128(buttonText);
    m_dialog->setTextListener(this);
}

void TextDialogButton::setText(const char* prompt, const char* initialText)
{
    m_dialog->setText(prompt, initialText);
}

const char* TextDialogButton::getText()
{
    return m_dialog->getText();
}

void TextDialogButton::draw(CDrawContext* context)
{
    m_buttonText->draw(context);
}

CMessageResult TextDialogButton::notify(CBaseObject* sender, IdStringPtr message)
{
    if (sender == modalView)
    {
        unSplash();
        IControlListener* listener = getListener();
        if (message && listener)
        {
            listener->valueChanged(this);
        }
    }
    return kMessageNotified;
}

CMouseEventResult TextDialogButton::onMouseEntered (CPoint& where, const CButtonState& buttons) 
{
		m_buttonText->onMouseEntered (where,buttons);
		setDirty();
		return  kMouseEventHandled;
}

CMouseEventResult TextDialogButton::onMouseExited (CPoint& where, const CButtonState& buttons)
{
		m_buttonText->onMouseExited (where,buttons);
		setDirty();
		return  kMouseEventHandled;
}

void TextDialogButton::setViewSize(const CRect& size)
{
	m_buttonText->setViewSize(size);
}

void TextDialogButton::setDisplayArea(const CRect& rect)
{
	CSplashScreen::setDisplayArea(rect);

	m_dialog->setViewSize(rect);
}


//==============================================================================
//
// MultiActionDialog implementation
//

MultiActionDialog::MultiActionDialog(const CRect &size, const char* actionName1, const char* actionName2)
 : CViewContainer(size)
{
    const size_t numActions = (actionName1 != 0) + (actionName2 != 0);
    const CCoord xScale = 0.38;
    const CCoord yScale = 0.39;
    const CCoord width = xScale*size.getWidth(); // size of the whole box
    const CCoord height = yScale*size.getHeight();
    const CCoord kIndent = 0.05*width;
    const CCoord kVerticalSpace = 0.01*height;
    const CCoord kDialogWidth = width;
    const CCoord kDialogHeight = height;
    const CCoord kControlHeight = kDialogHeight / (3 + numActions);

    const CCoord leftMargin = kIndent;
    const CCoord rightMargin = width - kIndent;
    CCoord buttonTopMargin =  kVerticalSpace;
    CCoord buttonBottomMargin = kDialogHeight - kVerticalSpace;

    CRect rBackground(0, 0, kDialogWidth ,kDialogHeight);
    rBackground.offset(xScale*width, yScale*height);
    CViewContainer* m_Background = new CViewContainer(rBackground);
    m_Background->setBackgroundColor(MakeCColor(53, 53, 53, 255));
    addView(m_Background);

    CRect promptRect(leftMargin, kVerticalSpace, rightMargin, kVerticalSpace+kControlHeight/2);
    m_prompt = new CSimpleLabel(promptRect.offset(xScale*width,yScale*height));
    addView(m_prompt);

	CRect textRect(promptRect); 
	m_text = new CTextEdit(textRect.offset(0,kControlHeight+kVerticalSpace), this, -1, "");
	m_text->setBackColor(MakeCColor(188, 188, 188, 255));
	m_text->setFontColor(MakeCColor(35, 35, 35, 255));
	m_text->getFont()->setStyle(kBoldFace);
	m_text->setFont(m_text->getFont());
	m_text->setStyle(VSTGUI::kNoFrame);
	addView(m_text);

	if (actionName1)
	{
		CRect actionButtonRect(promptRect); 
		CTextButton* actionButton = new CTextButton(actionButtonRect.offset(0,2.0*kControlHeight+kVerticalSpace), this, 0, actionName1);
		addView(actionButton);
		CFontRef fondId = actionButton->getFont();
		fondId->setSize(actionButtonRect.getHeight()*0.689);
		actionButton->setFont(fondId);
		m_actionButtons.push_back(actionButton);
		buttonTopMargin += kControlHeight;
		buttonBottomMargin += kControlHeight;
	}

	if (actionName2)
	{
		CRect actionButtonRect(promptRect); 
		CTextButton* actionButton = new CTextButton(actionButtonRect.offset(0,3.0*kControlHeight+kVerticalSpace), this, 1, actionName2);
		addView(actionButton);
		CFontRef fondId = actionButton->getFont();
		fondId->setSize(actionButtonRect.getHeight()*0.689);
		actionButton->setFont(fondId);
		m_actionButtons.push_back(actionButton);
		buttonTopMargin += kControlHeight;
		buttonBottomMargin += kControlHeight;
	}

	CRect cancelButtonRect(promptRect); 
	m_cancelButton = new CTextButton(cancelButtonRect.offset(0,4.0*kControlHeight+kVerticalSpace), this, -1, "Cancel");
	CFontRef fondId = m_cancelButton->getFont();
	m_cancelButton->setFont(fondId);
	addView(m_cancelButton);

    const CColor  azure = MakeCColor(40, 120, 255, 111);
    setBackgroundColor(azure);

    //setAlphaValue(0.75);
}

void MultiActionDialog::show(const char* prompt, const char* initialText)
{
    m_prompt->setString128(prompt);
    m_text->setText(initialText);
    handleShow();
}

void MultiActionDialog::setActionVisibility(size_t actionNum, bool isVisible)
{
    if (actionNum < m_actionButtons.size())
    {
        m_actionButtons[actionNum]->setVisible(isVisible);
    }
}

#if 0
void MultiActionDialog::addAction(const char* actionName)
{
    CRect actionButtonRect(leftMargin, buttonTopMargin, (kDialogWidth - kIndent) * 0.5, buttonBottomMargin);
    CTextButton* actionButton = new CTextButton(actionButtonRect.offset(scale*width,scale*height), this, -1, actionName);
    addView(actionButton);
    CFontRef fondId = actionButton->getFont();
    fondId->setSize(actionButtonRect.getHeight()*0.689);
    actionButton->setFont(fondId);
    m_actionButtons.push_back(actionButton);
}
#endif

void MultiActionDialog::valueChanged(CControl* pControl)
{
    if (pControl == m_text)
    {
        handleTextChanged();
    }
    else
    {
        // VSTGUI buttons have a strange design where you get 2 value change events
        // when the button is pressed: firstly the value is changed to 1, then back
        // to 0. We are only interested in the final event, to indicate the end of
        // the button press.
        float value = pControl->getValue();
        if (value == 0.0)
        {
            if (pControl == m_cancelButton)
            {
                notifyCancel();
            }
            else if (pControl == m_actionButtons[0] || pControl == m_actionButtons[1])
            {
                if (!handleActionPressed(pControl->getTag(), value))
                {
                    notifyCancel();
                }
            }
        }
    }
}

void MultiActionDialog::notifyCancel()
{
    if (m_resultHandler)
    {
        m_resultHandler->cancelPressed();
    }
}

void MultiActionDialog::notifyActionResult()
{
    if (m_resultHandler)
    {
        m_resultHandler->actionPressed();
    }
}

const char* MultiActionDialog::getText()
{
    return m_text->getText();
}


//==============================================================================
//
// MultiActionDialogButton implementation
//

MultiActionDialogButton::MultiActionDialogButton(const CRect& size,
                                                 IControlListener* listener,
                                                 int32_t tag,
                                                 const char* buttonText,
                                                 MultiActionDialog* splashView)
 : CSplashScreen(size, listener, tag, splashView),
   m_dialog(splashView)
{
    m_buttonText = new CSimpleLabel(size, true);
    m_buttonText->setString128(buttonText);
    m_dialog->setDialogResultHandler(this);
}

void MultiActionDialogButton::setText(const char* prompt, const char* initialText)
{
    m_dialog->show(prompt, initialText);
}

void MultiActionDialogButton::cancelPressed()
{
    unSplash();
}

void MultiActionDialogButton::actionPressed()
{
    unSplash();
    IControlListener* listener = getListener();
    if (listener)
    {
        // Prompt the listener to query the dialog for a result.
        listener->valueChanged(this);
    }
}

const char* MultiActionDialogButton::getText()
{
    return m_dialog->getText();
}

void MultiActionDialogButton::draw(CDrawContext* context)
{
    m_buttonText->draw(context);
}

CMouseEventResult MultiActionDialogButton::onMouseEntered (CPoint& where, const CButtonState& buttons) 
{
        m_buttonText->onMouseEntered (where,buttons);
        setDirty();
        return  kMouseEventHandled;
}

CMouseEventResult MultiActionDialogButton::onMouseExited (CPoint& where, const CButtonState& buttons)
{
        m_buttonText->onMouseExited (where,buttons);
        setDirty();
        return  kMouseEventHandled;
}

void MultiActionDialogButton::setViewSize(const CRect& size, bool doInvalid)
{
    CSplashScreen::setViewSize(size, doInvalid);
    m_buttonText->setViewSize(size, doInvalid);
}

void MultiActionDialogButton::setDisplayArea(const CRect& rect)
{
    CSplashScreen::setDisplayArea(rect);

	m_dialog->setViewSize(rect);
}

void MultiActionDialogButton::setDisplayFont(CFontDesc* asFont = 0)
{
	m_buttonText->setDisplayFont(asFont);
}
