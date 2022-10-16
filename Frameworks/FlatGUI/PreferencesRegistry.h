//
//  PreferencesRegistry.h
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#pragma once

#include "Platform/PreferencesRegistryStore.h"

#include <string>


class PreferencesRegistry
{
public:
    /// @ param pluginKey
    ///     A string used as the final component of a registry key or preferences
    ///     domain name. This should uniquely identify the plugin.
    /// @ param defaultSkinIndex
    ///     The index representing the default size of the GUI when the plugin is first run.
    /// @ param defaultAppearanceFileName
    ///     The name of the appearance file used when the plugin is first run.
    PreferencesRegistry(const char *pluginKey,
                        float defaultSkinIndex = 2.0,
                        const char* defaultAppearanceFileName = "pure");

    ~PreferencesRegistry();

    /// @{
    /// Loads and saves the skin currently selected by the user, where the size
    /// is specified by an index into an array of possible skin sizes.
    float loadSkinSizeIndex();
    void saveSkinSizeIndex(float value);
    /// @}

    /// @{
    /// Loads and saves the name and file name of the appearance theme currently selected by the user.
    std::string loadAppearanceName();
    std::string loadAppearanceFileName();
    void saveAppearanceName(const std::string& value);
    void saveAppearanceFileName(const std::string& value);
    void setDefaultAppearanceName(const std::string& value);
    /// @}

    /// @{
    /// Loads and saves the tooltip view state. (Currently only used by Peti SA)
    bool loadTooltipViewState();
    void saveTooltipViewState(bool value);
    /// @}

    /// @{
	/// Loads and saves the kAddViews view state. 
	int loadMultiControlsViewState();
	void saveMultiControlsViewState(float value);
	/// @}

	/// @{
	/// Loads and saves the kMicroTuneEdit view state. 
	int loadMicroTuneViewState();
	void saveMicroTuneViewState(float value);
	/// @}

	/// @{
	/// Loads and saves the firt run state. 
	int loadFirstRunState();
	void saveFirstRunState(float value);
	/// @}
		/// @{
	/// Loads and saves the firt run state. 
	int loadScopeViewState();
	void saveScopeViewState(float value);
	/// @}

private:
    /// Prevent copying.
    PreferencesRegistry(const PreferencesRegistry &);

    /// Prevent assignment.
    PreferencesRegistry &operator=(const PreferencesRegistry &);

    Platform::PreferencesRegistryStore m_prefsStore;
    float m_defaultSkinIndex;
    const char* m_defaultAppearanceFileName;
    std::string m_defaultAppearanceName;
};
