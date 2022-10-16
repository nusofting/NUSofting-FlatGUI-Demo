//
//  AppearanceController.h
//
//  Copyright 2016 NUSofting. All rights reserved.
//

#pragma once

#include "AppearanceManager/AppearanceManager.h"
#include "Config.h"
#include "vstgui/vstgui.h"

class Appearance;
class AppearanceFactory;
class CToolTip;
class PreferencesRegistry;

enum ColourItemId
{
    // Overall background
    CLR_MAIN_BACKGROUND,

    // Generic control fill colours
    CLR_CONTROL_BACKGROUND,     // Control background
    CLR_CONTROL_ACTIVE,         // Highlight colour for the "active" part of a control or an "on" state
    CLR_CONTROL_INACTIVE,       // Highlight colour for an "inactive" part of a control or an "off" state

    // Line and frame colours
    CLR_HIGHLIGHT_LINE,         // Lines to be specially emphasised by a highlight colour
    CLR_PLAIN_LINE,             // Lines that don't need a special emphasis
    CLR_HIGHLIGHT_FRAME,        // Frames to be more obviously boxed off against the background
    CLR_PLAIN_FRAME,            // Frames to be subtly boxed off against the background

    // Font colours
    CLR_DISPLAY_VALUE_FONT,     // Parameter display or other control display value
    CLR_ACTIVE_VALUE_FONT,      // Highlight colour for an active selection in a multi-part control
    CLR_INACTIVE_VALUE_FONT,    // Highlight colour for an inactive selection in a multi-part control
    CLR_LABEL_FONT,             // Generic labels
    CLR_TOOLTIP_FONT,           // Separate colour for tooltips, if so desired

    // Specific control colours
    CLR_SLIDER_TRACK_BACK,      // The background of a slider track (i.e. the part that represents the full range)
    CLR_SLIDER_TRACK_FRONT,     // The foreground of a slider track (i.e. the part that represents the selected value)
    CLR_KNOB_RIM,               // The outer rim of a knob plus connections between related knobs
    CLR_KNOB_INDICATOR,         // The knob's position indicator, in case it should be coloured differently to the active highlight

    // Graphs, plots and meters colours
    CLR_GRAPH_BACKGROUND,       // Graph background
    CLR_GRAPH_SIGNAL,           // The bar, line, curve, waveform representing what is being graphed
    CLR_GRAPH_PEAK,             // A peak level or range indicator
    CLR_GRAPH_CLIP,             // Clipping indicator

    // Decorations
    CLR_DECO,                   // Decoration highlight? or background?
};

enum DecorationId
{
    DECO_PLAIN,
    DECO_BITMAP1,
	DECO_BITMAP2,
	DECO_BITMAP3,
	DECO_GRADIENTS,
	DECO_SHADOWING1,
	DECO_SHADOWING2,
	DECO_SHADOWING3
};

class AppearanceController
{
public:
    AppearanceController(AppearanceFactory& appearanceFactory,
                         AppearanceManager& appearanceManager,
                         PreferencesRegistry& prefs);

    ~AppearanceController();

    void reload();
    void selectAppearance(size_t itemNum);

    CColor getColourById(ColourItemId colourId) const;
    DecorationId getDecorationType() const;
    const ConfigKnob &getKnobTheme() const { return m_configKnob; }
    const ConfigSlider &getSliderTheme() const { return m_configSlider; }
    const ConfigSwitch &getSimpleSwitchTheme() const { return m_configSimpleOnOffSwitch; }
    const ConfigSwitch &getMultiSwitchTheme() const { return m_configMultiSwitch; }
    const ConfigGraphView &getGraphViewTheme() const { return m_configGraphView; }
    void styleOptionMenu(COptionMenu* optionMenu) const;
    void styleToolTip(CToolTip* tooltip) const;

	PreferencesRegistry& getRegPref() { return m_prefs;} 

private:
    /// Prevent copying.
    AppearanceController(const AppearanceController &);

    /// Prevent assignment.
    AppearanceController &operator=(const AppearanceController &);

    void updateTheme();

    AppearanceManager& m_appearanceManager;
    Appearance* m_currentAppearance;
    PreferencesRegistry& m_prefs;

    /// Flat control themes. These structs are used to configure the colours for the flat control objects.
    /// @{
    // All sliders
    ConfigSlider m_configSlider;

    // Two different styles for different types of switches
    ConfigSwitch m_configMultiSwitch;
    ConfigSwitch m_configSimpleOnOffSwitch;

    // Knobs
    ConfigKnob m_configKnob; 

    // Graphs and plots
    ConfigGraphView m_configGraphView;

    // Zoom buttons - only in the toolbar, for which we are not yet allowing user styling
    //ConfigKick configKick;
    /// @}
};
