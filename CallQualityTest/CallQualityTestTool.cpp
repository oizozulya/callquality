/**
	Implements main calss for test tool process
*/

#include <iostream>
#include <fstream>
#include <string>
#include "CallQualityTestTool.h"
#include "CallingBot.h"
#include "MessagingBroker.h"

unsigned int CCallQualityTestTool::m_unSeed = time(NULL);
std::mutex seedMutex;
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
	seedMutex.lock();
	m_unSeed+=5;
	srand((unsigned int)clock() + m_unSeed);
	unsigned int rand = m_nBeginRange + std::rand() % (m_nEndRange - m_nBeginRange);
	seedMutex.unlock();
	//printf("CCallQualityTestTool::GetRandomDuration() : %d\n", rand);
	return rand;
}

unsigned int CCallQualityTestTool::GetRandomCallee(int nCaller) {
	seedMutex.lock();
	srand(nCaller + (unsigned int)clock() + m_unSeed++);
	unsigned int rand = std::rand() % (m_nNumberOfBots);
	seedMutex.unlock();
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


bool CCallQualityTestTool::Initialize() {
	CCallingBot* pBot;
	m_pMessagingBroker = &CMessagingBroker::Instance();
	m_pTelemetryStorage = &CTelemetryStorage::Instance();
	while(CCallingBot::m_nBotCount < m_nNumberOfBots) {
		pBot = new CCallingBot();
	}
	printf("Bots are created \n");
	m_pMessagingBroker->Initialize();
	printf("MessagingBroker is initialized. \n");
	StartBots();
	printf("MessagingBroker starting bots. \n");
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
	}
	return true;
}

bool CCallQualityTestTool::Terminate() {
	for (unsigned int i = 0; i < m_nNumberOfBots; i++) {
		delete CMessagingBroker::Instance().m_Bots[i];
	}
	delete m_pMessagingBroker;
	m_pTelemetryStorage->Cleanup();
	delete m_pTelemetryStorage;

	return true;
}

bool CCallQualityTestTool::ShutDown() {
	printf("Shutting down the test tool. Stop threads.\n");

	m_pMessagingBroker->Terminate();
	StopBots();

	return true;
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

	test->ShutDown();

	try	{
		outputFile.open("TestResults.txt");
	}
	catch (...)	{
		return 1;
	}
	test->m_pTelemetryStorage->CalculateStatistics(outputFile);	
	try	{
		outputFile.close();
	}
	catch (...)	{
		return 1;
	}
	std::cin >> tmp;
	
	test->Terminate();

	return 1;
}