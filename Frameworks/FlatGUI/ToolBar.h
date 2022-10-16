/*-----------------------------------------------------------------------------

(c) 2015 nusofting.com - Liqih

ToolBar.h
Written by Luigi Felici and Bernie Maier
Copyright (c) 2015 NUSofting

-----------------------------------------------------------------------------*/

#pragma once

#include "vstgui/vstgui.h"

#include "AboutBoxText.hpp"
#include "SharedToolBarRects.hpp"

namespace Platform
{
	class Logger;
}

class AppearanceManager;
class CKickPreviousNext;
class CKickTwiceZoom;
class CSimpleLabel;
class EditorToPluginCoreInterface;
class MultiActionDialogButton;
class PresetManager;
class SaveDialog;
class ScaledFontFactory;
class CHelpFileLaunch;
class CRandWidget;
class ToolBarListener;

/// The toolbar includes a "settings" menu that provides a single place to select various settings and optional actions.
/// The contents of this menu includes mostly standard items that we want to offer in all plugins, but some may only
/// apply to a subset of plugins.
/// This enum defines bit flags that when ored together define the menu items the toolbar adds to the settings menu.
enum SettingsMenuOptionFlags
{
	/// Link to the main web site for the plugin.
	SMOF_WEB_SITE_LINK   = 0x0001,

	/// Display an about screen.
	SMOF_ABOUT           = 0x0002,

	/// Open the user presets directory in an Explorer / Finder window.
	SMOF_EXPLORE_PRESETS = 0x0004,

	/// Open the user appearance themes directory in an Explorer / Finder window.
	SMOF_EXPLORE_THEMES  = 0x0008,

	/// Trigger a refresh of the appearance themes list.
	SMOF_RELOAD_THEMES   = 0x0010,

	/// Allows the user to convert an old style preset or bank file to YAML presets.
	/// Only useful for older plugins being updated to the new framework.
	SMOF_CONVERT_PRESETS = 0x0020,

	/// Opens a viewer to display logged messages.
	/// If this option is specified, the menu item is only added if there are any logged messages to display.
	SMOF_DISPLAY_ERRORS  = 0x0040
};

/// Enum defining some common combinations of the option flags defined in SettingsMenuOptionFlags
enum SettingsMenuOptionsSets
{
	/// Settings menu options shared by most new plugins created using this framework.
	SMOS_STANDARD_NEW_PLUGIN =   SMOF_WEB_SITE_LINK
	                           | SMOF_ABOUT
	                           | SMOF_EXPLORE_PRESETS
	                           | SMOF_EXPLORE_THEMES
	                           | SMOF_RELOAD_THEMES
	                           | SMOF_DISPLAY_ERRORS,

	/// Settings menu options shared by most old plugins updated to use this framework, and that have presets in an
	/// older format that can be converted. // now also used in sinmad full vs sinmad beat yaml conversion
	SMOS_STANDARD_OLD_PLUGIN = SMOS_STANDARD_NEW_PLUGIN | SMOF_CONVERT_PRESETS
};

/// Encapsulates the product-specific static configuration of the toolbar.
struct ToolBarOptions
{
    int m_settingsOptionsTag;
    int m_zoomOptionsTag;
    const char* m_name;
	const char* m_version;
	const char* m_copyright;
	const char* m_infoFileName;
	const char* m_extraLine;
    int32_t m_settingsMenuOptions;
	MainToolBarRects m_rMainRects;
	bool bActivateLoadAndResaveYamlsOnMenu; // so far this is used to add a presets conversion tool in Sinmad Beat or Sinmad full, when needed.
	bool bFastOverwriteFactoryPreset; // true only in dev Windows builds.
	const char* m_FileSelectorHint;

