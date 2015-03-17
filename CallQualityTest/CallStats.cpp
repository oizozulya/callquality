/**
* Implements class for collecting call statistic information
*/

#include <iostream>
#include "CallStats.h"

CCallStats::CCallStats() {
}

CCallStats::CCallStats(int nFarEndId)
:m_bAnswered(false),
m_nTimeToCall(0),
m_nFarEndId(-1),
m_nStartTime()
{
}

CCallStats::~CCallStats() {
}

bool CCallStats::CalcTimeToCall() {
	if (m_nStartTime == 0) {
		printf("CCallStats::CalcTimeToCall() Invalid start time. \n");
		return false;
	}
	else {
		m_nTimeToCall = (clock() - m_nStartTime);
		return true;
	}
}

void CCallStats::SetAnswered() {
	m_bAnswered = true;
	return;
}

void CCallStats::CallStarted() {
	m_nStartTime = clock();
	return;
}

void CCallStats::SetFarEndId(int nId) {
	m_nFarEndId = nId;
}

bool CCallStats::IsAnswered() {
	return m_bAnswered;
}

bool CCallStats::IsOutgoing() {
	return m_bOutgoing;
}

int CCallStats::GetFarEndId() {
	return m_nFarEndId;
}

int CCallStats::GetTimeToCall() {
	return m_nTimeToCall;
}


void CCallStats::Clean() {
	m_bAnswered = false;
	m_bOutgoing = false;
	m_nStartTime = 0;
	m_nTimeToCall = 0;
	m_nFarEndId = 0;
	return;
}