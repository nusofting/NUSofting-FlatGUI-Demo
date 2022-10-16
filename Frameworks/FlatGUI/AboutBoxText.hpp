/*-----------------------------------------------------------------------------

(c) 2022 nusofting.com - Liqih

-----------------------------------------------------------------------------*/

#pragma once

#include <string>

#define StandardAboutBoxText_mainPluginCredits "Project and DSP code by Luigi Felici"
#define StandardAboutBoxText_guiCredits        "GUI code by Luigi Felici and Bernie Maier, with Branislav contributions"
#define StandardAboutBoxText_frameworkCredits  "macOS port and framework code by Bernie Maier"
#define StandardAboutBoxText_vstCredits        "VST plugin technology by Steinberg"
#define StandardAboutBoxText_auCredits         "AU plugin technology by Apple"


/// Defines an interface providing about box text that the toolbar can display in a MessAgeBox.
class AboutBoxText
{
public:
    AboutBoxText(const char* pluginName,
                 const char* version,
                 const char* copyright,
                 const char* extraLine,
                 const char* infoFileName);

	~AboutBoxText() 
	{
	}

	/// Provide an array of C strings, one for each line of text to display in the about box.
    const char** getLines(int& m_numLines);

	/// The number of lines to display.
	//int numLines() { return m_numLines; }


	const char* m_pluginName;
	const char* m_version;
	const char* m_copyright;
	const char* m_mainPluginCredits;
	const char* m_guiCredits;
	const char* m_frameworkCredits;
	const char* m_vstCredits;
	const char* m_auCredits;
	/// The name of a HTML file included in the plugin's resources, that can be launched in a browser when the
	/// user clicks on a button in the about box. This is the plugin's manual / help page.
	/// This may be null, in which case no button is drawn in the about box.
	const char* m_infoFileName;
	const char* m_extraLine;

    /// Version with plugin type and machine architecture added (where available, initially for macOS builds).
    std::string m_detailedVersion;

};
