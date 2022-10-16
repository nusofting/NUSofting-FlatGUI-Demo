//
//  Platform.cpp
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "../Platform.h"

#import <Cocoa/Cocoa.h>
#include <dirent.h>
#include <sys/stat.h>


namespace
{
	NSString *getLibraryAppSupportDirectoryStr()
	{
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
		return [paths objectAtIndex:0];
	}

	NSString *getLibraryAudioDirectoryStr()
	{
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
		return [[paths objectAtIndex:0] stringByAppendingPathComponent:@"Audio"];
	}

	NSString *getAppUserDataDirectoryStr()
	{
		return [getLibraryAppSupportDirectoryStr()
					stringByAppendingPathComponent:[NSString stringWithFormat:@"NUSofting Data/%s", Platform::getPluginKey()]];
	}

	NSString *getPresetsDirectoryStr()
	{
		return [getLibraryAudioDirectoryStr()
					stringByAppendingPathComponent:[NSString stringWithFormat:@"Presets/%s", Platform::getPluginKey()]];
	}
}

namespace Platform
{

std::string getPathForAppResource(const char* fileName)
{
	AutoreleasePool pool;
	static std::string appResDir;
	if (appResDir.empty())
	{
		NSString *bundleId = [NSString stringWithCString:BUNDLE_ID encoding:NSUTF8StringEncoding];
		NSBundle *bundle = [NSBundle bundleWithIdentifier:bundleId];
		appResDir = [[NSFileManager defaultManager] fileSystemRepresentationWithPath:bundle.resourcePath];
		appResDir.append("/");
	}
	return appResDir + std::string(fileName);
}

std::string getAppUserDataDirectory()
{
	AutoreleasePool pool;
	static std::string appDataDir;
	if (appDataDir.empty())
	{
		appDataDir = [[NSFileManager defaultManager] fileSystemRepresentationWithPath:getAppUserDataDirectoryStr()];
		appDataDir.append("/");
	}
	return appDataDir;
}

std::string getPresetsDirectory(bool forUser)
{
	AutoreleasePool pool;
	static std::string factoryPresetsDir;
	static std::string userPresetsDir;
	if (forUser)
	{
		if (userPresetsDir.empty())
		{
			NSString* presetsDirStr = getPresetsDirectoryStr();
			NSFileManager* fileManager = [NSFileManager defaultManager];
			userPresetsDir = [fileManager fileSystemRepresentationWithPath:presetsDirStr];
			userPresetsDir.append("/");
			// Pre-emptively create the user presets directory if it doesn't already exist.
			NSError *error = nil;
			BOOL succeeded = [fileManager createDirectoryAtPath:presetsDirStr
									withIntermediateDirectories:YES
													 attributes:nil
														  error:&error];
			if (!succeeded)
			{
				NSLog(@"Error creating user presets directory: %@", [error localizedDescription]);
			}
		}
	}
	else
	{
		if (factoryPresetsDir.empty())
		{
			NSString *bundleId = [NSString stringWithCString:BUNDLE_ID encoding:NSUTF8StringEncoding];
			NSBundle *bundle = [NSBundle bundleWithIdentifier:bundleId];
			NSString *factoryPresetsDirStr = [NSString stringWithFormat:@"%@/Presets", bundle.resourcePath];
			/// @todo see also -[pathsForResourcesOfType:inDirectory:] for an alternative design where we maybe
			/// return an opaque object containing the searched for items, e.g. preset files and that object can
			/// provide an enumeration over the collection.
			factoryPresetsDir = [[NSFileManager defaultManager] fileSystemRepresentationWithPath:factoryPresetsDirStr];
			factoryPresetsDir.append("/");
		}
	}
	return forUser ? userPresetsDir : factoryPresetsDir;
}

std::string getAppearanceDirectory(bool forUser)
{
	AutoreleasePool pool;
	static std::string factoryAppearanceDir;
	static std::string userAppearanceDir;
	if (forUser)
	{
		if (userAppearanceDir.empty())
		{
			NSString* appearanceDirStr = [getAppUserDataDirectoryStr() stringByAppendingPathComponent:@"Appearance"];
			NSFileManager* fileManager = [NSFileManager defaultManager];
			userAppearanceDir = [fileManager fileSystemRepresentationWithPath:appearanceDirStr];
			userAppearanceDir.append("/");
			// Pre-emptively create the user presets directory if it doesn't already exist.
			NSError *error = nil;
			BOOL succeeded = [fileManager createDirectoryAtPath:appearanceDirStr
									withIntermediateDirectories:YES
													 attributes:nil
														  error:&error];
			if (!succeeded)
			{
				NSLog(@"Error creating user appearance directory: %@", [error localizedDescription]);
			}
		}
	}
	else
	{
		if (factoryAppearanceDir.empty())
		{
			NSString *bundleId = [NSString stringWithCString:BUNDLE_ID encoding:NSUTF8StringEncoding];
			NSBundle *bundle = [NSBundle bundleWithIdentifier:bundleId];
			NSString *factoryAppearanceDirStr = [NSString stringWithFormat:@"%@/Appearance", bundle.resourcePath];
			/// @todo see also -[pathsForResourcesOfType:inDirectory:] for an alternative design where we maybe
			/// return an opaque object containing the searched for items, e.g. preset files and that object can
			/// provide an enumeration over the collection.
			factoryAppearanceDir = [[NSFileManager defaultManager] fileSystemRepresentationWithPath:factoryAppearanceDirStr];
			factoryAppearanceDir.append("/");
		}
	}
	return forUser ? userAppearanceDir : factoryAppearanceDir;
}

void PopupMessage(const char* text, const char* caption)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        AutoreleasePool pool;
        NSAlert *alert = [NSAlert alertWithMessageText:[NSString stringWithCString:caption
                                                                          encoding:NSUTF8StringEncoding]
                                         defaultButton:nil
                                       alternateButton:nil
                                           otherButton:nil
                             informativeTextWithFormat:@"%@", [NSString stringWithCString:text
                                                                                 encoding:NSUTF8StringEncoding]];
            [alert runModal];
    });
}

