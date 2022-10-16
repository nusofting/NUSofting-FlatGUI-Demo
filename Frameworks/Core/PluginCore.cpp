//
// PluginCore.cpp
//
// Copyright 2016 Bernie Maier. All rights reserved.
// Licensed to NUSofting.
//

#include "PluginCore.h"

#include "DspEngine.h"
#include "EditorCore.h"
#include "Parameters.h"
#include "PluginFactory.h"
#include "Vst2Plugin.h"

#include "PresetManager/Program.h"
#include "Utils/Utils.h"

#include <cassert>
#include <fstream>
#include <memory>

#include "vst2.4/public.sdk/source/vst2.x/audioeffect.h"
#include "vst2.4/public.sdk/source/vst2.x/vstfxstore.h"

//------------------------------------------------------------------------------

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
    PluginFactory* factory = PluginCore::entry();
    const PluginProperties& pp = factory->getProperties();
    DspEngine& dsp = factory->getDspEngine();
    PluginCore* plugin = factory->getPluginInstance(); // Creates a subclass of PluginCore
    PresetManager& presetManager = plugin->getPresetManager();
    Vst2Plugin *vst2Plugin = new Vst2Plugin(pp, *plugin, dsp, presetManager);
    plugin->connectVst2Plugin(vst2Plugin);
    vst2Plugin->connectHost(audioMaster);
    return vst2Plugin;
}

PluginCore::PluginCore(const PluginProperties& pluginProps,
                       Parameters& parameters,
                       DspEngine& dsp,
                       PluginFactory* factory)
 : m_pluginProperties(pluginProps),
   m_parameters(parameters),
   m_numParams(pluginProps.numParams),
   m_programFactory(m_parameters, pluginProps.nameAsId, pluginProps.versionStr, m_errorLog),
   m_presetManager(m_programFactory),
   m_notifyEditor(0),
   m_dspToPluginNotifier(*this),
   m_dspToEditorNotifier(*this),
   m_dsp(dsp),
   m_vst2Plugin(0),
   m_factory(factory)
{
    // We can't do all the initialisation here because some of the initialisation needs to happen in the subclass,
    // and the subclass constructor isn't called until this class's constructor is complete.
    m_dsp.connectToCore(m_dspToPluginNotifier, m_dspToEditorNotifier);
    m_dsp.globalInit();
    m_dsp.initProcess();
    m_presetManager.loadPresets();
    m_presetManager.selectProgram(0, 0);
}

PluginCore::~PluginCore()
{
}

PluginFactory* PluginCore::entry()
{
    PluginFactory* factory = PluginFactory::create();
    const PluginProperties& pp = factory->getProperties();
    DspEngine& dsp = factory->getDspEngine();
    Parameters& parameters = factory->getParameters();
    dsp.defineParameters(parameters);
    assert(parameters.getNumParameters() == pp.numParams);
    Platform::init(pp.name, pp.nameAsId);
    PluginCore* plugin = factory->getPluginInstance(); // Creates a subclass of PluginCore
    plugin->init();
    return factory;    
}

void PluginCore::init()
{
    // Can now call virtual functions to pick up the subclass implementations.
    defineParameterIdMappings();
    m_notifyEditor = getEditor();
}

void PluginCore::connectVst2Plugin(Vst2Plugin* vst2Plugin)
{
    if (m_notifyEditor)
    {
        vst2Plugin->connectEditor(*m_notifyEditor);
    }
    setActiveProgram(m_presetManager.getCurrentProgram(), SRC_SET_PROGRAM);
    m_vst2Plugin = vst2Plugin;
}

void PluginCore::shutdown()
{
    // If the AudioEffect object has a non-null plugin pointer, it deletes it.
    // This is bad design! A library class shouldn't delete objects it didn't create.
    // At least we can clear out the editor from AudioEffect and let the editor be deleted by the object that created it.
    m_vst2Plugin->setEditor(0);
    m_notifyEditor = 0;
    PluginFactory::destroy(m_factory);
}

void PluginCore::setProgramFromHost(size_t program)
{
    /// Nothing to do because we have only one program in the host menu and programs are set internally either via MIDI
    /// program change or via the preset controls on the editor.)
}

