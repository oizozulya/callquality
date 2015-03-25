#include "stdafx.h"
#include "CppUnitTest.h"
#include "CallingBot.h"
#include "CallQualityTestTool.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CallQualityTest_UnitTest
{		
	TEST_CLASS(CCallingBotTestSuite)
	{
	public:
		class CTestCallingBot : public CCallingBot {
	
		public:
			std::vector<CMessage> m_MessageStorage;
			bool m_bStatisticSent;

			bool SendMessage(CMessage* pMessage) {
				m_MessageStorage.push_back(*pMessage);
				return true;
			}

			bool SendStatistic() {
				m_bStatisticSent = true;
				return true;
			}

			void CallClearStats() {
				ClearStats();
			}

			void SetState(eBotState botState) {
				m_BotState = botState;
				return;
			}

			void ResetChangeStateTime() {
				m_nChangeStateTime = 0;
				return;
			}

			void CleanupBot() {
				CallClearStats();
				m_MessageStorage.clear();
				SetState(eSTATE_UNDEFINED);
				ResetChangeStateTime();
			}

			eBotState GetState() {
				return m_BotState;
			}

			unsigned int GetChangeStateTime() {
				return m_nChangeStateTime;
			}
			
			eMessageType GetLastMessageType() {	  //returns type of last message in m_MessageStorage
				return m_MessageStorage.back().m_nMessageType;
			}

		};

		TEST_METHOD(TestClearStats)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			pTestCallingBot->m_CallStatistic.SetFarEndId(2);
			pTestCallingBot->m_CallStatistic.SetCallStarted();
			pTestCallingBot->m_CallStatistic.SetCallAnswered();

			pTestCallingBot->CallClearStats();
			Assert::AreEqual(-1, pTestCallingBot->m_CallStatistic.GetFarEndId());	//Check if call statistics are cleaned up
			Assert::AreEqual(0, pTestCallingBot->m_CallStatistic.GetTimeToRing());
			Assert::IsFalse(pTestCallingBot->m_CallStatistic.IsAnswered());

			delete pTestCallingBot;
		}

		TEST_METHOD(TestMakeCall)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;

			pTestCallingBot->MakeCall();

			Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());	//check that m_nChangeStateTime initialized correctly
			Assert::AreNotEqual(pTestCallingBot->m_nBotId, pTestCallingBot->m_CallStatistic.GetFarEndId());		//check correctness og FarEndId
			Assert::IsTrue((pTestCallingBot->GetState()) == eSTATE_INIT_CALL);	//check the bot's state
			Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_CALL);		//check if type of sent message is correct
		
			delete pTestCallingBot;
		}

		TEST_METHOD(TestStartRinging)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;

			pTestCallingBot->StartRinging();

			Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());	//check that m_nChangeStateTime initialized correctly
			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_RINGING);	//check the bot's state
			Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_RING);		//check if type of sent message is correct
			
			delete pTestCallingBot;
		}

		TEST_METHOD(TestAnswerCall)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;

			pTestCallingBot->AnswerCall();

			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_ON_INCOMING_CALL);	//check the bot's state
			Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_ANSWER);		//check if type of sent message is correct
			
			delete pTestCallingBot;
		}

		TEST_METHOD(EndCall_InvalidMessage)			//invalid far end id - nothing should change
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			pTestCallingBot->SetState(eSTATE_ON_OWN_CALL);

			pTestCallingBot->EndCall(pTestCallingBot->m_CallStatistic.GetFarEndId() + 1, false);

			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_ON_OWN_CALL);	//check the bot's state, shouldn't change
			Assert::IsFalse(pTestCallingBot->m_bStatisticSent);
			Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//check that no message sent
			delete pTestCallingBot;
		}

		TEST_METHOD(EndCall_ValidMessage_IdleState_NoSendMessage)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			pTestCallingBot->SetState(eSTATE_IDLE);

			pTestCallingBot->EndCall(pTestCallingBot->m_CallStatistic.GetFarEndId(), false);

			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_IDLE);	//check the bot's state
			Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//check that no message sent
			Assert::IsFalse(pTestCallingBot->m_bStatisticSent);
			delete pTestCallingBot;
		}

		TEST_METHOD(EndCall_ValidMessage_IdleState_SendMessage)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			pTestCallingBot->SetState(eSTATE_IDLE);

			pTestCallingBot->EndCall(pTestCallingBot->m_CallStatistic.GetFarEndId(), true);

			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_IDLE);	//check the bot's state
			Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//check that no message sent
			Assert::IsFalse(pTestCallingBot->m_bStatisticSent);
			delete pTestCallingBot;
		}

		TEST_METHOD(EndCall_ValidMessage_FarRingingState_NoSendMessage)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			pTestCallingBot->SetState(eSTATE_FAR_RINGING);

			pTestCallingBot->EndCall(pTestCallingBot->m_CallStatistic.GetFarEndId(), false);

			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_IDLE);	//check the bot's state
			Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());	//check that m_nChangeStateTime initialized correctly
			Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//check that no message sent
			Assert::IsTrue(pTestCallingBot->m_bStatisticSent);
			delete pTestCallingBot;
		}

		TEST_METHOD(EndCall_ValidMessage_FarRingingState_SendMessage)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			pTestCallingBot->SetState(eSTATE_FAR_RINGING);

			pTestCallingBot->EndCall(pTestCallingBot->m_CallStatistic.GetFarEndId(), true);

			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_IDLE);	//check the bot's state
			Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());	//check that m_nChangeStateTime initialized correctly
			Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_END);		//check that end message sent
			Assert::IsTrue(pTestCallingBot->m_bStatisticSent);
			delete pTestCallingBot;
		}

		TEST_METHOD(EndCall_ValidMessage_InitCallState_NoSendMessage)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			pTestCallingBot->SetState(eSTATE_INIT_CALL);

			pTestCallingBot->EndCall(pTestCallingBot->m_CallStatistic.GetFarEndId(), false);

			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_IDLE);	//check the bot's state
			Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());	//check that m_nChangeStateTime initialized correctly
			Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//check that no message sent
			Assert::IsTrue(pTestCallingBot->m_bStatisticSent);
			delete pTestCallingBot;
		}

		TEST_METHOD(EndCall_ValidMessage_InitCallState_SendMessage)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			pTestCallingBot->SetState(eSTATE_INIT_CALL);

			pTestCallingBot->EndCall(pTestCallingBot->m_CallStatistic.GetFarEndId(), true);

			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_IDLE);	//check the bot's state
			Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());	//check that m_nChangeStateTime initialized correctly
			Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_END);		//check that end message sent
			Assert::IsTrue(pTestCallingBot->m_bStatisticSent);
			delete pTestCallingBot;
		}

		TEST_METHOD(EndCall_ValidMessage_OnOwnCallState_NoSendMessage)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			pTestCallingBot->SetState(eSTATE_ON_OWN_CALL);

			pTestCallingBot->EndCall(pTestCallingBot->m_CallStatistic.GetFarEndId(), false);

			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_IDLE);	//check the bot's state
			Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());	//check that m_nChangeStateTime initialized correctly
			Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//check that no message sent
			Assert::IsTrue(pTestCallingBot->m_bStatisticSent);
			delete pTestCallingBot;
		}

		TEST_METHOD(EndCall_ValidMessage_OnOwnCallState_SendMessage)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			pTestCallingBot->SetState(eSTATE_ON_OWN_CALL);

			pTestCallingBot->EndCall(pTestCallingBot->m_CallStatistic.GetFarEndId(), true);

			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_IDLE);	//check the bot's state
			Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());	//check that m_nChangeStateTime initialized correctly
			Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_END);		//check that end message sent
			Assert::IsTrue(pTestCallingBot->m_bStatisticSent);
			delete pTestCallingBot;
		}

		TEST_METHOD(EndCall_ValidMessage_OnIncomingCallState_NoSendMessage)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			pTestCallingBot->SetState(eSTATE_ON_INCOMING_CALL);

			pTestCallingBot->EndCall(pTestCallingBot->m_CallStatistic.GetFarEndId(), false);

			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_IDLE);	//check the bot's state
			Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());	//check that m_nChangeStateTime initialized correctly
			Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//check that no message sent
			Assert::IsFalse(pTestCallingBot->m_bStatisticSent);			//incoming call, no statistic should be sent
			delete pTestCallingBot;
		}

		TEST_METHOD(EndCall_ValidMessage_OnIncomingCallState_SendMessage)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			pTestCallingBot->SetState(eSTATE_ON_INCOMING_CALL);

			pTestCallingBot->EndCall(pTestCallingBot->m_CallStatistic.GetFarEndId(), true);

			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_IDLE);	//check the bot's state
			Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());	//check that m_nChangeStateTime initialized correctly
			Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_END);		//check that end message sent
			Assert::IsFalse(pTestCallingBot->m_bStatisticSent);		//incoming call, no statistic should be sent
			delete pTestCallingBot;
		}

		TEST_METHOD(EndCall_ValidMessage_RingingState_NoSendMessage)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			pTestCallingBot->SetState(eSTATE_RINGING);

			pTestCallingBot->EndCall(pTestCallingBot->m_CallStatistic.GetFarEndId(), false);

			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_IDLE);	//check the bot's state
			Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());	//check that m_nChangeStateTime initialized correctly
			Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//check that no message sent
			Assert::IsFalse(pTestCallingBot->m_bStatisticSent);			//incoming call, no statistic should be sent
			delete pTestCallingBot;
		}

		TEST_METHOD(EndCall_ValidMessage_RingingState_SendMessage)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			pTestCallingBot->SetState(eSTATE_ON_INCOMING_CALL);

			pTestCallingBot->EndCall(pTestCallingBot->m_CallStatistic.GetFarEndId(), true);

			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_IDLE);	//check the bot's state
			Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());	//check that m_nChangeStateTime initialized correctly
			Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_END);		//check that end message sent
			Assert::IsFalse(pTestCallingBot->m_bStatisticSent);		//incoming call, no statistic should be sent
			delete pTestCallingBot;
		}

		TEST_METHOD(TestOnCall)
		{
			// Check possible bot sates
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			CMessage testMessage(eMSG_CALL, CCallQualityTestTool::Instance().GetRandomCallee(pTestCallingBot->m_nBotId),  pTestCallingBot->m_nBotId);

			for (int i = 0; i <= (int)(eSTATE_ON_INCOMING_CALL); i++) {
				pTestCallingBot->SetState((eBotState)i);
	
				pTestCallingBot->OnMessageReceived(&testMessage);
				if (i == 0) {	//eSTATE_IDLE
					Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_RINGING);	//check the bot's state
					Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());
					Assert::AreNotEqual(-1, pTestCallingBot->m_CallStatistic.GetFarEndId());
					Assert::AreNotEqual(0, (int)pTestCallingBot->m_MessageStorage.size());	
					Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_RING);	
				}
				else if (i == 1) { //eSTATE_INIT_CALL
					Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_INIT_CALL);	//check the bot's state
					Assert::AreNotEqual(0, (int)pTestCallingBot->m_MessageStorage.size());	
					Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_DECLINE);
				}
				else if (i == 2) { //eSTATE_FAR_RINGING
					Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_FAR_RINGING);	//check the bot's state
					Assert::AreNotEqual(0, (int)pTestCallingBot->m_MessageStorage.size());	
					Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_DECLINE);				
				}
				else if (i == 3) { //eSTATE_RINGING
					Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_RINGING);	//check the bot's state
					Assert::AreNotEqual(0, (int)pTestCallingBot->m_MessageStorage.size());	
					Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_DECLINE);					
				}
				else if (i == 4) { //eSTATE_ON_OWN_CALL
					if (pTestCallingBot->GetLastMessageType() == eMSG_DECLINE) {
						Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_ON_OWN_CALL);
						Assert::AreNotEqual(0, (int)pTestCallingBot->m_MessageStorage.size());	
						Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_DECLINE);	
					} 
					else {
						Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_RINGING);	//check the bot's state
						Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());
						Assert::AreNotEqual(-1, pTestCallingBot->m_CallStatistic.GetFarEndId());
						Assert::AreNotEqual(0, (int)pTestCallingBot->m_MessageStorage.size());	
						Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_RING);							//check that current state is eSTATE_RINGING and eMSG_RING exist in Message Storage

						//Remove eMSG_RING message from Message Storage
						pTestCallingBot->m_MessageStorage.pop_back();

						//check that eMSG_ENG message was send
						Assert::AreNotEqual(0, (int)pTestCallingBot->m_MessageStorage.size());	
						Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_END);	
					}
				}
				else if (i == 5) { //eSTATE_ON_INCOMING_CALL
					if (pTestCallingBot->GetLastMessageType() == eMSG_DECLINE) {
						Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_ON_INCOMING_CALL);
						Assert::AreNotEqual(0, (int)pTestCallingBot->m_MessageStorage.size());	
						Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_DECLINE);	
					} 
					else {
						Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_RINGING);	//check the bot's state
						Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());
						Assert::AreNotEqual(-1, pTestCallingBot->m_CallStatistic.GetFarEndId());
						Assert::AreNotEqual(0, (int)pTestCallingBot->m_MessageStorage.size());	
						Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_RING);							//check that current state is eSTATE_RINGING and eMSG_RING exist in Message Storage

						//Remove eMSG_RING message from Message Storage
						pTestCallingBot->m_MessageStorage.pop_back();

						//check that eMSG_ENG message was send
						Assert::AreNotEqual(0, (int)pTestCallingBot->m_MessageStorage.size());	
						Assert::IsTrue(pTestCallingBot->GetLastMessageType() == eMSG_END);	
					}				
				}
				//cleanup test bot
				pTestCallingBot->CleanupBot();
			}

			delete pTestCallingBot;
		}

		TEST_METHOD(TestOnRing)
		{
			// Check possible bot sates
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			CMessage testMessage(eMSG_RING, CCallQualityTestTool::Instance().GetRandomCallee(pTestCallingBot->m_nBotId),  pTestCallingBot->m_nBotId);

			for (int i = 0; i <= (int)(eSTATE_ON_INCOMING_CALL); i++) {
				pTestCallingBot->SetState((eBotState)i);
				
				pTestCallingBot->OnMessageReceived(&testMessage);
				if (i == 1) {	//eSTATE_INIT_CALL
					Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_FAR_RINGING);	//check the bot's state
					Assert::AreNotEqual(0, pTestCallingBot->m_CallStatistic.GetTimeToRing());	//Time-to-Ring is set
					Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//no messages in Message storage
				}
				else { //all other states - ignore
					Assert::IsTrue(pTestCallingBot->GetState() == (eBotState)i);	//check the bot's state, state remains same
					Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//no messages in Message storage
				}

				//cleanup test bot
				pTestCallingBot->CleanupBot();
			}

			delete pTestCallingBot;
		}

		TEST_METHOD(TestOnAnswer)
		{
			// Check possible bot sates
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			CMessage testMessage(eMSG_ANSWER, CCallQualityTestTool::Instance().GetRandomCallee(pTestCallingBot->m_nBotId),  pTestCallingBot->m_nBotId);

			for (int i = 0; i <= (int)(eSTATE_ON_INCOMING_CALL); i++) {
				pTestCallingBot->SetState((eBotState)i);
	
				pTestCallingBot->OnMessageReceived(&testMessage);
				if (i == 2) {	//eSTATE_FAR_RINGING
					Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_ON_OWN_CALL);	//check the bot's state
					Assert::IsTrue(pTestCallingBot->m_CallStatistic.IsAnswered());
					Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());
					Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//no messages in Message storage
				}
				else { //all other states - ignore
					Assert::IsTrue(pTestCallingBot->GetState() == (eBotState)i);	//check the bot's state, state remains same
					Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//no messages in Message storage
				}
				//cleanup test bot
				pTestCallingBot->CleanupBot(); 				
			}

			delete pTestCallingBot;
		}

		TEST_METHOD(TestOnDecline)
		{
			// Check possible bot sates
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			CMessage testMessage(eMSG_DECLINE, CCallQualityTestTool::Instance().GetRandomCallee(pTestCallingBot->m_nBotId),  pTestCallingBot->m_nBotId);

			for (int i = 0; i <= (int)(eSTATE_ON_INCOMING_CALL); i++) {
				pTestCallingBot->SetState((eBotState)i);
	
				pTestCallingBot->m_CallStatistic.SetFarEndId(testMessage.m_nSourceId);	//to emulate decline for existing call
				pTestCallingBot->OnMessageReceived(&testMessage);
				if (i == 1) {	//eSTATE_INIT_CALL
					
					Assert::IsTrue(pTestCallingBot->m_bStatisticSent);	
					Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_IDLE);	//check the bot's state
					Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());
					Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//no messages in Message storage
				}
				else { //all other states - ignore
					Assert::IsTrue(pTestCallingBot->GetState() == (eBotState)i);	//check the bot's state, state remains same
					Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//no messages in Message storage
				}
				//cleanup test bot
				pTestCallingBot->CleanupBot(); 				
			}

			delete pTestCallingBot;
		}

		TEST_METHOD(TestOnEnd)
		{
			// Check possible bot sates
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);
			pTestCallingBot->m_bStatisticSent = false;
			CMessage testMessage(eMSG_END, CCallQualityTestTool::Instance().GetRandomCallee(pTestCallingBot->m_nBotId),  pTestCallingBot->m_nBotId);

			for (int i = 0; i <= (int)(eSTATE_ON_INCOMING_CALL); i++) {
				pTestCallingBot->SetState((eBotState)i);
	
				pTestCallingBot->m_CallStatistic.SetFarEndId(testMessage.m_nSourceId);	//to emulate end for existing call
				pTestCallingBot->OnMessageReceived(&testMessage);
				if (i == 4 || i == 5) {	// eSTATE_ON_OWN_CALL or eSTATE_ON_INCOMING_CALL
					Assert::IsTrue(pTestCallingBot->m_bStatisticSent);	
					Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_IDLE);	//check the bot's state
					Assert::IsTrue(pTestCallingBot->GetChangeStateTime() > clock());
					Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//no messages in Message storage
				}
				else { //all other states - ignore
					Assert::IsTrue(pTestCallingBot->GetState() == (eBotState)i);	//check the bot's state, state remains same
					Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//no messages in Message storage
				}
				//cleanup test bot
				pTestCallingBot->CleanupBot(); 				
			}

			delete pTestCallingBot;
		}

		TEST_METHOD(TestOnMessageReceived)
		{
			CTestCallingBot* pTestCallingBot = new CTestCallingBot();
			pTestCallingBot->SetState(eSTATE_ON_INCOMING_CALL);
			pTestCallingBot->m_bStatisticSent = false;
			CCallQualityTestTool::Instance().SetParams(6, 500, 800);

			CMessage testMessage(eMSG_UNDEFINED, 5,  pTestCallingBot->m_nBotId);
			pTestCallingBot->OnMessageReceived(&testMessage);

			//Message type is undefined, no changes

			Assert::IsTrue(pTestCallingBot->GetState() == eSTATE_ON_INCOMING_CALL);	//check the bot's state
			Assert::AreEqual(0, (int)pTestCallingBot->m_MessageStorage.size());		//check that no message sent
			Assert::IsFalse(pTestCallingBot->m_bStatisticSent);		//no statistic should be sent

			delete pTestCallingBot;
		}
	};
}