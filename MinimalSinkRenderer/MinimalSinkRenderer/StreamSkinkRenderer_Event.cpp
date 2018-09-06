//----------------------------------------------------------------------------------------------
// StreamSkinkRenderer_Event.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HRESULT CStreamSkinkRenderer::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent){

	TRACE_STREAM((L"StreamRenderer::GetEvent"));

	HRESULT hr;

	IMFMediaEventQueue* pQueue = NULL;

	{
		AutoLock lock(m_CriticSection);

		LOG_HRESULT(hr = CheckShutdown());

		if(SUCCEEDED(hr)){
			pQueue = m_pEventQueue;
			pQueue->AddRef();
		}
	}

	if(SUCCEEDED(hr)){
		LOG_HRESULT(hr = pQueue->GetEvent(dwFlags, ppEvent));
	}

	SAFE_RELEASE(pQueue);

	return hr;
}

HRESULT CStreamSkinkRenderer::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState){

	TRACE_STREAM((L"StreamRenderer::BeginGetEvent"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	LOG_HRESULT(hr = m_pEventQueue->BeginGetEvent(pCallback, punkState));

	return hr;
}

HRESULT CStreamSkinkRenderer::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent){

	TRACE_STREAM((L"StreamRenderer::EndGetEvent"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	LOG_HRESULT(hr = m_pEventQueue->EndGetEvent(pResult, ppEvent));

	return hr;
}

HRESULT CStreamSkinkRenderer::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue){

	TRACE_STREAM((L"StreamRenderer::QueueEvent : %s", MFEventString(met)));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	LOG_HRESULT(hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue));

	return hr;
}