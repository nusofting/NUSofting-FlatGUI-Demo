#pragma once
//
//  Modulations.h
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include <string>
#include <vector>

/// Encapsulates information about the modulation sources and targets used by the synth / effect.
/// This is currently just a name and ID, but extracted out into a simple class for future expansion.
class ModulationDefinition
{
public:
    ModulationDefinition(size_t id, const char* name);
    ~ModulationDefinition();

    size_t getId() const { return m_id; }
    const std::string& getName() const { return m_name; }

private:
    /// Numeric ID used to identify the modulation.
    size_t m_id;

    /// The name of the modulation source or target.
    std::string m_name;
};

/// Forward declaration of an implementation detail class that keeps track of a
/// mapping from a modulation source to a modulation target.
class ModulationMap;

/// Encapsulates the collection of all modulation definitions for the synth / effect.
class Modulations
{
public:
    Modulations();
    ~Modulations();

    /// Define a new modulation source and add it to the collection. Modulations sources must be
    /// defined in numerical sequence, without gaps.
    ModulationDefinition* defineSource(size_t id, const char* name);

    /// Define a new modulation target and add it to the collection. Modulations sources must be
    /// defined in numerical sequence, without gaps.
    ModulationDefinition* defineTarget(size_t id, const char* name);

    /// Creates the actual modulation matrix, with all entries zeroed out.
    /// This must be called after all the modulation sources and modulation targets have been defined.
    void createMatrix();

    size_t getNumModulationSources() const { return m_sources.size(); }
    size_t getNumModulationTargets() const { return m_targets.size(); }

    const std::string& getSourceName(size_t id) const;
    const std::string& getTargetName(size_t id) const;

    /// Define the allowed targets for each source. Some synths / effects may allow any source
    /// to be mapped to any target, but others may want a more restricted mapping.
    void addTargetForSource(size_t sourceId, size_t targetId);

    /// Return the list of allowed target names for the specified source. This vector contains
    /// names in the same order as the targets were added to the source, and the index of the
    /// target in this vector can (and must) be used as the index identifying this target in
    /// function calls that query information about this target's modulation relationship to
    /// this source ID only.
    std::vector<const char*> getTargetNamesForSource(size_t sourceId);

    /// Select a modulation target as the exclusive target for the specified source.
    /// That is, all previously selected targets are zeroed out for this source in the
    /// modulation matrix.
    ///
    /// @param sourceId
    ///     The identifier for the modulation source.
    /// @param targetIndexInMap
    ///     The modulation target identified by its index within the vector of allowed
    ///     targets for the specified source.
    void selectExclusiveTargetForSource(size_t sourceId, size_t targetIndexInMap);

    /// Returns the target ID of the target previously selected for the specified source.
    size_t getExclusiveTargetForSource(size_t sourceId);

    /// Sets an input value for the specified source into the "slot" for the currently
    /// selected target for that source.
    void inputFromSource(size_t sourceId, float inputValue);

    /// Gets the modulation output value for the specified combination of source and target.
    float outputForTarget(size_t sourceId, size_t targetId);

private:
    std::vector<ModulationDefinition*> m_sources;
    std::vector<ModulationDefinition*> m_targets;
    std::vector<ModulationMap*> m_maps;
    /// Helper type definition for an implementation detail that keeps track of how
    /// a single modulation source is distributed to all the targets.
    typedef std::vector<float> ModulatorOutputs;
    /// Helper type definition for an implementation detail that keeps track of how
    /// modulation sources are distributed to their current targets.
    typedef std::vector<ModulatorOutputs> ModulationMatrix;
    /// Modulation outputs indexed first by source ID and then by target ID.
    ModulationMatrix m_sourceMatrix;
};
