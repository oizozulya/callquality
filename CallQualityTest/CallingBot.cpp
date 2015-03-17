/**
* Implements class of CallingBot
*/

#include <thread>
#include <chrono>
#include "CallingBot.h"
#include "CallQualityTestTool.h"

unsigned int CCallingBot::m_nBotCount = 0; 
std::mutex CCallingBot::m_Lock;

CCallingBot::CCallingBot()
:m_bShutdown(false),
m_bOnACall(false),
m_bRinging(false),
m_bCaller(false)
{
	m_nBotId = m_nBotCount;
	printf("Creating bot with ID = %d \n", m_nBotId);
	m_nBotCount++;
	CCallStats m_CallStatistic;
	Register();		//move to another method
}

CCallingBot::~CCallingBot() {

}

bool CCallingBot::Initialize() {
	//try
	m_Lock.lock();//TODO Why do we need that?
	m_BotThread = std::thread(&CCallingBot::RunMethod, this);
	m_Lock.unlock();//TODO Why do we need that?
	//catch
	//Register();
	return true;
}

bool CCallingBot::Terminate() {
	m_bShutdown = true;
	printf("CCallingBot::Terminate(): joining...\n");
	//try
	m_BotThread.join();
	//catch
	printf("CCallingBot::Terminate(): joined to thread.\n");
	return true;
}

bool CCallingBot::Register() {
	CMessagingBroker::Instance().RegisterBot(m_nBotId, this);
	return true;
}

bool CCallingBot::PutMessage(CMessage* pMessage) {
	if (pMessage == NULL) {
		printf("CCallingBot::PutMessage() Invalid message pointer was passed. Exiting. \n");
		return false;
	}

	CMessage* pClone = pMessage->Clone();
	
	m_Lock.lock();
	m_BotQueue.push(pClone);
	m_Lock.unlock();
	return true;
}

bool CCallingBot::SendMessage(CMessage* pMessage) {
	if (pMessage == NULL) {
		printf("CCallingBot::SendMessage() Invalid message pointer was passed. Exiting. \n");
		return false;
	}
	return CMessagingBroker::Instance().PutMessage(pMessage);
}

bool CCallingBot::GetDecision() {
	int res = (int)clock()%2;
	printf("Decision on %d: %d \n", m_nBotId, (int)res);
	return (res == 1) ? true : false;
}

void CCallingBot::ClearStats() {
	m_CallStatistic.Clean();
	return;
}

void CCallingBot::OnMessageReceived(CMessage* pMessage) {
	//printf("CCallingBot::OnMessageReceived Bot = %d, message = %d \n", m_nBotId, pMessage->m_nMessageType);
	switch(pMessage->m_nMessageType)
	{
	case eMSG_CALL:
		OnCall(pMessage);
		break;
	case eMSG_RING:
		OnRing(pMessage);
		break;
	case eMSG_ANSWER:
		OnAnswer(pMessage);
		break;
	case eMSG_DECLINE:
		OnDecline(pMessage);
		break;
	case eMSG_END:
		OnEnd(pMessage);
		break;
	default:
		printf("CCallingBot::OnMessageReceived: Undefined type of message \n");
		break;
	}
	return;
}

int CCallingBot::OnCall(CMessage* pMessage) {
	if (m_bOnACall) {
		//Cannot accept call when have current call in ringing state
		if (!m_bRinging && GetDecision()) {
			EndCall();
		}
		else {
			CMessage DeclineMessage(eMSG_DECLINE, m_nBotId, pMessage->m_nSourceId);
			SendMessage(&DeclineMessage);
			return 0;	//If decision is flase, then do nothing, leave this method.
		}
	}
	//If current call was ended or no active call
	//Start ringing and notify caller about ringing
	m_CallStatistic.SetFarEndId(pMessage->m_nSourceId);
	StartRingingAndAnswerCall();
	return 1;
}

int CCallingBot::OnRing(CMessage* pMessage) {
	//Calculate time-to-ring
	if (m_CallStatistic.CalcTimeToCall()) {
		return 1;
	}
	else {
		printf("CCallingBot::OnRing Cannot calculate Time-To-Ring \n");
		return 0;
	}
}

