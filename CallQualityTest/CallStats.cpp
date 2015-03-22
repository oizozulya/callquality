/**
* Implements class for collecting call statistic information
*/

#include <iostream>
#include "CallStats.h"

CCallStats::CCallStats():m_bAnswered(false),
m_nTimeToRing(0),
m_nFarEndId(-1),
m_nStartTime() {
}


CCallStats::~CCallStats() {
}

void CCallStats::SetCallDeclined() {
	m_nTimeToRing = (clock() - m_nStartTime);
	m_bAnswered = false;
}

void CCallStats::SetCallRinging() {
	m_nTimeToRing = (clock() - m_nStartTime);
}

void CCallStats::SetCallAnswered() {
	m_bAnswered = true;
}

void CCallStats::SetCallStarted() {
	m_nStartTime = clock();
}

void CCallStats::SetFarEndId(int nId) {
	m_nFarEndId = nId;
}

bool CCallStats::IsAnswered() {
	return m_bAnswered;
}

int CCallStats::GetFarEndId() {
	return m_nFarEndId;
}

int CCallStats::GetTimeToRing() {
	return m_nTimeToRing;
}


void CCallStats::Clean() {
	m_bAnswered = false;
	m_nStartTime = 0;
	m_nTimeToRing = 0;
	m_nFarEndId = -1;
	return;
}