//
//  AppearanceManager.h
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#pragma once

#include <map>
#include <string>
#include <vector>


class Appearance;
class AppearanceFactory;

// Helper typedef for an array (vector) of pointers to Appearance objects.
typedef std::vector<Appearance*> Appearances;

/// Encapsulates the collection of appearance themes loaded from the file system.
/// Appearance themes can be loaded from a factory directory and an optional
/// user directory. Appearance themes are read-only.
class AppearanceManager
{
public:
    AppearanceManager(AppearanceFactory& appearanceFactory);

    ~AppearanceManager();

    void loadThemes();
    bool allAppearanceFilesValid() const { return m_allAppearanceFilesValid; }
    void clearThemes();
    void addAppearance(Appearance* newAppearance);
    Appearance* selectAppearance(size_t itemId);

    /// Attempts to load the previously saved appearance by its file name. If the user has renamed the file, then
    /// attempt to load the appearance by its name (which is not necessarily unique, but may be better than just
    /// loading the default). If the user has deleted the file, then this may result in a different appearance with the
    /// same name being loaded, but is more likely to result in the default (i.e. first) appearance being loaded.
    Appearance* selectAppearance(const std::string& fileName, const std::string& name);

    Appearance* getCurrentAppearance() const;
    size_t getCurrentAppearanceIndex() const { return m_currentAppearanceIndex; }
    std::string getCurrentAppearanceName() const;
    std::string getAppearanceName(size_t itemId) const;

    size_t numThemes() const;

private:
    void loadYamlAppearanceFile(const std::string& filePath);
    void createDefaultAppearance();

    class YamlFileCallback;
    AppearanceFactory &m_appearanceFactory;
    Appearances m_appearances;
    Appearance* m_defaultAppearance;
    Appearance* m_currentAppearance;
    size_t m_currentAppearanceIndex;
    /// Maps appearance names *and* file names to their index in m_appearances. This is used for looking up appearances
    /// by file name and name. There is no need to have two separate variables for the two kinds of lookup, because the
    /// lookup keys are strings that are very unlikely to overlap.
    typedef std::map<std::string, size_t> NameToIndexMap;
    NameToIndexMap m_appearancesByName;
    bool m_allAppearanceFilesValid;

    /// Prevent copying.
    AppearanceManager(const AppearanceManager&);

    /// Prevent assignment.
    AppearanceManager& operator=(const AppearanceManager&);
};

