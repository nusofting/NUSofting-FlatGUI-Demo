//****************************************************************************************************//
//		Custom GUI Widgets written by Luigi Felici for nusofting.com 2015
//****************************************************************************************************//

#pragma warning( disable : 4244)//conversion from 'double' to 'float'
#pragma once

#include "vstgui/vstgui.h"
#include <math.h>


struct ConfigSlider
{
	CColor colBkground1;
	CColor colFrame; 
	CColor colHandle;	
	CColor colFrameHandle;	
	CColor colFont;
	CColor colFontInactive;
	CColor colTrackBack;
	CColor colTrackFront;

	ConfigSlider()
	{
		colBkground1(122, 122, 122, 255);
		colFrame(32, 32, 32, 255);
		colHandle(0, 0, 0, 255);
		colFrameHandle(201, 201, 201, 255);
		colFont(215, 215, 255, 255);
		colFontInactive(128, 128, 128, 255);
		colTrackBack(5, 5, 5, 255);
		colTrackFront(255, 115, 115, 255);
	}
};

struct ConfigLabel
{
	CColor colBkground1; // not currently used
	CColor colFrame; // used if bDrawFrame
	CColor colFont;

	ConfigLabel()
	{
		colBkground1(122, 122, 122, 255);
		colFrame(32, 32, 32, 255);
		colFont(215, 215, 255, 255);
	}
};

struct ConfigKick
{
	CColor colBkground1;
	CColor colBkground2;
	CColor colFrame; 
	CColor colFont;

	ConfigKick()
	{
		colBkground1(122, 122, 122, 255);
		colBkground2(29, 29, 32, 255);
		colFrame(32, 32, 32, 255);
		colFont(215, 215, 228, 255);
	}
};

struct ConfigGraphView // Various graphs and signal plots
{
	CColor colBkground1;
	CColor colBkground2; // used if zoom slider present
	CColor colFrame;     // used if bDrawFrame
	CColor colSignal;
	CColor colPeak;
	CColor colClip;
	CColor colFont;
	CColor colHandle; // used if zoom slider present

	ConfigGraphView()
	{
		colBkground1(46, 46, 46, 255);
		colBkground2(98, 98, 98, 255);
		colFrame(32, 32, 32, 255);
		colSignal(200, 200, 200, 255);
		colPeak(215, 215, 215, 255);
		colClip(215, 0, 0, 255);
		colFont(221, 221, 255, 255);
		colHandle(98,255,98, 255);
	}
};

struct ConfigKnob
{
	CColor colBkground1;
	CColor colBkground2; // used if bDrawCirle2
	CColor colFrame;	
	CColor colShadowHandle;
	CColor colHandle;
	CColor colFont;
	CColor colInactive;
	bool bDrawCirle2;
	CCoord iCursOffsetRel;

	ConfigKnob()
	{
		colBkground2 = colBkground1(122, 122, 122, 255);
		colFrame(32, 32, 32, 255);
		colFont = colHandle(215, 215, 255, 255);
		colShadowHandle(186, 186, 186, 255);
		colInactive(115, 115, 115, 255);
		bDrawCirle2 = false;
		iCursOffsetRel = 19; // range 2...19
	}
};

struct ConfigEQWidget
{
	CColor colBkground1;
	CColor colFrame; 
	CColor colHandle;	
	CColor colFrameHandle;	
	CColor colFont;

	ConfigEQWidget()
	{
		colBkground1(122, 122, 122, 255);
		colFrame(3, 3, 3, 255);
		colHandle(201, 201, 201, 255);
		colFrameHandle(201, 201, 201, 255);
		colFont(215, 215, 255, 255);
	}
};

struct ConfigMultiFader
{
	CColor colBkground1;
	CColor colFrame; 
	CColor colHandle;	
	CColor colFrameHandle;	
	CColor colFont;

