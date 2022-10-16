/*
 *  macUtils.h
 *  BDM
 *
 *  Created by Adrian Pflugshaupt on 2/9/08.
 *
 * This defines some stuff which does not exist on MAC in order to have less noise in the code
 */

#ifndef __macUtils__
#define __macUtils__


#define __forceinline __attribute__((always_inline))

#include <string.h>

inline int stricmp(const char* a, const char* b) 
{ 
	return strcasecmp(a, b); 
}

inline int strnicmp(const char* a, const char* b, size_t c) 
{ 
	return strncasecmp(a, b, c); 
}

inline short endianFlipShort(const short value)
{
	return	((value & 0xff00) >> 8) |
			((value & 0x00ff) << 8);
}

inline long endianFlipLong(const long value)
{
	return	((value & 0xff000000) >> 24) |
			((value & 0x00ff0000) >>  8) |
			((value & 0x0000ff00) <<  8) |
			((value & 0x000000ff) << 24);
}

inline int endianFlipInt(const int value)
{
	return	((value & 0xff000000) >> 24) |
			((value & 0x00ff0000) >>  8) |
			((value & 0x0000ff00) <<  8) |
			((value & 0x000000ff) << 24);
}

inline float endianFlipFloat(const float floatvalue)
{
	const int* val = (const int*)&floatvalue;
	float result;
	*((int*)&result) = 
		((*val & 0xff000000) >> 24) |
		((*val & 0x00ff0000) >>  8) |
		((*val & 0x0000ff00) <<  8) |
		((*val & 0x000000ff) << 24);
	return result;
}

#ifndef MAX_PATH
 #define MAX_PATH (2048)
#endif

#if defined(__cplusplus)
class AutoreleasePool
{
public:
	AutoreleasePool();
	~AutoreleasePool();
private:
	void *m_pool;
};

class ScopedLogger
{
public:
    ScopedLogger(void *ptr, const char *desc);
    ScopedLogger(void *ptr, const char *desc, int arg);
    ScopedLogger(void *ptr, const char *desc, int arg1, int arg2);
    ~ScopedLogger();
private:
    void *m_ptr;
    const char *m_desc;
};
#endif // defined(__cplusplus)

// Helper functions for saving and loading preferences.
// Ideally these would be in a "Platform" class in a more complex project.
void saveFloatPreference(const char *key, float value);
float loadFloatPreference(const char *key, float defaultValue);

#endif // __macUtils__
