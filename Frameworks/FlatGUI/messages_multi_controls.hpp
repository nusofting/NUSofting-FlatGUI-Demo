#pragma warning( disable : 4244)//conversion from 'double' to 'float'
#pragma once

#include "vstgui/vstgui.h"
#include "VSTGUI-extension/Dialogs.h" 
#include "FlatGUI/editor_colours.hpp"
#include "Platform/Platform.h"
#include "vstgui/lib/platform/iplatformbitmap.h"

//-----------------------------------------------------------------------------
//  MessAgeBox() routine
//------------------------------------------------------------------------

class cCustomDialogue : public CView// for About box and other passive messages
{
public:
	CView* newCopy () const { return static_cast<CView*>(new cCustomDialogue(*this)); }

	cCustomDialogue(const CRect& size, const char* Msg1, const char** Msglines, int LinesToDraw,
	                const char* infoFileName = 0, CView* saveModalView = 0) 
		: CView (size), m_saveModalView(saveModalView)
	{
		msg1=Msg1;
		typedef char* pChar;
		msgLines = new pChar[LinesToDraw];
		for(int i = 0; i< LinesToDraw; ++i) // all rows
		{
			msgLines[i] = new char[1024];
			if(Msglines[i]) memcpy(msgLines[i], Msglines[i], sizeof(char)*1024);
		}

		linesToDraw = LinesToDraw;

		bitmap = 0;

		if (infoFileName)
		{
			readPath = Platform::getPathForAppResource(infoFileName);
		}
	}

	virtual	void createBitmapDisplay(const char* m_path_to_img)
	{
		std::string absolutePath = Platform::getPathForAppResource(m_path_to_img);
		IPlatformBitmap* tempBmp = IPlatformBitmap::createFromPath (absolutePath.c_str());
		if(bitmap == 0) 
		{
				bitmap = new CBitmap(tempBmp);
				bitmap->setPlatformBitmap(tempBmp);
		}
	}

	virtual ~cCustomDialogue()
	{ 
		if(msg1) msg1 = NULL; 
		if(msgLines){
			for(int i = 0; i< linesToDraw; ++i) // all rows
			{
				delete[] msgLines[i];
			}
			delete[] msgLines;
			msgLines = NULL;
		}
		if(bitmap){
			bitmap->forget();
			bitmap = 0;
		}
	}
	virtual void draw(CDrawContext *pContext)
	{
		pContext->setFrameColor(MyColours::sea);
		pContext->setFontColor(MyColours::yellow);
		pContext->setFont (kNormalFontVeryBig, 14);
		pContext->setLineWidth(2);

		pContext->setFillColor(MyColours::azure);
		pContext->drawRect(size,kDrawFilledAndStroked); 

		const CCoord scaleX = size.getWidth()/25;
		const CCoord scaleY = size.getHeight()/25;

		const CCoord  X1 = size.getTopLeft ().x;
		const CCoord  Y1 = size.getTopLeft ().y;
		const CCoord  X2 = size.getBottomRight ().x;
		const CCoord  Y2 = size.getBottomRight ().y;

		CRect box( 
			X1 + 2.3*scaleX,
			Y1 + 3.9*scaleY,
			X2 - 2.3*scaleX,
			Y2 - 3.9*scaleY		
			);	

		const CCoord  X1b = box.getTopLeft ().x;
		const CCoord  Y1b = box.getTopLeft ().y; 
		const CCoord  X2b = box.getBottomRight ().x;
		const CCoord  Y2b = box.getBottomRight ().y;
		const CCoord  boxWidth = X2b-X1b;

		pContext->setFillColor(MyColours::sea);
		pContext->drawRect(box,kDrawFilledAndStroked);

		if(msg1)
		{
			char title[1024] = {0};
			strcat(title, "[   ");
			strcat(title, msg1);
			strcat(title, "   ]");


			box.top = Y1b+scaleY;
			box.bottom = box.top+scaleY;

			pContext->drawString (title, box);
		}
		box.offset(scaleX,2*scaleY);

		for(int i = 0; i< linesToDraw; ++i) // all rows
		{
			if(!msgLines[i]) goto img; //here below
			box.offset(0,scaleY); 
			pContext->drawString (msgLines[i], box, kLeftText);
		}	

		if(readPath.length() > 1)
		{
			UTF8StringPtr LinkLabel = "Click here to open the HTML manual in your browser.";
			const CCoord side = (boxWidth-pContext->getStringWidth(LinkLabel))/2.0;
			readLink (X1+side, Y2-8.0*scaleY, X2-side, Y2-7.0*scaleY);
			pContext->setLineWidth(1);
			pContext->setFrameColor(MyColours::yellow);
			pContext->drawRect(readLink);
			pContext->drawString (LinkLabel, readLink, kCenterText);
		}

img:		if(bitmap){
			const CCoord varXbmp = bitmap->getWidth();
			const CCoord varYbmp = bitmap->getHeight();
			CRect area(size);			
			pContext->drawBitmap(bitmap, area);
		}

		CRect ok(
			X1b + boxWidth*0.5 - scaleX,
			Y2b - scaleY,
			X1b + boxWidth*0.5 + scaleX,
			Y2b		
			);

		if(bitmap){ ok.offset(0.0,2.9*scaleY); }

		pContext->setFillColor(MyColours::red);
		pContext->drawRect(ok,kDrawFilledAndStroked);
		ok.offset(0,2);
		pContext->drawString ("OK", ok);

		setDirty(false);
	}

