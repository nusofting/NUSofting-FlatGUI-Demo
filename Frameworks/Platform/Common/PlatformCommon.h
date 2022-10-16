//
//  PlatformCommon.h
//
// While I want to avoid conditional compilation as much as possible, there
// exists some code, such as the dirent facility, that is a standard system
// facility on one platform with a user-contributed implementation on other
// platforms. This typically means needing to include headers via the system
// include path on the first platform but the user include path on the other
// platforms (i.e. included via angle brackets or quotes).
//
// The pragmatic approach is to just use conditional compilation here, for the
// moment.
//
//  Copyright 2016 Bernie Maier. All rights reserved.
//  Licensed to NUSofting.
//

#if defined(__APPLE__) && defined(__MACH__)

    #include <dirent.h>

#elif defined(_WIN32)

    #include "../Win/dirent.h"

#endif
