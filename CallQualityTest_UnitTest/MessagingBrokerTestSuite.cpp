#include "stdafx.h"
#include "CppUnitTest.h"
#include "MessagingBroker.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CallQualityUnitTest
{		
	TEST_CLASS(CMessagingBrokerTestSuite)
	{
	public:

		TEST_METHOD(TestInstance)
		{
			CMessagingBroker* pMessagingBroker1 = &CMessagingBroker::Instance();
			CMessagingBroker* pMessagingBroker2 = &CMessagingBroker::Instance();

			//Check if both pointers point to same object
			Assert::IsTrue(pMessagingBroker1 == pMessagingBroker2);

		}

		TEST_METHOD(TestMessagingBrokerDelayQueue)
		{
			pair<int, CMessage*> pair1(30, NULL);
			pair<int, CMessage*> pair2(10, NULL);
			pair<int, CMessage*> pair3(20, NULL);

			priority_queue<pair <int, CMessage*>, vector<pair<int, CMessage*>>, Compare> testQueue;
			testQueue.push(pair1);
			testQueue.push(pair2);
			testQueue.push(pair3);
			Assert::AreEqual(10, testQueue.top().first);
			testQueue.pop();
			Assert::AreEqual(20, testQueue.top().first);
			testQueue.pop();
			Assert::AreEqual(30, testQueue.top().first);
			testQueue.pop();
			Assert::AreEqual(0, (int)testQueue.size());
		}

	};
}