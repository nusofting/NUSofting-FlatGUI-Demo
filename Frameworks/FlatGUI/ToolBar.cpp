/*-----------------------------------------------------------------------------

(c) 2015 nusofting.com - Liqih

ToolBar.cpp
Written by Luigi Felici and Bernie Maier
Copyright (c) 2015 NUSofting

-----------------------------------------------------------------------------*/

#include "Core/EditorCore.h" // Due to bad design in the VST SDK and VSTGUI headers, we need to include this before ToolBar.h
#include "ToolBar.h"

#include "FlatGUI/messages_multi_controls.hpp"

#include "AppearanceManager/AppearanceManager.h"
#include "FlatGUI/Controls.h"
#include "FlatGUI/StringsViewer.h"
#include "FlatGUI/CRandWidget.h"
#include "Platform/Platform.h"
#include "PresetManager/PresetManager.h"
#include "PresetManager/Program.h"
#include "VSTGUI-extension/Dialogs.h"


#include <cassert>

static size_t kRandomizer = 11111111; // to be shared with plugin


// menu font size scaling
#define menuFontTB 12

class SaveDialog : public MultiActionDialog
{
public:
    SaveDialog(const CRect& size, PresetManager& presetManager);
    virtual ~SaveDialog() { }
    virtual void handleShow();
    virtual bool handleActionPressed(size_t actionNum, float value);
    bool handleSaveNewPressed(size_t actionNum);
    bool handleOverwritePressed(size_t actionNum);
    virtual CMessageResult notify(CBaseObject* sender, IdStringPtr message);
    bool overwriteCurrentProgram() { return m_overwriteCurrentProgram; }

private:
	static IdStringPtr kOverwrite;
	static IdStringPtr kDuplicateName;

	PresetManager& m_presetManager;
	bool m_overwriteCurrentProgram;
};

IdStringPtr SaveDialog::kOverwrite = "Overwrite";
IdStringPtr SaveDialog::kDuplicateName = "DuplicateName";

SaveDialog::SaveDialog(const CRect& size, PresetManager& presetManager)
: MultiActionDialog(size, "Save new user preset", "Overwrite current preset with same name"),
   m_presetManager(presetManager),
   m_overwriteCurrentProgram(false)
{
}

void SaveDialog::handleShow()
{
	m_overwriteCurrentProgram = false;
	Program *program = m_presetManager.getCurrentProgram();
	setActionVisibility(1, !program->isFactoryPreset() && !program->isUnsavedChunk());
}

bool SaveDialog::handleActionPressed(size_t actionNum, float value)
{
	bool handled = false;
	if (actionNum == 0)
	{
		handled = handleSaveNewPressed(actionNum);
	}
	else if (actionNum == 1)
	{
		handled = handleOverwritePressed(actionNum);
	}
	return handled;
}

bool SaveDialog::handleSaveNewPressed(size_t actionNum)
{
	Program *program = m_presetManager.getCurrentProgram();
	const char* newName = getText();
	if (!program->isUnsavedChunk() && !program->isFactoryPreset() && program->getName().compare(newName) == 0)
	{
		cMessageDialogue* temptell = new cMessageDialogue(getFrame()->getViewSize(),
														  "Name hasn't changed, overwrite current preset?",
														  kOverwrite, this);
		this->addView(temptell);
	}
	else if (m_presetManager.userPresetNameExists(newName))
	{
		// The user has entered the name of an existing preset, but we have already checked
		// that they've changed the name, so it must be for a different preset. Since
		// duplicated names in the same bank could be confusing, we tell the user that they
		// can't use an existing name.
		cMessageDialogue* temptell = new cMessageDialogue(getFrame()->getViewSize(),
														  "Name already used for different user preset. "
														  "Please choose another name.");
		this->addView(temptell);
	}
	else
	{
		notifyActionResult();
	}
	return true;
}

bool SaveDialog::handleOverwritePressed(size_t actionNum)
{
	Program *program = m_presetManager.getCurrentProgram();
	// We can only overwrite user presets, so it's a logic error in the code if
	// we get here with a factory preset. And since it's a logic error, a user
	// message is inappropriate; we need to catch this at development time.
	assert(!program->isFactoryPreset());
	if (program->isFactoryPreset())
	{
		fprintf(stderr, "Expecting a user preset here, not factory / project preset %s\n", program->getName().c_str());
		return false;
	}
	m_overwriteCurrentProgram = true;
	notifyActionResult();
	return true;
}

CMessageResult SaveDialog::notify(CBaseObject* sender, IdStringPtr message)
{
	CMessageResult result = (message == cMessageDialogue::kAnswerNo) ? kMessageNotified : kMessageUnknown;
	if (message == kOverwrite || message == kDuplicateName)
	{
		m_overwriteCurrentProgram = message == kOverwrite;
		notifyActionResult();
		result = kMessageNotified;
	}
	return result;
}


