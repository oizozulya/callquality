/**
* Defines class for collecting call statistic information
*/

#include <time.h>

class CCallStats {
public:
	CCallStats();
	~CCallStats();

	void SetCallStarted();
	void SetCallAnswered();
	void SetCallDeclined();
	void SetCallRinging();

	void SetFarEndId(int nId);
	bool IsAnswered();
	int GetFarEndId();
	int GetTimeToRing();
	void Clean();

protected:
	bool m_bAnswered;
	int m_nStartTime;
	int m_nTimeToRing;
	int m_nFarEndId;
};