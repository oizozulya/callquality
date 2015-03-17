/**
	Implements main calss for test tool process
*/

#include <iostream>
#include <fstream>
#include <string>
#include "CallQualityTestTool.h"
#include "CallingBot.h"
#include "MessagingBroker.h"

unsigned int CCallQualityTestTool::m_unSeed = 0;
CCallQualityTestTool* CCallQualityTestTool::m_pSelf = NULL;

CCallQualityTestTool::CCallQualityTestTool()
:m_nNumberOfBots(0),
 m_nBeginRange(0),
 m_nEndRange(0),
 m_pMessagingBroker(NULL),
 m_pTelemetryStorage(NULL)
{}

CCallQualityTestTool::~CCallQualityTestTool() {}

CCallQualityTestTool& CCallQualityTestTool::Instance() {
	if (m_pSelf == NULL)	{
		m_pSelf = new CCallQualityTestTool();
	}

	return *m_pSelf;
}

unsigned int CCallQualityTestTool::GetRandomDuration() {
	srand((unsigned int)clock() ^ m_unSeed++);
	unsigned int rand = m_nBeginRange + std::rand() % (m_nEndRange - m_nBeginRange);
	//printf("CCallQualityTestTool::GetRandomDuration() : %d\n", rand);
	return rand;
}

unsigned int CCallQualityTestTool::GetRandomCallee(int nCaller) {
	srand((unsigned int)clock() ^ m_unSeed++);
	unsigned int rand = std::rand() % (m_nNumberOfBots);
	if (rand == nCaller) {
		rand = GetRandomCallee(nCaller);
	}
	//printf("CCallQualityTestTool::GetRandomCallee() : %d\n", rand);
	return rand;
}

void CCallQualityTestTool::SetParams(unsigned int nNumberOfBots, unsigned int nBeginRange, unsigned int nEndRange) {
	m_nNumberOfBots = nNumberOfBots;
	m_nBeginRange = nBeginRange;
	m_nEndRange = nEndRange;

	printf("m_nNumberOfBots = %d, m_nBeginRange = %d, m_nEndRange = %d \n", m_nNumberOfBots, m_nBeginRange, m_nEndRange);

	return;
}


int CCallQualityTestTool::Run() {
	unsigned int numBots;
	unsigned int begRange;
	unsigned int endRange;

	while(m_nNumberOfBots == 0) {
		printf ("Please enter number of bots. Creating more than 100 bots could affect system: \n");
		std::cin >> numBots;
		if ((numBots > 1) && (numBots < 1000)) {
			m_nNumberOfBots = numBots;
		}
		else {
			printf("Incorrect value. Please try again. \n");
		}
	}

	while(m_nBeginRange == 0) {
		printf ("Please enter lowest value of delay in milliseconds (>0): \n");
		std::cin >> begRange;
		if ((begRange > 0) && (begRange < 100000)) {
			m_nBeginRange = begRange;
		}
		else {
			printf("Incorrect value. Please try again.\n");
		}
	}

	while(m_nEndRange == 0) {
		printf ("Please enter highest value of delay in milliseconds (>0). Should be more than %d: \n", m_nBeginRange);
		std::cin >> endRange;
		if ((endRange > 0) && (endRange > m_nBeginRange) && (endRange < 1000000)) {
			m_nEndRange = endRange;
		}
		else {
			printf("Incorrect value. Please try again.\n");
		}
	}

	printf("m_nNumberOfBots = %d, m_nBeginRange = %d, m_nEndRange = %d \n", m_nNumberOfBots, m_nBeginRange, m_nEndRange);
	return 1;
}

bool CCallQualityTestTool::Initialize() {
	CCallingBot* pBot;
	char tmp;
	m_pMessagingBroker = &CMessagingBroker::Instance();
	m_pTelemetryStorage = &CTelemetryStorage::Instance();
	while(CCallingBot::m_nBotCount < m_nNumberOfBots) {
		pBot = new CCallingBot();
	}
	printf("Bots are created \n");
	//cin >> tmp;
	m_pMessagingBroker->Initialize();
	printf("MessagingBroker is initialized. \n");
	//cin >> tmp;
	StartBots();
	printf("MessagingBroker starting bots. \n");
	//cin >> tmp;
	return true;
}

bool CCallQualityTestTool::StartBots() {
	for (unsigned int i = 0; i < m_nNumberOfBots; i++) {
		CMessagingBroker::Instance().m_Bots[i]->Initialize();
	}
	return true;
}

bool CCallQualityTestTool::StopBots() {
	for (unsigned int i = 0; i < m_nNumberOfBots; i++) {
		CMessagingBroker::Instance().m_Bots[i]->Terminate();
		delete CMessagingBroker::Instance().m_Bots[i];
	}
	return true;
}

int CCallQualityTestTool::ShutDown() {
	printf("Shutting down the test tool.\n");

	m_pMessagingBroker->Terminate();
	StopBots();
	delete m_pMessagingBroker;
	delete m_pTelemetryStorage;
	return 1;
}


int main (int __argc, char** __argv)
{
	std::ofstream outputFile;
	eReturnCode rc = RC_UNKNOWN;
	unsigned int params[3];
	char* pszArg;
	char tmp;
	int i;

	CCallQualityTestTool* test;

	if (__argc != 4) {
		printf("Incorrect param count: %d instead of 4. \n", __argc);
		rc = RC_INCORRECT_PARAMS;
		return rc;
	}
	
	for (i = 0; i < __argc - 1; i++){
		pszArg = __argv[i + 1];
		params[i] = atoi(pszArg);
		if ((i == 1) && ((params[i] < 2) || (params[i] > 1000))) {     //Number of bots should be more than 2, less than 1000
			printf("Number of bots is too high. Exit.\n");
			rc = RC_INCORRECT_PARAMS;
		}
		if ((i == 2) && ((params[i] < 0) || (params[i] > 60000))) {     //Beginning of range (ms) should be positive, but less than 60 sec 
			printf("Beginning of range is incorrect. Exit.\n");
			rc = RC_INCORRECT_PARAMS;
		} 
		if ((i == 3) && ((params[i] < params[2]) || (params[i] > 60000))) {     //End of range (ms) should be more than beginning and less than 60 sec
			printf("Eng of range is incorrect. Exit. \n");
			rc = RC_INCORRECT_PARAMS;
		}
	}

	if (rc == RC_INCORRECT_PARAMS) {
		return rc;	
	}

	

	test = &CCallQualityTestTool::Instance();
	test->SetParams(params[0], params[1], params[2]);
	test->Initialize();
	printf("test.Initialize called \n");
	printf("Running...\n");
	printf("Press any key to stop \n");
	std::cin >> tmp;

	test->m_pTelemetryStorage->CalculateStatistics();	
	std::cin >> tmp;
	
	test->ShutDown();
	return 1;
}