//==============================================================================

ToolBar::ToolBar(PresetManager& presetManager,
                 EditorToPluginCoreInterface& notifyPluginCore,
                 ToolBarListener& listener,
	        	 AppearanceManager& appearanceManager,
	        	 Platform::Logger& errorLog)
 : m_presetManager(presetManager),
   m_notifyPluginCore(notifyPluginCore),
   m_listener(listener),
   m_appearanceManager(appearanceManager),
   m_presetIsDirty(false),
   m_Settings(0),
   m_errorLog(errorLog), 
   m_aboutText(0),
   bActivateLoadAndResaveYamlsOnMenu(false),
   bFastOverwriteFactoryPreset(false),
   sFileSelectorHint(0)
{

}

ToolBar::~ToolBar()
{
	if(m_aboutText) delete m_aboutText;
}

void ToolBar::addBanksToMenu() 
{
	std::vector<std::string> categories(m_presetManager.getCategories());
	for (size_t i = 0; i < categories.size(); ++i)
	{
	    Programs programs = m_presetManager.getProgramsInCategory(categories[i].c_str());
	    if (!programs.empty())
	    {
			m_Banks->addEntry(categories[i].c_str());
	    }
	}
	pKickPreviousNext1->setMax(float(m_Banks->getNbEntries()-1));
	if (!m_presetManager.allPresetFilesValid())
	{
		// Add a dummy menu item with a warning message.
		// Make it disabled so the user can't select it, they can only see it.
		m_Banks->addSeparator();
		CMenuItem* messageItem = m_Banks->addEntry("Some preset files are missing");
		messageItem->setEnabled(false);
	}
}

void ToolBar::addPresetsToMenu()
{
	CMenuItem* currentBankItem = m_Banks->getCurrent();
	const char* category = currentBankItem->getTitle();
    Programs programs = m_presetManager.getProgramsInCategory(category);

	for (size_t i = 0; i < programs.size(); ++i)
	{
		m_Presets->addEntry(programs[i]->getName().c_str());
	}
	pKickPreviousNext2->setMax(float(m_Presets->getNbEntries()-1));
	if (!m_presetManager.allPresetFilesValid())
	{
		// Add a dummy menu item with a warning message.
		// Make it disabled so the user can't select it, they can only see it.
		m_Presets->addSeparator();
		CMenuItem* messageItem = m_Presets->addEntry("Some preset files are missing");
		messageItem->setEnabled(false);
	}
}


