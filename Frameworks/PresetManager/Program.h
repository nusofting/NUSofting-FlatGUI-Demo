//
//  Program.h
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#pragma once

#include <string>
#include <vector>


/// Defines an interface for encapsulating the collection of the values of each
/// published parameter of the plugin. When stored, a program is also a preset.
class Program
{
public:
    Program(size_t numParams, bool isFactory = false);

    ~Program();

    void setName(const std::string &name);
    void setName(const char *name);
    const std::string& getName() const { return m_name; }
    void setCategory(const std::string& category);
    const std::string& getCategory() const { return m_category; }
    /// Return the name prefixed by the category name
    const std::string getFullName() const;
    void markDirty(bool isDirty = true);

    bool isDirty() const
    {
        return m_isDirty;
    }

    /// Set the initial (i.e. preset) value of the parameter. This is used when
    /// loading the program / preset.
    void setInitialParameterValue(size_t paramId, float value);

    /// Update the value of the parameter. This is used when adjusting the program,
    /// either in the editor or via host automation or via MIDI learn.
    void changeParameterValue(size_t paramId, float value);

    /// Revert the parameters to their initial values.
    void revert();

    enum ValueType
    {
        VALUE_INITIAL,
        VALUE_CURRENT
    };

    /// Switch between getParameterValue() returning the original values or the
    /// currently edited values.
    void selectValueSet(ValueType valueType);
    ValueType getGetValueSetType() const;

    /// Return the parameter value from the currently selected set (initial vs current).
    float getParameterValue(size_t paramId) const;

    /// @todo MIDI learn, anything else?

    bool isFactoryPreset() const
    {
        return m_isFactory;
    }

    /// Return whether or not the program was originally loaded from a chunk and has not yet been saved to a file.
    bool isUnsavedChunk() const;

    void convertToUserPreset();
    void setFilePath(const std::string& filePath);
    const std::string& getFilePath() const { return m_filePath; }

    size_t getNumParams() const { return m_initialParams.size(); }

    Program(const Program& other);
    Program& operator=(Program rhs);

private:
    // @note: if name is longer than VST allows (24) it will be truncated in VST host display and in FXP files.
    std::string m_name;
    std::string m_category;
    std::string m_filePath;
    std::vector<float> m_initialParams;
    std::vector<float> m_currentParams;
    std::vector<float>* m_activeParams;
    bool m_isDirty;
    bool m_isFactory;
};

