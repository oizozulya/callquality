/**
*	Defines main calss for test tool process
*/

#include <iostream>
#include <list>
#include <mutex>
#include "MessagingBroker.h"
#include "TelemetryStorage.h"

enum eReturnCode {
	RC_UNKNOWN = -1,
	RC_FAILED = 0,
	RC_SUCCESS,
	RC_INCORRECT_PARAMS,
	RC_CANCELLED
};

class CCallQualityTestTool {
private:
	static CCallQualityTestTool* m_pSelf;

public:

	~CCallQualityTestTool();

	static CCallQualityTestTool& Instance();		//part of singleton's implementation

	/**
	*	Starts MessagingBroker, bots, creates telemetry storage
	*	@return true if all parts started successfully, otherwise return false 
	*/
	bool Initialize();

	/**
	*	Deletes  MessagingBroker, bots, telemetry storage
	*	@return true if all parts deleted successfully, otherwise return false 
	*/
	bool Terminate();

	/**
	*	Stops threads for bots and MessagingBroker. Should be used before calculating common statistics
	* @return if all pars stopped successfully
	*/
	bool ShutDown();
	
	/**
	*	Sets params of running testing tool
	*/
	void SetParams(unsigned int m_nNumberOfBots, unsigned int nBeginRange, unsigned int m_nEndRange);

	/**
	* Starts bots one by one
	*/
	bool StartBots();

	/**
	* Stops bots one by one
	*/
	bool StopBots();

	/**
	*	Returns random duration - value for ringTime, speakTime, sleepTime etc.
	*	@return value from range configured on start
	*/
	unsigned int GetRandomDuration();

	/**
	*	Returns random Id of callee
	*	@param nCaller Id of bot who calls this method
	*	@return value from 0 to number of bots, but not equal ti n Caller
	*/

	unsigned int GetRandomCallee(int nCaller);

protected:
		CCallQualityTestTool();

public:
	unsigned int m_nNumberOfBots;	//config param: number of bots
	unsigned int m_nBeginRange;		//config param: begin of the range
	unsigned int m_nEndRange;		//config param: end of the range
	CMessagingBroker* m_pMessagingBroker;	//pointer to MessagingBroker
	CTelemetryStorage* m_pTelemetryStorage;	//pointer to TelemetryStorage

private: 
	static unsigned int m_unSeed;	//seed for getting random values

};


