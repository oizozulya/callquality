/**
*	Defines Telemetry Storage and algorithms for calculating statistics.
*/


#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <fstream>
#include "MessagingBroker.h"

using namespace std;

class CTelemetryStorage {

private: 
	static std::mutex m_Lock;			//mutes for protection from multiple access to TelemetryStorage
	static CTelemetryStorage* m_pSelf;	//part of singleton implementation

public:
	virtual ~CTelemetryStorage();

	static CTelemetryStorage& Instance();	//part of singleton implementation

	bool PutStatistic(unsigned int nTimeToCall, bool bCallAnswered);	//Message for adding statistic to TelemetryStorage, should be called by bot

	void CalculateStatistics(std::ofstream& outputFile);		//calculates required statistic, writes it to file
	int CalculatePercentile(int nPercentileLevel);				//calculates percentile on provided level
	int CalculateAnsCallsPercentage();							//calculates percentage of answered calls
	void Cleanup();												//cleans up all statistics
	
protected: 
	CTelemetryStorage();

protected: 
	std::vector<unsigned int> m_Stats;							//storage of TimeToRing values
	unsigned int m_nTotalCallCount;								// count of calls - total
	unsigned int m_nAnswCallCount;								//count of cals - answered
};