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
	//unsigned int m_nMessageId;
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
	static std::mutex m_Lock;	//TBD: static?
	static CMessagingBroker* m_pSelf;

public:
	~CMessagingBroker();
	bool Initialize();
	bool Terminate();
	//bool StartBots();

	static CMessagingBroker& Instance();

	void OnMessageReceived(CMessage* message);
	bool PutMessage(CMessage* message);
	//CMessage* GetMessage();		replaced with m_Queue.front()
	bool SendMessage(CMessage* message);
		
	/**
	 * RunMethod used in reading messages from the message queue.
	 */
	void RunMethod();

	friend class CCallingBot;

	bool RegisterBot(int nBotId, CCallingBot* pBot);


protected: 
	CMessagingBroker();

	//friend class CCallingBot;

		
	std::thread m_MessagingBroker;

	bool m_bShutdown;

public:
	//TODO move to private or protected (tests don't allow)
	priority_queue<pair <int, CMessage*>, vector<pair<int, CMessage*>>, Compare> m_Queue;
	map<int, CCallingBot*> m_Bots;

};

#endif MESSAGINGBROKER_H