void PluginCore::selectFactoryProgramByIndex(size_t index)
{
    m_presetManager.selectFactoryProgramByIndex(index);
    setActiveProgram(m_presetManager.getCurrentProgram(), SRC_SET_PROGRAM);
}

void PluginCore::setParameterFromHost(size_t index, float value)
{
    notifyParameterChange(index, value, SRC_HOST_AUTOMATION);
}

const Program *PluginCore::getProgramForProjectSave()
{
#if DEMO
    if (m_notifyEditor)
    {
        static bool shownOnce = false;
        if (!shownOnce)
        {
            m_notifyEditor->displayStaticMessage("Demo version doesn't save project / host presets.", "Warning!");
        }
        shownOnce = true;
    }
    // We need to supply *a* program to the host, even though we don't support
    // saving the user's program in demo mode. This is because if we don't
    // supply a chunk to the host, it defaults to grabbing the parameter values
    // one by one.
    return m_presetManager.getDefaultProgram();
#else
    return m_presetManager.getCurrentProgram();
#endif
}

VstInt32 PluginCore::getChunk(void **data, bool isSingleProgram)
{
    VstInt32 size = 0;
    *data = 0;
    m_mostRecentChunk = m_programFactory.getYamlForProgram(getProgramForProjectSave(), true);
    *data = &m_mostRecentChunk[0];
    size = m_mostRecentChunk.size();
    return size;
}

VstInt32 PluginCore::setChunk(void *data, VstInt32 byteSize, bool isSingleProgram)
{
    std::string chunk = std::string(static_cast<char *>(data), byteSize);
    Program *newProgram = loadYamlChunk(chunk);
    bool chunkHasBeenSet = newProgram != 0;
    return chunkHasBeenSet;
}

Program* PluginCore::loadYamlChunk(const std::string& chunk)
{
    static const char yamlDocHeader[] = "---";
    static const size_t yamlDocHeaderLen = strlen(yamlDocHeader);
    Program *newProgram = 0;
    if (chunk.rfind(yamlDocHeader, 0) == 0)
    {
        // This is a new style chunk using the YAML format.
        newProgram = m_presetManager.loadYamlChunk(chunk);
        if (newProgram)
        {
            setActiveProgram(newProgram, SRC_HOST_CHUNK);
        }
        else
        {
            Platform::LogLine(m_errorLog) << "Failed to convert host YAML chunk to program";
        }
    }
    else
    {
        const char* debug = chunk.c_str();
        Platform::LogLine(m_errorLog) << "Host chunk apparently does not contain a YAML header: " << debug[0] << " " << debug[1] << " " << debug[2];
    }
    return newProgram;
}

//==============================================================================
//
// Notifications interface
//

void PluginCore::programChanged()
{
    setActiveProgram(m_presetManager.getCurrentProgram(), SRC_INTERNAL);
    // Call the VST audio master to notify the host it should update the preset name.
    m_vst2Plugin->updateDisplay();
}

void PluginCore::paramValueSetChanged()
{
    Program *currentProgram = m_presetManager.getCurrentProgram();
    for (size_t i = 0; i < m_numParams; i++)
    {
        notifyParameterChange(i, currentProgram->getParameterValue(i), SRC_INTERNAL);
    }
}

void PluginCore::paramValueChanged(size_t index, float value) 
{
    if (m_vst2Plugin && index < m_pluginProperties.numParams)
    {
        m_vst2Plugin->setParameterAutomated(index, value);
        notifyParameterChange(index, value, SRC_EDITOR);
    }
}

bool PluginCore::notifySizeChange(unsigned int newWidth, unsigned int newHeight)
{
    if (m_vst2Plugin && m_vst2Plugin->canHostDo((char*)"sizeWindow"))
    {
        if (m_vst2Plugin->sizeWindow(newWidth, newHeight))
        {
            return true;
        }
    }
    return false;
}

void PluginCore::notifyBeginEdit(size_t index)
{
    if (m_vst2Plugin && index < m_pluginProperties.numParams)
    {
        m_vst2Plugin->beginEdit(index);
    }
}

void PluginCore::notifyEndEdit(size_t index)
{
    if (m_vst2Plugin && index < m_pluginProperties.numParams)
    {
        m_vst2Plugin->endEdit(index);
    }
}

