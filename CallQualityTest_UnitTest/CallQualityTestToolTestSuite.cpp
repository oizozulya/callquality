#include "stdafx.h"
#include "CppUnitTest.h"
#include "CallQualityTestTool.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CallQualityTest_UnitTest
{		
	TEST_CLASS(CTelemetryStorageTestSuite)
	{
	public:
		TEST_METHOD_INITIALIZE(Initialize)
		{
			CCallQualityTestTool::Instance().SetParams(8, 1000, 5000);
		}

		TEST_METHOD_CLEANUP(Cleanup)
		{
			CCallQualityTestTool::Instance().m_nNumberOfBots = 0;
			CCallQualityTestTool::Instance().m_nBeginRange = 0;
			CCallQualityTestTool::Instance().m_nEndRange = 0;
		}

		TEST_METHOD(TestGetRandomDuration) 
		{
			int random1 = CCallQualityTestTool::Instance().GetRandomDuration();
			int random2 = CCallQualityTestTool::Instance().GetRandomDuration();

			Assert::AreNotEqual(random1, random2);
		}

		TEST_METHOD(TestGetRandomCallee)
		{
			int random1 = CCallQualityTestTool::Instance().GetRandomCallee(5);
			int random2 = CCallQualityTestTool::Instance().GetRandomCallee(5);

			Assert::AreNotEqual(random1, random2);
		}
	};
}