/*-----------------------------------------------------------------------------

(c) 2017 nusofting.com - Liqih & Blurk

ZoomableEditor.cpp
Written by Bernie Maier refactoring and extending original code by Luigi Felici
Copyright (c) 2017 NUSofting

-----------------------------------------------------------------------------*/

#include "ZoomableEditor.h"
#include "AppearanceController.h"
#include "PreferencesRegistry.h"

#include "AppearanceManager/AppearanceManager.h"
#include "FlatGUI/Controls.h"
#include "PresetManager/PresetManager.h"
#include "PresetManager/Program.h"
#include "vstgui/lib/platform/iplatformfont.h"
#include "vstgui/plugin-bindings/plugguieditor.h"
#include "FlatGUI/messages_multi_controls.hpp"


//-----------------------------------------------------------------------------------

class VstguiEditor : public PluginGUIEditor
{
public:
    VstguiEditor(EditorToPluginCoreInterface& ed) : PluginGUIEditor(0), m_ed(ed)
    {
    }
    ~VstguiEditor() {}

    virtual int32_t getKnobMode() const { return kLinearMode; }

    virtual void beginEdit(int32_t index)
    {
        m_ed.notifyBeginEdit(index);
    }

    virtual void endEdit(int32_t index)
    {
        m_ed.notifyEndEdit(index);
    }

    virtual bool beforeSizeChange(const CRect& newSize, const CRect& oldSize)
    {
        return m_ed.notifySizeChange(newSize.getWidth(), newSize.getHeight());
    }

private:
    EditorToPluginCoreInterface& m_ed;
};



//-----------------------------------------------------------------------------------
ZoomableEditorController::ZoomableEditorController(PresetManager& presetManager,
                                                   EditorToPluginCoreInterface& notifyPluginCore,
                                                   AppearanceManager& appearanceManager,
                                                   Platform::Logger& errorLog,
                                                   AppearanceController& appearanceController,
                                                   PreferencesRegistry& prefs,
                                                   ZoomableEditorConfiguration& editorConfig,
                                                   float* sharedFloatPool)
  : EditorCore(notifyPluginCore),
    m_notifyPluginCore(notifyPluginCore),
    m_presetManager(presetManager),
    m_toolbarListener(*this),
    m_toolbar(presetManager, notifyPluginCore, m_toolbarListener, appearanceManager, errorLog),
    m_scaledFontFactory(0),
    m_vstguiEditorAdapter(0),
    m_editorConfig(editorConfig),
    m_zoomableEditorView(0),
    m_editorContentView(0),
    m_appearanceController(appearanceController),
    m_prefs(prefs),
    m_sharedFloatPool(sharedFloatPool),
    m_refWidth(m_editorConfig.getReferenceWidth()),
    m_refHeight(m_editorConfig.getReferenceHeight()),
    m_guiWidth(0),
    m_guiHeight(0)
{
    m_vstguiEditorAdapter = new VstguiEditor(notifyPluginCore);
    // Initial size must be loaded from preferences *before* the editor is opened.
    // Why? Because VST is a crazy interface and it appears that hosts can and do query the rectangle before opening
    // the GUI. This explains why Ableton Live on macOS used to get the initial scale of the GUI wrong for the VST.
    float skinSizeIndex = m_prefs.loadSkinSizeIndex();
    updateDimensions(skinSizeIndex);


   m_fontRegistry = ICustomFontRegistry::create();
   bool res = m_fontRegistry->registerFont(m_editorConfig.getFontName(), m_editorConfig.getFontFilePath().c_str());
   assert(res);
    m_scaledFontFactory = new ScaledFontFactory(m_editorConfig.getFontName());

}

ZoomableEditorController::~ZoomableEditorController()
{
    delete m_scaledFontFactory;
    delete m_vstguiEditorAdapter;
}

