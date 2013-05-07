#pragma once

// STL incldues
#include <functional>
#include <vector>

// Local includes
#include "singleton.h"

namespace StressTest
{
	class Test;

	class TestRunner : public Singleton<TestRunner>
	{
	public:
		typedef std::function<StressTest::Test*(void)> TestCreatorFunctionPointer;

		~TestRunner();

		void registerTest(TestCreatorFunctionPointer testCreator);

		static int run();
		
		friend class Singleton<TestRunner>;

	private:
		TestRunner();

		int _internalRun();

	private:
		std::vector<TestCreatorFunctionPointer> _tests;
	};

}