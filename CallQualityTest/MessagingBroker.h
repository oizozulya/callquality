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
	CMessage* Clone();		//Allows to clone message

public:
	eMessageType m_nMessageType;	//type of message
	unsigned int m_nSourceId;		//Id of bot that sent a message
	unsigned int m_nDestId;			//Id of bot that should receive a message

};

//////////////////////////////////////////////////////////////////////////////

//Comparator for pairs (waitTime, Message)
class Compare {
public:
    bool operator() (const pair<int, CMessage*>& a, const pair<int, CMessage*>& b)
    {
        return a.first > b.first;
    }
};

/////////////////////////////////////////////////////////////////////////////

class CMessagingBroker {
private: 
	static std::mutex m_Lock;				//mutes for protection from multiple access to MessagingBroker's queue
	static CMessagingBroker* m_pSelf;		//part of singleton implementation

public:
	~CMessagingBroker();
	bool Initialize();						//starts thread for MessagingBroker
	bool Terminate();						//stops thread for MessagingBroker

	static CMessagingBroker& Instance();	//part of singleton implementation

	bool PutMessage(CMessage* message);		//Method for adding messages into MessageBroker's queue, could be called by bots
		
	/**
	 * RunMethod used in reading messages from the message queue.
	 */
	void RunMethod();

	friend class CCallingBot;

	bool RegisterBot(int nBotId, CCallingBot* pBot);	//Bot is added into m_Bots


protected: 
	CMessagingBroker();

	std::thread m_MessagingBroker;			// MessagingBroker thread
	bool m_bShutdown;						//Indicates that user is stopping application


public: 
	map<int, CCallingBot*> m_Bots;			//this map stores info about all bots

private:
	priority_queue<pair <int, CMessage*>, vector<pair<int, CMessage*>>, Compare> m_Queue;	//MessagingBroker's message queue
	mutex m_QueueLock;						// mutex for proper handling of thread sleep
	condition_variable m_ConditionVariable;	//condition variable for notifying all threads that are wait for m_QueueLock
};

#endif MESSAGINGBROKER_H