    ToolBarOptions(int settingsOptionsTag,
                   int zoomOptionsTag,
                   const char* name,
                   const char* version,
                   const char* copyright,
                   const char* infoFileName,
                   const char* extraLine = 0) :
        m_settingsOptionsTag(settingsOptionsTag),
        m_zoomOptionsTag(zoomOptionsTag),
		m_rMainRects(),
		m_name(name),
		m_version(version),
		m_copyright(copyright),
		m_infoFileName(infoFileName),
		m_extraLine(extraLine),
        m_settingsMenuOptions(SMOS_STANDARD_NEW_PLUGIN)
    {
		bActivateLoadAndResaveYamlsOnMenu = false;
		bFastOverwriteFactoryPreset = false;
		m_FileSelectorHint  = " ";
    }

    /// Add a menu option for converting presets from the old FXP / FXB style to YAML presets.
    /// (Only needed for old plugins moved to the new framework.)
    void addOldPluginPresetConversion( const char* sFileSelectorHint = NULL)
    {
        m_settingsMenuOptions |= SMOF_CONVERT_PRESETS;
		m_FileSelectorHint = sFileSelectorHint? sFileSelectorHint : NULL;
    }
};


class ToolBar : public IControlListener
{
public:
	ToolBar(PresetManager& presetManager,
	        EditorToPluginCoreInterface& notifyPluginCore,
	        ToolBarListener& listener,
	        AppearanceManager& appearanceManager,
	        Platform::Logger& errorLog);
	~ToolBar();

	// from IControlListener
	void valueChanged(CControl* pControl);

	//void close();

	void programChanged(size_t categoryIndex, size_t programInCategoryIndex);
	void CreateToolBarView(CFrame* newFrame,
	                       float scaleGUI,
	                       ScaledFontFactory* scaledFontFactory,
                           ToolBarOptions& options);
	void setInitialSelections(float zoomValue);

	/// Perform any custom resizing logic needed when resizing the UI
	void onZoom();

	// Check if anything in here or the main editor has dirtied the preset.
	void checkIfPresetBecameDirty();

	COptionMenu* getOptionMenu() {if(m_Settings) return m_Settings; else return 0;}

private:
	void addBanksToMenu();
	void addPresetsToMenu();
	void resyncToolbarPresetControls();
	void handleColourSelected();
	void handleAppearanceReload();
	void handleConvertOldPresetsBank();
	void handleLoadAndAutosaveYamlsToUser();
	bool bActivateLoadAndResaveYamlsOnMenu; // this is used to add a presets conversion tool in Sinmad Beat or Sinmad full, when needed.
	bool bFastOverwriteFactoryPreset;
	const char* sFileSelectorHint;

	PresetManager& m_presetManager;
	EditorToPluginCoreInterface& m_notifyPluginCore;
	ToolBarListener &m_listener;
	AppearanceManager& m_appearanceManager;

	// Styling
	char* thisFontUsed;

	// Controls and frame
	CFrame* m_frame;

	COptionMenu* m_Settings;// About, links, etc...
	COptionMenu* m_Banks;
	COptionMenu* m_Presets;
	CKickPreviousNext* pKickPreviousNext1;
	CKickPreviousNext* pKickPreviousNext2;
	CSimpleLabel* m_LaunchHTMLFile;
	COptionMenu* m_Colours;
	CRandWidget* m_PickRand;
	CKickTwiceZoom* m_SetupZoomOptions;
	SaveDialog* m_saveDialog;
	MultiActionDialogButton* m_saveDialogButton;
	CSimpleLabel* m_compare;
	CSimpleLabel* m_compareDesc;
	CSimpleLabel* m_FastOverwrite;


	char infoFileName_[1024];
	float factGUIScaling;

	bool m_presetIsDirty;
	ScaledFontFactory* m_scaledFontFactory;
	Platform::Logger& m_errorLog;

	AboutBoxText* m_aboutText;
};

/// Defines an interface for sending events sourced in the generic toolbar back
/// to the plug-in specific editor.
class ToolBarListener
{
public:
	virtual void setColours(int32_t colourIndex) = 0;
	virtual void handleZoom(float value) = 0;
	virtual void requestApppearanceReload() = 0;
};
