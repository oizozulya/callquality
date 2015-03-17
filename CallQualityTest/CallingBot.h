/**
*	Defines class of calling bot 
*/

#include "MessagingBroker.h"
#include "CallStats.h"

class CCallingBot {
private:
	static std::mutex m_Lock;	//TBD: static?
public:
	static unsigned int m_nBotCount; 

public: 
	CCallingBot();
	//CCallingBot(int nBotId);
	~CCallingBot();

	bool Initialize();
	bool Terminate();
	bool Register();
	int MakeCall();
	void StartRingingAndAnswerCall();
	int EndCall();
	void OnMessageReceived(CMessage* pMessage);
	bool PutMessage(CMessage* pMessage);
		
	/**
	 * RunMethod used in reading messages from the message queue.
	 */
	void RunMethod();

protected:
	bool SendMessage(CMessage* pMessage);
	bool SendStatistic();
	void ClearStats();

	int OnCall(CMessage* pMessage);
	int OnRing(CMessage* pMessage);
	int OnAnswer(CMessage* pMessage);
	int OnDecline(CMessage* pMessage);
	int OnEnd(CMessage* pMessage);

	bool GetDecision();


public:
	int m_nBotId;
	//int m_nSleepTime;
	//int m_nRingTime;
	//int m_nSpeakTime;
	bool m_bOnACall;
	bool m_bRinging;
	bool m_bCaller;

	//call statistic
	CCallStats m_CallStatistic;

protected:
	std::thread m_BotThread;
	std::queue<CMessage*> m_BotQueue;
	bool m_bShutdown;
	unsigned int m_nChangeStateTime;

};