	virtual	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons)
	{
		if (buttons & kLButton)
		{
			if (getFrame ())
			{
				if(readLink.pointInside(where) && readPath.length() > 1)
				{
					Platform::launchBrowserWithLocalFile(readPath.c_str());

					return kMouseDownEventHandledButDontNeedMovedOrUpEvents;		
				}
				else				
				{
					((VSTGUI::CView*)this)->looseFocus ();
					CFrame* frame = getFrame();
					frame->setModalView(NULL);
					frame->setModalView(m_saveModalView);
					return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
				}
			}			
		}
		return kMouseEventNotHandled;
	}



private:

	const char* msg1;
	char** msgLines;
	int linesToDraw;
	CBitmap* bitmap;
	std::string readPath;
	CRect readLink;
	CView* m_saveModalView;


};

static bool MessAgeBox(CFrame* pOwner = NULL, const char* msg1 = NULL, const char** msgLines = NULL, int LinesToDraw = 0, const char* infoFileName = 0)
{	
	if(pOwner)
	{
		CRect ThisRect = pOwner->getViewSize();

		CView* saveModalView = pOwner->getModalView();
		cCustomDialogue* tempBox = new cCustomDialogue(ThisRect, msg1, msgLines, LinesToDraw, infoFileName, saveModalView);
		if(tempBox)
		{
			// VSTGUI only allows one modal view at a time. We have saved the current one (if any) for cCustomDialogue to restore,
			// so blank out the one VSTGUI may be currently showing (if none, this does no harm).
			pOwner->setModalView(0);
			pOwner->setModalView(tempBox);
			//tempBox->setDirty(); not needed
			((VSTGUI::CView*)tempBox)->takeFocus (); //not needed and seems to cause a crash in views update
			return true;
		}
		else
			return false;
	}

	return false;
};

static bool MessAgePic(CFrame* pOwner = NULL, const char* picPath = 0)
{	
	if(pOwner)
	{
		CRect ThisRect = pOwner->getViewSize();

		CView* saveModalView = pOwner->getModalView();
		cCustomDialogue* tempBox = new cCustomDialogue(ThisRect, 0, 0, 0, 0, saveModalView);
		if(tempBox && picPath)
		{
			tempBox->createBitmapDisplay(picPath);
			// VSTGUI only allows one modal view at a time. We have saved the current one (if any) for cCustomDialogue to restore,
			// so blank out the one VSTGUI may be currently showing (if none, this does no harm).
			pOwner->setModalView(0);
			pOwner->setModalView(tempBox);
			//tempBox->setDirty(); not needed
			((VSTGUI::CView*)tempBox)->takeFocus (); //not needed and seems to cause a crash in views update
			return true;
		}
		else
			return false;
	}

	return false;
};

