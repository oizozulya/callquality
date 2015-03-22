#include "stdafx.h"
#include "CppUnitTest.h"
#include "../CallQualityTest/CallStats.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CallQualityTest_UnitTest
{		
	TEST_CLASS(CCallStatsTestSuite)
	{
	public:
		
		TEST_METHOD(TestSetFarEndId)
		{
			CCallStats stats;
			stats.SetFarEndId(5);	//Set Far End Id and check if it's set correctly
			Assert::AreEqual(5, stats.GetFarEndId());
			stats.SetFarEndId(11);	//Update Far End Id and check if it has correct value
			Assert::AreEqual(11, stats.GetFarEndId());
		}

		TEST_METHOD(TestClean)
		{
			CCallStats stats;
			stats.SetFarEndId(10);
			stats.SetCallStarted();
			stats.SetCallRinging();
			stats.SetCallAnswered();
			Assert::IsTrue(stats.IsAnswered());

			stats.Clean();	//check that all fields are cleaned up properly
			Assert::IsFalse(stats.IsAnswered());
			Assert::AreEqual(-1, stats.GetFarEndId());
			Assert::AreEqual(0, stats.GetTimeToRing());
		}

	};
}