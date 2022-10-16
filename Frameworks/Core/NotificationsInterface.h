//
//  NotificationsInterface.h
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//


/// We want to separate the internal API of the synth / effect from any
/// specific plugin API (e.g. VST). This is partly due to not tying us to any
/// specific type of plugin but more importantly it allows us to separate the
/// sources of changes to the plugin state. A specific example: if we used the
/// VST API throughout, then we wouldn't be able to distinguish between changes
/// sent by the host vs changes made by the user in the UI.

//#include "Types.h"


namespace Platform
{
    class Logger;
}

/// Abstract interface defining notifications sent between the various components
/// of the synth / effect: the core, the editor and the DSP engine.
/// The typical structure is that the core will know the specific classes for
/// the editor and the DSP engine and the plugin host but the latter three do
/// not need to know each other's specific classes: they all communicate via
/// this interface.
class NotificationsInterface
{
public:
    virtual ~NotificationsInterface() { }

    /// Notify that the currently selected program / preset has changed in the
    /// preset manager. Query the preset manager for current values.
    virtual void programChanged() = 0;
    // @todo delete if we don't need this, programChanged() should be sufficient
    //virtual void selectProgram(size_t categoryIndex, size_t programIndexInCategory) = 0;

    /// Notify that the current program's value set (i.e. initial vs currently
    /// edited parameter values) has changed. Query the current program in the
    /// preset manager for the current values to use.
    virtual void paramValueSetChanged() = 0;

    /// Request that a preset or bank file from an older plugin version be converted.
    /// The file can be a DashLibrary .bnk or a VST FXB or FXP file.
    ///
    /// @param filePath
    ///     The full path for the file containing the preset or bank to convert.
    /// @param logger
    ///     A logger instance to capture a summary of the conversion, along with any errors or issues.
    ///     These messages are suitable for displaying to the user.
   // virtual void convertPresetsFile(const char* filePath, Platform::Logger& logger) = 0;
};
