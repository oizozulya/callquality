/**
* Implements class for TelemetryStorage
*/

#include <algorithm>
#include "TelemetryStorage.h"

CTelemetryStorage* CTelemetryStorage::m_pSelf = NULL;
std::mutex CTelemetryStorage::m_Lock;

CTelemetryStorage::CTelemetryStorage()
:m_nAnswCallCount(0),
m_nTotalCallCount(0){
	std::vector<unsigned int> m_Stats;
}

CTelemetryStorage::~CTelemetryStorage() {
	m_Stats.clear();
}

CTelemetryStorage& CTelemetryStorage::Instance() {
	if (m_pSelf == NULL)	{
		m_pSelf = new CTelemetryStorage();
	}

	return *m_pSelf;
}

void CTelemetryStorage::Cleanup() {
	m_nAnswCallCount = 0;
	m_nTotalCallCount = 0;
	m_Stats.clear();
	return;
}


bool CTelemetryStorage::PutStatistic(unsigned int nTimeToCall, bool bCallAnswered) {

	m_Lock.lock();
	m_Stats.push_back(nTimeToCall);
	m_nTotalCallCount++;
	if (bCallAnswered) {
		m_nAnswCallCount++;
	}
	m_Lock.unlock();
	return true;
}

void CTelemetryStorage::CalculateStatistics() {
	for (std::vector<unsigned int>::iterator it = m_Stats.begin(); it != m_Stats.end(); it++) {
		std::cout << *it << ' ';
	}
	std::cout << '\n';
	m_Lock.lock();
	double nAnsCallPerc = 	CalculateAnsCallsPercentage();
	printf("Percentage of answered calls = %f \n", nAnsCallPerc);
	unsigned int n50Perc = CalculatePercentile(50);
	printf("50th percentile = %d \n", n50Perc);
	unsigned int n90Perc = CalculatePercentile(90);
	printf("90th percentile = %d \n", n90Perc);
	m_Lock.unlock();
	return;
}

int CTelemetryStorage::CalculateAnsCallsPercentage() {
	printf("m_nAnswCallCount = %d, m_nTotalCallCount = %d \n", m_nAnswCallCount, m_nTotalCallCount);
	if (m_nTotalCallCount !=0) {
		double tmpRes = (double)m_nAnswCallCount / m_nTotalCallCount;
		res = (int)(100 * tmpRes);
	}
	else {
		res = 0;
	}
	return res;
}

int CTelemetryStorage::CalculatePercentile(int  nPercentileLevel) {
	int nIndex = 0;
	printf("CTelemetryStorage::CalculatePercentile(%d).\n", nPercentileLevel);
	if ((nPercentileLevel <= 0) || (nPercentileLevel >= 100)) {     //percentile level should be from 1 to 99
		printf("Incorrect value of percentile level. Aborting. \n");
		return -1;
	}

	if (m_Stats.size() > 0) {
		nIndex = (m_Stats.size() - 1) * nPercentileLevel / 100;
		printf("Index = %d \n", nIndex);
	}
	else {
		printf("m_Stats is empty, no data to calculate percentile. \n");
		return -1;
	}
	std::nth_element(m_Stats.begin(), m_Stats.begin() + nIndex, m_Stats.end());

	printf(" %d th Percentile is %d \n", nPercentileLevel, m_Stats[nIndex]);

	return m_Stats[nIndex];
}


