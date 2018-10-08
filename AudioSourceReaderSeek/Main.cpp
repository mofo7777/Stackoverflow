//----------------------------------------------------------------------------------------------
// Main.cpp
//----------------------------------------------------------------------------------------------
#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "Propsys")

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
#include <WinSDKVer.h>
#include <new>
#include <windows.h>
#include <strsafe.h>
#include <propvarutil.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <mfreadwrite.h>

template <class T> inline void SAFE_RELEASE(T*& p){

	if(p){
		p->Release();
		p = NULL;
	}
}

void Trace(const WCHAR* sFormatString, ...){

	HRESULT hr = S_OK;
	va_list va;

	const DWORD TRACE_STRING_LEN = 512;

	WCHAR message[TRACE_STRING_LEN];

	va_start(va, sFormatString);
	hr = StringCchVPrintf(message, TRACE_STRING_LEN, sFormatString, va);
	va_end(va);

	if(SUCCEEDED(hr)){

		OutputDebugString(message);
	}
}

// One msec in ns
const LONG ONE_MSEC = 10000;
// One second in ns
const MFTIME ONE_SECOND = 10000000;
// One minute in ns
const MFTIME ONE_MINUTE = 600000000;
// One hour in ns
const MFTIME ONE_HOUR = 36000000000;
// One day in ns
const MFTIME ONE_DAY = 864000000000;

// Audio wave reference
// MF_MT_AUDIO_SAMPLES_PER_SECOND   48000
// MF_MT_AUDIO_AVG_BYTES_PER_SECOND 192000
// MF_MT_AUDIO_BITS_PER_SAMPLE      16
// MF_MT_AUDIO_NUM_CHANNELS         2
// MF_MT_AUDIO_BLOCK_ALIGNMENT      4

// Change the URL
#define AUDIO_FILE L"Audio-48000-192000-16-2.wav"
//#define AUDIO_FILE L"Audio-48000-192000-16-2.mp3"
//#define AUDIO_FILE L"Audio-48000-192000-16-2.m4a"

HRESULT ProcessAudioSeek();
HRESULT CheckReaderSeekPosition(IMFSourceReader*, const LONGLONG&);
HRESULT SetSeekPosition(IMFSourceReader*, const LONGLONG&);
HRESULT GetSourceFlags(IMFSourceReader*, ULONG*);
void MFTraceTimeString(const MFTIME&);

void main(){

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	if(SUCCEEDED(hr)){

		hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);

		if(SUCCEEDED(hr)){

			hr = ProcessAudioSeek();

			hr = MFShutdown();
		}

		CoUninitialize();
	}
}

HRESULT ProcessAudioSeek(){

	IMFSourceReader* pReader = NULL;
	IMFMediaType* pType = NULL;
	DWORD dwMediaTypeIndex = 0;
	ULONG ulFlags = 0;

	HRESULT hr = MFCreateSourceReaderFromURL(AUDIO_FILE, NULL, &pReader);

	// Check native media type
	if(SUCCEEDED(hr))
		hr = pReader->GetNativeMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, dwMediaTypeIndex, &pType);

	SAFE_RELEASE(pType);

	// Check current media type
	if(SUCCEEDED(hr))
		hr = pReader->GetCurrentMediaType(dwMediaTypeIndex, &pType);

	SAFE_RELEASE(pType);

	// Be sure source can seek
	if(SUCCEEDED(hr))
		hr = GetSourceFlags(pReader, &ulFlags);

	if((ulFlags & MFMEDIASOURCE_CAN_SEEK) == 0)
		hr = E_FAIL;

	if(SUCCEEDED(hr)){

		// You can change values as needed
		const LONGLONG llSeekPositioTab[] = {

			0,
			ONE_SECOND,
			ONE_SECOND + (500 * ONE_MSEC),
			2 * ONE_SECOND,
			(2 * ONE_SECOND) + (500 * ONE_MSEC),
			3 * ONE_SECOND,
			4 * ONE_SECOND,
			5 * ONE_SECOND,
			30 * ONE_SECOND,
			(30 * ONE_SECOND) + (500 * ONE_MSEC),
			ONE_MINUTE,
			ONE_MINUTE + (500 * ONE_MSEC),
			ONE_MINUTE + ONE_SECOND,
			ONE_MINUTE + (2 * ONE_SECOND),
			ONE_MINUTE + (3 * ONE_SECOND),
			ONE_MINUTE + (4 * ONE_SECOND),
			ONE_MINUTE + (5 * ONE_SECOND),
			ONE_MINUTE + (30 * ONE_SECOND),
			2 * ONE_MINUTE
		};

		int iSizeTab = sizeof(llSeekPositioTab) / sizeof(LONGLONG);

		for(int i = 0; i < iSizeTab; i++)
			CheckReaderSeekPosition(pReader, llSeekPositioTab[i]);
	}

	SAFE_RELEASE(pReader);

	return hr;
}