	ConfigMultiFader()
	{
		colBkground1(102, 102, 102, 255);
		colFrame(3, 3, 3, 255);
		colHandle(201, 201, 201, 255);
		colFrameHandle(201, 201, 201, 255);
		colFont(215, 215, 255, 255);
	}
};

struct ConfigSwitch
{
	CColor colBkground1;
	CColor colBkground2;
	CColor colFrame; 
	CColor colFont1;
	CColor colFont2;

	ConfigSwitch()
	{
		colBkground2(122, 122, 122, 255);
		colBkground1(201, 201, 201, 255);
		colFrame(122, 122, 122, 128);
		colFont2(254, 254, 254, 255);
		colFont2(32, 32, 32, 255);		
	}
};


//Colour Functions

static void RGBtoHSV(unsigned char &Hue, unsigned char &Sat, unsigned char &Value, CColor colorRGB) 
{ 
	float r, g, b, h, s, v; //this function works with floats between 0 and 1 
	r = colorRGB.red / 256.0f; 
	g = colorRGB.green / 256.0f; 
	b = colorRGB.blue / 256.0f;
	float maxColor = std::max<>(r, std::max<>(g, b));
	float minColor = std::min<>(r, std::min<>(g, b));
	v = maxColor;

	if(maxColor == 0.0f)//avoid division by zero when the color is black
	{  
		s = 0.0f;
	}
	else  
	{      
		s = (maxColor - minColor) / maxColor;
	}

	if(s == 0.0f)
	{
		h = 0.0f; //it doesn't matter what value it has
	}   
	else
	{ 
		if(r == maxColor) h = (g - b) / (maxColor-minColor); 
		else if(g == maxColor) h = 2.0f + (b - r) / (maxColor - minColor);
		else h = 4.0f + (r - g) / (maxColor - minColor);       
		h /= 6.0f; //to bring it to a number between 0 and 1
		if (h < 0.0f) h++;

	}

	Hue = int(h * 255.0f);
	Sat = int(s * 255.0f);
	Value = int(v * 255.0f);
}

struct ColorHSV 
{
	unsigned char h;
	unsigned char s;
	unsigned char v;

	ColorHSV()
	{
		memset(this, 0, sizeof(*this));
	}
};


static void HSVtoRGB(unsigned char &Red, unsigned char &Green, unsigned char &Blue,ColorHSV colorHSV) 
{ 
	float r, g, b, h, s, v; //this function works with floats between 0 and 1 
	h = colorHSV.h / 256.0f; 
	s = colorHSV.s / 256.0f; 
	v = colorHSV.v / 256.0f;

	//If saturation is 0, the color is a shade of gray
	if(s == 0.0f) r = g = b = v;

	//If saturation > 0, more complex calculations are needed
	else
	{
		float f, p, q, t;
		int i;
		h *= 6; //to bring hue to a number between 0 and 6, better for the calculations
		i = int(floor(h));  //e.g. 2.7 becomes 2 and 3.01 becomes 3 or 4.9999 becomes 4
		f = h - i;  //the fractional part of h
		p = v * (1.0f - s);   
		q = v * (1.0f - (s * f));     
		t = v * (1.0f - (s * (1.0f - f)));   
		switch(i)       
		{         
		case 0: r = v; g = t; b = p; break;
		case 1: r = q; g = v; b = p; break;
		case 2: r = p; g = v; b = t; break;
		case 3: r = p; g = q; b = v; break;
		case 4: r = t; g = p; b = v; break;
		case 5: r = v; g = p; b = q; break;  
		}
	}

	Red = int(r * 255.0f);
	Green = int(g * 255.0f);
	Blue = int(b * 255.0f);
}

