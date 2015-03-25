/**
* Implements class of CallingBot
*/

#include <thread>
#include <chrono>
#include "CallingBot.h"
#include "CallQualityTestTool.h"

unsigned int CCallingBot::m_nBotCount = 0; 

CCallingBot::CCallingBot()
:m_bShutdown(false),
m_BotState(eSTATE_IDLE)
{
	std::queue<CMessage*> m_BotQueue;
	std::mutex m_Lock;
	std::mutex m_QueueLock;
	condition_variable m_ConditionVariable;

	m_nBotId = m_nBotCount;
	printf("Creating bot with ID = %d \n", m_nBotId);
	m_nBotCount++;
	CCallStats m_CallStatistic;
	Register();		//move to another method
}

CCallingBot::~CCallingBot() {

}

bool CCallingBot::Initialize() {
	//Add check for isInialized
	m_BotThread = std::thread(&CCallingBot::RunMethod, this);
	return true;
}

bool CCallingBot::Terminate() {
	m_bShutdown = true;
	m_ConditionVariable.notify_all();
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
	m_ConditionVariable.notify_all();
	return true;
}

bool CCallingBot::SendMessage(CMessage* pMessage) {
	if (pMessage == NULL) {
		printf("CCallingBot::SendMessage() Invalid message pointer was passed. Exiting. \n");
		return false;
	}
	return CMessagingBroker::Instance().PutMessage(pMessage);
}

