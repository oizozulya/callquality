/**
*	Defines Telemetry Storage and algorithms for calculating statistics.
*/


#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include "MessagingBroker.h"

using namespace std;

class CTelemetryStorage {

private: 
	static std::mutex m_Lock;	//TBD: static?
	static CTelemetryStorage* m_pSelf;

public:
	virtual ~CTelemetryStorage();
	bool Initialize();
	bool Terminate();

	static CTelemetryStorage& Instance();

	bool PutStatistic(unsigned int nTimeToCall, bool bCallAnswered);

	void CalculateStatistics();
	unsigned int CalculatePercentile(int nPercentileLevel);
	int CalculateAnsCallsPercentage();
	
protected: 
	CTelemetryStorage();

protected: 
	std::vector<unsigned int> m_Stats;
	unsigned int m_nTotalCallCount;
	unsigned int m_nAnswCallCount;
};