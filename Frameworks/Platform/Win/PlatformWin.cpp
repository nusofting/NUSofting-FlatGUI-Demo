//
//  Platform.cpp
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "../Platform.h"

#include <Windows.h>
#include <io.h>
#include <sys/stat.h>


// Defined in the VST 2.4 SDK file vstplugmain.cpp but no extern declaration supplied.
extern void* hInstance;

bool getProductDir(std::string &dest);

namespace Platform
{

std::string getPathForAppResource(const char* fileName)
{
	static std::string appResDir;
	if (appResDir.empty())
	{
		if(!getProductDir(appResDir))
			Platform::PopupMessage("Plugin Data not found!");
	}
	return appResDir + std::string(fileName);
}

std::string getAppUserDataDirectory()
{
	static std::string appDataDir;
	if (appDataDir.empty())
	{
		if(!getProductDir(appDataDir))
			Platform::PopupMessage("Plugin Data not found!");
	}
	return appDataDir;
}

std::string getPresetsDirectory(bool forUser)
{
	static std::string factoryPresetsDir;
	static std::string userPresetsDir;
	if (factoryPresetsDir.empty())
	{
		if(!getProductDir(factoryPresetsDir))
			Platform::PopupMessage("Plugin Data not found!"); // @todo Only output error message once
		factoryPresetsDir.append("Presets\\Factory\\");
	}
	if (userPresetsDir.empty())
	{
		if(!getProductDir(userPresetsDir))
			Platform::PopupMessage("Plugin Data not found!"); // @todo Only output error message once
		userPresetsDir.append("Presets\\User\\");
		// Pre-emptively create the user presets directory if it doesn't already exist.
		// Ignore any errors. (@todo Consider at least logging the error somewhere, but don't bother the user with it.)
		CreateDirectory(userPresetsDir.c_str(), NULL);
	}

    return forUser ? userPresetsDir : factoryPresetsDir;
}

std::string getAppearanceDirectory(bool forUser)
{
    static std::string factoryAppearanceDir;
    static std::string userAppearanceDir;
    if (factoryAppearanceDir.empty())
    {
        if(!getProductDir(factoryAppearanceDir))
            Platform::PopupMessage("Plugin Data not found!"); // @todo Only output error message once
        factoryAppearanceDir.append("Appearance\\Factory\\");
    }
    if (userAppearanceDir.empty())
    {
        if(!getProductDir(userAppearanceDir))
            Platform::PopupMessage("Plugin Data not found!"); // @todo Only output error message once
        userAppearanceDir.append("Appearance\\User\\");
        // Pre-emptively create the user appearance directory if it doesn't already exist.
        // Ignore any errors. (@todo Consider at least logging the error somewhere, but don't bother the user with it.)
        CreateDirectory(userAppearanceDir.c_str(), NULL);
    }

    return forUser ? userAppearanceDir : factoryAppearanceDir;
}

void PopupMessage(const char* text, const char* caption)
{
    MessageBox(0, text, caption, MB_OK);
}

std::string appendPathComponent(const std::string& basePath, const char* extraPathComponent)
{
    std::string filePath(basePath);
    if (basePath[basePath.size() - 1] != '\\')
    {
        filePath.append("\\");
    }
    filePath.append(extraPathComponent);
    return filePath;
}

unsigned long long getFileId(HANDLE hFile)
{
    unsigned long long result = -1;
    BY_HANDLE_FILE_INFORMATION fileInfo;
    BOOL infoResult = GetFileInformationByHandle(hFile, &fileInfo);
    if (infoResult)
    {
        result = fileInfo.nFileIndexHigh;
        result <<= 32;
        result |= fileInfo.nFileIndexLow;
    }
    return result;
}

unsigned long long getFileId(const char* name)
{
    unsigned long long result = -1;
    std::string filePath(getPresetsDirectory(true));
    filePath.append("/");
    filePath.append(name);
    filePath.append(".yaml");

    HANDLE hFile = CreateFile(filePath.c_str(), // file to open
                              GENERIC_READ,     // open for reading
                              FILE_SHARE_READ,  // share for reading
                              NULL,             // default security
                              OPEN_EXISTING,    // existing file only
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // normal file
                              NULL);            // no attr. template
    if (hFile != INVALID_HANDLE_VALUE)
    {
        result = getFileId(hFile);
        CloseHandle(hFile);
    }
    return result;
}

unsigned long long getFileId(FILE* openFile)
{
    unsigned long long result = -1;
    int fd = fileno(openFile);
    HANDLE hFile = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
    if (hFile != INVALID_HANDLE_VALUE)
    {
        result = getFileId(hFile);
    }
    return result;
}

bool exploreDirectory(const char *directory)
{
	HINSTANCE result = ShellExecute(NULL, "explore", directory, NULL, NULL, SW_SHOWNORMAL); // works in Windows XP and sup?
	return (reinterpret_cast<HINSTANCE>(32) <= result);
}

bool launchBrowser(const char *url)
{
	HINSTANCE result = ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
	return (reinterpret_cast<HINSTANCE>(32) <= result);
}

bool launchBrowserWithLocalFile(const char *path)
{
    // In Windows, you can just supply a file path to the shell and have it open
    // in the web browser (presumably if the extension is appropriate).
    return launchBrowser(path);
}

int utf8CaseInsensitiveCompare(const std::string& str1, const std::string& str2)
{
	int wideLen1 = MultiByteToWideChar(CP_UTF8, 0, str1.c_str(), -1, NULL, 0);
	wchar_t* wcstr1 = new wchar_t[wideLen1];
	MultiByteToWideChar(CP_UTF8, 0, str1.c_str(), -1, wcstr1, wideLen1);
	int wideLen2 = MultiByteToWideChar(CP_UTF8, 0, str2.c_str(), -1, NULL, 0);
	wchar_t* wcstr2 = new wchar_t[wideLen2];
	MultiByteToWideChar(CP_UTF8, 0, str2.c_str(), -1, wcstr2, wideLen2);
	LCID lcid = GetUserDefaultLCID();
	int result = CompareStringW(lcid, NORM_IGNORECASE, wcstr1, -1, wcstr2, -1);
	delete[] wcstr1;
	delete[] wcstr2;
	// For some strange reason, the Windows API doesn't use the same convention almost everyone else in the world
	// uses for string comparisons, which is to return <0 for string before, 0 for same and >0 for after.
	// Instead, Microsoft decided to use 1, 2 and 3.
	return (result == CSTR_LESS_THAN) ? -1 : (result == CSTR_GREATER_THAN) ? 1 : 0;
}

} // namespace Platform

bool getProductDir(std::string &dest)
{
	char dir[1024];

	// get DLL path
    // Note that in non 16-bit Windows, HMODULE and HINSTANCE are the same thing.
    // See https://blogs.msdn.microsoft.com/oldnewthing/20040614-00/?p=38903 for a discussion of the history.
	char* scan = dir+GetModuleFileName((HMODULE)hInstance, dir, sizeof dir);
	while (*scan != '\\') --scan;
	scan[1] = '\0';	

	dest = dir;

	if(dest.size() > 5)
		return true;
	else
		return false;
}

