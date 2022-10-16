//
//  Platform.h
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#pragma once

#include <sstream>
#include <string>
#include <vector>

/// A namespace encapsulating functions that retrieve platform-specific data or have
/// platform-specific implementations. These are gathered here with platform-netral
/// interfaces in order to prevent too much conditional compilation in the rest of
/// the code. The platform-specific implementations reside in different implementation
/// files for each platform.
///
/// This class should be configured quite early during plugin startup (via the
/// Platform::init() function, because it provides services throughout the plugin
/// lifetime and in some platforms / implementations there may be some OS-specific
/// initialisation that needs to happen quite early.
namespace Platform
{

/// Perform platform-specific initialisation.
///
/// @param pluginName
///     The user-visible name for the plugin. This may contain spaces.
/// @param pluginKey
///     A string used as the final component of a registry key or preferences
///     domain name. This should uniquely identify the plugin and ideally
///     not contain spaces.
/// @todo This plugin identification info should be moved out of the platform and
///     into some kind of configuration object.
void init(const char *pluginName, const char *pluginKey);

/// Retrieve the initialises plugin name and key. This is currently mainly for
/// the convenience of the platform implementation.
/// @todo See if there is a better way of stashing the plugin name and key for the platform.
const char* getPluginName();
const char* getPluginKey();

/// Returns the full path for the specified file in the plugin resource data
/// directory for the platform. The resource data directory is the directory
/// tree that contains files the user should not be allowed to modify.
std::string getPathForAppResource(const char* fileName);

/// Returns the root directory for plugin data that we permit the user to modify
/// (e.g. skins etc.) for the platform.
std::string getAppUserDataDirectory();

/// Returns the full path for the specified file in the user data directory for
/// the platform. This is typically used for files that the user can supply to
/// override plugin resources, e.g. replacing appearance background tiles.
std::string getPathForUserResource(const char* fileName);

/// Returns the default presets directory for the platform.
///
/// @param forUser
///     If true, returns the user presets directory otherwise returns the
///     factory presets directory.
std::string getPresetsDirectory(bool forUser = true);

/// Returns the directory containing the appearance files.
///
/// @param forUser
///     If true, returns the directory containing the user's appearance files,
///     otherwise returns the factory appearance directory.
std::string getAppearanceDirectory(bool forUser = true);

/// Builds a file name using only characters safe in both Windows and OS X file
/// systems.
std::string buildSafeFileName(const std::string& path, const std::string& unsafeName, const char* extension);

/// Builds a file name for saving diagnostic messages, using the plugin name and the current date/time.
/// This is intended to be used as a default file name in a file "save as" dialog.
std::string buildMessagesFileName();

/// Builds a version string decorated with plugin format (if available) and machine architecture.
std::string buildVersionString(const std::string& versionNumberPrefix);

/// Displays a popup message in a platform-specific message box / alert.
///
/// This can be called from any thread, including the audio thread if
/// interrupting the audio is acceptable (e.g. beta / demo expiry).
/// @note On macOS, this is asynchronous, because UI code must only run on the
/// main (GUI) thread. On Windows, it appears to be permitted to call MessageBox
/// from any thread and block that thread. Because this has to be asynchronous
/// on one platform, it cannot ever be used to return a user choice (i.e. a Yes / No
/// question cannot be posed with this function).
///
/// @param text
///     The text of the message to display.
/// @param caption
///     The caption for the message box / alert window. Defaults to "Error!".
void PopupMessage(const char* text, const char* caption = "Error!");

/// Encapsulates functions called for each file and subdirectory when
/// iterating through a directory tree.
class DirTreeCallback
{
public:
    /// Report a file found in the directory.
    ///
    /// @param filePath
    ///     The full path to the found file.
    virtual void run(const std::string &filePath) = 0;

    /// Report recursion into a subdirectory.
    ///
    /// @param leafSubdir
    ///     The name of the subdirectory only (i.e. not the full path)
    /// @return
    ///     Returns a pointer to an optional string to restore when
    ///     leaving the subdirectory. In practice, this is likely to be
    ///     the previous subdirectory (if any) that was set.
    virtual const char* enterSubdir(const char* leafSubdir) = 0;

