#pragma once

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define SUKINES_PLATFORM_WINDOWS
#endif

#if defined(DEBUG) || defined(_DEBUG) || defined(_SUKINES_DEBUG)
#define SUKINES_DEBUG
#elif defined(_SUKINES_FINAL)
#define SUKINES_FINAL
#elif defined(NDEBUG) || defined(_SUKINES_RELEASE)
#define SUKINES_RELEASE
#endif

typedef unsigned char u8, uint8;
typedef signed char s8, sint8;
typedef unsigned short u16, uint16;
typedef signed short s16, sint16;
typedef unsigned int u32, uint32;
typedef signed int s32, sint32;

typedef u8 byte;
typedef s8 offset;
typedef u32 dword;
typedef s32 sdword;

#define SUKINES_KB(x) (x * 1024u)
#define SUKINES_BIT(x) (1 << x)

#define ForceBreakpoint() __debugbreak()

#include "types.h"

typedef ManagedWord word;
