//
//  AppearanceController.cpp
//
//  Copyright 2016 NUSofting. All rights reserved.
//

#include "AppearanceController.h"

#include "AppearanceManager/Appearance.h"
#include "AppearanceManager/AppearanceFactory.h"
#include "AppearanceManager/AppearanceManager.h"
#include "PreferencesRegistry.h"
#include "FlatGUI/CToolTip.h"


AppearanceController::AppearanceController(AppearanceFactory& appearanceFactory,
                                           AppearanceManager& appearanceManager,
                                           PreferencesRegistry& prefs)
    : m_appearanceManager(appearanceManager),
      m_currentAppearance(0),
      m_prefs(prefs)
{
    appearanceFactory.defineColourItem(CLR_MAIN_BACKGROUND, "mainBackgroundRGB", AppearanceColourItem(24, 26, 29));
    appearanceFactory.defineColourItem(CLR_CONTROL_BACKGROUND, "controlBackgroundRGB", AppearanceColourItem(43, 43, 43));
    appearanceFactory.defineColourItem(CLR_CONTROL_ACTIVE, "controlActiveRGB", AppearanceColourItem(16, 214, 97));
    appearanceFactory.defineColourItem(CLR_CONTROL_INACTIVE, "controlInactiveRGB", AppearanceColourItem(104, 104, 104));
    appearanceFactory.defineColourItem(CLR_HIGHLIGHT_LINE, "highlightLineRGB", AppearanceColourItem(16, 214, 97));
    appearanceFactory.defineColourItem(CLR_PLAIN_LINE, "plainLineRGB", AppearanceColourItem(132, 132, 132));
    appearanceFactory.defineColourItem(CLR_HIGHLIGHT_FRAME, "highlightFrameRGB", AppearanceColourItem(132, 132, 132));
    appearanceFactory.defineColourItem(CLR_PLAIN_FRAME, "plainFrameRGB", AppearanceColourItem(32, 32, 32));
    appearanceFactory.defineColourItem(CLR_DISPLAY_VALUE_FONT, "displayValueFontRGB", AppearanceColourItem(192, 192, 192));
    appearanceFactory.defineColourItem(CLR_ACTIVE_VALUE_FONT, "activeValueFontRGB", AppearanceColourItem(16, 214, 97));
    appearanceFactory.defineColourItem(CLR_INACTIVE_VALUE_FONT, "inactiveValueFontRGB", AppearanceColourItem(129, 129, 129));
    appearanceFactory.defineColourItem(CLR_LABEL_FONT, "labelFontRGB", AppearanceColourItem(192, 192, 192));
    appearanceFactory.defineColourItem(CLR_TOOLTIP_FONT, "tooltipFontRGB", AppearanceColourItem(197, 197, 197));
    appearanceFactory.defineColourItem(CLR_SLIDER_TRACK_BACK, "sliderTrackBackRGB", AppearanceColourItem(5, 5, 7));
    appearanceFactory.defineColourItem(CLR_SLIDER_TRACK_FRONT, "sliderTrackFrontRGB", AppearanceColourItem(98, 98, 98));
    appearanceFactory.defineColourItem(CLR_KNOB_RIM, "knobRimRGB", AppearanceColourItem(5, 5, 7));
    appearanceFactory.defineColourItem(CLR_KNOB_INDICATOR, "knobIndicatorRGB", AppearanceColourItem(250, 250, 250));
    appearanceFactory.defineColourItem(CLR_GRAPH_BACKGROUND, "graphBackgroundRGB", AppearanceColourItem(43, 43, 43));
    appearanceFactory.defineColourItem(CLR_GRAPH_SIGNAL, "graphSignalRGB", AppearanceColourItem(200, 200, 200));
    appearanceFactory.defineColourItem(CLR_GRAPH_PEAK, "graphPeakRGB", AppearanceColourItem(16, 214, 97));
    appearanceFactory.defineColourItem(CLR_GRAPH_CLIP, "graphClipRGB", AppearanceColourItem(215, 0, 0));
    appearanceFactory.defineColourItem(CLR_DECO, "decoRGBA", AppearanceColourItem(82, 0, 0, 0));

    appearanceFactory.defineDecorationName(DECO_PLAIN, "none");
    appearanceFactory.defineDecorationName(DECO_BITMAP1, "bitmap1");
	appearanceFactory.defineDecorationName(DECO_BITMAP2, "bitmap2");
	appearanceFactory.defineDecorationName(DECO_BITMAP3, "bitmap3");
	appearanceFactory.defineDecorationName(DECO_GRADIENTS, "gradients");
	appearanceFactory.defineDecorationName(DECO_SHADOWING1, "bitmap2AndShadows");
	appearanceFactory.defineDecorationName(DECO_SHADOWING2, "bitmap3AndShadows");
	appearanceFactory.defineDecorationName(DECO_SHADOWING3,"gradientsAndShadows");
}

AppearanceController::~AppearanceController()
{
}

