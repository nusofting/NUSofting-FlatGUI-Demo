//
//  PreferencesRegistryStore.h
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#pragma once

#include <memory>

namespace Platform
{

/// Defines an interface encapsulating the platform-specific mechanisms for
/// saving and loading user preference data, with support for default values.
/// Platform-specific classes implement this interface.
class PreferencesRegistryStore
{
public:
    /// Constructor.
    ///
    /// @ param pluginKey
    ///     A string used as the final component of a registry key or preferences
    ///     domain name. This should uniquely identify the plugin.
    PreferencesRegistryStore(const char *pluginKey);

    ~PreferencesRegistryStore();

    /// Saves the specified value for the specified key.
    /// Currently the template type is only allowed to be int, float or std::string.
    template <typename T>
    void saveValue(const char *key, const T &value);

    /// Loads the previously saved value for the specified key, or the
    /// specified default value if the value wasn't previously saved.
    /// Currently the template type is only allowed to be int, float or std::string.
    template <typename T>
    T loadValue(const char *key, const T &defaultValue);

private:
    /// Prevent copying.
    PreferencesRegistryStore(const PreferencesRegistryStore &);

    /// Prevent assignment.
    PreferencesRegistryStore &operator=(const PreferencesRegistryStore &);

    /// Private, implementation-specific data.
    std::auto_ptr<struct PreferencesRegistryStoreData> m_privateData;
};

} // namespace Platform
