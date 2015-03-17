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

	static CCallQualityTestTool& Instance();

	int Run();
	int ShutDown();
	bool Initialize();
	bool Terminate();
	void SetParams(unsigned int m_nNumberOfBots, unsigned int nBeginRange, unsigned int m_nEndRange);
	bool StartBots();
	bool StopBots();

	//unsigned int GetCount();
	unsigned int GetRandomDuration();
	unsigned int GetRandomCallee(int nCaller);

protected:
		CCallQualityTestTool();

public:
	unsigned int m_nNumberOfBots;
	unsigned int m_nBeginRange;
	unsigned int m_nEndRange;
	CMessagingBroker* m_pMessagingBroker;
	CTelemetryStorage* m_pTelemetryStorage;

private: 
	static unsigned int m_unSeed;

};


