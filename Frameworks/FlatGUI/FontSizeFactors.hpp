
#ifndef _FontSizeFactors_
#define _FontSizeFactors_

#if defined(MAC)
// Fonts using the same point size are displayed larger on Windows than on OS X.
// This is possibly due to OS X typically using a resolution of 72 DPI vs Windows using 96 DPI.
const CCoord kFontSizeScaleFactor = 72.0 / 96.0; // used in flat_objects_controls.cpp
// Also, text is vertically centred differently in OS X vs Windows. The proper way to do this
// probably involves calculating things from the font metrics, but even those are interpreted
// differently on the two platforms. So just use a fudge factor from visually checking the results.
const CCoord kVerticalCentreTextFudge = 2.0; // used ONLY in flat_objects_controls.cpp

static const CCoord kShiftY = 0.0; // not used yet
static const CCoord kFixX = 2.0; // less than 1.0 shift left // used in PTLBackgroundBitmap.h
static const CCoord kFixY = 1.0; // less than 1.0 shift up  // used in PTLBackgroundBitmap.h

const CCoord kFontSizeSmall = 12.0;
const CCoord kFontSizeMedium = 14.0;
const CCoord kFontSizeBig = 16.0;

#else // WINDOWS

const CCoord kVerticalCentreTextFudge = 5.0; // used in flat_objects_controls.cpp
const CCoord kFontSizeScaleFactor  = 1.0; // used in flat_objects_controls.cpp

static const CCoord kShiftY = 0.0; // not used yet
static const CCoord kFixX = 1.0;
static const CCoord kFixY = 1.0;

const CCoord kFontSizeSmall = 12.0;
const CCoord kFontSizeMedium = 14.0;
const CCoord kFontSizeBig = 16.0;
#endif


#endif //_FontSizeFactors_
