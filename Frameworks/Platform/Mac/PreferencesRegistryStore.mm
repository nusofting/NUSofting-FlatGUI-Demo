//
//  PreferencesRegistryStore.mm
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "../PreferencesRegistryStore.h"
#include "MacUtils.h"

#include <string>

#import <Cocoa/Cocoa.h>

namespace Platform
{

#pragma mark --
#pragma mark PreferencesImplementation interface

/// The actual implementation class that writes to / reads from the user preferences system.
class PreferencesImplementation
{
public:
    PreferencesImplementation(const char *pluginKey);

    ~PreferencesImplementation();

    /// Set a default value for the specified key.
    /// This must be called before attempting to load the value for that key.
    void setDefaultValue(const char *key, NSObject *valueObj);

    /// Saves the specified value for the specified key.
    void saveValue(const char *key, NSObject *valueObj);

    /// Loads the previously value for the specified key, or the specified
    /// default value if the value wasn't previously saved.
    NSObject *loadValue(const char *key, NSObject *defaultValue);

private:
    /// Prevent copying.
    PreferencesImplementation(const PreferencesImplementation &);

    /// Prevent assignment.
    PreferencesImplementation &operator=(const PreferencesImplementation &);

    NSUserDefaults *m_userDefaults;
    NSString *m_prefsBundleId;
};


#pragma mark --
#pragma mark PreferencesRegistryStoreData implementation

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


#pragma mark --
#pragma mark PreferencesRegistryStore implementation

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
    AutoreleasePool pool;
    NSNumber *valueObj = [NSNumber numberWithInt:value];
    m_privateData->saveValue(key, valueObj);
}

template <> 
void PreferencesRegistryStore::saveValue<float>(const char *key, const float &value)
{
    AutoreleasePool pool;
    NSNumber *valueObj = [NSNumber numberWithFloat:value];
    m_privateData->saveValue(key, valueObj);
}

template <> 
void PreferencesRegistryStore::saveValue<std::string>(const char *key, const std::string &value)
{
    AutoreleasePool pool;
    NSString *valueObj = [NSString stringWithUTF8String:value.c_str()];
    m_privateData->saveValue(key, valueObj);
}

template <> 
int PreferencesRegistryStore::loadValue<int>(const char *key, const int &defaultValue)
{
    AutoreleasePool pool;
    NSNumber *valueObj = static_cast<NSNumber *>(m_privateData->loadValue(key, [NSNumber numberWithInt:defaultValue]));
    int value = [valueObj intValue];
    return value;
}

template <> 
float PreferencesRegistryStore::loadValue<float>(const char *key, const float &defaultValue)
{
    AutoreleasePool pool;
    NSNumber *valueObj = static_cast<NSNumber *>(m_privateData->loadValue(key, [NSNumber numberWithFloat:defaultValue]));
    float value = [valueObj floatValue];
    return value;
}

template <> 
std::string PreferencesRegistryStore::loadValue<std::string>(const char *key, const std::string &defaultValue)
{
    AutoreleasePool pool;
    NSString *defaultValueObj = [NSString stringWithUTF8String:defaultValue.c_str()];

    NSString *valueObj = static_cast<NSString *>(m_privateData->loadValue(key, defaultValueObj));
    return std::string([valueObj UTF8String]);
}


#pragma mark --
#pragma mark PreferencesImplementation implementation

PreferencesImplementation::PreferencesImplementation(const char *pluginKey)
{
    m_userDefaults = [NSUserDefaults standardUserDefaults];
    m_prefsBundleId = [[NSString stringWithFormat:@"com.NUSofting.%s", pluginKey] retain];
}

PreferencesImplementation::~PreferencesImplementation()
{
    [m_prefsBundleId release];
}

void PreferencesImplementation::saveValue(const char *key, NSObject *valueObj)
{
    NSString *keyStr = [NSString stringWithCString:key encoding:NSUTF8StringEncoding];
    NSDictionary *bundlePrefs = [m_userDefaults persistentDomainForName:m_prefsBundleId];
    NSMutableDictionary *newPrefs = [NSMutableDictionary dictionaryWithDictionary:bundlePrefs];
    [newPrefs setObject:valueObj forKey:keyStr];
    [m_userDefaults setPersistentDomain:newPrefs forName:m_prefsBundleId];
}

NSObject *PreferencesImplementation::loadValue(const char *key, NSObject *defaultValue)
{
    NSString *keyStr = [NSString stringWithCString:key encoding:NSUTF8StringEncoding];
    NSDictionary *bundlePrefs = [m_userDefaults persistentDomainForName:m_prefsBundleId];
    NSObject *valueObj = [bundlePrefs objectForKey:keyStr];
    if (!valueObj)
    {
        valueObj = defaultValue;
    }
    return valueObj;
}

} // namespace Platform
