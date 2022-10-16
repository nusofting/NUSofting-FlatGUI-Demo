//
//  Utils.h
//
// A collection of utility functions that don't belong anywhere else.
//
//  Copyright 2015 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#pragma once

#include <cstring>

namespace Utils
{

inline bool startsWith(const char *prefix, const char *str)
{
    return strncmp(prefix, str, strlen(prefix)) == 0;
}

inline bool endsWith(const char *str, const char *suffix = 0)
{
    bool endingMatches = suffix == 0;
    if (!endingMatches)
    {
        size_t suffixLen = strlen(suffix);
        size_t strLen = strlen(str);
        endingMatches = strLen >= suffixLen && strcmp(str + strLen - suffixLen, suffix) == 0;
    }
    return endingMatches;
}

} // namespace Utils
