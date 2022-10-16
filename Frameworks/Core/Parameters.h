//
//  Parameters.h
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#pragma once

#include <string>
#include <vector>

/// Encapsulates information about the parameters used by the synth / effect,
/// including parameter names, default values, displayed values and dependencies.
class ParameterDefinition
{
public:
    ParameterDefinition(size_t id, const char* shortName, float normalisedDefaultValue, bool dynamicLabel = false);
    ParameterDefinition(size_t id, const char* shortName, float normalisedDefaultValue, const char *label);
    ~ParameterDefinition();
    void setFullName(const char* fullName);
    /// Sets the minimum and maximum values this parameter can have when scaled from the normalised [0.0..1.0] range
    /// back to the parameter's natural range.
    void setNaturalValueRange(float minimumValue, float maximumValue);
    void addDisplayDependent(size_t id);

    size_t getId() const { return m_id; }
    const std::string& getShortName() const { return m_shortName; }
    const std::string& getFullName() const;
    const std::string& getLabel() const { return m_label; }
    bool hasLabel() const { return m_labelIsDynamic || !m_label.empty(); }
    bool labelIsDynamic() const { return m_labelIsDynamic; }
    float getNormalisedDefaultValue() const { return m_normalisedDefaultValue; }
    float getNaturalDefaultValue() const;
    float getNaturalMinimumValue() const { return m_minimumValue; }
    float getNaturalMaximumValue() const { return m_maximumValue; }

private:
    /// Numeric ID used to identify the parameter.
    size_t m_id;

    /// The short name of the parameter, for use in plugin formats (such as VST)
    /// that limit the length of names.
    std::string m_shortName;

    /// The unabbreviated name of the parameter (can be the same as the short name).
    /// Defaults to the short name.
    std::string m_fullName;

    /// A label (such as units) that a host can request to use when displaying
    /// parameter values.
    std::string m_label;

    /// The default value of the parameter.
    float m_normalisedDefaultValue;

    /// The minimum value of the parameter. Defaults to 0.0.
    float m_minimumValue;

    /// The maximum value of the parameter. Defaults to 1.0.
    float m_maximumValue;

    /// True if the parameter label is set at run-time, i.e. the label depends on the value of this parameter or even
    /// a related parameter. False if the parameter label is absent or static. If true, the plugin-specific DSP engine
    /// subclass is expected to implement a function that returns the dynamic label.
    bool m_labelIsDynamic;

    /// Identifiers for dependent parameters that need to be redisplayed when
    /// the value of this parameter changes.
    std::vector<size_t> m_redisplayDependents;
};

/// Encapsulates the collection of all parameter definitions for the synth / effect.
class Parameters
{
public:
    Parameters();
    ~Parameters();

    /// Define a new parameter and add it to the collection. Parameters must be
    /// defined in numerical sequence, without gaps.
    ParameterDefinition* define(size_t id, const char* shortName, float normalisedDefaultValue, bool dynamicLabel = false);
    ParameterDefinition* define(size_t id, const char* shortName, float normalisedDefaultValue, const char *label);

    size_t getNumParameters() const { return m_definitions.size(); }

    const std::string& getShortName(size_t id) const;
    const std::string& getFullName(size_t id) const;
    const std::string& getLabel(size_t id) const;
    bool hasLabel(size_t id) const;
    bool labelIsDynamic(size_t id) const;
    float getNormalisedDefaultValue(size_t id) const;
    float getNaturalDefaultValue(size_t id) const;
    float getNaturalMinimumValue(size_t id) const;
    float getNaturalMaximumValue(size_t id) const;

private:
    std::vector<ParameterDefinition*> m_definitions;
};

/// Encapsulates a mapping from parameter IDs used in a previous version of the plugin to the current IDs.
///
/// @note
///     This does not mean that a new version of a plugin with the same plugin ID can remove parameters, because hosts
///     still save things like automation info using a parameter ID, so renumbering parameter IDs will break projects.
///     This mapping is used when creating a new plugin, with a different plugin ID, that supercedes an older plugin.
///     Such a plugin won't be substituted for the old plugin in an existing project, but it can import presets, banks
///     and chunks from the older plugin.
class ParameterIdMap
{
public:
    ParameterIdMap();
    ~ParameterIdMap();

    /// Define a new parameter ID mapping.
    /// Mappings and ignores must be defined in the numerical sequence of the old parameter IDs, without gaps.
    void define(size_t oldId, size_t newId);

    /// Define an old parameter ID as no longer being used.
    /// Mappings and ignores must be defined in the numerical sequence of the old parameter IDs, without gaps.
    void ignore(size_t oldId);

    size_t getNumOldParameters() const { return m_newParameterIds.size(); }

    /// Returns the new ID for the specified old parameter ID, or returns the value kIgnored if the old parameter ID is
    /// no longer used.
    size_t getNewIdForOldParameter(size_t oldParameterId) const;

    enum
    {
        kIgnored = -1
    };

private:
    /// Vector with values being the new parameter ID corresponding to the old parameter ID used as an index into the
    /// vector.
    std::vector<size_t> m_newParameterIds;
};
