#include "testrunner.h"

// STL includes
#include <cstdlib>

// Local includes
#include "test.h"

namespace StressTest
{
	TestRunner* TestRunner::_instance = nullptr;

	TestRunner::TestRunner()
	{
	}

	void TestRunner::registerTest(TestRunner::TestCreatorFunctionPointer testCreatorFunction)
	{
		_tests.push_back(testCreatorFunction);
	}

	int TestRunner::run(int argc, char** argv)
	{
		return TestRunner::self()._internalRun(argc, argv);
	}

	int TestRunner::_internalRun(int argc, char** argv)
	{
		int returnCode = 0;

		for(auto& testCreatorFunction : _tests)
		{
			auto test = testCreatorFunction();
			if (test)
			{
				if (argc > 0)
				{
					auto arg = argv[1];
					auto testName = test->testName();
					if (strcmp(arg, testName) != 0)
					{
						continue;
					}
				}

				bool result = test->run();
				if (!result)
				{
					returnCode = 1;
				}

				fprintf(stderr, "%s: ", test->testName());
				if (result)
				{
					fprintf(stderr, "[ OK ]\n");
				}
				else
				{
					fprintf(stderr, "[ ");
					test->printFailure();
					fprintf(stderr, " ]\n");
				}
			}
		}

		return returnCode;
	}
}