void ToolBar::CreateToolBarView(CFrame* newFrame,
                                float scaleGUI,
                                ScaledFontFactory* scaledFontFactory,
                                ToolBarOptions& options)
{		

	if (!m_aboutText)
    {
        m_aboutText = new AboutBoxText(options.m_name,
                                       options.m_version,
                                       options.m_copyright,
                                       options.m_extraLine,
                                       options.m_infoFileName);
    }

	factGUIScaling = 1.0f;
	strcpy(infoFileName_, m_aboutText->m_infoFileName);
	factGUIScaling =scaleGUI;

    CRect frameSize = newFrame->getViewSize();
	m_frame = newFrame;
	m_scaledFontFactory = scaledFontFactory;
	CFontDesc* bigFont = scaledFontFactory->getScaledBigFont();

	///// objects for the toolbar /////

	/* elements from left to right
	- the "?" menu
	- compare button (+label below)
	- save button
	- banks menu (+kick buttons below)
	- presets menu (+kick buttons below)
	- extra space (rand widget or other tools)
	- open HTML button
	- appearances menu
	- zoom buttons

	>>> widths and X positions expressed as scaleGUI or frame width factors 
	*/
		const CCoord width = frameSize.getWidth();
		const CCoord settingsWidth = options.m_rMainRects.m_rSettings.getWidth();
		const CCoord compareWidth = options.m_rMainRects.m_rCompare.getWidth();
		const CCoord banksWidth = options.m_rMainRects.m_rBanks.getWidth();
		const CCoord presetsWidth = options.m_rMainRects.m_rPresets.getWidth();
		const CCoord GapX = width*0.014;

		CRect rCompareDesc(options.m_rMainRects.m_rCompare);
		rCompareDesc.offset(0, options.m_rMainRects.m_rCompare.getHeight()+3*scaleGUI); 

		CRect rBanksButtons(CPoint(0,0),CPoint(banksWidth, 12*scaleGUI));
		rBanksButtons.offset(options.m_rMainRects.m_rSave.right+GapX, 32*scaleGUI);

		CRect rPresetsButtons = CRect(CPoint(0,0),CPoint(presetsWidth, 12*scaleGUI));
		rPresetsButtons.offset(options.m_rMainRects.m_rBanks.right, 32*scaleGUI);

	// About, links, etc...
	m_Settings = new COptionMenu(options.m_rMainRects.m_rSettings,this,options.m_settingsOptionsTag,0,0,kNoFrame|kNoDrawStyle|kNoTextStyle);
	if (options.m_settingsMenuOptions & SMOF_WEB_SITE_LINK)
	{
		CMenuItem* item = m_Settings->addEntry("nusofting.com");
		item->setTag(SMOF_WEB_SITE_LINK);
	}
	if (options.m_settingsMenuOptions & SMOF_ABOUT)
	{
		CMenuItem* item = m_Settings->addEntry("about and help");
		item->setTag(SMOF_ABOUT);
	}
	if (options.m_settingsMenuOptions & SMOF_EXPLORE_PRESETS)
	{
		CMenuItem* item = m_Settings->addEntry("explore user presets");
		item->setTag(SMOF_EXPLORE_PRESETS);
	}
	if (options.m_settingsMenuOptions & SMOF_EXPLORE_THEMES)
	{
		CMenuItem* item = m_Settings->addEntry("explore user appearance themes");
		item->setTag(SMOF_EXPLORE_THEMES);
	}
	if (options.m_settingsMenuOptions & SMOF_RELOAD_THEMES)
	{
		CMenuItem* item = m_Settings->addEntry("reload appearance themes");
		item->setTag(SMOF_RELOAD_THEMES);
	}
	if (options.m_settingsMenuOptions & SMOF_CONVERT_PRESETS)
	{
		CMenuItem* item = m_Settings->addEntry(options.bActivateLoadAndResaveYamlsOnMenu? "convert presets": "convert old preset / bank file");
		item->setTag(SMOF_CONVERT_PRESETS);
	}
	if (options.m_settingsMenuOptions & SMOF_DISPLAY_ERRORS)
	{
		if (!m_presetManager.allPresetFilesValid() || !m_appearanceManager.allAppearanceFilesValid())
		{
			CMenuItem* item = m_Settings->addEntry("open diagnostic log");
			item->setTag(SMOF_DISPLAY_ERRORS);
		}
	}
	
	bFastOverwriteFactoryPreset = options.bFastOverwriteFactoryPreset;  // true only in dev Windows builds.
	bActivateLoadAndResaveYamlsOnMenu = options.bActivateLoadAndResaveYamlsOnMenu;
	sFileSelectorHint = options.m_FileSelectorHint;

	m_Settings->setBackColor(MakeCColor(32,32,32,0));
	m_frame->addView (m_Settings);

	// Clickable |compare|
	m_compare = new CSimpleLabel(options.m_rMainRects.m_rCompare,true);
	m_compare->setString128("compare");
	m_compare->setDisplayFont(scaledFontFactory->getScaledSmallFont());
	m_compare->setListener(this);
	m_frame->addView(m_compare);

	// Compare description
	m_compareDesc = new CSimpleLabel(rCompareDesc);
	m_compareDesc->setString128("current"); 
	m_compareDesc->setDisplayFont(scaledFontFactory->getScaledSmallFont());
	m_compareDesc->setMouseEnabled(false);
	m_compareDesc->setFontColourOnly(MakeCColor(162,162,162,255));
	m_frame->addView(m_compareDesc);

	if(bFastOverwriteFactoryPreset)  // true only in dev Windows builds.
	{
		CRect rHere = rCompareDesc;
		rHere.offset(rCompareDesc.getWidth()*1.2, -4.0);
		m_FastOverwrite = new CSimpleLabel(rHere, true);
		m_FastOverwrite->setString128("overwrite"); 
		m_FastOverwrite->setDisplayFont(scaledFontFactory->getScaledSmallFont());
		m_FastOverwrite->setFontColourOnly(MakeCColor(0,0,255,255));
		m_FastOverwrite->setListener(this);
		m_frame->addView(m_FastOverwrite);
	}

	// Save preset dialog and the button that shows the dialog
	CRect rDialog(frameSize);
	m_saveDialog = new SaveDialog(rDialog, m_presetManager);
	m_saveDialogButton = new MultiActionDialogButton(options.m_rMainRects.m_rSave, this, -1, "save", m_saveDialog);
	m_saveDialogButton->setDisplayFont(scaledFontFactory->getScaledSmallFont());
	m_frame->addView(m_saveDialogButton);

	// Buttons
	pKickPreviousNext1 = new CKickPreviousNext(rBanksButtons,this,-1);
	m_frame->addView(pKickPreviousNext1);
	pKickPreviousNext2 = new CKickPreviousNext(rPresetsButtons,this,-1);
	m_frame->addView(pKickPreviousNext2);

	//Banks
	m_Banks = new COptionMenu(options.m_rMainRects.m_rBanks,this,-1,0,0,kCheckStyle|kNoFrame);	

	addBanksToMenu();
	m_Banks->setCurrent(0);
	m_Banks->setBackColor(MakeCColor(32,32,32,0));
	m_Banks->setFontColor(MakeCColor(192, 192, 192, 255));
	m_Banks->setFont(bigFont);
	m_frame->addView (m_Banks);

	//Presets
	m_Presets = new COptionMenu(options.m_rMainRects.m_rPresets,this,-1,0,0,kCheckStyle|kNoFrame);

	addPresetsToMenu();
	m_Presets->setCurrent(0);
	m_Presets->setBackColor(MakeCColor(32,32,32,0));
	m_Presets->setFontColor(MakeCColor(192, 192, 192, 255));
	m_Presets->setFont(bigFont); // same as for m_Banks
	m_frame->addView (m_Presets);

	// Clickable  browser launcher
	m_LaunchHTMLFile = new CSimpleLabel(options.m_rMainRects.m_rHelp);
	m_LaunchHTMLFile->setString128("open help doc in browser");
	m_LaunchHTMLFile->usePopUp(true);
	m_LaunchHTMLFile->setDisplayFont(scaledFontFactory->getScaledSmallFont());
	m_LaunchHTMLFile->setFontColourOnly(MakeCColor(129, 129, 129, 214));
	m_LaunchHTMLFile->setListener(this);
	m_LaunchHTMLFile->setMouseEnabled(true);
	m_frame->addView(m_LaunchHTMLFile);

	// Colours // reminder: this entries must match those in handleColourSelected()
	m_Colours = new COptionMenu(options.m_rMainRects.m_rColours,this,-1,0,0,kCheckStyle|kNoFrame|kNoDrawStyle|kNoTextStyle);
	size_t numColourThemes = m_appearanceManager.numThemes();
	for (size_t i = 0; i < numColourThemes; ++i)
	{
		m_Colours->addEntry(m_appearanceManager.getAppearanceName(i).c_str());
	}
	if (!m_appearanceManager.allAppearanceFilesValid())
	{
		// Add a dummy menu item with a warning message.
		// Make it disabled so the user can't select it, they can only see it.
		m_Colours->addSeparator();
		CMenuItem* messageItem = m_Colours->addEntry("Some appearance files are missing");
		messageItem->setEnabled(false);
	}
	m_Colours->setBackColor(MakeCColor(32,32,32,0));
	m_frame->addView (m_Colours);

	if(options.m_rMainRects.m_rRandWidget.getHeight() > 0.0)
	{
		m_PickRand = new CRandWidget (options.m_rMainRects.m_rRandWidget , this, *m_scaledFontFactory);
		m_PickRand->setTag(kRandomizer);
		m_frame->addView (m_PickRand);
	}


	// Zoom
	CRect rZoomOptions(frameSize);
	rZoomOptions.left = frameSize.right-46*scaleGUI;
	rZoomOptions.right = frameSize.right-2*scaleGUI;
	rZoomOptions.top = 9.0f*scaleGUI;
	rZoomOptions.bottom = rZoomOptions.top+18.0f*scaleGUI;
	m_SetupZoomOptions = new CKickTwiceZoom(rZoomOptions,this,options.m_zoomOptionsTag);
	m_SetupZoomOptions->setMax(5.0f);
	m_SetupZoomOptions->setMin(0.0f);
	m_frame->addView (m_SetupZoomOptions);

#if FALSE // set TRUE to debug positions
	m_Settings->setFrameColor(MakeCColor(255,32,32,255));
	m_Settings->setStyle(2); // draw frame
	m_compare->setFrameColor(MakeCColor(255,32,32,255));
	m_compare->setStyle(2); // draw frame
	m_compareDesc->setFrameColor(MakeCColor(255,32,32,255));
	m_compareDesc->setStyle(2); // draw frame
	m_Banks->setFrameColor(MakeCColor(255,32,32,255));
	m_Banks->setStyle(2); // draw frame
	m_Presets->setFrameColor(MakeCColor(255,32,32,255));
	m_Presets->setStyle(2); // draw frame
	labelZoom->setFrameColor(MakeCColor(255,32,32,255));
	labelZoom->setStyle(2); // draw frame
	m_Colours->setFrameColor(MakeCColor(255,32,32,255));
	m_Colours->setStyle(2); // draw frame
#endif
};

