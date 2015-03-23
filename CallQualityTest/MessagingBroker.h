/** 
*	Defines protocol for communication between bots.
*/

/** 
*	Defines messaging broker that routs all messages from calling bots 
*/

#ifndef MESSAGINGBROKER_H
#define MESSAGINGBROKER_H

#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <map>
#include <condition_variable>
//#include "CallingBot.h"

using namespace std;

enum eMessageType {
	eMSG_UNDEFINED = -1,
	eMSG_CALL,
	eMSG_RING,
	eMSG_ANSWER,
	eMSG_DECLINE,
	eMSG_END
};

class CMessage {
public:
	CMessage();
	CMessage(const CMessage &in);
	CMessage(eMessageType nType, unsigned int nSource, unsigned int nDest);
	virtual ~CMessage();
	CMessage* Clone();

public:
	eMessageType m_nMessageType;
	unsigned int m_nSourceId;
	unsigned int m_nDestId;

};

class Compare {
public:
    bool operator() (const pair<int, CMessage*>& a, const pair<int, CMessage*>& b)
    {
        return a.first > b.first;
    }
};

class CMessagingBroker {
private: 
	static std::mutex m_Lock;	
	static CMessagingBroker* m_pSelf;

public:
	~CMessagingBroker();
	bool Initialize();
	bool Terminate();

	static CMessagingBroker& Instance();

	bool PutMessage(CMessage* message);
		
	/**
	 * RunMethod used in reading messages from the message queue.
	 */
	void RunMethod();

	friend class CCallingBot;

	bool RegisterBot(int nBotId, CCallingBot* pBot);


protected: 
	CMessagingBroker();

	std::thread m_MessagingBroker;

	bool m_bShutdown;


public: 
	map<int, CCallingBot*> m_Bots;

private:
	priority_queue<pair <int, CMessage*>, vector<pair<int, CMessage*>>, Compare> m_Queue;
	mutex m_QueueLock;
	condition_variable m_ConditionVariable;
};

#endif MESSAGINGBROKER_H





