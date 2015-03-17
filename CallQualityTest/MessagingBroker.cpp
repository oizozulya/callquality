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
std::mutex CMessagingBroker::m_Lock;

CMessagingBroker::CMessagingBroker():m_bShutdown(false) {
	std::queue<CMessage*> m_Queue;
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

	return true;	//TODO: Exception?
}

bool CMessagingBroker::Terminate() {
	m_bShutdown = true;
	printf("CMessagingBroker::Terminate(): joining...\n");
	m_MessagingBroker.join();
	printf("CMessagingBroker::Terminate(): joined to thread.\n");
	return true;
}

void CMessagingBroker::RunMethod() {

	printf("CMessagingBroker::RunMethod() Starting. \n");
	while(!this->m_bShutdown) {

		if (m_Queue.size() != 0) {
			CMessage* pMessage = m_Queue.front();	//get first Message from queue
			
			if (pMessage != NULL) {
				OnMessageReceived(pMessage);
				m_Queue.pop();   // Remove message from Is it safe?
				delete pMessage;
			}
			else {
				printf("CMessagingBroker::RunMethod pMessage == NULL \n");
			}
		}
		else {
			//printf("CMessagingBroker::RunMethod m_Queue is empty, continue. \n");
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
	
	m_Lock.lock();
	m_Queue.push(pClone);
	m_Lock.unlock();
	return true;
}

bool CMessagingBroker::SendMessage(CMessage* pMessage) {
	bool rc = false;
	rc = m_Bots[pMessage->m_nDestId]->PutMessage(pMessage);
	return rc;
}

void CMessagingBroker::OnMessageReceived(CMessage* pMessage) {
	//printf("CMessagingBroker::OnMessageReceived message = %d \n", pMessage->m_nMessageType);
	std::this_thread::sleep_for(std::chrono::milliseconds(CCallQualityTestTool::Instance().GetRandomDuration()));
	SendMessage(pMessage);
	return;
}

bool CMessagingBroker::RegisterBot(int nBotId, CCallingBot* pBot) {
	m_Bots[nBotId] = pBot;
	return true;
}

/* bool CMessagingBroker::StartBots() {
	for (int i = 0; i < (int)m_Bots.size(); i++) {
		m_Bots[i]->Initialize();
	}
	return true;
}*/

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