void ToolBar::setInitialSelections(float zoomValue)
{
    int32_t colourIndex = m_appearanceManager.getCurrentAppearanceIndex();
	// Colours
	if (colourIndex >= m_Colours->getNbEntries())
	{
		// If the colour index saved in the user preferences was somehow corrupted,
		// or we ship a new version with fewer colours, default to the first colour.
		colourIndex = 0;
	}
	m_Colours->setValue(colourIndex);
	// Fake the user selection of colour. But calls one more time textureFrame();
	handleColourSelected();

	// Zoom
	m_SetupZoomOptions->setValue(zoomValue);

	size_t categoryIndex = m_presetManager.getCurrentCategoryIndex();
	size_t programIndexInCategory = m_presetManager.getCurrentProgramIndexInCategory();
	programChanged(categoryIndex, programIndexInCategory);
};

void ToolBar::valueChanged(CControl* pControl)
{

	if(pControl == pKickPreviousNext1)
	{
		m_Banks->setValue(pKickPreviousNext1->getValue());
		valueChanged(m_Banks); // now let the preset menu handler actually change the bank
	}
	else if(pControl == pKickPreviousNext2)
	{
		m_Presets->setValue(pKickPreviousNext2->getValue());
		valueChanged(m_Presets); // now let the preset menu handler actually change the program
	}
	else if(pControl == m_Banks)
	{
		m_Presets->removeAllEntry();
		addPresetsToMenu();
		m_Presets->setValue(0.0); // VSTGUI is dumb and doesn't reset the current index when removing all entries!
		valueChanged(m_Presets); // now let the preset menu handler actually change the program (and trigger a redraw)
	}
	else if(pControl == m_Presets)
	{
		int32_t selectedCategoryIndex = m_Banks->getCurrentIndex();
		int32_t selectedProgramInCategoryIndex = m_Presets->getCurrentIndex();
		m_presetManager.selectProgram(selectedCategoryIndex, selectedProgramInCategoryIndex);
		resyncToolbarPresetControls();
		// Force the plugin interface to realise the program has been changed.
		m_notifyPluginCore.programChanged();

		// needed to do not garble the controls covered by the menu, but it will slow down the update a little because
		// it forces a full redraw
		m_frame->invalidRect(m_frame->getViewSize());
	}
	else if(pControl == m_compare)
	{
		Program* currentProgram = m_presetManager.getCurrentProgram();
		if (currentProgram->getGetValueSetType() == Program::VALUE_INITIAL)
		{
			m_compareDesc->setString128("current");
			currentProgram->selectValueSet(Program::VALUE_CURRENT);
		}
		else
		{
			m_compareDesc->setString128("original");
			currentProgram->selectValueSet(Program::VALUE_INITIAL);
		}
		m_notifyPluginCore.paramValueSetChanged();
	}
	else if(pControl == m_Settings)
	{
		CMenuItem* selectedItem = m_Settings->getCurrent();
		switch(int(selectedItem->getTag()))
		{
		case SMOF_WEB_SITE_LINK:
			Platform::launchBrowser("http://nusofting.com/");
			break;
		case SMOF_ABOUT:
		
			if (m_aboutText)
			{
				/// *** Warning! Code change
				int numLines = 0;
				const char** lines = m_aboutText->getLines(numLines);
				MessAgeBox(m_frame, "Credits", lines , numLines, m_aboutText->m_infoFileName);
				/* Warning! Using this way 0therwiser the first call returned numLines == 0 **/
			}
			else
			{
				MessAgeBox(m_frame, "Credits", 0, 0);
			}
			break;
		case SMOF_EXPLORE_PRESETS:
			Platform::exploreDirectory(Platform::getPresetsDirectory(true).c_str());
			break;
		case SMOF_EXPLORE_THEMES:
			Platform::exploreDirectory(Platform::getAppearanceDirectory(true).c_str());
			break;
		case SMOF_RELOAD_THEMES:
			handleAppearanceReload();
			break;
		case SMOF_CONVERT_PRESETS:
			if(bActivateLoadAndResaveYamlsOnMenu)
				handleLoadAndAutosaveYamlsToUser();
			else
				handleConvertOldPresetsBank();
			break;
		case SMOF_DISPLAY_ERRORS:
			{
				StringsViewer* stringsViewer = new StringsViewer(m_frame,
																 "[ Diagnostic log ]",
																 m_errorLog,
																 m_scaledFontFactory);
				m_frame->setModalView(stringsViewer);
				stringsViewer->forget();
			}
			break;
		default:
			break;
		}
	}
	else if(pControl == m_saveDialogButton)
	{
		float value = pControl->getValue();
		if (value == 1.0)
		{
			Program* program = m_presetManager.getCurrentProgram();
			std::string name(program->getName());
			const char saveStr[] = "Save ";
			const char editedStr[] = "edited ";
			const char factoryStr[] = "factory ";
			const char userStr[] = "user ";
			const char presetAsStr[] = "preset as ";
			const char presetNamedStr[] = "preset named:";
			std::string message;
			message.reserve(sizeof saveStr + sizeof editedStr + sizeof factoryStr +
							sizeof presetAsStr + sizeof userStr + sizeof presetNamedStr);
			message.assign(saveStr);
			if (program->isDirty())
			{
				message.append(editedStr);
			}
			if (program->isFactoryPreset())
			{
				message.append(factoryStr).append(presetAsStr).append(userStr).append(presetNamedStr);
				name.append(" user edit");
			}
			else
			{
				message.append(userStr).append(presetAsStr).append(presetNamedStr);
			}
			m_saveDialogButton->setText(message.c_str(), name.c_str());
		}
		else
		{
			std::string message;
			if (m_saveDialog->overwriteCurrentProgram())
			{
				Program* program = m_presetManager.getCurrentProgram();
				const std::string& programName = program->getName();
				bool saved = m_presetManager.overwrite(program);
				if (saved)
				{
					const char presetStr[] = "Preset \"";
					const char overwrittenStr[] = "\" has been overwritten";
					message.reserve(sizeof presetStr + programName.size() + sizeof overwrittenStr);
					message.assign(presetStr);
					message.append(programName);
					message.append(overwrittenStr);
					m_presetIsDirty = false;
					resyncToolbarPresetControls();
				}
				else
				{
					const char failureStr[] = "Failed to save preset ";
					message.reserve(sizeof failureStr + programName.size());
					message.assign(failureStr);
					message.append(programName);
				}
			}
			else
			{
				const char* newName = m_saveDialog->getText();
				Program* oldProgram = m_presetManager.getCurrentProgram();
				Program* newProgram = m_presetManager.copyCurrentProgram(newName);
				bool saved = m_presetManager.saveNew(newProgram, Platform::getPresetsDirectory(true));
				if (saved)
				{
					const char presetStr[] = "New preset was saved with the name \"";
					const char endStr[] = "\"";
					message.reserve(sizeof presetStr + strlen(newName) + sizeof endStr);
					message.assign(presetStr);
					message.append(newName);
					message.append(endStr);
					// We've saved a new preset, now we should make it current, restore the original state
					// of the old preset and update the menu to include the new preset.
					m_presetManager.addProgram(newProgram, true);
					oldProgram->revert();
					// Ask the preset manager which category the program was copied into,
					// so we can select it and force a reload.
					size_t categoryIndex = m_presetManager.getCurrentCategoryIndex();
					if (m_Banks->getNbEntries() != m_presetManager.numCategories())
					{
						// The number of banks has changed which means the user category was added
						// by saving this program. We need to recreate the banks menu.
						m_Banks->removeAllEntry();
						addBanksToMenu();
					}
					m_Banks->setValue(categoryIndex);
					m_Presets->removeAllEntry();
					addPresetsToMenu();
					m_Presets->setValue(m_presetManager.getCurrentProgramIndexInCategory());
					valueChanged(m_Presets); // now let the preset menu handler actually change the program
				}
				else
				{
					const char failureStr[] = "Failed to copy preset ";
					message.reserve(sizeof failureStr + strlen(newName));
					message.assign(failureStr);
					message.append(newName);
				}
			}

			cTimedResponse*  pResponse = new cTimedResponse(m_frame->getViewSize(), message.c_str(), 5);
			m_frame->setModalView(pResponse); // high CPU load here
		}
	}
	else if(pControl == m_LaunchHTMLFile)
	{
		std::string readPath = Platform::getPathForAppResource(infoFileName_);
		Platform::launchBrowserWithLocalFile(readPath.c_str());
	}
	else if(pControl == m_Colours)
	{
		handleColourSelected();
	}
	else if(pControl == m_SetupZoomOptions)
	{
		m_listener.handleZoom(pControl->getValue()); // value between 0.0f and 7.0f

		CRect rMessage = m_frame->getViewSize();
		rMessage.inset(factGUIScaling*25.0,factGUIScaling*90.0);
		cTimedResponse*  pResponse = new cTimedResponse(rMessage, "You may need to *close and open again* the plugin window for proper display.", 5);
		m_frame->setModalView(pResponse); // high CPU load here
	}
	else if(pControl == m_PickRand)
	{		
		m_notifyPluginCore.changedDisplayParameterInt(kRandomizer, static_cast<int>(1000.0f*m_PickRand->getValueNormalized()+0.5f));
		//*** this above to call:
		// m_vst2Plugin->setParameterAutomated(index, value); is this needed for kRandomizer ? Looks like it's not is VST2 Host
		// notifyParameterChange(index, value, SRC_EDITOR); calls m_dsp.setParameter(index, value);
		//*** because of SRC_EDITOR it calls:
		//  m_dsp.setParameter(index, value);
		//  m_notifyEditor->refreshParameter(index, value); // @todo make safe when called in audio render thread

		m_notifyPluginCore.programChanged(); // this is enough for GUI animation in Windows 10
		//*** This above to call:
		//  setActiveProgram(m_presetManager.getCurrentProgram(), SRC_INTERNAL);
		//  m_vst2Plugin->updateDisplay(); yes: host display

		//*** The setActiveProgram()  calls:
		// notifyParameterChange(i, newProgram->getParameterValue(i), SRC_INTERNAL);
		// m_dsp.handleNewProgram(); not used in demoLibVST so far

		//*** because of SRC_INTERNAL notifyParameterChange() only calls :
		// m_dsp.setParameter(index, value); AGAIN?!

        
		m_frame->invalid(); // only needed on macOS for proper animation.
		//** alternative to m_frame->setDirty();
		// with also CView::kDirtyCallAlwaysOnMainThread  = true;
		// ** Because VSTGUI says:
		//virtual void setDirty (bool val = true);		///< set the view to dirty so that it is redrawn in the next idle. Thread Safe !
		//static bool kDirtyCallAlwaysOnMainThread;		///< if this is true, setting a view dirty will call invalid() instead of checking it in idle. Default value is false.

		// Hence invalid() is NOT Thread Safe ?
	}
	else if(m_FastOverwrite && pControl == m_FastOverwrite)
	{
		m_FastOverwrite->setString128("overwrite");

		Program* program = m_presetManager.getCurrentProgram();
		if(program->isDirty())
		{
			if (m_presetManager.overwrite(program))
			{
				resyncToolbarPresetControls();		
				m_FastOverwrite->setString128("done");
			}
			else
				m_FastOverwrite->setString128("oops");
			
		}

		m_FastOverwrite->invalid();
		m_notifyPluginCore.paramValueSetChanged();
	}

	checkIfPresetBecameDirty();
}

