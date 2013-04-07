#include "Assert.h"

#ifdef SUKINES_PLATFORM_WINDOWS
#include <windows.h>
#endif

// STL includes
#include <cstdio>

#ifdef SUKINES_PLATFORM_WINDOWS
void __sukiPrintAssert(const char* condition, const char* message, const char* fileName, const int line)
{
	char messageBuffer[2048];
	sprintf_s(messageBuffer, "Assert triggered in %s:%d: Condition %s failed.", fileName, line, condition);
	if (message)
	{
		sprintf_s(messageBuffer, "%s Reason: %s", messageBuffer, message);
	}

	if (IsDebuggerPresent())
	{
		OutputDebugStringA(messageBuffer);
	}
	else
	{
		MessageBoxA(nullptr, messageBuffer, "Assert failed !", MB_OK | MB_ICONERROR);
	}
}
#else
void __sukiPrintAssert(const char* condition, const char* message, const char* fileName, const int line)
{
	fprintf(stderr, "Assert triggered in %s:%d: Condition %s failed.", fileName, line, condition);
	if (message)
	{
		fprintf(stderr, " Reason: %s", message);
	}
	fprintf(stderr, "\n");
}
#endif
