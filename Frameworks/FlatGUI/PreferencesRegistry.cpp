//
//  PreferencesRegistry.cpp
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "PreferencesRegistry.h"

#include <string>

namespace
{
    const char *kKeySkinIndex = "skin";
    const char *kKeyAppearanceName = "appearanceName";
    const char *kKeyAppearanceFileName = "appearanceFileName";
    const char *kKeyTooltipViewState = "tooltipViewState";
    const int   kDefaultTooltipViewState = static_cast<int>(true);

    const char *kKeyMultiControlsViewState = "multiControlsViewState";
    const int   kDefaultMultiControlsViewState = static_cast<int>(0.0f);

	const char *kKeyMicroTuneViewState = "microTuneViewState";
    const int   kDefaultMicroTuneViewState = static_cast<int>(0.0f);

	const char *kKeyFirstRunState = "FirstRunState";
    const int   kDefaultFirstRunState = static_cast<int>(1.0f);

	const char *kKeyScopeViewState = "ScopeViewState";
	const int   kDefaultScopeViewtate = static_cast<int>(1.0f);
}

PreferencesRegistry::PreferencesRegistry(const char *pluginKey,
                                         float defaultSkinIndex,
                                         const char* defaultAppearanceFileName)
    : m_prefsStore(pluginKey),
      m_defaultSkinIndex(defaultSkinIndex),
      m_defaultAppearanceFileName(defaultAppearanceFileName),
      m_defaultAppearanceName(defaultAppearanceFileName)
{
}

PreferencesRegistry::~PreferencesRegistry()
{
}

float PreferencesRegistry::loadSkinSizeIndex()
{
    return m_prefsStore.loadValue<float>(kKeySkinIndex, m_defaultSkinIndex);
}

void PreferencesRegistry::saveSkinSizeIndex(float value)
{
    m_prefsStore.saveValue(kKeySkinIndex, value);
}

std::string PreferencesRegistry::loadAppearanceName()
{
    return m_prefsStore.loadValue<std::string>(kKeyAppearanceName, m_defaultAppearanceName);
}

std::string PreferencesRegistry::loadAppearanceFileName()
{
    return m_prefsStore.loadValue<std::string>(kKeyAppearanceFileName, m_defaultAppearanceFileName);
}

void PreferencesRegistry::saveAppearanceName(const std::string& value)
{
    m_prefsStore.saveValue<std::string>(kKeyAppearanceName, value);
}

void PreferencesRegistry::saveAppearanceFileName(const std::string& value)
{
    m_prefsStore.saveValue<std::string>(kKeyAppearanceFileName, value);
}

void PreferencesRegistry::setDefaultAppearanceName(const std::string& value)
{
    m_defaultAppearanceName = value;
}

bool PreferencesRegistry::loadTooltipViewState()
{
    return m_prefsStore.loadValue<int>(kKeyTooltipViewState, kDefaultTooltipViewState);
}

void PreferencesRegistry::saveTooltipViewState(bool value)
{
    // Save as an int rather than a bool, because that will give us more options for the future.
    m_prefsStore.saveValue(kKeyTooltipViewState, static_cast<int>(value));
}
int PreferencesRegistry::loadMultiControlsViewState()
{
    return m_prefsStore.loadValue<int>(kKeyMultiControlsViewState, kDefaultMultiControlsViewState);
}

void PreferencesRegistry::saveMultiControlsViewState(float value)
{
    // Save as an int rather than a float, because that will give us more options for the future.
	m_prefsStore.saveValue(kKeyMultiControlsViewState, (value >= 0.5f)? 1 : 0);
}

int PreferencesRegistry::loadMicroTuneViewState()
{
    return m_prefsStore.loadValue<int>(kKeyMicroTuneViewState, kDefaultMicroTuneViewState);
}

void PreferencesRegistry::saveMicroTuneViewState(float value)
{
    // Save as an int rather than a float, because that will give us more options for the future.
	m_prefsStore.saveValue(kKeyMicroTuneViewState, (value >= 0.5f)? 1 : 0);
}

int PreferencesRegistry::loadFirstRunState()
{
    return m_prefsStore.loadValue<int>(kKeyFirstRunState, kDefaultFirstRunState);
}

void PreferencesRegistry::saveFirstRunState(float value)
{
    // Save as an int rather than a float, because that will give us more options for the future.
	m_prefsStore.saveValue(kKeyFirstRunState, (value >= 0.5f)? 1 : 0);
}

int PreferencesRegistry::loadScopeViewState()
{
    return m_prefsStore.loadValue<int>(kKeyScopeViewState, kDefaultScopeViewtate);
}

void PreferencesRegistry::saveScopeViewState(float value)
{
    // Save as an int rather than a float, because that will give us more options for the future.
	m_prefsStore.saveValue(kKeyScopeViewState, (value >= 0.5f)? 1 : 0);
}
	
