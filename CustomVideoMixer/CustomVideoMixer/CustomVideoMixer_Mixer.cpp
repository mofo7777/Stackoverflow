//----------------------------------------------------------------------------------------------
// CustomVideoMixer_Transform.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HRESULT CCustomVideoMixer::GetStreamOutputRect(DWORD, MFVideoNormalizedRect*) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetStreamOutputRect"));

	return S_OK;
}

HRESULT CCustomVideoMixer::GetStreamZOrder(DWORD, DWORD*) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetStreamZOrder"));

	return S_OK;
}

HRESULT CCustomVideoMixer::SetStreamOutputRect(DWORD, const MFVideoNormalizedRect*) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetStreamOutputRect"));

	return S_OK;
}

HRESULT CCustomVideoMixer::SetStreamZOrder(DWORD, DWORD) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetStreamZOrder"));

	return S_OK;
}

HRESULT CCustomVideoMixer::GetAvailableVideoProcessorModes(UINT*, GUID**) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetAvailableVideoProcessorModes"));

	return S_OK;
}

HRESULT CCustomVideoMixer::GetBackgroundColor(COLORREF*) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetBackgroundColor"));

	return S_OK;
}

HRESULT CCustomVideoMixer::GetFilteringRange(DWORD, DXVA2_ValueRange*) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetFilteringRange"));

	return S_OK;
}

HRESULT CCustomVideoMixer::GetFilteringValue(DWORD, DXVA2_Fixed32*) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetFilteringValue"));

	return S_OK;
}

HRESULT CCustomVideoMixer::GetProcAmpRange(DWORD, DXVA2_ValueRange*) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetProcAmpRange"));

	return S_OK;
}

HRESULT CCustomVideoMixer::GetProcAmpValues(DWORD, DXVA2_ProcAmpValues*) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetProcAmpValues"));

	return S_OK;
}

HRESULT CCustomVideoMixer::GetVideoProcessorCaps(LPGUID, DXVA2_VideoProcessorCaps*) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetVideoProcessorCaps"));

	return S_OK;
}

HRESULT CCustomVideoMixer::GetVideoProcessorMode(LPGUID) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetVideoProcessorMode"));

	return S_OK;
}

HRESULT CCustomVideoMixer::SetBackgroundColor(COLORREF) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetBackgroundColor"));

	return S_OK;
}

HRESULT CCustomVideoMixer::SetFilteringValue(DWORD, DXVA2_Fixed32*) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetFilteringValue"));

	return S_OK;
}

HRESULT CCustomVideoMixer::SetProcAmpValues(DWORD, DXVA2_ProcAmpValues*) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetProcAmpValues"));

	return S_OK;
}

HRESULT CCustomVideoMixer::SetVideoProcessorMode(LPGUID) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetVideoProcessorMode"));

	return S_OK;
}