bool ZoomableEditorController::open(void* ptr)
{
    m_appearanceController.reload();
    float skinSizeIndex = m_prefs.loadSkinSizeIndex();
    float scaling = updateDimensions(skinSizeIndex);
    m_scaledFontFactory->setVerticalScaleFactor(scaling);

    m_zoomableEditorView = new ZoomableEditorView(m_toolbar);
    m_editorContentView = m_editorConfig.getContentView(*this,
                                                        m_appearanceController,
                                                        m_prefs,
                                                        m_scaledFontFactory,
                                                        m_notifyPluginCore,
                                                        m_sharedFloatPool);
    CFrame* newFrame = m_zoomableEditorView->open(ptr, m_guiWidth, m_guiHeight, m_vstguiEditorAdapter, m_editorContentView);
    m_toolbar.CreateToolBarView(newFrame, scaling, m_scaledFontFactory, m_editorConfig.getToolBarOptions());
    m_toolbar.setInitialSelections(skinSizeIndex);

    //-- sync parameters
    // needs to be at the bottom of open()
    Program* currentProgram = m_presetManager.getCurrentProgram();
    size_t numParams = currentProgram->getNumParams();
    for (int i = 0; i < numParams; i++)
    {
        m_editorContentView->refreshParameter(i, currentProgram->getParameterValue(i));
    }

    return true;
}

void ZoomableEditorController::close()
{
    if (m_editorContentView)
    {
        m_editorContentView->close();
    }
    if (m_zoomableEditorView)
    {
        m_zoomableEditorView->close();
    }
    m_editorConfig.releaseContentView(m_editorContentView);
    m_editorContentView = 0;
}

void ZoomableEditorController::idle()
{
    if (m_zoomableEditorView)
    {
        m_zoomableEditorView->idle();
    }
    if (m_editorContentView)
    {
        m_editorContentView->idle();
    }
}

void ZoomableEditorController::refreshParameter(int index, float value)
{
    if (m_editorContentView)
    {
        m_editorContentView->refreshParameter(index, value);
    }
}

void ZoomableEditorController::programChanged(size_t categoryIndex, size_t programInCategoryIndex)
{
    if (m_editorContentView)
    {
        m_toolbar.programChanged(categoryIndex, programInCategoryIndex);
    }
}

void ZoomableEditorController::setDisplayParameterInt(int msgIndex, int value)
{
    if (m_editorContentView)
    {
        m_editorContentView->setDisplayParameterInt(msgIndex, value);
    }
}

void ZoomableEditorController::displayStaticMessage(const char* message, const char* title)
{
    if (m_zoomableEditorView)
    {
        m_zoomableEditorView->displayStaticMessage(message, title);
    }
}

void ZoomableEditorController::checkIfPresetBecameDirty()
{
    m_toolbar.checkIfPresetBecameDirty();
}

const Program& ZoomableEditorController::getCurrentProgram()
{
	return *m_presetManager.getCurrentProgram();
}

CCoord ZoomableEditorController::updateDimensions(int32_t zoomIndex)
{
    CCoord scaling = getAbsoluteScalingFactorForZoomIndex(zoomIndex);
    m_guiWidth = scaling * m_refWidth;
    m_guiHeight = scaling * m_refHeight;
    return scaling;
}

void ZoomableEditorController::handleZoom(float initialZoomIndex, float finalZoomIndex)
{
    CCoord absScaling = updateDimensions(finalZoomIndex);
    m_scaledFontFactory->setVerticalScaleFactor(absScaling);
    CCoord relScaling = getRelativeScalingFactorBetweenZoomIndexes(initialZoomIndex, finalZoomIndex);
    CRect newSize = m_zoomableEditorView->handleZoom(m_guiWidth, m_guiHeight, relScaling);
    m_editorContentView->handleZoom(newSize);
}

CCoord ZoomableEditorController::getAbsoluteScalingFactorForZoomIndex(float zoomIndex)
{
    CCoord scaling = 1.0f;
    switch (static_cast<int>(zoomIndex))
    {
    case 0:
        scaling = 0.89f;
        break;
    case 1:
        scaling = 1.0f;
        break;
    case 2:
        scaling = 1.22f;
        break;
    case 3:
        scaling = 1.4f;
        break;
    case 4:
        scaling = 1.7f;
        break;
    case 5:
        scaling = 1.98f;
        break;
    default:
        scaling = 1.0f;
    }
    return scaling;
}

CCoord ZoomableEditorController::getRelativeScalingFactorBetweenZoomIndexes(float initialZoomIndex, float finalZoomIndex)
{
    CCoord initialScaling = getAbsoluteScalingFactorForZoomIndex(initialZoomIndex);
    CCoord finalScaling = getAbsoluteScalingFactorForZoomIndex(finalZoomIndex);
    return finalScaling / initialScaling;
}