    /// Report leaving a subdirectory recursed into.
    ///
    /// @param restoreData
    ///     The string pointer that was returned from the enterSubdir() call.
    virtual void leaveSubdir(const char* restoreData) = 0;
};

/// Iterates through the files in the specified directory, optionally filtered
/// by matching an extension. Optionally recurses through subdirectories.
///
/// @param basePath
///     The directory to search.
/// @param callback
///     A callback object encapsulating a function to call for each matching
///     file.
/// @param extension
///     If non-null, only the files that case-sensitively end with the
///     specified extention are returned.
/// @param maxDepth
///     The maximum depth of the directory tree to search, where a value of
///     1 means search the immediate directory only and a value of 0 means
///     search the full subtree.
void forEachFileInDir(const std::string &basePath,
                      DirTreeCallback& callback,
                      const char* extension = 0,
                      size_t maxDepth = 0);

/// Appends a path component (e.g. a file name) to a base path, adding a platform-specific path separator if necessary.
///
/// @param basePath
///     The base path to be appended to. May already end with a platform-specific path separator but this is not
///     compulsory.
/// @param extraPathComponent
///     The path component to be appended to the base path. Should not start with a path separator.
/// @return
///     The combined path, with an included path separator if it was needed.
std::string appendPathComponent(const std::string& basePath, const char* extraPathComponent);

unsigned long long getFileId(const char* name);
unsigned long long getFileId(FILE* openFile);

/// Opens a system directory viewing window at the specified directory.
/// On Windows, this is the File Explorer, on OS X it is a Finder window.
///
/// @return
///     true if the system says the file viewer was successfully opened,
///     false if the file viewer failed to be opened.
bool exploreDirectory(const char *directory);

/// Opens specified URL in the system default web browser.
///
/// @return
///     true if the system says the browser was successfully launched,
///     false if the browser launch failed.
bool launchBrowser(const char *url);

/// Opens the specified local file in the system default web browser.
///
/// @param path
///     The path to the local file to open.
///
/// @return
///     true if the system says the browser was successfully launched,
///     false if the browser launch failed.
bool launchBrowserWithLocalFile(const char *path);

/// Case-insensitively (within the limits of the platform's APIs) compares two
/// strings in UTF-8 encoding.
/// @return
///     0   if the string strings are considered equal (according to the platform),
///     < 0 if str1 lexically sorts before str2 (according to the platform),
///     > 0 if str1 lexically sorts after str2 (according to the platform)
int utf8CaseInsensitiveCompare(const std::string& str1, const std::string& str2);


/// Encapsulates a container for logged messages that can be displayed to the
/// user upon request.
/// This is in the platform layer, because a future enhancement may be to also
/// send the logged messages to a platform-specific log target, e.g. the log
/// console on OS X or a log file or debug viewer on Windows.
class Logger
{
public:
    Logger() { }
    typedef std::vector<std::string> BufferType;
    BufferType& getLines() { return m_lines; }
    void appendLine(const std::string& line) { m_lines.push_back(line); }

    /// Saves the logged data to the specified file.
    bool saveFile(const std::string& filePath);

private:
    std::vector<std::string> m_lines;
};

/// Helper class for logging a single log line at a time to the specified logger
/// object. This class makes use of the C++ constructor and destructor semantics
/// to accumulate (via a C++ output stream) all the parts of a log line. Then,
/// when the object goes out of scope, it passes the buffered string to the
/// actual logger object as a single, complete line. It is typically used via
/// the following construct:
///     Platform::LogLine(logger) << "line 1 string " << variable << " etc.";
///     Platform::LogLine(logger) << "line 2 string " << whatever << " more";
/// Each instance of Platform::LogLine(logger) is a temporary variable that
/// goes out of scope (and thus triggers the destructor) at the trailing semicolon.
///
/// This class was adapted from the following StackOverflow answer:
/// http://stackoverflow.com/questions/8337300/c11-how-do-i-implement-convenient-logging-without-a-singleton
class LogLine
{
public:
    LogLine(Logger& log) : m_log(log) {}
    ~LogLine()
    {
        m_log.appendLine(m_lineBuffer.str());
    }

    /// Template member function that allows anything type that can write to C++
    /// output streams to append to the log line buffer.
    template <class T>
    LogLine& operator<<(const T& thing) { m_lineBuffer << thing; return *this; }

private:
    std::stringstream m_lineBuffer;
    Logger& m_log;
};

} // namespace Platform

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
