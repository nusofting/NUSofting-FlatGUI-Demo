//
//  PlatformCommon.cpp
//
// The parts of the Platform namespace that have a common (portable) implementation
// on both OS X and Windows.
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "../Platform.h"

#include "PlatformCommon.h"
#include "Utils/Utils.h"

#include <cassert>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>


namespace Platform
{

// statics are shared between all instances of the plugin. This would normally be a code smell,
// but in practice the plugin name and key are effectively compile-time constants. Avoid
// adding more statics, however.
// @todo Consider just pushing these in as -D defines at the project level (like BUNDLE_ID).
static const char* sPluginName;
static const char* sPluginKey;

void init(const char *pluginName, const char *pluginKey)
{
    sPluginName = pluginName;
    sPluginKey = pluginKey;
}

const char* getPluginName()
{
	return sPluginName;
}

const char* getPluginKey()
{
	return sPluginKey;
}

std::string getPathForUserResource(const char* fileName)
{
	return getAppUserDataDirectory() + std::string(fileName);
}

std::string buildSafeFileName(const std::string& path, const std::string& unsafeName, const char* extension)
{
	// Transform unsafe characters into safe characters:
	// - all control codes map to ^ (these should never appear anyway)
	// - unsafe punctuation maps to visually similar characters:
	//   " -> ' (double quote to single quote)
	//   * -> x (asterisk to letter x)
	//   / -> % (slash to percent, OK not very similar but has a slash inside the symbol!)
	//   : -> ; (colon to semicolon)
	//   < -> [ (less than to left square bracket)
	//   > -> ] (greater than to right square bracket)
	//   ? -> ! (question mark to exclamation mark)
	//   \ -> % (backslash to percent, just by analogy with forward slash)
	//   | -> ! (vertical bar to exclamation mark)
	// - Bytes with the high bit set are mapped to @ (arbitrary choice)
	static const char safeMap[] = "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"  // bytes 0x00-0x1F
	                              " !'#$%&'()x+,-.%0123456789;;[=]!"  // bytes 0x20-0x3F
	                              "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[%]^_"  // bytes 0x40-0x5F
	                              "`abcdefghijklmnopqrstuvwxyz{!}~^"; // bytes 0x60-0x7F
	assert(sizeof safeMap == 128 + 1);
	size_t unsafeLen = unsafeName.length();
	std::string filePath(path);
	filePath.reserve(path.length() + unsafeLen + strlen(extension) + 1); // reserve the right amount of space
	filePath.append("/");
	const char* unsafeStr = unsafeName.c_str();
	for (size_t i = 0; i < unsafeLen; ++i)
	{
		char ch = unsafeStr[i];
		if ((ch & 0x80) == 0x80)
		{
			filePath.append(1, '@');
		}
		else
		{
			filePath.append(1, safeMap[ch]);
		}
	}
	bool unique = false;
	unsigned int duplicateCount = 0;
	while (!unique)
	{
		std::stringstream ss;
		ss << filePath;
		if (duplicateCount > 0)
		{
			ss << duplicateCount;
		}
		ss << '.';
		ss << extension;
		//std::string testFilePath(ss.str());
		// There is a well-known race condition here, between the check if the file
		// doesn't exist and trying to write to it. Since the user (or someone else
		// on their system) has to be maliciously attacking us we will ignore this.
		// (TOCTOU, see https://en.wikipedia.org/wiki/Time_of_check_to_time_of_use)
		struct stat fileInfo;
		const std::string& testFilePath = ss.str();
		unique = stat(testFilePath.c_str(), &fileInfo) != 0;
		if (unique)
		{
			filePath = testFilePath;
		}
		else
		{
			++duplicateCount;
		}
	}
	return filePath;
}

std::string buildMessagesFileName()
{
    std::time_t rawtime;
    std::tm* timeinfo;
    char buffer[80];

    std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);

    std::strftime(buffer, 80, "%Y-%m-%d-%H-%M-%S", timeinfo);

	std::stringstream nameBuilder;
	nameBuilder << sPluginKey << "-Messages-" << buffer << ".txt";
	return std::string(nameBuilder.str());
}

#if !defined(PLUGIN_TYPE)
#define PLUGIN_TYPE ""
#endif

#if !defined(ARCH_STRING)
    #if defined(__aarch64__)
        #define ARCH_STRING "(ARM)"
    #elif defined(__x86_64__)
        #define ARCH_STRING "(x86_64)"
    #else
        #define ARCH_STRING ""
    #endif
#endif

std::string buildVersionString(const std::string& versionNumberPrefix)
{
    std::stringstream versionBuilder;
    versionBuilder << versionNumberPrefix << " " << PLUGIN_TYPE << " " << ARCH_STRING;
    return std::string(versionBuilder.str());
}

void forEachFileInDir(const std::string& basePath, DirTreeCallback& callback, const char* extension, size_t maxDepth)
{
	DIR *dir = opendir(basePath.c_str());
	if (dir) 
	{
		// Traverse the directory, ignoring any file system objects that are
		// neither regular files nor subdirectories.
		struct dirent *ent;
		while ((ent = readdir(dir)) != 0)
		{
			if (ent->d_type == DT_REG)
			{
				if (Utils::endsWith(ent->d_name, extension))
				{
					std::string filePath(appendPathComponent(basePath, ent->d_name));
					callback.run(filePath);
				}
			}
			else if (   ent->d_type == DT_DIR
					 && ent->d_name[0] != '.' // Ignore special / hidden subdirectories
					 && maxDepth != 1)
			{
				const char* previous = callback.enterSubdir(ent->d_name);
				std::string subdirPath(appendPathComponent(basePath, ent->d_name));
				// If maxDepth is 0, don't change it because we are doing an unbounded traversal.
				// If maxDepth > 1, decrement it (maxDepth cannot equal 1 in this branch so decrementing is OK)
				size_t newMaxDepth = maxDepth > 1 ? maxDepth - 1 : maxDepth;
				forEachFileInDir(subdirPath, callback, extension, newMaxDepth);
				callback.leaveSubdir(previous);
			}
		}
		closedir(dir);
	}
}

bool Logger::saveFile(const std::string& filePath)
{
	std::ofstream outFile(filePath.c_str(), std::fstream::app);
	if (outFile)
	{
		for (size_t i = 0; i < m_lines.size(); ++i)
		{
			outFile << m_lines[i] << "\n";
		}
	}
	return static_cast<bool>(outFile);
}

} // namespace Platform