int CCallingBot::OnAnswer(CMessage* pMessage) {
	int rc = 0;
	m_CallStatistic.SetAnswered();
	m_nChangeStateTime = clock() + CCallQualityTestTool::Instance().GetRandomDuration();
	return rc;
}

int CCallingBot::OnDecline(CMessage* pMessage) {
	int rc = 0;
	m_CallStatistic.CalcTimeToCall();
	rc = EndCall();	//end call locally (without sending message to far-end)
	return rc;
}

int CCallingBot::OnEnd(CMessage* pMessage) {
	int rc = 0;
	if (m_bOnACall) {
		rc = EndCall();		//End call locally
	}
	return rc;

}

//////////////////////////////////////////////////////////////////////////////////


//Methods for messageHandlers
void CCallingBot::StartRingingAndAnswerCall() {
	m_bRinging = true;
	m_bOnACall = true;
	CMessage RingMessage(eMSG_RING, m_nBotId, m_CallStatistic.GetFarEndId());
	printf("StartRinging from %d to %d \n",m_nBotId, m_CallStatistic.GetFarEndId());
	SendMessage(&RingMessage);
	std::this_thread::sleep_for(std::chrono::milliseconds(CCallQualityTestTool::Instance().GetRandomDuration()));

	CMessage AnswerMessage(eMSG_ANSWER, m_nBotId, m_CallStatistic.GetFarEndId());
	printf("AnswerCall from %d to %d \n",m_nBotId, m_CallStatistic.GetFarEndId());
	SendMessage(&AnswerMessage);
	m_bRinging = false;
	return;
}

// Methods for RunMethod
int CCallingBot::MakeCall() {
	m_bOnACall = true;
	m_bCaller = true;
	unsigned int nCallee = CCallQualityTestTool::Instance().GetRandomCallee(m_nBotId);
	m_CallStatistic.SetFarEndId(nCallee);
	printf("MakeCall from %d to %d \n",m_nBotId, m_CallStatistic.GetFarEndId());
	CMessage CallMessage(eMSG_CALL, m_nBotId, m_CallStatistic.GetFarEndId());
	SendMessage(&CallMessage);
	m_CallStatistic.CallStarted();
	return 1;
}

int CCallingBot::EndCall() {
	printf("EndCall from %d to %d. Outgoing: %d \n",m_nBotId, m_CallStatistic.GetFarEndId(), m_bCaller);
	if (m_bCaller) {//Only for call initiator
		if (m_CallStatistic.IsAnswered()) {
			CMessage EndMessage(eMSG_END, m_nBotId, m_CallStatistic.GetFarEndId());
			SendMessage(&EndMessage);
		}
		SendStatistic();
		ClearStats();
	}
	m_bOnACall = false;
	m_bCaller = false;
	m_bRinging = false;
	m_nChangeStateTime = clock() + CCallQualityTestTool::Instance().GetRandomDuration();
	return 1;	
}

void CCallingBot::RunMethod() {

	printf("CCallingBot::RunMethod() Starting. \n");

	m_nChangeStateTime = clock() + CCallQualityTestTool::Instance().GetRandomDuration();
	while(!this->m_bShutdown) {

		if (clock() >= m_nChangeStateTime) {
			//Time for party!
			if (m_bOnACall) {
				EndCall();
			} else {
				MakeCall();
			}
		}


		if (m_BotQueue.size() != 0) {
			CMessage* pMessage = m_BotQueue.front();	//get first Message from queue
			
			if (pMessage != NULL) {
				OnMessageReceived(pMessage);
				m_BotQueue.pop();   // Remove message from Is it safe?
				delete pMessage;
			}
			else {
				printf("CCallingBot::RunMethod pMessage == NULL \n");
			}
		}
		else {
			//printf("CCallingBot::RunMethod m_Queue is empty, continue. \n");
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	printf("CCallingBot::RunMethod Exiting thread. \n");
	return;
}

bool CCallingBot::SendStatistic() {
	bool rc = false;
	printf("CCallingBot::SendStatistic(). \n");
	rc = CTelemetryStorage::Instance().PutStatistic(m_CallStatistic.GetTimeToCall(), m_CallStatistic.IsAnswered());
	return rc;
}



