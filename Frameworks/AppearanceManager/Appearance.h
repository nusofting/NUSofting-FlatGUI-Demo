//
//  Appearance.h
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>


/// Semantic typedefs for a type to hold the value of a single colour / alpha component.
typedef uint8_t ColourComponentValue;
typedef float ColourAlphaValue;

/// A simple struct for storing the RGB colour information for a colour item in
/// an appearance theme.
struct AppearanceColourItem
{
    ColourComponentValue red;
    ColourComponentValue green;
    ColourComponentValue blue;
    ColourAlphaValue alpha;
    AppearanceColourItem()
      : red(0), green(0), blue(0), alpha(1.0) { }
    AppearanceColourItem(ColourComponentValue red_,
                         ColourComponentValue green_,
                         ColourComponentValue blue_,
                         ColourAlphaValue alpha_ = 1.0)
      : red(red_), green(green_), blue(blue_), alpha(alpha_) { }
};

/// Defines an interface for encapsulating the customisable parts of the appearance of the UI.
class Appearance
{
public:
    Appearance(size_t numColourItems, bool isFactory = false);

    ~Appearance();

    void setName(const std::string& name);
    void setName(const char* name);
    const std::string& getName() const { return m_name; }

    /// Set the initial value of the colour item. This is used when loading an appearance theme.
    void setColourItemValue(size_t itemId, const AppearanceColourItem& value);

    /// Return the colours for the specified item.
    const AppearanceColourItem& getColourItemValue(size_t itemId) const;

    void setDecorationType(size_t decorationType);
    size_t getDecorationType() const { return m_decorationType; }

    bool isFactoryPreset() const
    {
        return m_isFactory;
    }

    void setFilePath(const std::string& filePath);
    const std::string& getFilePath() const { return m_filePath; }

    Appearance(const Appearance& other);
    Appearance& operator=(Appearance rhs);

private:
    std::string m_name;
    std::vector<AppearanceColourItem> m_colourItems;
    size_t m_decorationType;
    bool m_isFactory;
    std::string m_filePath;
};

