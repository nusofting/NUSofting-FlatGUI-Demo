//
//  Modulations.cpp
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#include "Modulations.h"

#include <cassert>


class ModulationMap
{
public:
    ModulationMap(size_t sourceId);
    ~ModulationMap() { };

    void addAllowedTarget(ModulationDefinition* target);
    size_t getNumAllowedTargets() const { return m_allowedTargets.size(); }

    /// Returns the name of the target at the specified index in the vector of
    /// allowed targets in this modulation map. This index is not necessarily
    /// the same as the target's ID.
    const char* getTargetName(size_t targetIndexInMap);

    /// Select an active modulation target by index in the vector of allowed targets
    /// in this modulation map.
    ///
    /// @return
    ///     The original definition ID for the selected target index.
    size_t selectTarget(size_t targetIndexInMap);

    size_t getSelectedTargetId() const { return m_selectedTargetId; }

private:
    size_t m_sourceId;
    std::vector<ModulationDefinition*> m_allowedTargets;
    size_t m_selectedTargetIndex;
    size_t m_selectedTargetId;
};


//==============================================================================

ModulationDefinition::ModulationDefinition(size_t id, const char* name)
 : m_id(id),
   m_name(name ? name : "")
{
}

ModulationDefinition::~ModulationDefinition()
{
}


//==============================================================================

Modulations::Modulations()
{
}

Modulations::~Modulations()
{
}

ModulationDefinition* Modulations::defineSource(size_t id, const char* name)
{
    ModulationDefinition* modDefinition = new ModulationDefinition(id, name);
    m_sources.push_back(modDefinition);
    assert(m_sources.size() - 1 == id);
    m_maps.push_back(new ModulationMap(id));
    return modDefinition;
}

ModulationDefinition* Modulations::defineTarget(size_t id, const char* name)
{
    ModulationDefinition* modDefinition = new ModulationDefinition(id, name);
    m_targets.push_back(modDefinition);
    assert(m_targets.size() - 1 == id);
    return modDefinition;
}

void Modulations::createMatrix()
{
    assert(!m_sources.empty() && !m_targets.empty());
    for (size_t i = 0; i < m_sources.size(); ++i)
    {
        m_sourceMatrix.push_back(std::vector<float>(m_targets.size(), 1.0f)); // changed to 1.0f because modulators will be multiplied
    }
}

const std::string& Modulations::getSourceName(size_t id) const
{
    return m_sources[id]->getName();
}

const std::string& Modulations::getTargetName(size_t id) const
{
    return m_targets[id]->getName();
}

void Modulations::addTargetForSource(size_t sourceId, size_t targetId)
{
    assert(sourceId < m_maps.size());
    ModulationMap *mapForSource = m_maps[sourceId];
    assert(mapForSource);
    assert(targetId < m_targets.size());
    mapForSource->addAllowedTarget(m_targets[targetId]);
}

std::vector<const char*> Modulations::getTargetNamesForSource(size_t sourceId)
{
    assert(sourceId < m_maps.size());
    ModulationMap *mapForSource = m_maps[sourceId];
    assert(mapForSource);
    std::vector<const char*> names;
    for (size_t i = 0; i < mapForSource->getNumAllowedTargets(); ++i)
    {
        names.push_back(mapForSource->getTargetName(i));
    }
    return names;
}

void Modulations::selectExclusiveTargetForSource(size_t sourceId, size_t targetIndexInMap)
{
    assert(sourceId < m_maps.size());
	assert( sourceId < m_sourceMatrix.size());
    ModulationMap *mapForSource = m_maps[sourceId];
    assert(mapForSource);
    size_t targetId = mapForSource->selectTarget(targetIndexInMap); // return not used
    //ModulatorOutputs modOuts = m_sourceMatrix[sourceId]; UNUSED!
    //for (size_t i = 0; i < modOuts.size(); ++i)
    //{
    //    if (i != targetId)
    //    {
    //        modOuts[i] = 0.0f; // zero the unused input jacks
    //    }
    //}
}

size_t Modulations::getExclusiveTargetForSource(size_t sourceId)
{
    assert(sourceId < m_maps.size() && sourceId < m_sourceMatrix.size());
    ModulationMap *mapForSource = m_maps[sourceId];
    assert(mapForSource);
    return mapForSource->getSelectedTargetId();
}

void Modulations::inputFromSource(size_t sourceId, float inputValue) //this sets an input into the matrix
{
    assert(sourceId < m_maps.size() && sourceId < m_sourceMatrix.size());
    ModulationMap *mapForSource = m_maps[sourceId];
    assert(mapForSource);
    // Currently we only support one target per source, but in principle we can
    // extend this design to a full modulation matrix if we want to in the future.
    size_t selectedTargetId = mapForSource->getSelectedTargetId();

    ModulatorOutputs& sourceModOuts = m_sourceMatrix[sourceId];
    sourceModOuts[selectedTargetId] = inputValue;
}

float Modulations::outputForTarget(size_t sourceId, size_t targetId) // this gets an output from the matrix
{
    assert(sourceId < m_sourceMatrix.size());
    ModulatorOutputs& sourceModOuts = m_sourceMatrix[sourceId];
    assert(targetId < sourceModOuts.size());
    return sourceModOuts[targetId];
}

//==============================================================================

ModulationMap::ModulationMap(size_t sourceId)
 : m_sourceId(sourceId),
   m_selectedTargetIndex(0),
   m_selectedTargetId(0)
{
}

void ModulationMap::addAllowedTarget(ModulationDefinition* target)
{
    m_allowedTargets.push_back(target);
}

const char* ModulationMap::getTargetName(size_t targetIndexInMap)
{
    assert(targetIndexInMap < m_allowedTargets.size());
    ModulationDefinition *modDef = m_allowedTargets[targetIndexInMap];
    assert(modDef);
    return modDef->getName().c_str(); /// @todo check string lifetime.
}

size_t ModulationMap::selectTarget(size_t targetIndexInMap)
{
    assert(targetIndexInMap < m_allowedTargets.size());
    m_selectedTargetIndex = targetIndexInMap;
    ModulationDefinition *modDef = m_allowedTargets[targetIndexInMap];
    assert(modDef);
    m_selectedTargetId = modDef->getId();
    return m_selectedTargetId;
}