class cMessageDialogue : public CView, public IDependency // for "save" and "overwrite" options
{
public:
	cMessageDialogue(const CRect& size, const char* Msg1, IdStringPtr yesAnswer = 0, CBaseObject *notifyReceiver = 0) 
		: CView (size)
	{
		msg1=Msg1;
		mYesAnswer = yesAnswer;
		iTheAnswer = 0;
		if (notifyReceiver)
		{
			// Add as a dependency of this message dialogue, an object to receive the notify()
			// function call telling us whether the user pressed "Yes" or "No" in the dialogue.
			// If the user pressed "Yes", then we send the message stored in mYesAnswer.
			addDependency(notifyReceiver);
		}
	}

	/// @{
	///	Message constants used in VSTGUI's IDependency interface. Since these are constants,
	/// the receiving code is permitted to just compare against the addresses of the constants,
	/// no string comparison is needed. Arguably we don't even need any contents for the strings.
	/// Note that normally KAnswerYes is not used, rather the code using this class specifies
	/// (via the constructor) its own message string for this class to send when the user clicks
	/// on the YES button.
	static IdStringPtr kAnswerYes;
	static IdStringPtr kAnswerNo;
	/// @}

	virtual void draw(CDrawContext *pContext)
	{
        CRect newSize = CRect(size);
        newSize.setHeight(size.getHeight()+2.0);
        //pContext->setClipRect(newSize);
        
		pContext->setFrameColor(MyColours::sea);
		pContext->setFontColor(MyColours::yellow);
		pContext->setFont (kNormalFontVeryBig, 16);
		pContext->setLineWidth(2);

		pContext->setFillColor(MyColours::azure);
		pContext->drawRect(size,kDrawFilledAndStroked); 

		const CCoord scaleX = size.getWidth()/25;
		const CCoord scaleY = size.getHeight()/25;

		const CCoord  X1 = size.getTopLeft ().x;
		const CCoord  Y1 = size.getTopLeft ().y;
		const CCoord  X2 = size.getBottomRight ().x;
		const CCoord  Y2 = size.getBottomRight ().y;

		CRect box( 
			X1 + 2.3*scaleX,
			Y1 + 3.9*scaleY,
			X2 - 2.3*scaleX,
			Y2 - 3.9*scaleY		
			);	

		pContext->setFillColor(MyColours::sea);
		pContext->drawRect(box,kDrawFilledAndStroked);

		char title[1024] = {0};
		strcat(title, "[   ");
		strcat(title, msg1);
		strcat(title, "   ]");

		box.top = box.getTopLeft ().y+scaleY;
		box.bottom = box.top+scaleY;

		pContext->drawString (title, box);

		if(mYesAnswer)
		{
			pContext->setFillColor(MyColours::red);
			rYes = CRect(box); rYes.setWidth(2.0*scaleX);
			rYes.offset(5.3*scaleX, 5.3*scaleY); 
			rNo = CRect(rYes);
			rNo.offset(5.0*scaleX ,0);
			pContext->drawRect(rYes,kDrawFilledAndStroked);
			pContext->drawRect(rNo,kDrawFilledAndStroked);
			pContext->drawString("YES",rYes);
			pContext->drawString("NO",rNo);
		}

        //pContext->resetClipRect();
		setDirty(false);
	}

	int getAnswer()
	{
		return iTheAnswer;
	}

	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons)
	{
		if (buttons & kLButton)
		{	
			bool noChoice = (mYesAnswer == 0); // If we are not asking a question, a click anywhere dismisses.
			bool yesPressed = false;
			bool noPressed = false;
			if(!noChoice)
			{
				yesPressed = rYes.pointInside(where);
				noPressed = rNo.pointInside(where);
			}
			if(yesPressed)
			{
				iTheAnswer = 1;

				this->setVisible(false); //hidden but not destroyed yet
				changed(mYesAnswer);
			}
			else if(noPressed || noChoice)
			{
				iTheAnswer = 0;

				this->setVisible(false); //hidden but not destroyed yet
				getParentView()->setDirty(); // back to the save dialogue
				changed(kAnswerNo);
			}
			return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
		}
		return kMouseEventNotHandled;
	}

	