void ToolBar::checkIfPresetBecameDirty()
{
	// Check if anything in here or the main editor has dirtied the preset.
	// Only bother checking if we don't already know the preset is dirty!
	if (!m_presetIsDirty)
	{
		Program* currentProgram = m_presetManager.getCurrentProgram();
		bool presetIsDirty = currentProgram->isDirty();
		if (presetIsDirty != m_presetIsDirty)
		{
			m_presetIsDirty = presetIsDirty;
			resyncToolbarPresetControls();
		}
	}
}

void ToolBar::programChanged(size_t categoryIndex, size_t programInCategoryIndex)
{
	// The program change may be due to additional presets being loaded, so
	// we need to refresh the popup menus.
	m_Banks->removeAllEntry();
	addBanksToMenu();
	m_Banks->setValue(categoryIndex);
	m_Presets->removeAllEntry();
	addPresetsToMenu();
	m_Presets->setValue(programInCategoryIndex);
	resyncToolbarPresetControls();
}

void ToolBar::resyncToolbarPresetControls()
{
	Program* currentProgram = m_presetManager.getCurrentProgram();
	if (currentProgram->getGetValueSetType() == Program::VALUE_INITIAL)
	{
		m_compareDesc->setString128("original");
	}
	else
	{
		m_compareDesc->setString128("current");
	}
	m_presetIsDirty = currentProgram->isDirty();
	/// @todo Check this is working correctly - I'm seeing the compare button coloured when I think it shouldn't be.
	if (m_presetIsDirty)
	{
		m_compare->setFontColourOnly(MakeCColor(255,0,0,255)); 
	}
	else							// colours for any plugin
	{
		m_compare->setFontColourOnly(MakeCColor(102,102,104,255));
	}
	pKickPreviousNext1->setValue(m_Banks->getValue());
	pKickPreviousNext2->setValue(m_Presets->getValue());
}

