//----------------------------------------------------------------------------------------------
// StreamSkinkRenderer_Type.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HRESULT CStreamSkinkRenderer::IsMediaTypeSupported(IMFMediaType* pMediaType, IMFMediaType** ppMediaType){

	TRACE_STREAM((L"StreamRenderer::IsMediaTypeSupported"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pMediaType == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	GUID MajorType = GUID_NULL;
	GUID SubType = GUID_NULL;

	//LogMediaType(pMediaType);

	IF_FAILED_RETURN(hr = pMediaType->GetGUID(MF_MT_MAJOR_TYPE, &MajorType));
	IF_FAILED_RETURN(hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &SubType));

	IF_FAILED_RETURN(hr = (MajorType != MFMediaType_Video ? MF_E_INVALIDTYPE : S_OK));
	IF_FAILED_RETURN(hr = (SubType != MFVideoFormat_RGB32 ? MF_E_INVALIDTYPE : S_OK));

	if(ppMediaType){
		*ppMediaType = NULL;
	}

	return hr;
}

HRESULT CStreamSkinkRenderer::GetMediaTypeCount(DWORD* pdwTypeCount){

	TRACE_STREAM((L"StreamRenderer::GetMediaTypeCount"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pdwTypeCount == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*pdwTypeCount = 1;

	return hr;
}

HRESULT CStreamSkinkRenderer::GetMediaTypeByIndex(DWORD dwIndex, IMFMediaType** ppType){

	TRACE_STREAM((L"StreamRenderer::GetMediaTypeByIndex"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppType == NULL ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (dwIndex != 0 ? MF_E_NO_MORE_TYPES : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IMFMediaType* pType = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaType(&pType));
		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32));

		*ppType = pType;
		(*ppType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pType);

	return hr;
}

HRESULT CStreamSkinkRenderer::SetCurrentMediaType(IMFMediaType* pMediaType){

	TRACE_STREAM((L"StreamRenderer::SetCurrentMediaType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pMediaType == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = IsMediaTypeSupported(pMediaType, NULL));

	// Todo check valid size...

	SAFE_RELEASE(m_pMediaType);
	m_pMediaType = pMediaType;
	m_pMediaType->AddRef();

	if(m_State != StreamPaused)
		m_State = StreamReady;

	return hr;
}

HRESULT CStreamSkinkRenderer::GetCurrentMediaType(IMFMediaType** ppMediaType){

	TRACE_STREAM((L"StreamRenderer::GetCurrentMediaType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppMediaType == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_pMediaType == NULL ? MF_E_NOT_INITIALIZED : S_OK));

	*ppMediaType = m_pMediaType;
	(*ppMediaType)->AddRef();

	return hr;
}

HRESULT CStreamSkinkRenderer::GetMajorType(GUID* pguidMajorType){

	TRACE_STREAM((L"StreamRenderer::GetMajorType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pguidMajorType == NULL ? E_INVALIDARG : S_OK));

	*pguidMajorType = MFMediaType_Video;

	return hr;
}