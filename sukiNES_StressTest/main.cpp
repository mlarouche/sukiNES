// Local includes
#include "testrunner.h"

#ifdef SUKINES_PLATFORM_WINDOWS
void showpause()
{
	system("pause");
}
#endif

int main(int argc, char** argv)
{
#ifdef SUKINES_PLATFORM_WINDOWS
	atexit(showpause);
#endif

	return StressTest::TestRunner::run(argc, argv);
}