//-----------------------------------------------------------------------------------

ZoomableEditorController::TBL::TBL(ZoomableEditorController& controller) : m_controller(controller) { }

void ZoomableEditorController::TBL::setColours(int32_t colourIndex)
{
    m_controller.m_appearanceController.selectAppearance(colourIndex);
    m_controller.m_editorContentView->setColours();
}

void ZoomableEditorController::TBL::handleZoom(float value)
{
    float oldSkinSizeIndex = m_controller.m_prefs.loadSkinSizeIndex();
    m_controller.m_prefs.saveSkinSizeIndex(value);
    m_controller.handleZoom(oldSkinSizeIndex, value);
    //m_controller.m_toolbar.onZoom();
}

void ZoomableEditorController::TBL::requestApppearanceReload()
{
    m_controller.m_appearanceController.reload();
}


//-----------------------------------------------------------------------------------



ZoomableEditorView::ZoomableEditorView(ToolBar& toolbar)
  : m_toolbar(toolbar)
{
    m_messageToDisplay = 0;
	m_messageTitle = "Warning!";
}


ZoomableEditorView::~ZoomableEditorView()
{
}

//-----------------------------------------------------------------------------

CFrame* ZoomableEditorView::open(void* ptr, CCoord width, CCoord height, VstguiEditor* vstguiEditorAdapter,
                                 ZoomableEditorContentView* editorContentView)
{
    //-- first we create the frame with a size of X, Y and set the background to simple colour
    CRect frameSize (0, 0, width, height);
    CFrame* newFrame = new CFrame(frameSize, vstguiEditorAdapter); 
    newFrame->open(ptr);

    //-- set the member frame to our frame
    frame = newFrame;  
    editorContentView->open(width, height, frame);
    return newFrame;
}

void ZoomableEditorView::close()
{
	//-- on close we need to delete the frame object.
	//-- once again we make sure that the member frame variable is set to zero before we delete it
	//-- so that calls to setParameter won't crash.

	if (frame->getModalView())
	{
		frame->getModalView()->invalid();
		frame->setModalView (NULL);
	}

	CFrame* oldFrame = frame;
	frame = 0;
	oldFrame->forget();
}


CRect ZoomableEditorView::handleZoom(CCoord width, CCoord height, float relScaling)
{
    CRect oldSize;
    frame->getSize(oldSize);
    if(!frame->setSize(width,height))
        return oldSize;

    //* * * * * * * * * * start of experimental code 
    CRect NewSize(0, 0, width, height);
    frame->setViewSize(NewSize, true); // not called in setSize() due to a bug in VSTGUI 36

    frame->setMouseableArea(NewSize);

    frame->setDirty();

    //frame->setAutosizeFlags(kAutosizeNone); not needed

	  //tricky calls: work differently depending on the host
    frame->invalid ();
    frame->parentSizeChanged();
    //see above
    //* * * * * * * * * * end of experimental code 

    for (int c = 0; c < frame->getNbViews(); ++c)
    {               
        CView* pV = frame->getView(c);
        CRect rectOld = pV->getViewSize();
        CRect rectNew;
        rectNew.left  = rectOld.left*relScaling;
        rectNew.right = rectOld.right*relScaling;
        rectNew.top  = rectOld.top*relScaling;
        rectNew.bottom = rectOld.bottom*relScaling;

        pV->setViewSize(rectNew);
        pV->setMouseableArea(rectNew);
    }
    m_toolbar.onZoom();

	frame->getEditor()->beforeSizeChange(NewSize, oldSize);

    return NewSize; // not used?
}


void
ZoomableEditorView::idle()
{   
    // Pass idle onto the frame window
    if (frame)
        frame->idle ();

    if (m_messageToDisplay)
    {
        // We need to clear m_messageToDisplay *before* displaying the message
        // box, otherwise it will still be there in the next idle() run.
        const char* text = m_messageToDisplay;
        const char* lines[] = { text };
        m_messageToDisplay = 0;
        MessAgeBox(frame, m_messageTitle, lines, 1);
    }
}

void ZoomableEditorView::displayStaticMessage(const char* message, const char* title)
{
    m_messageToDisplay = message;
	m_messageTitle = title;
}