void PluginCore::requestPresetsFileConversion(const char* filePath, Platform::Logger& logger)
{
    Platform::LogLine(logger) << "Converting presets from the selected file:";
    Platform::LogLine(logger) << "    " << filePath;
    std::ifstream input(filePath, std::ios::binary);
    // Copy all data into buffer (we use a vector<char> because the data is binary and so should not be treated as a
    // null-terminated string, even though technically C++ strings can handle internal zero bytes).
    std::vector<char> buffer((std::istreambuf_iterator<char>(input)), (std::istreambuf_iterator<char>()));
    const char* data = &buffer[0];
    size_t byteSize = buffer.size();
    if (byteSize == 0)
    {
        Platform::LogLine(logger) << "Could not read anything from the selected file.";
        return;
    }
    bool fileFormatIsValid = false;

    // Calculate the sizes of the various parts in the various old style chunks, program and bank files.
    const size_t kParamDefinitionsSize = m_pluginProperties.oldNumParams * m_pluginProperties.paramDefinitionSize;
    const size_t kSingleProgramSize = m_pluginProperties.programNameSize + m_pluginProperties.oldNumParams * 4; // 4-byte floats are saved in the files / old chunks
    const size_t kAllProgramsSize = m_pluginProperties.oldNumPrograms * kSingleProgramSize;
    const size_t kFullParamsAndProgramsBankSize = kParamDefinitionsSize + kAllProgramsSize;

    bool chunkContainsParamDefinitions = false;
    // Strictly speaking, we should check the various magic fields and the size fields for consistency. However,
    // we would first need to do endian flips because the byte ordering of FXP and FXB files is defined to be
    // opposite that of current hardware. So we cheat and just check the file extensions and sizes.
    if (Utils::endsWith(filePath, "fxb"))
    {
        const fxBank* fxb = reinterpret_cast<const fxBank*>(data);
        byteSize -= (fxb->content.data.chunk - data);
        if (byteSize == kFullParamsAndProgramsBankSize)
        {
            fileFormatIsValid = true;
            chunkContainsParamDefinitions = true;
            data = fxb->content.data.chunk;
        }
    }
    else if (Utils::endsWith(filePath, "fxp"))
    {
        const fxProgram* fxp = reinterpret_cast<const fxProgram*>(data);
        byteSize -= (fxp->content.data.chunk - data);
        if (byteSize == kSingleProgramSize)
        {
            fileFormatIsValid = true;
            data = fxp->content.data.chunk;
        }
    }
    else if (Utils::endsWith(filePath, "bnk"))
    {
        // There are two kinds of .bnk, one with parameter definitions / customisation and one with just the program
        // data.
        if (byteSize == kFullParamsAndProgramsBankSize)
        {
            fileFormatIsValid = true;
            chunkContainsParamDefinitions = true;
        }
        else if (byteSize == kAllProgramsSize)
        {
            fileFormatIsValid = true;
        }
    }
    if (fileFormatIsValid)
    {
        if (chunkContainsParamDefinitions)
        {
            // Skip past the parameter definitions to the actual program data
            data += kParamDefinitionsSize;            
            byteSize -= kParamDefinitionsSize;
        }
        convertPresets(data, byteSize, true, logger);
    }
    else
    {
        Platform::LogLine(logger) << "Could not recognise the format of the data in the file.";
    }
}

void PluginCore::requestLoadAndAutosaveYamls(const char* filePath, Platform::Logger& logger)
{
	//** this could be called in a for loop to strip Sinamd parameters, unused in Sinamd Beat, from a YAML file
	Platform::LogLine(logger) << "Converting presets from the selected file:";
    Platform::LogLine(logger) << "    " << filePath;
    std::ifstream input(filePath, std::ios::binary);
    std::string chunk((std::istreambuf_iterator<char>(input)), (std::istreambuf_iterator<char>())); //?
    size_t byteSize = chunk.size();
    if (byteSize == 0)
    {
        Platform::LogLine(logger) << "Could not read anything from the selected file.";
        return;
    }
    if (Utils::endsWith(filePath, "yaml"))
	{
		Program *newProgram = PluginCore::loadYamlChunk(chunk);
		doLoadAndAutosaveYamls(newProgram, true, logger);
	}
}

