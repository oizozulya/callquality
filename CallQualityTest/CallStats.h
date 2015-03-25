/**
* Defines class for collecting call statistic information
*/

#include <time.h>

class CCallStats {
public:
	CCallStats();
	~CCallStats();

	void SetCallStarted();		//Sets m_nStartTime
	void SetCallAnswered();		//Sets m_bAnswered to true
	void SetCallDeclined();		//Modifies statistics for declined call
	void SetCallRinging();		// Sets m_nTimeToRing

	void SetFarEndId(int nId);	//Sets m_nFarEndId
	bool IsAnswered();			//Returns info if call is answered call or not
	int GetFarEndId();			//Returns far-end Id for current call
	int GetTimeToRing();		//Returns value of m_nTimeToRing
	void Clean();				//Clean uo statistics for the next call

protected:
	bool m_bAnswered;			// indicates if call was answered
	int m_nStartTime;			// time stamp of call start for TimeToRing calculation
	int m_nTimeToRing;			// TimeToRing that provided as a part of statistics
	int m_nFarEndId;			// Id of far-end ID for current call
};