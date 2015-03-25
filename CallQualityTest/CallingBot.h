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
	static unsigned int m_nBotCount;	//static variable for counting bots and assigning appropriate IDs

public: 
	CCallingBot();
	~CCallingBot();

	bool Initialize();	//Starts bot's thread 
	bool Terminate();	//Stops bot's thread
	bool Register();	//Register bot into MessagingBroker

	/**
	*	Creates outgoing call to random callee, no params
	*	@ return 1 if call started successfully, otherwise returns 0
	*/
	int MakeCall();

	/**
	*	Bot starts ringing if incoming call is received
	*/
	void StartRinging();

	/**
	*	Bot answers call after ringTime expires
	*/
	void AnswerCall();

	/**
	*	Bot ends existing call
	*	@param nFarEndId ID of far-end bot
	*	@param bSendMessage indicated if eMSG_END message need to be sent or not
	*	@return 1 if call has ended successfully, otherwise return 0
	*/
	int EndCall(int nFarEndId, bool bSendMessage);

	/**	
	*	Processes incoming messages
	*	@param pMessage
	*/
	void OnMessageReceived(CMessage* pMessage);

	/**
	*	Puts message into bot's queue. Could be called by MessagingBroker or other bot
	*	@param pMessage
	*	@return true if successful, false if not
	*/
	bool PutMessage(CMessage* pMessage);
		
	/**
	 * RunMethod used in reading messages from the message queue.
	 */
	void RunMethod();

protected:
	/**
	*	Sends message to MessagingBroker
	*	@param pMessage
	*	@return true if successful, false if not
	*/
	virtual bool SendMessage(CMessage* pMessage);

	/**
	*	Send statistic ti TelemetryStorage
	*	@return true if successful, false if not
	*/
	virtual bool SendStatistic();

	/**
	*	Clear call statistics
	*/
	void ClearStats();

	/**
	*	Methods for handling incoming messages
	*	@param pMessage
	*	@return 1 is successful, 0 if not
	*/
	int OnCall(CMessage* pMessage);
	int OnRing(CMessage* pMessage);
	int OnAnswer(CMessage* pMessage);
	int OnDecline(CMessage* pMessage);
	int OnEnd(CMessage* pMessage);

	/**
	*	Method for making decision if incoming call should be accepted or not
	*	@return decision: true or false
	*/
	bool RandomAcceptCall();


public:
	int m_nBotId;						//bot's ID
	CCallStats m_CallStatistic;			//call statistic

protected:
	std::thread m_BotThread;			//bot's thread
	std::queue<CMessage*> m_BotQueue;	//queue for bot's messages
	bool m_bShutdown;					//indicates when user wants to shutdown app
	unsigned int m_nChangeStateTime;	//time when bot'll change state
	eBotState m_BotState;				//current state

private:
	std::mutex m_Lock;					// mutex for handling access to bot, prevents adding two or more messages into bot's queue at same time
	std::mutex m_QueueLock;				// mutex for proper handling of sleepTime
	condition_variable m_ConditionVariable;	//condition variable for notifying all threads that are wait for m_QueueLock
};