void ToolBar::handleColourSelected()
{
	size_t colourIndex = m_Colours->getCurrentIndex();
	m_listener.setColours(colourIndex);
	// If the compare button has been coloured, we need to update the colour.
	//resyncToolbarPresetControls(); no more now, the ToolBar has its own colours fixed
};

void ToolBar::handleAppearanceReload()
{
	m_listener.requestApppearanceReload();
	m_Colours->removeAllEntry();
	size_t numColourThemes = m_appearanceManager.numThemes();
	for (size_t i = 0; i < numColourThemes; ++i)
	{
		m_Colours->addEntry(m_appearanceManager.getAppearanceName(i).c_str());
	}
	int32_t selectedThemeAfterReload = m_appearanceManager.getCurrentAppearanceIndex();
	// User appearance themes may have been added or removed as a result of the reload.
	// Therefore, the selected menu item index may have changed. Also, we cleared out all the old entries.
	// Reselect the new index in the menu.
	m_Colours->setValue(selectedThemeAfterReload);
	handleColourSelected();
}

void ToolBar::handleConvertOldPresetsBank()
{
	// If the user had previously installed a NUSofting plugin, the user banks file is likely to be in the presets
	// directory, so use that as the initial default.
    const std::string& presetsDir = Platform::getPresetsDirectory(true);
	const char *userPresetsPath = presetsDir.c_str();
	CNewFileSelector *fileSelector = CNewFileSelector::create(m_frame, CNewFileSelector::kSelectFile);
	/// @todo We shouldn't hard-code the plugin name in the toolbar because the toolbar is shared amongst several plugins.
	fileSelector->addFileExtension(CFileExtension ("Pre 2007 Peti custom banks", "bnk"));
	fileSelector->addFileExtension(CFileExtension ("Pre 2007 Peti VST banks", "fxb"));
	fileSelector->addFileExtension(CFileExtension ("Pre 2007 Peti VST programs", "fxp"));
	fileSelector->setDefaultExtension(CFileExtension ("Pre 2007 Peti VST banks", "fxb"));
	fileSelector->setTitle("Load bank file : BNK or FXB or FXP of old Peti");
	fileSelector->setInitialDirectory(userPresetsPath);
	if (fileSelector->runModal() && fileSelector->getNumSelectedFiles() > 0)
	{
		// The logger the captures the results of the conversion (and any errors / issues).
		Platform::Logger logger;
		m_notifyPluginCore.requestPresetsFileConversion(fileSelector->getSelectedFile(0), logger);
		// Show the summary of the conversion.
		StringsViewer* stringsViewer = new StringsViewer(m_frame,
														 "[ Preset conversion ]",
														 logger,
														 m_scaledFontFactory);
		m_frame->setModalView(stringsViewer);
		stringsViewer->forget();
		programChanged(m_presetManager.getCurrentCategoryIndex(),
					   m_presetManager.getCurrentProgramIndexInCategory());
	}
	fileSelector->forget();
}