void PluginCore::requestForFormattedParameterString(int index, char* buffer, size_t len)
{
    m_dsp.getParameterDisplay(index, buffer, len);
    size_t bytesUsed = strlen(buffer);
    if (m_parameters.hasLabel(index) && bytesUsed + 2 < len)
    {
        strcat(buffer, " ");
        ++bytesUsed;
        if (m_parameters.labelIsDynamic(index))
        {
            m_dsp.getParameterLabel(index, &buffer[bytesUsed], len);
        }
        else
        {
            const std::string& label = m_parameters.getLabel(index);
            if (label.size() < len - bytesUsed - 1)
            {
                strcat(buffer, label.c_str());
            }
        }
    }
}

float PluginCore::requestDefaultValue(int index)
{	
	return m_factory->getParameters().getNormalisedDefaultValue(index);
}

//==============================================================================
//
// Internal interface
//

void PluginCore::notifyParameterChange(size_t index, float value, ParamValueSource source)
{
    if (index < m_numParams)
    {
        if (source == SRC_HOST_AUTOMATION || source == SRC_EDITOR)
        {
            // If we are updating parameters due to setting a program (either from a preset
            // or a loaded chunk), the active program already must have the parameters set!
            m_presetManager.getCurrentProgram()->changeParameterValue(index, value);
        }
        m_dsp.setParameter(index, value);
        if (source != SRC_RELOAD_TO_DSP && source != SRC_EDITOR)
        {
            // setParameter() can be called in audio render thread as well as the main (GUI) thread,
            // so updating the editor inline may be unsafe.
            m_notifyEditor->refreshParameter(index, value); // @todo make safe when called in audio render thread
        }
    }
}

void PluginCore::midiProgramChange(size_t programNum)
{
    // We currently only support changing program in the current category (bank).
    // We could extend this either by supporting MIDI bank select or by doing something
    // like the U-he synths and letting the user define a mapping of program numbers to
    // named presets. Of course we would use a YAML file for this.
    size_t category = m_presetManager.getCurrentCategoryIndex();
    m_presetManager.selectProgram(category, programNum);
    setActiveProgram(m_presetManager.getCurrentProgram(), SRC_MIDI_PC);
}

void PluginCore::reloadCurrentProgram()
{
    setActiveProgram(m_presetManager.getCurrentProgram(), SRC_RELOAD_TO_DSP);
}

void PluginCore::pushToGui(unsigned int msgIndex, int value)
{
    // @todo Actually implement a facility to push to the GUI thread.
    m_notifyEditor->setDisplayParameterInt(msgIndex, value);
}

void PluginCore::setActiveProgram(Program* newProgram, ParamValueSource source)
{
    m_dsp.handleNewProgram();
    for (size_t i = 0; i < m_numParams; i++) // only for SRC_INTERNAL ? or
    {
        notifyParameterChange(i, newProgram->getParameterValue(i), source);
    }
    if (source == SRC_HOST_CHUNK || source == SRC_MIDI_PC || source == SRC_SET_PROGRAM)
    {
        m_notifyEditor->programChanged(m_presetManager.getCurrentCategoryIndex(),
                                       m_presetManager.getCurrentProgramIndexInCategory());
    }
}

