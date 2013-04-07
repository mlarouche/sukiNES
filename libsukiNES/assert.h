#pragma once

void __sukiPrintAssert(const char* conditionMessage, const char* userMessage, const char* fileName, const int line);

#define sukiUnused(x) do { (void)sizeof(x); } while(0)

#ifdef SUKINES_FINAL
#define sukiAssert(condition) do { sukiUnused(condition); } while(0)
#define sukiAssertWithMessage(condition, message) do { sukiUnused(condition); sukiUnused(message); } while(0)
#else
#define sukiAssert(condition) \
	do \
	{ \
		if (!(condition)) \
		{ \
			__sukiPrintAssert(#condition, nullptr, __FILE__, __LINE__); \
			ForceBreakpoint(); \
		} \
	} while(0)

#define sukiAssertWithMessage(condition, message) \
	do \
	{ \
		if (!(condition)) \
		{ \
			__sukiPrintAssert(#condition, (message), __FILE__, __LINE__); \
			ForceBreakpoint(); \
		} \
	} while(0)
#endif
