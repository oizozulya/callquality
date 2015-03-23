#include "stdafx.h"
#include "CppUnitTest.h"
#include "TelemetryStorage.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CallQualityTest_UnitTest
{		
	TEST_CLASS(CTelemetryStorageTestSuite)
	{
	public:

		//Doesnt work. Why?
		/*TEST_METHOD_CLEANUP(Cleanup)
		{
			CTelemetryStorage::Instance().Cleanup();
		}*/
		
		TEST_METHOD(TestInstance)
		{
			CTelemetryStorage* pTelemetryStorage1 = &CTelemetryStorage::Instance();
			CTelemetryStorage* pTelemetryStorage2 = &CTelemetryStorage::Instance();

			//Check if both pointers point to same object
			Assert::IsTrue(pTelemetryStorage1 == pTelemetryStorage2);

		}

		TEST_METHOD(TestCalculateAnsCallsPercentage_EmptyAndRegularCase)
		{
			CTelemetryStorage::Instance().Cleanup();
			CTelemetryStorage* pTelemetryStorage = &CTelemetryStorage::Instance();

			Assert::AreEqual(0, pTelemetryStorage->CalculateAnsCallsPercentage());	//No statistics yet, percentage should be 0

			pTelemetryStorage->PutStatistic(10, true);
			pTelemetryStorage->PutStatistic(20, true);
			pTelemetryStorage->PutStatistic(30, true);
			pTelemetryStorage->PutStatistic(40, true);
			pTelemetryStorage->PutStatistic(50, true);
			pTelemetryStorage->PutStatistic(60, true);
			pTelemetryStorage->PutStatistic(70, false);
			pTelemetryStorage->PutStatistic(80, false);
			pTelemetryStorage->PutStatistic(90, false);
			pTelemetryStorage->PutStatistic(100, false);

			Assert::AreEqual(60, pTelemetryStorage->CalculateAnsCallsPercentage());	//Percentage is 60%
		}

		TEST_METHOD(TestCalculateAnsCallsPercentage_NoAnsweredCalls)
		{
			CTelemetryStorage::Instance().Cleanup();
			CTelemetryStorage* pTelemetryStorage = &CTelemetryStorage::Instance();
			pTelemetryStorage->PutStatistic(70, false);
			pTelemetryStorage->PutStatistic(80, false);
			pTelemetryStorage->PutStatistic(90, false);
			pTelemetryStorage->PutStatistic(100, false);

			Assert::AreEqual(0, pTelemetryStorage->CalculateAnsCallsPercentage());	//No answered calls
			pTelemetryStorage->Cleanup();
		}

		TEST_METHOD(TestCalculateAnsCallsPercentage_AllCallsAnswered)
		{
			CTelemetryStorage::Instance().Cleanup();
			CTelemetryStorage* pTelemetryStorage = &CTelemetryStorage::Instance();
			pTelemetryStorage->PutStatistic(10, true);
			pTelemetryStorage->PutStatistic(20, true);
			pTelemetryStorage->PutStatistic(30, true);
			pTelemetryStorage->PutStatistic(40, true);
			pTelemetryStorage->PutStatistic(50, true);
			pTelemetryStorage->PutStatistic(60, true);

			Assert::AreEqual(100, pTelemetryStorage->CalculateAnsCallsPercentage());	//All calls are answered
			
		}

		TEST_METHOD(TestCalculateAnsCallsPercentage_CorrectnessOfRound)
		{
			CTelemetryStorage::Instance().Cleanup();
			CTelemetryStorage* pTelemetryStorage = &CTelemetryStorage::Instance();

			pTelemetryStorage->PutStatistic(10, true);
			pTelemetryStorage->PutStatistic(20, true);
			pTelemetryStorage->PutStatistic(30, true);
			pTelemetryStorage->PutStatistic(40, true);
			pTelemetryStorage->PutStatistic(50, true);
			pTelemetryStorage->PutStatistic(60, true);
			pTelemetryStorage->PutStatistic(70, false);
			pTelemetryStorage->PutStatistic(80, false);
			pTelemetryStorage->PutStatistic(90, false);
			pTelemetryStorage->PutStatistic(100, false);
			pTelemetryStorage->PutStatistic(85, false);
			pTelemetryStorage->PutStatistic(95, false);
			pTelemetryStorage->PutStatistic(110, false);


			Assert::AreEqual(46, pTelemetryStorage->CalculateAnsCallsPercentage());	//Percentage is ~46,2%, should be rounded to 46%

			pTelemetryStorage->PutStatistic(95, false);

			Assert::AreEqual(43, pTelemetryStorage->CalculateAnsCallsPercentage()); //Percentage is ~42,8%, should be rounded to 43%
		}

		TEST_METHOD(TestCalculatePercentile_StatsEmpty)
		{
			CTelemetryStorage::Instance().Cleanup();
			CTelemetryStorage* pTelemetryStorage = &CTelemetryStorage::Instance();

			Assert::AreEqual(-1, pTelemetryStorage->CalculatePercentile(50));	//Stats are empty, no data to calculate percentile

		}

		TEST_METHOD(TestCalculatePercentile_PercentileLevelInvalid)
		{
			CTelemetryStorage::Instance().Cleanup();
			CTelemetryStorage* pTelemetryStorage = &CTelemetryStorage::Instance();

			pTelemetryStorage->PutStatistic(10, true);
			pTelemetryStorage->PutStatistic(20, false);
			pTelemetryStorage->PutStatistic(30, true);
			pTelemetryStorage->PutStatistic(40, false);
			pTelemetryStorage->PutStatistic(50, true);

			Assert::AreEqual(-1, pTelemetryStorage->CalculatePercentile(-3));	//Percentile levels are invalid
			Assert::AreEqual(-1, pTelemetryStorage->CalculatePercentile(0));
			Assert::AreEqual(-1, pTelemetryStorage->CalculatePercentile(100));
			Assert::AreEqual(-1, pTelemetryStorage->CalculatePercentile(500));
		}


		TEST_METHOD(TestCalculatePercentile_OneElement)
		{
			CTelemetryStorage::Instance().Cleanup();
			CTelemetryStorage* pTelemetryStorage = &CTelemetryStorage::Instance();

			pTelemetryStorage->PutStatistic(10, true);

			Assert::AreEqual(pTelemetryStorage->CalculatePercentile(80), pTelemetryStorage->CalculatePercentile(50));	//Percentiles with any level will be equal
		}

		TEST_METHOD(TestCalculatePercentile_CalculationCorrectness)
		{
			CTelemetryStorage::Instance().Cleanup();
			CTelemetryStorage* pTelemetryStorage = &CTelemetryStorage::Instance();

			// 3, 10, 20, 32, 45, 68, 70, 87, 90, 96 

			pTelemetryStorage->PutStatistic(70, true);
			pTelemetryStorage->PutStatistic(20, false);
			pTelemetryStorage->PutStatistic(90, true);
			pTelemetryStorage->PutStatistic(45, false);
			pTelemetryStorage->PutStatistic(87, true);
			pTelemetryStorage->PutStatistic(10, true);
			pTelemetryStorage->PutStatistic(32, false);
			pTelemetryStorage->PutStatistic(96, true);
			pTelemetryStorage->PutStatistic(3, false);
			pTelemetryStorage->PutStatistic(68, true);

			Assert::AreEqual(45, pTelemetryStorage->CalculatePercentile(50));
			Assert::AreEqual(90, pTelemetryStorage->CalculatePercentile(90));
			Assert::AreEqual(3, pTelemetryStorage->CalculatePercentile(10));
			Assert::AreEqual(70, pTelemetryStorage->CalculatePercentile(75));
		}

		TEST_METHOD(TestCalculatePercentile_EdgeCase)
		{
			CTelemetryStorage::Instance().Cleanup();
			CTelemetryStorage* pTelemetryStorage = &CTelemetryStorage::Instance();

			pTelemetryStorage->PutStatistic(10, true);
			pTelemetryStorage->PutStatistic(20, false);
			pTelemetryStorage->PutStatistic(30, true);
			pTelemetryStorage->PutStatistic(40, false);
			pTelemetryStorage->PutStatistic(50, true);

			Assert::AreEqual(10,pTelemetryStorage->CalculatePercentile(1));		//Lowest percentile
			Assert::AreEqual(40,pTelemetryStorage->CalculatePercentile(99));	//Highest percentile
			Assert::AreEqual(20, pTelemetryStorage->CalculatePercentile(40));	// Percentiles 40th and 45th are equal, but 50th is the next element
			Assert::AreEqual(20, pTelemetryStorage->CalculatePercentile(45));
			Assert::AreEqual(30, pTelemetryStorage->CalculatePercentile(50));

		}

	};
}