bool CCallingBot::RandomAcceptCall() {
	int res = (int)clock()%2;
	//printf("Decision on %d: %d \n", m_nBotId, (int)res);
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

//Callee handler
int CCallingBot::OnCall(CMessage* pMessage) {	
	if (m_BotState == eSTATE_RINGING || m_BotState == eSTATE_INIT_CALL || m_BotState == eSTATE_FAR_RINGING) {
		//Cannot accept call when have current call is not established
		printf("%d >----x %d %d\n",pMessage->m_nSourceId, m_nBotId, clock());
		CMessage DeclineMessage(eMSG_DECLINE, m_nBotId, pMessage->m_nSourceId);
		SendMessage(&DeclineMessage);
		return 1;
	}
	if (m_BotState == eSTATE_IDLE) {
		//answer incoming call
		m_CallStatistic.SetFarEndId(pMessage->m_nSourceId);
		StartRinging();
		return 1;
	}
	//else if m_BotState == eSTATE_ON_OWN_CALL || m_BotState == eSTATE_ON_INCOMING_CALL
	if (RandomAcceptCall()) {
		if (m_BotState == eSTATE_ON_OWN_CALL || m_BotState == eSTATE_ON_INCOMING_CALL) {
			EndCall(m_CallStatistic.GetFarEndId(), true);
		}
		//If current call was ended or no active call
		//Start ringing and notify caller about ringing
		m_CallStatistic.SetFarEndId(pMessage->m_nSourceId);
		StartRinging();
	} 
	else {
		printf("%d >----x %d %d\n",pMessage->m_nSourceId, m_nBotId, clock());
		CMessage DeclineMessage(eMSG_DECLINE, m_nBotId, pMessage->m_nSourceId);
		SendMessage(&DeclineMessage);
	}
	return 1;
}

//Caller handlers
int CCallingBot::OnRing(CMessage* pMessage) {
	if (m_BotState == eSTATE_INIT_CALL) {
		m_CallStatistic.SetCallRinging();
		printf("%d >>-->> %d %d\n", m_nBotId, pMessage->m_nSourceId, clock());

		m_BotState = eSTATE_FAR_RINGING;
	}
	else {
		printf("IGNORED RINGING %d >>-->> %d %d\n", m_nBotId, pMessage->m_nSourceId, clock());
	}
	return 0;
}

int CCallingBot::OnAnswer(CMessage* pMessage) {
	if (m_BotState == eSTATE_FAR_RINGING) {
		m_CallStatistic.SetCallAnswered();
		printf("%d =----= %d %d\n", m_nBotId, pMessage->m_nSourceId, clock());

		m_BotState = eSTATE_ON_OWN_CALL;
		m_nChangeStateTime = clock() + CCallQualityTestTool::Instance().GetRandomDuration();
	}
	else {
		printf("IGNORED ANSWERED %d =----= %d %d\n", m_nBotId, pMessage->m_nSourceId, clock());
	}
	return 0;
}

int CCallingBot::OnDecline(CMessage* pMessage) {
	if (m_BotState == eSTATE_INIT_CALL) {
		m_CallStatistic.SetCallDeclined();
		EndCall(pMessage->m_nSourceId, false);
	}
	else {
		printf("IGNORED DECLINED %d >>---x %d %d\n", m_nBotId, pMessage->m_nSourceId, clock());
	}
	return 0;
}

//For caller and callee
int CCallingBot::OnEnd(CMessage* pMessage) {
	if (m_BotState == eSTATE_ON_OWN_CALL || m_BotState == eSTATE_ON_INCOMING_CALL) {
		EndCall(pMessage->m_nSourceId, false);
	}
	else {
		printf("IGNORED END %d =----x %d %d\n", m_nBotId, pMessage->m_nSourceId, clock());
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////


//Methods for messageHandlers
void CCallingBot::StartRinging() {
	CMessage RingMessage(eMSG_RING, m_nBotId, m_CallStatistic.GetFarEndId());
	printf("%d >--->> %d %d\n",m_CallStatistic.GetFarEndId(), m_nBotId, clock());
	SendMessage(&RingMessage);
	
	m_BotState = eSTATE_RINGING;
	m_nChangeStateTime = clock() + CCallQualityTestTool::Instance().GetRandomDuration();
}

// Methods for RunMethod
//For caller
int CCallingBot::MakeCall() {
	unsigned int nCallee = CCallQualityTestTool::Instance().GetRandomCallee(m_nBotId);
	m_CallStatistic.SetFarEndId(nCallee);
	m_CallStatistic.SetCallStarted();

	printf("%d >----> %d %d\n", m_nBotId, m_CallStatistic.GetFarEndId(), clock());

	CMessage CallMessage = CMessage(eMSG_CALL, m_nBotId, m_CallStatistic.GetFarEndId());
	SendMessage(&CallMessage);	

	m_BotState = eSTATE_INIT_CALL;
	m_nChangeStateTime = clock() + CCallQualityTestTool::Instance().GetRandomDuration();
	return 1;
}

int CCallingBot::EndCall(int nFarEndId, bool bSendMessage) {
	int nFarEndBotId = m_CallStatistic.GetFarEndId();

	if (nFarEndId != nFarEndBotId) {
		printf("%d ignored END_CALL message from %d %d\n", m_nBotId, nFarEndId, clock());
		return 0;
	}

	if (m_BotState == eSTATE_FAR_RINGING) {
		printf("%d %s---%s %d %d\n", m_nBotId, bSendMessage?"x":"-", bSendMessage?">>":"--",  m_CallStatistic.GetFarEndId(), clock());
		SendStatistic();
		ClearStats();
	} else if (m_BotState == eSTATE_INIT_CALL) {
		printf("%d %s----%s %d %d\n", m_nBotId, bSendMessage?"x":"-", bSendMessage?">":"-", m_CallStatistic.GetFarEndId(), clock());
		SendStatistic();
		ClearStats();
	}  else if (m_BotState == eSTATE_ON_OWN_CALL) {
		printf("%d %s----%s %d %d\n", m_nBotId, bSendMessage?"x":"-", bSendMessage?"=":"-", m_CallStatistic.GetFarEndId(), clock());
		SendStatistic();
		ClearStats();
	} else if (m_BotState == eSTATE_ON_INCOMING_CALL) {
		printf("%d %s----%s %d %d\n", m_CallStatistic.GetFarEndId(), bSendMessage?"=":"-", bSendMessage?"x":"-", m_nBotId, clock());
		ClearStats();
	} else if (m_BotState == eSTATE_RINGING) {
		printf("%d >---%s %d %d\n", m_CallStatistic.GetFarEndId(), bSendMessage?">x":"--", m_nBotId, clock());
		ClearStats();
	} else {
		printf("END_CALL for %d on IDLE STATE %d\n", m_nBotId, clock());
		return 0;
	}
	if (bSendMessage) {
		CMessage EndMessage(eMSG_END, m_nBotId, nFarEndBotId);
		SendMessage(&EndMessage);
	}

	m_BotState = eSTATE_IDLE; 
	m_nChangeStateTime = clock() + CCallQualityTestTool::Instance().GetRandomDuration();
	return 1;	
}

//For callee
void CCallingBot::AnswerCall() {
	CMessage AnswerMessage(eMSG_ANSWER, m_nBotId, m_CallStatistic.GetFarEndId());
	printf("%d >----= %d %d\n",m_CallStatistic.GetFarEndId(), m_nBotId, clock());
	SendMessage(&AnswerMessage);

	m_BotState = eSTATE_ON_INCOMING_CALL;
	m_nChangeStateTime = INT32_MAX;
	return;
}

void CCallingBot::RunMethod() {
	unique_lock<mutex> lock(m_QueueLock);

	printf("CCallingBot::RunMethod() Starting. \n");

	//set initial SleepTime
	m_nChangeStateTime = clock() + CCallQualityTestTool::Instance().GetRandomDuration();

	while(!this->m_bShutdown) {
		if (clock() >= m_nChangeStateTime) {
			if (m_BotState == eSTATE_ON_OWN_CALL) {
				//SpeakTime
				//Caller may end call
				EndCall(m_CallStatistic.GetFarEndId(), true);
			} else if (m_BotState == eSTATE_IDLE) {
				//SleepTime
				//Idle bot may initiate call
				MakeCall();
			} else if (m_BotState == eSTATE_RINGING) {
				//RingTime
				//Ringing bot may accept call
				AnswerCall();
			}
		}

		CMessage* pMessage = NULL;
		m_Lock.lock();
		if (m_BotQueue.size() != 0) {
			pMessage = m_BotQueue.front();	//get first Message from queue
			m_BotQueue.pop();
		}
		m_Lock.unlock();
			
		if (pMessage != NULL) {
			OnMessageReceived(pMessage);
			delete pMessage;
		}
		else {
			m_ConditionVariable.wait_for(lock, std::chrono::milliseconds(m_nChangeStateTime - clock()));
		}
	}
	printf("CCallingBot::RunMethod Exiting thread. \n");
	return;
}

bool CCallingBot::SendStatistic() {
	bool rc = false;
	printf("Statistics from %d: time-to-ring = %d, call %s. \n", m_nBotId, m_CallStatistic.GetTimeToRing(), m_CallStatistic.IsAnswered()? "answered":"not answered");
	rc = CTelemetryStorage::Instance().PutStatistic(m_CallStatistic.GetTimeToRing(), m_CallStatistic.IsAnswered());
	return rc;
}



