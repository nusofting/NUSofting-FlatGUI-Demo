/*-----------------------------------------------------------------------------

(c) 2017 nusofting.com - Liqih & Blurk

ZoomableEditor.h
Written by Bernie Maier refactoring and extending original code by Luigi Felici
Copyright (c) 2017 NUSofting

-----------------------------------------------------------------------------*/

#pragma once

#include "vstgui/vstgui.h"
#include "Core/EditorCore.h"
#include "FlatGUI/ToolBar.h"

class AppearanceController;
class PreferencesRegistry;
class Program;
class VstguiEditor;
class ZoomableEditorController;
struct ToolBarOptions;


/// Define the interface that must be implemented by the product-specific editor code to implement the editor's
/// content view, that sits inside a zoomable frame.
class ZoomableEditorContentView
{
public:
    ZoomableEditorContentView() {}
    virtual ~ZoomableEditorContentView() {}

    virtual bool open(CCoord width, CCoord height, CFrame* frame) = 0;

    virtual void close() = 0;

    virtual void idle() = 0;

    /// Inform the editor that either the parameter value has changed
    /// or otherwise needs to be redisplayed (e.g. a related parameter has
    /// changed in a way that changes its visual interpretation).
    ///
    /// @param index
    ///		The parameter ID of the parameter whose value has changed.
    /// @param value
    ///		The new value of the parameter.
    /// @note
    ///		This function is only for displaying parameter changes. It is not appropriate to recalculate and alter other
    ///		parameter values.
    virtual void refreshParameter(int index, float value) = 0;
    virtual void setDisplayParameterInt(int msgIndex, int value) = 0;

    virtual void setColours() = 0;
    virtual void handleZoom(const CRect &newSize) = 0;
};


/// Define the interface that must be implemented by the product-specific editor code to configure the zoomable editor
/// for the framework code.
class ZoomableEditorConfiguration
{
public:
    virtual ~ZoomableEditorConfiguration() {}

    /// Return the width of the editor's frame at zoom level 1x. This includes the space needed by the toolbar.
    virtual CCoord getReferenceWidth() = 0;

    /// Return the height of the editor's frame at zoom level 1x. This includes the space needed by the toolbar.
    virtual CCoord getReferenceHeight() = 0;

    /// Return the name of the custom font to use.
    virtual const char* getFontName() = 0;

    /// Return the full path to the font file for the custom font to use.
    virtual std::string getFontFilePath() = 0;

    virtual ToolBarOptions& getToolBarOptions() = 0;
    virtual ZoomableEditorContentView* getContentView(ZoomableEditorController& parentController,
                                                      AppearanceController& appearanceController,
                                                      PreferencesRegistry& prefs,
                                                      ScaledFontFactory* scaledFontFactory,
                                                      EditorToPluginCoreInterface& notifyPluginCore,
                                                      float* sharedFloatPool) = 0;
    virtual void releaseContentView(ZoomableEditorContentView *view) = 0;
};


//==============================================================================
//
// The following classes are for the zoomable editor framework implementation
// and do not need modifying per product.
//


/// Encapsulates the shared content view behaviour common to all the flat GUI / zoomable GUI based products.
/// This is intended to be standard framework code that does not need to be modified for each product.
class ZoomableEditorView
{
public:
    ZoomableEditorView(ToolBar& toolbar);
    ~ZoomableEditorView();

    CFrame* open(void* ptr, CCoord width, CCoord height, VstguiEditor* vstguiEditorAdapter, ZoomableEditorContentView* editorContentView);

    void close();

    void idle();

    void displayStaticMessage(const char* message, const char* title);

    CRect handleZoom(CCoord width, CCoord height, float relScaling);

private:
    const char* m_messageToDisplay;
	const char* m_messageTitle;

    ToolBar &m_toolbar;

    CFrame* frame;
};


/// Encapsulates the shared zoomable editor behaviour common to all the flat GUI / zoomable GUI based products.
/// It implements, on behalf of the product's editor, all the core behaviour required of the editor and its interaction
/// with the host.
/// This is intended to be standard framework code that does not need to be modified for each product. The product's
/// editor only needs to implement the ZoomableEditorContentView and ZoomableEditorConfiguration interfaces.
class ZoomableEditorController : public EditorCore
{
public:
    ZoomableEditorController(PresetManager& presetManager,
                             EditorToPluginCoreInterface& notifyPluginCore,
                             AppearanceManager& appearanceManager,
                             Platform::Logger& errorLog,
                             AppearanceController& appearanceController,
                             PreferencesRegistry& prefs,
                             ZoomableEditorConfiguration& editorConfig,
                             float* sharedFloatPool);
    ~ZoomableEditorController();

    /// From EditorCore
    /// @{
    virtual bool open(void* ptr);
    virtual void close();
    virtual void idle();

    /// Inform the editor that either the parameter value has changed
    /// or otherwise needs to be redisplayed (e.g. a related parameter has
    /// changed in a way that changes its visual interpretation).
    void refreshParameter(int index, float value);
    void programChanged(size_t categoryIndex, size_t programInCategoryIndex);
    void setDisplayParameterInt(int msgIndex, int value);
    void displayStaticMessage(const char* message, const char* title);

    virtual unsigned int getWidth() const { return static_cast<int>(m_guiWidth); }
    virtual unsigned int getHeight() const { return static_cast<int>(m_guiHeight); }
    /// @}

    void checkIfPresetBecameDirty();
    const Program& getCurrentProgram();

private:
    CCoord updateDimensions(int32_t zoomIndex);
    void handleZoom(float initialZoomIndex, float finalZoomIndex);

    /// Returns the scaling factor for the specified zoom index. The scaling factor is the amount by which to multiply
    /// the reference width or height to get the new width or height.
    ///
    /// @note Currently the vertical and horizontal scaling factors have the same value for each zoom index. This even
    /// scaling in each direction tends to look the best, so this is not a severe limitation.
    static CCoord getAbsoluteScalingFactorForZoomIndex(float zoomIndex);

    /// Returns the relative scaling factor needed to scale from the first zoom index to the second zoom index.
    /// The relative scaling factor is the amount by which to multiply the current width or height to get the new width
    /// or height.
    static CCoord getRelativeScalingFactorBetweenZoomIndexes(float initialZoomIndex, float finalZoomIndex);

    EditorToPluginCoreInterface& m_notifyPluginCore;
    PresetManager& m_presetManager;

    class TBL : public ToolBarListener
    {
    public:
        TBL(ZoomableEditorController& controller);
        virtual void setColours(int32_t colourIndex);
        virtual void handleZoom(float value);
        virtual void requestApppearanceReload();
    private:
        ZoomableEditorController& m_controller;
    };
    TBL m_toolbarListener;

    ToolBar m_toolbar;
    ScaledFontFactory* m_scaledFontFactory;

    VstguiEditor* m_vstguiEditorAdapter;
    ZoomableEditorConfiguration& m_editorConfig;
    ZoomableEditorView* m_zoomableEditorView;
    ZoomableEditorContentView* m_editorContentView;

    ICustomFontRegistry* m_fontRegistry;
    AppearanceController& m_appearanceController;
    PreferencesRegistry& m_prefs;

    /// Only needed for passing through to the editor content view.
    float* m_sharedFloatPool;

    /// The reference width of the GUI at the default scale of 1.0x.
    CCoord m_refWidth;

    /// The reference height of the GUI at the default scale of 1.0x.
    CCoord m_refHeight;

    /// The current width of the GUI.
    CCoord  m_guiWidth;

    /// The current height of the GUI.
    CCoord  m_guiHeight;
};

