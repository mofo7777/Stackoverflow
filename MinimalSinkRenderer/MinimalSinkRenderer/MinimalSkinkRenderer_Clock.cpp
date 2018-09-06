//----------------------------------------------------------------------------------------------
// MinimalSkinkRenderer_Clock.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

HRESULT CMinimalSkinkRenderer::OnClockStart(MFTIME /*hnsSystemTime*/, LONGLONG llClockStartOffset){

	TRACE_SINK((L"SinkRenderer::OnClockStart"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_pStreamSkinkRenderer->Start(llClockStartOffset));

	m_dwCurrentFrame = 0;

	return hr;
}

HRESULT CMinimalSkinkRenderer::OnClockStop(MFTIME /*hnsSystemTime*/){

	TRACE_SINK((L"SinkRenderer::OnClockStop"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_pStreamSkinkRenderer->Stop());

	return hr;
}

HRESULT CMinimalSkinkRenderer::OnClockPause(MFTIME /*hnsSystemTime*/){

	TRACE_SINK((L"SinkRenderer::OnClockPause"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_pStreamSkinkRenderer->Pause());

	return hr;
}

HRESULT CMinimalSkinkRenderer::OnClockRestart(MFTIME /*hnsSystemTime*/){

	TRACE_SINK((L"SinkRenderer::OnClockRestart"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_pStreamSkinkRenderer->Restart());

	return hr;
}

HRESULT CMinimalSkinkRenderer::OnClockSetRate(MFTIME /*hnsSystemTime*/, float /*flRate*/){

	TRACE_SINK((L"SinkRenderer::OnClockSetRate"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	return hr;
}