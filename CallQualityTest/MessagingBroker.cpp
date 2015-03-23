/** Implements protocol for communication between bots.
*/

/** Implements messaging broker that routs all messages from calling bots 
*/

#include "MessagingBroker.h"
#include "CallQualityTestTool.h"
#include "CallingBot.h"
#include <cassert>
#include <chrono>

using namespace std;

CMessagingBroker* CMessagingBroker::m_pSelf = NULL;
mutex CMessagingBroker::m_Lock;


CMessagingBroker::CMessagingBroker():m_bShutdown(false) {
	priority_queue<pair<int, CMessage*>, vector<pair<int, CMessage*>>, Compare> m_Queue;
	condition_variable m_ConditionVariable;
}

CMessagingBroker::~CMessagingBroker() {
}

CMessagingBroker& CMessagingBroker::Instance() {
	if (m_pSelf == NULL)	{
		m_pSelf = new CMessagingBroker();
	}

	return *m_pSelf;
}

bool CMessagingBroker::Initialize() {
	m_MessagingBroker = std::thread(&CMessagingBroker::RunMethod, this);
	return true;
}

bool CMessagingBroker::Terminate() {
	m_bShutdown = true;
	m_ConditionVariable.notify_all();
	printf("CMessagingBroker::Terminate(): joining...\n");
	m_MessagingBroker.join();
	printf("CMessagingBroker::Terminate(): joined to thread.\n");
	return true;
}

void CMessagingBroker::RunMethod() {
	unique_lock<mutex> lock(m_QueueLock);
	int nSleepTime = -1;
	pair<int, CMessage*> pPair(-1, NULL);

	printf("CMessagingBroker::RunMethod() Starting. \n");
	while(!this->m_bShutdown) {

		m_Lock.lock();
		if (m_Queue.size() != 0) {
			pPair = m_Queue.top();
			if (pPair.first <= clock()) {
				m_Queue.pop();
			} else {
				nSleepTime = pPair.first - clock();	//calculate remaining time
				pPair = pair<int, CMessage*>(-1, NULL);
			}
		}
		else {
			pPair = pair<int, CMessage*>(-1, NULL);
			nSleepTime = 1000;
		}
		
		m_Lock.unlock();
		if (pPair.second != NULL) {
			m_Bots[pPair.second->m_nDestId]->PutMessage(pPair.second);
			delete pPair.second;
		}
		else {
			m_ConditionVariable.wait_for(lock, std::chrono::milliseconds(nSleepTime));
			//std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}
	printf("CMessagingBroker::RunMethod Exiting thread. \n");
	return;
}



bool CMessagingBroker::PutMessage(CMessage* pMessage) {

	if (pMessage == NULL) {
		printf("CMessagingBroker::PutMessage() Invalid message pointer was passed. Exiting. \n");
		return false;
	}

	CMessage* pClone = pMessage->Clone();
	int nRunOn = clock() + CCallQualityTestTool::Instance().GetRandomDuration();
	
	m_Lock.lock();
	m_Queue.push(pair<int, CMessage*>(nRunOn, pClone));
	//printf(" --%d---------> from %d to %d type %d runOn %d\n", clock(), pClone->m_nSourceId, pClone->m_nDestId, pClone->m_nMessageType, nRunOn);
	m_Lock.unlock();
	m_ConditionVariable.notify_all();	//
	return true;
}

bool CMessagingBroker::RegisterBot(int nBotId, CCallingBot* pBot) {
	m_Bots[nBotId] = pBot;
	return true;
}

///////////////////////////////////////////////////////////////////////

CMessage::CMessage(const CMessage &in) {
	m_nMessageType = in.m_nMessageType;
	m_nSourceId = in.m_nSourceId;
	m_nDestId = in.m_nDestId;
}

CMessage::CMessage(eMessageType nType, unsigned int nSource, unsigned int nDest) {
	m_nMessageType = nType;
	m_nSourceId = nSource;
	m_nDestId = nDest;
}

CMessage::~CMessage() {}


CMessage* CMessage::Clone() {
	return new CMessage(*this);
}