private:
	const char* msg1;
	IdStringPtr mYesAnswer;
	int iTheAnswer;
	CRect rYes, rNo;
};

class cTimedResponse : public CView // for "save" and "overwrite" response
{
public:
	cTimedResponse (const CRect& size, const char* Msg1, size_t MS = 10) 
		: CView (size)
	{
		strncpy(msg1, Msg1, 512);

		timer = new CVSTGUITimer (this, 500); // max speed allowed, 500 millisecods per tick
		timer->start ();
	
		ms = MS; //  10 ticks and then self close
	}

	~cTimedResponse()
	{
        if(timer)
        {
            timer->stop();
            timer = 0;
            //timer->forget();
        }
    
        if (getFrame ()) getFrame ()->setModalView(NULL);
	}		

	virtual void draw(CDrawContext *pContext)
	{
		pContext->setFrameColor(MyColours::sea);
		pContext->setFontColor(MyColours::yellow);
		pContext->setFont (kNormalFontVeryBig, 16);
		pContext->setLineWidth(2);

		pContext->setFillColor(MyColours::azure);
		pContext->drawRect(size,kDrawFilledAndStroked); 

		const CCoord scaleX = size.getWidth()/25;
		const CCoord scaleY = size.getHeight()/25;

		const CCoord  X1 = size.getTopLeft ().x;
		const CCoord  Y1 = size.getTopLeft ().y;
		const CCoord  X2 = size.getBottomRight ().x;
		const CCoord  Y2 = size.getBottomRight ().y;

		CRect box( 
			X1 + 1.4*scaleX,
			Y1 + 3.9*scaleY,
			X2 - 1.4*scaleX,
			Y2 - 3.9*scaleY		
			);	

        pContext->setClipRect(box);
		pContext->setFillColor(MyColours::sea);
		pContext->drawRect(box,kDrawFilledAndStroked);

		char title[1024] = {0};
		strcat(title, "[   ");
		strcat(title, msg1);
		strcat(title, "   ]");

		char MS[32] = {0};
		sprintf(MS, " %d" , ms);

		strcat(title, MS); 

		box.top = box.getTopLeft ().y+scaleY;
		box.bottom = box.top+scaleY*2.0;

		pContext->drawString (title, box);

		rNumber(box.left,box.top,box.right,box.bottom);
		
        pContext->resetClipRect();
		setDirty(false);
	}

	//------------------------------------------------------------------------
	CMessageResult notify (CBaseObject* sender, IdStringPtr msg)
	{
		if (msg == CVSTGUITimer::kMsgTimer && timer)
		{
			ms--;

			if(ms&1) // halves refresh
			invalidRect(size); // much faster than setDirty() for the whole CView

			if(ms <= 0)
			{
				timer->stop();
				//getFrame()->removeView(this);
				if (getFrame ())
                {
                    getFrame ()->setModalView(NULL);
                }
			}

			return kMessageNotified;
		}
		return kMessageUnknown;
	}

	virtual	CMouseEventResult onMouseDown (CPoint& where, const CButtonState& buttons)
	{
		if (buttons & kLButton)
		{
			if (getFrame ())
			{
				timer->stop();
				timer = 0;
				((VSTGUI::CView*)this)->looseFocus (); //?
				getFrame ()->setModalView(NULL);
				//getFrame ()->removeView(this); wrong call if modal view
				return kMouseDownEventHandledButDontNeedMovedOrUpEvents;
			}			
		}
		return kMouseEventNotHandled;
	}



private:
	char msg1[512];
	CVSTGUITimer* timer; 
	CRect rNumber;
	int ms;
};