void PluginCore::convertPresets(const char* data, size_t byteSize, bool autoSave, Platform::Logger& logger)
{
    // NOTE: When autosaving, any presets with the same name as existing user presets will be skipped.
    std::string autoSaveDir = Platform::getPresetsDirectory(true); // User presets directory
    size_t numPresetsConverted = 0;
    size_t numPresetsSkipped = 0;
    while (byteSize > 0)
    {
        m_programFactory.startNewProgram();
        size_t bytesUsed = m_programFactory.loadOldStyleProgramChunk(data, byteSize, m_oldToNewParamIds);
        Program *newProgram = m_programFactory.getProgram();
        if (newProgram)
        {
            if (autoSave && m_presetManager.userPresetNameExists(newProgram->getName().c_str()))
            {
                ++numPresetsSkipped;
                Platform::LogLine(logger) << "Skipping preset with existing user preset name: "
                                          << newProgram->getName();
                m_programFactory.abandonProgram();
            }
            else
            {
                ++numPresetsConverted;
                Platform::LogLine(logger) << "Converting preset named: " << newProgram->getName();
                remapPresetParameters(newProgram);
                if (autoSave)
                {
                    // @todo: Make a "converted<date> subdirectory?"
                    m_presetManager.saveNew(newProgram, autoSaveDir);
                }
                // Auto-saved programs are categorised as user programs,
                // the others are put in their own "converted" category.
                m_presetManager.addConvertedProgram(newProgram, autoSave);
            }
        }
        data += bytesUsed;
        byteSize = byteSize > bytesUsed ? byteSize - bytesUsed : 0;
    }
    if (numPresetsSkipped > 0)
    {
        Platform::LogLine(logger) << numPresetsSkipped << " presets were skipped.";
    }
    if (numPresetsConverted > 0)
    {
        if (autoSave)
        {
            Platform::LogLine(logger) << "Presets saved to the user presets directory: " << autoSaveDir;
        }
        const char* convertedSingle = " preset was";
        const char* convertedPlural = " presets were";
        const char* convertedDesc = (numPresetsConverted == 1) ? convertedSingle : convertedPlural;
        const char* autoSaveAdditionSingle = " and saved as a user preset.";
        const char* autoSaveAdditionPlural = " and saved as user presets.";
        const char* autoSaveAddition = (numPresetsConverted == 1) ? autoSaveAdditionSingle : autoSaveAdditionPlural;
        Platform::LogLine(logger) << numPresetsConverted
                                  << convertedDesc
                                  << " converted"
                                  << (autoSave ? autoSaveAddition : ".");
    }
    else
    {
        Platform::LogLine(logger) << "No presets were converted.";
    }
}

void PluginCore::doLoadAndAutosaveYamls(Program *newProgram, bool autoSave, Platform::Logger& logger)
{
	 // NOTE: When autosaving, any presets with the same name as existing user presets will be skipped.
    std::string autoSaveDir = Platform::getPresetsDirectory(true); // User presets directory
    size_t numPresetsConverted = 0;
    size_t numPresetsSkipped = 0;
	
        if (newProgram)
        {
            if (autoSave && m_presetManager.userPresetNameExists(newProgram->getName().c_str()))
            {
                ++numPresetsSkipped;
                Platform::LogLine(logger) << "Skipping preset with existing user preset name: "
                                          << newProgram->getName();
                m_programFactory.abandonProgram();
            }
            else
            {
                ++numPresetsConverted;
                Platform::LogLine(logger) << "Converting preset named: " << newProgram->getName();

				// we just need to save using SinmadBeat m_presetManager
                if (autoSave)
                {
                     m_presetManager.saveNew(newProgram, autoSaveDir);
                }
				//** better not to use add, for the many programs we save
                //m_presetManager.addConvertedProgram(newProgram, autoSave);
            }
        }
  
    if (numPresetsSkipped > 0)
    {
        Platform::LogLine(logger) << numPresetsSkipped << " presets were skipped.";
    }
    if (numPresetsConverted > 0)
    {
        if (autoSave)
        {
            Platform::LogLine(logger) << "Presets saved to the user presets directory: " << autoSaveDir;
        }
        const char* convertedSingle = " preset was";
        const char* convertedPlural = " presets were";
        const char* convertedDesc = (numPresetsConverted == 1) ? convertedSingle : convertedPlural;
        const char* autoSaveAdditionSingle = " and saved as a user preset.";
        const char* autoSaveAdditionPlural = " and saved as user presets.";
        const char* autoSaveAddition = (numPresetsConverted == 1) ? autoSaveAdditionSingle : autoSaveAdditionPlural;
        Platform::LogLine(logger) << numPresetsConverted
                                  << convertedDesc
                                  << " converted"
                                  << (autoSave ? autoSaveAddition : ".");
    }
    else
    {
        Platform::LogLine(logger) << "No presets were converted.";
    }
}

//==============================================================================

DspToPlugin::DspToPlugin(PluginCore& plugin)
 : m_pluginCore(plugin)
{
}

void DspToPlugin::setProgram(size_t programNum)
{
    m_pluginCore.midiProgramChange(programNum);
}

void DspToPlugin::reloadCurrentProgram()
{
    m_pluginCore.reloadCurrentProgram();
}

//------------------------------------------------------------------------------------------

DspToEditor::DspToEditor(PluginCore& plugin)
 : m_pluginCore(plugin)
{
}

void DspToEditor::postDisplayParameterInt(int msgIndex, int value)
{
    m_pluginCore.pushToGui(msgIndex, value);
}