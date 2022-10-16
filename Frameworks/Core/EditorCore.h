//
// EditorCore.h
//
// Copyright 2016 Bernie Maier. All rights reserved.
// Licensed to NUSofting.
//
#pragma once


class EditorToPluginCoreInterface;
namespace Platform
{
    class Logger;
}


/// Define the interface that the plugin core uses to communicate with the GUI.
class EditorCore
{
public:
    EditorCore(EditorToPluginCoreInterface& notifyPluginCore)
     : m_notifyPluginCore(notifyPluginCore)
    {
    }

    virtual bool open(void* ptr) = 0;
    virtual void close() = 0;
    virtual void idle() = 0;

    /// Inform the editor that either the parameter value has changed
    /// or otherwise needs to be redisplayed (e.g. a related parameter has
    /// changed in a way that changes its visual interpretation).
    virtual void refreshParameter(int index, float value) = 0;
    virtual void programChanged(size_t categoryIndex, size_t programInCategoryIndex) = 0;

    /// Notify parameter change from core to editor.
    virtual void setDisplayParameterInt(int msgIndex, int value) = 0;

    /// Display a message box with static message text. The text must be static
    /// because this could be called from the audio thread and we must not make
    /// any memory allocations in the audio thread.
    ///
    /// The editor should display this message in its next idle run.
    virtual void displayStaticMessage(const char* message, const char* title) = 0;

    /// Return the width and height of the editor window.
    /// @{
    virtual unsigned int getWidth() const = 0;
    virtual unsigned int getHeight() const = 0;
    /// @}

protected:
    EditorToPluginCoreInterface& m_notifyPluginCore;
};


/// Abstract interface defining notifications sent from the editor to the plugin
/// core. The editor does not / must not communicate directly with the DSP code,
/// the VST SDK or the plugin core except via this interface.
///
/// @note
/// This class / interface currently cheats a little bit because it includes some
/// functions that do more than just notify the plugin core about something, they
/// actually request either an action be performed or data be returned.
/// @todo Clean up these non-notification functions in a future framework version.
class EditorToPluginCoreInterface
{
public:
    virtual ~EditorToPluginCoreInterface() { }

    /// Notifications
    /// @{

    /// Notify that the currently selected program / preset has changed in the
    /// preset manager. Query the preset manager for current values.
    virtual void programChanged() = 0;

    /// Notify that the current program's value set (i.e. initial vs currently
    /// edited parameter values) has changed. Query the current program in the
    /// preset manager for the current values to use.
    virtual void paramValueSetChanged() = 0;

    /// Notify that the value of a parameter in the current program has changed.
    virtual void paramValueChanged(size_t index, float value) = 0;

    /// Notify "display" parameter change from editor to core.
    /// Display parameters are extra parameters defined by the editor rather
    /// than the DSP engine, and are thus not known to the host.
    virtual void changedDisplayParameterInt(int msgIndex, int value) = 0;

    /// Notify the core that the editor is changing its window size.
    virtual bool notifySizeChange(unsigned int newWidth, unsigned int newHeight) = 0;

    /// Notify the core that a control is starting to be manipulated in the
    /// editor and thus its value is likely to change.
    ///
    /// @param index
    ///     The tag defined for the control being used. This tag can be for any
    ///     control, not just the controls representing parameters. The core is
    ///     reponsible for separating parameter controls from other controls.
    ///     Parameter controls must be tagged with their parameter ID and no
    ///     other controls can use tags within the range of valid parameter IDs.
    virtual void notifyBeginEdit(size_t index) = 0;

    /// Notify the core that a control is no longer being manipulated in the
    /// editor.
    ///
    /// @param index
    ///     The tag defined for the control being used. This tag can be for any
    ///     control, not just the controls representing parameters. The core is
    ///     reponsible for separating parameter controls from other controls.
    ///     Parameter controls must be tagged with their parameter ID and no
    ///     other controls can use tags within the range of valid parameter IDs.
    virtual void notifyEndEdit(size_t index) = 0;

    /// @}

    /// Requests
    /// @{

    /// Request that a preset or bank file from an older plugin version be converted.
    /// The file can be a DashLibrary .bnk or a VST FXB or FXP file.
    ///
    /// @param filePath
    ///     The full path for the file containing the preset or bank to convert.
    /// @param logger
    ///     A logger instance to capture a summary of the conversion, along with any errors or issues.
    ///     These messages are suitable for displaying to the user.
    virtual void requestPresetsFileConversion(const char* filePath, Platform::Logger& logger) = 0;

	virtual void requestLoadAndAutosaveYamls(const char* filePath, Platform::Logger& logger) = 0;

    /// Request that the formatted value of the indexed parameter is copied to the specified buffer.
    /// This is expected to be called from the GUI thread, and can access cached parameter values in the DSP engine.
    virtual void requestForFormattedParameterString(int index, char* buffer, size_t len) = 0;
	   /// @}

	virtual float requestDefaultValue(int index) = 0; 
};
