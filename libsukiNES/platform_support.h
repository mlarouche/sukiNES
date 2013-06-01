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

typedef unsigned char uint8;
typedef signed char sint8;
typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned int uint32;
typedef signed int sint32;

typedef uint8 byte;
typedef sint8 offset;
typedef uint32 dword;
typedef sint32 sdword;

static_assert(sizeof(uint8) == 1, "uint8 is not equals to 1 byte on this platform");
static_assert(sizeof(sint8) == 1, "s8 is not equals to 1 byte on this platform");
static_assert(sizeof(uint16) == 2, "uint16 is not equals to 2 bytes on this platform");
static_assert(sizeof(sint16) == 2, "sint16 is not equals to 2 bytes on this platform");
static_assert(sizeof(uint32) == 4, "uint32 is not equals to 4 bytes on this platform");
static_assert(sizeof(sint32) == 4, "sint32 is not equals to 4 bytes on this platform");

#define SUKINES_KB(x) (x * 1024u)
#define SUKINES_BIT(x) (1 << x)

#define ForceBreakpoint() __debugbreak()

#include "types.h"

typedef ManagedWord word;
