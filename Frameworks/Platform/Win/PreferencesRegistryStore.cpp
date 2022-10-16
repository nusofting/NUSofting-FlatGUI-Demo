//
//  PreferencesRegistryStore.cpp
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "../PreferencesRegistryStore.h"

#include <map>
#include <string>
#include <Windows.h>

namespace Platform
{

/// The actual implementation class that writes to / reads from the Windows registry.
class PreferencesImplementation
{
public:
    PreferencesImplementation(const char *pluginKey);

    ~PreferencesImplementation();

    /// Saves the specified value for the specified key.
    void saveValue(const char *key, const BYTE *valueBytes, size_t valueSize, DWORD valueType);

    /// Loads the previously saved value for the specified key, or the specified
    /// default value if the value wasn't previously saved.
    template <typename T>
    T loadValue(const char *key, const T &defaultValue);
    std::string loadValue(const char *key, const std::string &defaultValue);

private:
    /// Prevent copying.
    PreferencesImplementation(const PreferencesImplementation &);

    /// Prevent assignment.
    PreferencesImplementation &operator=(const PreferencesImplementation &);

    std::string regKeyPath;
};


//==============================================================================

/// Hook the actual implementation class up to the public interface via the
/// structure defined in the public interface for implementation-specific data.
/// By making the structure defined in the public interface from the implementation
/// class defined in this file, we effectively make the two things one and the same,
/// but without disclosing the private, platform-specific implementation details
/// to the public interface.
struct PreferencesRegistryStoreData : public PreferencesImplementation
{
    PreferencesRegistryStoreData(const char *pluginKey) : PreferencesImplementation(pluginKey) { }
};


//==============================================================================

PreferencesRegistryStore::PreferencesRegistryStore(const char *pluginKey)
    : m_privateData(new PreferencesRegistryStoreData(pluginKey))
{
}

PreferencesRegistryStore::~PreferencesRegistryStore()
{
}

template <>
void PreferencesRegistryStore::saveValue<int>(const char *key, const int &value)
{
    const BYTE *valueBytes = reinterpret_cast<const BYTE *>(&value);
    DWORD valueSize = sizeof value;
    m_privateData->saveValue(key, valueBytes, valueSize, REG_DWORD);
}

template <>
void PreferencesRegistryStore::saveValue<float>(const char *key, const float &value)
{
    const BYTE *valueBytes = reinterpret_cast<const BYTE *>(&value);
    DWORD valueSize = sizeof value;
    m_privateData->saveValue(key, valueBytes, valueSize, REG_BINARY);
}

template <>
void PreferencesRegistryStore::saveValue<std::string>(const char *key, const std::string &value)
{
    const BYTE *valueBytes = reinterpret_cast<const BYTE *>(value.c_str());
    DWORD valueSize = strlen(value.c_str()) + 1;
    m_privateData->saveValue(key, valueBytes, valueSize, REG_SZ);
}

template <typename T> 
T PreferencesRegistryStore::loadValue(const char *key, const T &defaultValue)
{
    return m_privateData->loadValue(key, defaultValue);
}

template <> 
std::string PreferencesRegistryStore::loadValue<std::string>(const char *key, const std::string &defaultValue)
{
    return m_privateData->loadValue(key, defaultValue);
}

// Explicit instantations of the template functions for the actual types
// supported for plugin use. Note: we do not need to include the functions for
// which we have specialised the template functions, because they will already
// get resolved by the linker.
template int PreferencesRegistryStore::loadValue<int>(const char *key, const int &defaultValue);
template float PreferencesRegistryStore::loadValue<float>(const char *key, const float &defaultValue);

//==============================================================================

PreferencesImplementation::PreferencesImplementation(const char *pluginKey)
 : regKeyPath(std::string("Software\\NUSofting\\") + pluginKey)
{
}

PreferencesImplementation::~PreferencesImplementation()
{
}

void PreferencesImplementation::saveValue(const char *key, const BYTE *valueBytes, size_t valueSize, DWORD valueType)
{
    HKEY regKey;
    LONG result = RegCreateKeyEx(HKEY_CURRENT_USER,
                                 regKeyPath.c_str(),
                                 0, NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_READ | KEY_WRITE,
                                 NULL,
                                 &regKey, NULL);
    if (result == ERROR_SUCCESS)
    {
        if (RegSetValueEx(regKey, key, 0, valueType, valueBytes, valueSize) == ERROR_SUCCESS)
        {
            RegFlushKey(regKey);
        }
        RegCloseKey(regKey);
    }   
}

template <typename T>
T PreferencesImplementation::loadValue(const char *key, const T &defaultValue)
{
    T value = defaultValue;
    HKEY regKey = 0;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, regKeyPath.c_str(), 0, KEY_READ, &regKey) == ERROR_SUCCESS)
    {
        T valueFromReg;
        BYTE *valueBytes = reinterpret_cast<BYTE *>(&valueFromReg);
        DWORD valueSize = sizeof valueFromReg;
        if (RegQueryValueEx(regKey, key, NULL, NULL, valueBytes, &valueSize) == ERROR_SUCCESS)
        {
            value = valueFromReg;
        }
        RegCloseKey(regKey);
    }
    return value;
}

std::string PreferencesImplementation::loadValue(const char *key, const std::string &defaultValue)
{
    std::string value(defaultValue);
    HKEY regKey = 0;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, regKeyPath.c_str(), 0, KEY_READ, &regKey) == ERROR_SUCCESS)
    {
        char buffer[512];
        BYTE *valueBytes = reinterpret_cast<BYTE *>(buffer);
        DWORD valueSize = sizeof buffer;
        if (RegQueryValueEx(regKey, key, NULL, NULL, valueBytes, &valueSize) == ERROR_SUCCESS)
        {
            value = std::string(buffer);
        }
        RegCloseKey(regKey);
    }

    return value;
}

} // namespace Platform