void AppearanceController::reload()
{
    m_appearanceManager.loadThemes();
    std::string savedAppearanceFileName(m_prefs.loadAppearanceFileName());
    std::string savedAppearanceName(m_prefs.loadAppearanceName());
    // We attempt to load the previously saved appearance by its file name. If the user has renamed the file, then
    // we attempt to load the appearance by its name (which is not necessarily unique, but may be better than just
    // loading the default). If the user has deleted the file, then this may result in a different appearance with the
    // same name being loaded, but is more likely to result in the default (i.e. first) appearance being loaded.
    m_currentAppearance = m_appearanceManager.selectAppearance(savedAppearanceFileName, savedAppearanceName);
    updateTheme();
}

void AppearanceController::selectAppearance(size_t itemNum)
{
    m_currentAppearance = m_appearanceManager.selectAppearance(itemNum);
    updateTheme();
    m_prefs.saveAppearanceFileName(m_currentAppearance->getFilePath());
    m_prefs.saveAppearanceName(m_currentAppearance->getName());
}

CColor AppearanceController::getColourById(ColourItemId colourId) const
{
    const AppearanceColourItem& ci = m_currentAppearance->getColourItemValue(colourId);

//#if defined(MAC)
//    const float scale = 0.55f;
//#else
    const float scale = 0.95f;
//#endif

	uint8_t redScaled = static_cast<uint8_t>(ci.red*scale);
	uint8_t greenScaled = static_cast<uint8_t>(ci.green*scale);
	uint8_t blueScaled = static_cast<uint8_t>(ci.blue*scale);

    return MakeCColor(redScaled, greenScaled, blueScaled, ci.alpha * 255);
}

DecorationId AppearanceController::getDecorationType() const
{
    return static_cast<DecorationId>(m_currentAppearance->getDecorationType());
}

void AppearanceController::updateTheme()
{
    m_configKnob.colBkground1 = getColourById(CLR_CONTROL_BACKGROUND);
    //m_configKnob.colBkground2 = not used?
    m_configKnob.colFrame = getColourById(CLR_KNOB_RIM);
    m_configKnob.colShadowHandle = getColourById(CLR_CONTROL_ACTIVE);
    m_configKnob.colHandle = getColourById(CLR_KNOB_INDICATOR);
    m_configKnob.colFont = getColourById(CLR_DISPLAY_VALUE_FONT);
    m_configKnob.colInactive = getColourById(CLR_CONTROL_INACTIVE);
    m_configKnob.iCursOffsetRel = 19;

    m_configMultiSwitch.colFrame = getColourById(CLR_HIGHLIGHT_FRAME);
    m_configMultiSwitch.colBkground1  = getColourById(CLR_CONTROL_ACTIVE);
    m_configMultiSwitch.colBkground2 = getColourById(CLR_CONTROL_BACKGROUND);
    m_configMultiSwitch.colFont1 = getColourById(CLR_INACTIVE_VALUE_FONT);
    m_configMultiSwitch.colFont2  = getColourById(CLR_ACTIVE_VALUE_FONT);
    m_configSimpleOnOffSwitch = m_configMultiSwitch;
    m_configSimpleOnOffSwitch.colFrame = getColourById(CLR_PLAIN_FRAME);

    m_configSlider.colBkground1 = getColourById(CLR_CONTROL_BACKGROUND);
    m_configSlider.colFrameHandle = getColourById(CLR_KNOB_RIM);
    m_configSlider.colHandle = getColourById(CLR_CONTROL_ACTIVE);
    m_configSlider.colFrame = getColourById(CLR_HIGHLIGHT_FRAME);
    m_configSlider.colFont = getColourById(CLR_ACTIVE_VALUE_FONT);
    m_configSlider.colFontInactive = getColourById(CLR_INACTIVE_VALUE_FONT);
    m_configSlider.colTrackBack = getColourById(CLR_SLIDER_TRACK_BACK);
    m_configSlider.colTrackFront = getColourById(CLR_SLIDER_TRACK_FRONT);

    m_configGraphView.colBkground1 = getColourById(CLR_GRAPH_BACKGROUND);
    m_configGraphView.colBkground2 = getColourById(CLR_SLIDER_TRACK_BACK);
    m_configGraphView.colFrame = getColourById(CLR_HIGHLIGHT_FRAME);
    m_configGraphView.colSignal = getColourById(CLR_GRAPH_SIGNAL);
    m_configGraphView.colPeak = getColourById(CLR_GRAPH_PEAK);
    m_configGraphView.colClip = getColourById(CLR_GRAPH_CLIP);
    m_configGraphView.colFont = getColourById(CLR_LABEL_FONT);
    m_configGraphView.colHandle = getColourById(CLR_CONTROL_ACTIVE);
}

void AppearanceController::styleOptionMenu(COptionMenu* optionMenu) const
{
    optionMenu->setBackColor(getColourById(CLR_CONTROL_BACKGROUND));
    optionMenu->setFrameColor(getColourById(CLR_HIGHLIGHT_FRAME));
    optionMenu->setFontColor(getColourById(CLR_ACTIVE_VALUE_FONT));
}

void AppearanceController::styleToolTip(CToolTip* tooltip) const
{
    tooltip->setColours(getColourById(CLR_HIGHLIGHT_FRAME), getColourById(CLR_TOOLTIP_FONT));
}
