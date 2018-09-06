//----------------------------------------------------------------------------------------------
// MFTime.h
//----------------------------------------------------------------------------------------------
#ifndef MFTIME_H
#define MFTIME_H

// One second in hns
const MFTIME ONE_SECOND = 10000000;

// One msec in hns
const LONG ONE_MSEC = 1000;

inline LONG MFTimeToMilliSec(const LONGLONG& time){

		return (LONG)(time / (ONE_SECOND / ONE_MSEC));
}

inline LONG MFTimeToSec(const LONGLONG& time){

		return (LONG)(time / ONE_SECOND);
}

#if (_DEBUG && MF_USE_LOGGING)

inline void MFTimeString(const MFTIME& Duration){

		MFTIME DurationInMilliSec = 0;
		MFTIME DurationInSec = 0;
		MFTIME Hours = 0;
		MFTIME Minutes = 0;
		MFTIME Seconds = 0;

		DurationInSec = MFTimeToSec(Duration);
		DurationInMilliSec = MFTimeToMilliSec(Duration);

		if(DurationInSec > 60){

				Minutes = DurationInSec / 60;

				if(Minutes > 60){
						
						Hours = Minutes / 60;
						Minutes = Minutes % 60;
				}
				
				Seconds = (DurationInSec % 60);
		}
		else{
				
				Seconds = DurationInSec;
		}

		if(Seconds){
				DurationInMilliSec = DurationInMilliSec % (Seconds * ONE_MSEC);
		}

		TRACE((L"%02dh:%02dmn:%02ds:%03dms\n", (int)Hours, (int)Minutes, (int)Seconds, DurationInMilliSec));
}

#else
  #define MFTimeString(x)
#endif

#endif