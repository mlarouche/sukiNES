#include "test.h"

// STL includes
#include <cstdlib>

// Local includes
#include "testrunner.h"

namespace StressTest
{
	TestRegister::TestRegister(TestRegister::TestCreatorFunctionPointer testCreatorFunction)
	{
		TestRunner::self().registerTest(testCreatorFunction);
	}

	Test::Test()
	: _failureMessage(nullptr)
	{
	}

	Test::~Test()
	{
		if (_failureMessage)
		{
			delete[] _failureMessage;
		}
	}

	void Test::printFailure()
	{
		if(_failureMessage)
		{
			fprintf(stderr, "FAILURE: %s", _failureMessage);
			printExtraFailureMessage();
		}
		else
		{
			fprintf(stderr, "FAILURE !");
			printExtraFailureMessage();
		}
	}

	void Test::printExtraFailureMessage()
	{
	}

	void Test::_generateFailureMessage(const char* message)
	{
		_allocateFailureMessage();

		sprintf(_failureMessage, message);
	}

	void Test::_allocateFailureMessage()
	{
		if(_failureMessage)
		{
			delete[] _failureMessage;
			_failureMessage = nullptr;
		}

		_failureMessage = new char[SUKINES_KB(2)];
	}
}