void ToolBar::handleLoadAndAutosaveYamlsToUser()
{
	// If the user had previously installed a NUSofting plugin, the user banks file is likely to be in the presets
	// directory, so use that as the initial default.
    const std::string& presetsDir = Platform::getAppUserDataDirectory();
	const char *userPresetsPath = presetsDir.c_str();
	CNewFileSelector *fileSelector = CNewFileSelector::create(m_frame, CNewFileSelector::kSelectFile);
	//** Note this can handle many files at once!
	fileSelector->setAllowMultiFileSelection(true); 
	fileSelector->addFileExtension(CFileExtension ("NUSofting Presets", "yaml"));
	 if(sFileSelectorHint) fileSelector->setTitle(sFileSelectorHint);
	fileSelector->setInitialDirectory(userPresetsPath);
	if (fileSelector->runModal() && fileSelector->getNumSelectedFiles() > 0)
	{
		// The logger the captures the results of the conversion (and any errors / issues).
		Platform::Logger logger;

		for(int index = 0; index < fileSelector->getNumSelectedFiles(); ++index)
		{
			//** Note this could handle many files at once!
			m_notifyPluginCore.requestLoadAndAutosaveYamls(fileSelector->getSelectedFile(index), logger);
		}

		// Show the summary of the conversion.
		StringsViewer* stringsViewer = new StringsViewer(m_frame,
														 "[ Preset conversion ]",
														 logger,
														 m_scaledFontFactory);
		m_frame->setModalView(stringsViewer);
		stringsViewer->forget();
		programChanged(m_presetManager.getCurrentCategoryIndex(),
					   m_presetManager.getCurrentProgramIndexInCategory());
		
	}
	fileSelector->forget();
}