static void setValuesHSB(float Hue, float Sat, float Brite, CColor &dColor)
{
	ColorHSV TempsHSV; 

	RGBtoHSV(TempsHSV.h, TempsHSV.s, TempsHSV.v, dColor); 

	// do the filter

	TempsHSV.h += static_cast<unsigned char>(48*(2*Hue-1));
	TempsHSV.s += static_cast<unsigned char>((255-TempsHSV.s)*(2*Sat-1)-(TempsHSV.s-255)*(2*Sat-1));
	TempsHSV.v += static_cast<unsigned char>(128*Brite);//= max(TempsHSV.v*2*Brite,TempsHSV.v);

	// return mod colour
	HSVtoRGB(dColor.red, dColor.green, dColor.blue,TempsHSV);
};

static VSTGUI::CColor reverseColour(VSTGUI::CColor c)
{
	c.red   = ~c.red;
	c.green = ~c.green;
	c.blue  = ~c.blue;

	return c;
}


static VSTGUI::CColor brightnessColour(VSTGUI::CColor dColor, const float Brite = 0.5f, const float add = 25.0f) // default no change, 0.0f black, 1.0f white
{
	float maxBright = add+dColor.red/3.0f+dColor.green/3.0f+dColor.blue/3.0f;
	if(maxBright > 255.0f)  maxBright = 255.0f;

	// do the filter	
	float shift = dColor.red*2.0f*Brite;
	if(shift >= maxBright) dColor.red = maxBright;
	else if( shift <= 0.0f) dColor.red = 0;
	else dColor.red = static_cast<unsigned char>(shift);

	shift = dColor.green*2.0f*Brite;
	if(shift >= maxBright) dColor.green = maxBright;
	else if( shift <= 0.0f) dColor.green = 0;
	else dColor.green = static_cast<unsigned char>(shift);

	shift = dColor.blue*2.0f*Brite;
	if(shift >= maxBright) dColor.blue = maxBright;
	else if( shift <= 0.0f) dColor.blue = 0;
	else dColor.blue = static_cast<unsigned char>(shift);

	return dColor;
}

static VSTGUI::CColor brightnessMax(VSTGUI::CColor dColor, const float Brite = 0.5f) // default no change, 0.0f black, 1.0f white
{
	float maxBright = 255.0f;

	// do the filter	
	float shift = dColor.red*2.0f*Brite;
	if(shift >= maxBright) dColor.red = 255;
	else if( shift <= 0.0f) dColor.red = 0;
	else dColor.red = static_cast<unsigned char>(shift);

	shift = dColor.green*2.0f*Brite;
	if(shift >= maxBright) dColor.green = 255;
	else if( shift <= 0.0f) dColor.green = 0;
	else dColor.green = static_cast<unsigned char>(shift);

	shift = dColor.blue*2.0f*Brite;
	if(shift >= maxBright) dColor.blue = 255;
	else if( shift <= 0.0f) dColor.blue = 0;
	else dColor.blue = static_cast<unsigned char>(shift);

	return dColor;
}


static float TemperedTable[250];

static void make250Table(float* TheTable, float c0Hz, float ratio)
{
	for (int i = 0; i < 250; i++)
	{
		TheTable[i] = c0Hz;
		c0Hz = c0Hz*ratio;
	}
};

/// Encapsulates a factory for creating scaled fonts with standard sizes for the
/// scaleable flat GUI (small, medium, large).
class ScaledFontFactory
{
public:
	ScaledFontFactory(UTF8StringPtr name, const int32_t style = 0);
	~ScaledFontFactory ();

	void setVerticalScaleFactor(float scaleY_);
	CFontDesc* getScaledSmallFont();
	CFontDesc* getScaledMediumFont();
	CFontDesc* getScaledBigFont();

protected:
	CFontDesc m_smallFont;
	CFontDesc m_mediumFont;
	CFontDesc m_bigFont;
	float m_scaleY;
}; // end class ScaledFontFactory


static const float floatDelta = 0.0001f;
static bool notSameFloat(const float& var1, const float& var2)
{
	return fabs(var1-var2) > floatDelta;
};

static bool bSameString(const char* string1, const char* string2)
{
	return (0 == strcmp(string1, string2));
}