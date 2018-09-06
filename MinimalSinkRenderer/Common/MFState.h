//----------------------------------------------------------------------------------------------
// MFState.h
//----------------------------------------------------------------------------------------------
#ifndef MFSTATE_H
#define MFSTATE_H

enum SessionState{

	SessionClosed = 0,
	SessionReady,
	SessionOpenPending,
	SessionStarted,
	SessionPaused,
	SessionStopped,
	SessionClosing,
	SessionAbort
};

enum StreamState{

	StreamTypeNotSet = 0,
	StreamReady,
	StreamStarted,  
	StreamStopped,
	StreamPaused,
	StreamFinalized,

	StreamCount = StreamFinalized + 1
};

enum StreamRequest{

	RequestSetMediaType = 0,
	RequestStart,
	RequestRestart,
	RequestPause,
	RequestStop,
	RequestProcessSample,
	RequestPlaceMarker,
	RequestFinalize,

	RequestCount = RequestFinalize + 1
};

enum SourceState{

	SourceInvalid,
	SourceOpening,
	SourceStopped,
	SourcePaused,
	SourceStarted,
	SourceShutdown
};

enum RendererState{

	RendererStarted,
	RendererStopped,
	RendererPaused,
	RendererShutdown
};

#if (_DEBUG && MF_USE_LOGGING)

inline LPCWSTR ClockStateString(MFCLOCK_STATE State){

	switch(State){

		RETURN_STRING(MFCLOCK_STATE_INVALID);
		RETURN_STRING(MFCLOCK_STATE_RUNNING);
		RETURN_STRING(MFCLOCK_STATE_STOPPED);
		RETURN_STRING(MFCLOCK_STATE_PAUSED);

	default:
		return L"Unknown Clock State";
	}
}

inline LPCWSTR SessionStateString(SessionState State){

	switch(State){

		RETURN_STRING(SessionClosed);
		RETURN_STRING(SessionReady);
		RETURN_STRING(SessionOpenPending);
		RETURN_STRING(SessionStarted);
		RETURN_STRING(SessionPaused);
		RETURN_STRING(SessionStopped);
		RETURN_STRING(SessionClosing);

	default:
		return L"Unknown Session State";
	}
}

inline LPCWSTR StreamStateString(StreamState State){

	switch(State){

		RETURN_STRING(StreamTypeNotSet);
		RETURN_STRING(StreamReady);
		RETURN_STRING(StreamStarted);
		RETURN_STRING(StreamStopped);
		RETURN_STRING(StreamPaused);
		RETURN_STRING(StreamFinalized);
		RETURN_STRING(StreamCount);

	default:
		return L"Unknown Stream State";
	}
}

inline LPCWSTR StreamRequestStateString(StreamRequest State){

	switch(State){

		RETURN_STRING(RequestSetMediaType);
		RETURN_STRING(RequestStart);
		RETURN_STRING(RequestRestart);
		RETURN_STRING(RequestPause);
		RETURN_STRING(RequestStop);
		RETURN_STRING(RequestProcessSample);
		RETURN_STRING(RequestPlaceMarker);
		RETURN_STRING(RequestFinalize);
		RETURN_STRING(RequestCount);

	default:
		return L"Unknown StreamRequest State";
	}
}

inline LPCWSTR SourceStateString(SourceState State){

	switch(State){

		RETURN_STRING(SourceInvalid);
		RETURN_STRING(SourceOpening);
		RETURN_STRING(SourceStopped);
		RETURN_STRING(SourcePaused);
		RETURN_STRING(SourceStarted);
		RETURN_STRING(SourceShutdown);

	default:
		return L"Unknown Source State";
	}
}

inline LPCWSTR RendererStateString(RendererState State){

	switch(State){

		RETURN_STRING(RendererStarted);
		RETURN_STRING(RendererStopped);
		RETURN_STRING(RendererPaused);
		RETURN_STRING(RendererShutdown);

	default:
		return L"Unknown Renderer State";
	}
}

#else

#define ClockStateString(x) L""
#define SessionStateString(x) L""
#define StreamStateString(x) L""
#define StreamRequestStateString(x) L""
#define SourceStateString(x) L""
#define RendererStateString(x) L""

#endif

#endif