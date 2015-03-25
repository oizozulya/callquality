/**
*	Defines class of calling bot 
*/

#include "MessagingBroker.h"
#include "CallStats.h"

enum eBotState {
	eSTATE_UNDEFINED = -1,
	eSTATE_IDLE,
	eSTATE_INIT_CALL,
	eSTATE_FAR_RINGING,
	eSTATE_RINGING,
	eSTATE_ON_OWN_CALL,
	eSTATE_ON_INCOMING_CALL
};

class CCallingBot {

public:
	static unsigned int m_nBotCount; 

public: 
	CCallingBot();
	~CCallingBot();

	bool Initialize();
	bool Terminate();
	bool Register();

	int MakeCall();
	void StartRinging();
	void AnswerCall();
	int EndCall(int nFarEndId, bool bSendMessage);

	void OnMessageReceived(CMessage* pMessage);
	bool PutMessage(CMessage* pMessage);
		
	/**
	 * RunMethod used in reading messages from the message queue.
	 */
	void RunMethod();

protected:
	virtual bool SendMessage(CMessage* pMessage);
	virtual bool SendStatistic();
	void ClearStats();

	int OnCall(CMessage* pMessage);
	int OnRing(CMessage* pMessage);
	int OnAnswer(CMessage* pMessage);
	int OnDecline(CMessage* pMessage);
	int OnEnd(CMessage* pMessage);

	bool RandomAcceptCall();


public:
	int m_nBotId;

	//call statistic
	CCallStats m_CallStatistic;

protected:
	std::thread m_BotThread;
	std::queue<CMessage*> m_BotQueue;
	bool m_bShutdown;
	unsigned int m_nChangeStateTime;
	eBotState m_BotState;

private:
	std::mutex m_Lock;
	std::mutex m_QueueLock;
	condition_variable m_ConditionVariable;
};