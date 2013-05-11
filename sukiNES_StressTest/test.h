#pragma once

#include <functional>

#ifdef SUKINES_PLATFORM_WINDOWS
#include <windows.h>
#endif

#ifdef SUKINES_PLATFORM_WINDOWS
#define assertIsEqual(actual, expected, message) \
	do { \
		if ( actual != expected ) { \
			if (IsDebuggerPresent()) ForceBreakpoint(); \
			_generateFailureMessage(message, actual, expected); \
			return false; \
		} \
	} while(0)
#else
#define assertIsEqual(actual, expected, message) \
	do { \
		if ( actual != expected ) { \
			_generateFailureMessage(message, actual, expected); \
			return false; \
		}
	} while(0)
#endif

#define STRESSTEST_REGISTER_TEST(Class, TestName) \
	extern "C" StressTest::Test* __create_Test_##Class##__() \
	{ \
		auto *test = new Class; \
		test->setTestName(#TestName); \
		return test; \
	} \
	StressTest::TestRegister __register_Test_##Class##(__create_Test_##Class##__); \

namespace StressTest
{
	class Test
	{
	public:
		Test();
		virtual ~Test();

		virtual bool run() = 0;

		void printFailure();

		const char* testName() const
		{
			return _testName;
		}

		void setTestName(const char* testName)
		{
			_testName = testName;
		}

	protected:
		virtual void printExtraFailureMessage();

		void _generateFailureMessage(const char* message);

		template<typename T1>
		void _generateFailureMessage(const char* message, T1 filename);

		template<typename T1, typename T2>
		void _generateFailureMessage(const char* message, T1 actual, T2 expected);

	private:
		void _allocateFailureMessage();

	private:
		char* _failureMessage;
		const char* _testName;
	};

	template<typename T1>
	void Test::_generateFailureMessage(const char* message, T1 filename)
	{
		_allocateFailureMessage();

		sprintf(_failureMessage, message, filename);
	}

	template<typename T1, typename T2>
	void Test::_generateFailureMessage(const char* message, T1 actual, T2 expected)
	{
		_allocateFailureMessage();

		sprintf(_failureMessage, "%s: actual=%#X, expected=%#X", message, actual, expected);
	}

	class TestRegister
	{
	public:
		typedef std::function<StressTest::Test*(void)> TestCreatorFunctionPointer;

		TestRegister(TestCreatorFunctionPointer testCreatorFunction);
	};
}