std::string appendPathComponent(const std::string& basePath, const char* extraPathComponent)
{
    std::string filePath(basePath);
    if (basePath[basePath.size() - 1] != '/')
    {
        filePath.append("/");
    }
    filePath.append(extraPathComponent);
    return filePath;
}

unsigned long long getFileId(const char* name)
{
    unsigned long long result = -1;
    std::string filePath(getPresetsDirectory(true));
    filePath.append("/");
    filePath.append(name);
    filePath.append(".yaml");

    struct stat fileInfo;
    int statResult = stat(filePath.c_str(), &fileInfo);
    if (statResult == 0)
    {
        result = fileInfo.st_ino;
    }
    return result;
}

unsigned long long getFileId(FILE* openFile)
{
    unsigned long long result = -1;
    struct stat fileInfo;
    int fd = fileno(openFile);
    int statResult = fstat(fd, &fileInfo);
    if (statResult == 0)
    {
        result = fileInfo.st_ino;
    }
    return result;
}

bool exploreDirectory(const char *directory)
{
	AutoreleasePool pool;
	NSString *dirString = [NSString stringWithUTF8String:directory];
	return [[NSWorkspace sharedWorkspace] selectFile:nil inFileViewerRootedAtPath:dirString];
}

bool launchBrowser(const char *url)
{
	AutoreleasePool pool;
	NSString *urlString = [NSString stringWithUTF8String:url];
	return [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:urlString]];
}

bool launchBrowserWithLocalFile(const char *path)
{
	AutoreleasePool pool;
	NSString *pathString = [NSString stringWithUTF8String:path];
	return [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:pathString]];
}

int utf8CaseInsensitiveCompare(const std::string& str1, const std::string& str2)
{
    AutoreleasePool pool;
	NSString *nsstr1 = [NSString stringWithUTF8String:str1.c_str()];
	NSString *nsstr2 = [NSString stringWithUTF8String:str2.c_str()];
	return [nsstr1 localizedCaseInsensitiveCompare:nsstr2];
}

} // namespace Platform
