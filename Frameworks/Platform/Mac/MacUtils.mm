//
//  MacUtils.mm
//  knagalis
//
//  Created by Bernie Maier on 5/05/2013.
//  Copyright 2013. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "MacUtils.h"
#include <pthread.h>

bool getProductDir(const char *product, char *dest)
{
    AutoreleasePool pool;
    NSString *bundleId = [NSString stringWithCString:BUNDLE_ID encoding:NSUTF8StringEncoding];
    NSBundle *bundle = [NSBundle bundleWithIdentifier:bundleId];
    strcpy(dest, [[NSFileManager defaultManager] fileSystemRepresentationWithPath:bundle.resourcePath]);
    strcat(dest, "/");
    return true;
}


AutoreleasePool::AutoreleasePool()
{
    m_pool = [[NSAutoreleasePool alloc] init];
}

AutoreleasePool::~AutoreleasePool()
{
    [(NSAutoreleasePool *)m_pool release];
}


ScopedLogger::ScopedLogger(void *ptr, const char *desc)
 :  m_ptr(ptr),
    m_desc(desc)
{
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    const char* nonMainThreadDecorator = ([NSThread isMainThread]) ? "" : ":*";
    fprintf(stderr, ">>> <%p:%llx%s> %s\n", m_ptr, tid, nonMainThreadDecorator, m_desc);
}

ScopedLogger::ScopedLogger(void *ptr, const char *desc, int arg)
 :  m_ptr(ptr),
    m_desc(desc)
{
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    const char* nonMainThreadDecorator = ([NSThread isMainThread]) ? "" : ":*";
    fprintf(stderr, ">>> <%p:%llx%s> %s(%d)\n", m_ptr, tid, nonMainThreadDecorator, m_desc, arg);
}

ScopedLogger::ScopedLogger(void *ptr, const char *desc, int arg1, int arg2)
 :  m_ptr(ptr),
    m_desc(desc)
{
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    const char* nonMainThreadDecorator = ([NSThread isMainThread]) ? "" : ":*";
    fprintf(stderr, ">>> <%p:%llx%s> %s(%d, %d)\n", m_ptr, tid, nonMainThreadDecorator, m_desc, arg1, arg2);
}

ScopedLogger::~ScopedLogger()
{
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    const char* nonMainThreadDecorator = ([NSThread isMainThread]) ? "" : ":*";
    fprintf(stderr, "<<< <%p:%llx%s> %s\n", m_ptr, tid, nonMainThreadDecorator, m_desc);
}

void saveFloatPreference(const char *key, float value)
{
    AutoreleasePool pool;
    NSString *keyStr = [NSString stringWithCString:key encoding:NSUTF8StringEncoding];
    NSNumber *valueObj = [NSNumber numberWithFloat:value];
    NSString *bundleId = [NSString stringWithCString:BUNDLE_ID encoding:NSUTF8StringEncoding];
    NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
    NSDictionary *bundlePrefs = [prefs persistentDomainForName:bundleId];
    NSMutableDictionary *newPrefs = [NSMutableDictionary dictionaryWithDictionary:bundlePrefs];
    [newPrefs setObject:valueObj forKey:keyStr];
    [prefs setPersistentDomain:newPrefs forName:bundleId];
}

float loadFloatPreference(const char *key, float defaultValue)
{
    AutoreleasePool pool;
    NSString *keyStr = [NSString stringWithCString:key encoding:NSUTF8StringEncoding];
    NSString *bundleId = [NSString stringWithCString:BUNDLE_ID encoding:NSUTF8StringEncoding];
    NSUserDefaults *prefs = [NSUserDefaults standardUserDefaults];
    NSDictionary *bundlePrefs = [prefs persistentDomainForName:bundleId];
    NSNumber *valueObj = [bundlePrefs objectForKey:keyStr];
    float value = valueObj ? [valueObj floatValue] : defaultValue;
    return value;
}
