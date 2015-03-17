/**
* Defines class for collecting call statistic information
*/

#include <time.h>

class CCallStats {
public:
	CCallStats();
	CCallStats(int nFarEndId);
	~CCallStats();

	bool CalcTimeToCall();
	void CallStarted();
	void SetAnswered();

	void SetFarEndId(int nId);
	bool IsAnswered();
	bool IsOutgoing();
	int GetFarEndId();
	int GetTimeToCall();
	void Clean();

protected:
	bool m_bAnswered;
	bool m_bOutgoing;
	int m_nStartTime;
	int m_nTimeToCall;
	int m_nFarEndId;
};