HRESULT CheckReaderSeekPosition(IMFSourceReader* pReader, const LONGLONG& llSeekPosition){

	DWORD streamIndex = 0;
	DWORD flags = 0;
	LONGLONG llTimeStamp = 0;
	IMFSample* pSample = NULL;

	Trace(L"\r\n\r\nSeek to        :\t");
	MFTraceTimeString(llSeekPosition);

	HRESULT hr = pReader->Flush((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM);

	if(SUCCEEDED(hr))
		hr = SetSeekPosition(pReader, llSeekPosition);

	if(SUCCEEDED(hr))
		hr = pReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &streamIndex, &flags, &llTimeStamp, &pSample);

	if(SUCCEEDED(hr))
		hr = (flags == 0 ? S_OK : E_FAIL);

	if(SUCCEEDED(hr)){

		Trace(L"\r\nReader seek to :\t");
		MFTraceTimeString(llTimeStamp);
	}
	else{

		Trace(L"\r\nReader Failed");
	}

	SAFE_RELEASE(pSample);

	return hr;
}

HRESULT SetSeekPosition(IMFSourceReader* pReader, const LONGLONG& llPosition){

	PROPVARIANT var;
	HRESULT hr = InitPropVariantFromInt64(llPosition, &var);

	if(SUCCEEDED(hr)){

		hr = pReader->SetCurrentPosition(GUID_NULL, var);
		PropVariantClear(&var);
	}

	return hr;
}

HRESULT GetSourceFlags(IMFSourceReader* pReader, ULONG* pulFlags){

	ULONG ulFlags = 0;
	PROPVARIANT var;
	PropVariantInit(&var);

	HRESULT hr = pReader->GetPresentationAttribute((DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_SOURCE_READER_MEDIASOURCE_CHARACTERISTICS, &var);

	if(SUCCEEDED(hr)){
		hr = PropVariantToUInt32(var, &ulFlags);
	}

	if(SUCCEEDED(hr)){
		*pulFlags = ulFlags;
	}

	PropVariantClear(&var);

	return hr;
}

void MFTraceTimeString(const MFTIME& Duration){

	MFTIME Hours = 0;
	MFTIME Minutes = 0;
	MFTIME Seconds = 0;
	MFTIME MilliSeconds = 0;

	if(Duration < ONE_SECOND){

		MilliSeconds = Duration / ONE_MSEC;
	}
	else if(Duration < ONE_MINUTE){

		Seconds = Duration / ONE_SECOND;
		MilliSeconds = (Duration - (Seconds * ONE_SECOND)) / ONE_MSEC;
	}
	else if(Duration < ONE_HOUR){

		Minutes = Duration / ONE_MINUTE;
		LONGLONG llMinutes = Minutes * ONE_MINUTE;
		Seconds = (Duration - llMinutes) / ONE_SECOND;
		MilliSeconds = (Duration - (llMinutes + (Seconds * ONE_SECOND))) / ONE_MSEC;
	}
	else if(Duration < ONE_DAY){

		Hours = Duration / ONE_HOUR;
		LONGLONG llHours = Hours * ONE_HOUR;
		Minutes = (Duration - llHours) / ONE_MINUTE;
		LONGLONG llMinutes = Minutes * ONE_MINUTE;
		Seconds = (Duration - (llHours + llMinutes)) / ONE_SECOND;
		MilliSeconds = (Duration - (llHours + llMinutes + (Seconds * ONE_SECOND))) / ONE_MSEC;
	}
	else{

		Trace(L"todo : not implemented (more than one day)");
		return;
	}

	Trace(L"%02dh:%02dmn:%02ds:%03dms", (int)Hours, (int)Minutes, (int)Seconds, MilliSeconds);
}