void ToolBar::onZoom()
{
	m_saveDialogButton->setDisplayArea(m_frame->getViewSize()); // for the splashView
	m_Banks->setFont(m_scaledFontFactory->getScaledBigFont());
	m_Presets->setFont(m_scaledFontFactory->getScaledBigFont());
}

//==============================================================================

AboutBoxText::AboutBoxText(const char* pluginName,
                           const char* version,
                           const char* copyright,
                           const char* extraLine,
                           const char* infoFileName)
: m_pluginName(pluginName),
  m_version(version),
  m_copyright(copyright),
  m_mainPluginCredits(StandardAboutBoxText_mainPluginCredits),
  m_guiCredits(StandardAboutBoxText_guiCredits),
  m_frameworkCredits(StandardAboutBoxText_frameworkCredits),
  m_vstCredits(StandardAboutBoxText_vstCredits),
  m_auCredits(StandardAboutBoxText_auCredits),
  m_infoFileName(infoFileName),
  m_extraLine(extraLine),
  m_detailedVersion(Platform::buildVersionString(version))
{
}

const char** AboutBoxText::getLines(int& m_numLines) {
    static const char* lines[] =
    {
        m_pluginName,
        m_detailedVersion.c_str(),
        m_copyright,
        m_mainPluginCredits,
        m_guiCredits,
        m_frameworkCredits,
        m_vstCredits,
        m_auCredits,
        m_extraLine
    };
    m_numLines = sizeof lines / sizeof lines[0];
    if (!m_extraLine) {
        m_numLines--;
    }
    /// *** Warning warning warning!!!
    /// We have to repeat setting the detailed version because in some hosts,
    /// when reinitialising the plugin it treats the static lines array above as
    /// already initialised, but the built m_detailedVersion buffer will be in a
    /// different memory location. So we have to get the C string buffer again.
    /// Everything is compile time constants. so stay in the same place.
    lines[1] = m_detailedVersion.c_str();
    return lines;
}
