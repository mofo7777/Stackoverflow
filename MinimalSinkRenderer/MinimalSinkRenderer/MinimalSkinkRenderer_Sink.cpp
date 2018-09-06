//----------------------------------------------------------------------------------------------
// MinimalSkinkRenderer_Sink.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

HRESULT CMinimalSkinkRenderer::GetCharacteristics(DWORD* pdwCharacteristics){

	TRACE_SINK((L"SinkRenderer::GetCharacteristics"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pdwCharacteristics == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*pdwCharacteristics = MEDIASINK_FIXED_STREAMS;

	return hr;
}

HRESULT CMinimalSkinkRenderer::AddStreamSink(DWORD /*dwStreamSinkIdentifier*/, IMFMediaType* /*pMediaType*/, IMFStreamSink** /*ppStreamSink*/){

	TRACE_SINK((L"SinkRenderer::AddStreamSink"));
	return MF_E_STREAMSINKS_FIXED;
}

HRESULT CMinimalSkinkRenderer::RemoveStreamSink(DWORD /*dwStreamSinkIdentifier*/){

	TRACE_SINK((L"SinkRenderer::RemoveStreamSink"));
	return MF_E_STREAMSINKS_FIXED;
}

HRESULT CMinimalSkinkRenderer::GetStreamSinkCount(DWORD* pcStreamSinkCount){

	TRACE_SINK((L"SinkRenderer::GetStreamSinkCount"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pcStreamSinkCount == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*pcStreamSinkCount = 1;

	return hr;
}

HRESULT CMinimalSkinkRenderer::GetStreamSinkByIndex(DWORD dwIndex, IMFStreamSink** ppStreamSink){

	TRACE_SINK((L"SinkRenderer::GetStreamSinkByIndex"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppStreamSink == NULL ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (dwIndex != 0 ? MF_E_INVALIDINDEX : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*ppStreamSink = m_pStreamSkinkRenderer;
	(*ppStreamSink)->AddRef();

	return hr;
}

HRESULT CMinimalSkinkRenderer::GetStreamSinkById(DWORD dwStreamSinkIdentifier, IMFStreamSink** ppStreamSink){

	TRACE_SINK((L"SinkRenderer::GetStreamSinkById"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppStreamSink == NULL ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (dwStreamSinkIdentifier != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*ppStreamSink = m_pStreamSkinkRenderer;
	(*ppStreamSink)->AddRef();

	return hr;
}

HRESULT CMinimalSkinkRenderer::SetPresentationClock(IMFPresentationClock* pPresentationClock){

	TRACE_SINK((L"SinkRenderer::SetPresentationClock"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	if(m_pClock){
		IF_FAILED_RETURN(hr = m_pClock->RemoveClockStateSink(this));
	}

	if(pPresentationClock){
		IF_FAILED_RETURN(hr = pPresentationClock->AddClockStateSink(this));
	}

	SAFE_RELEASE(m_pClock);

	if(pPresentationClock){

		m_pClock = pPresentationClock;
		m_pClock->AddRef();
	}

	return hr;
}

HRESULT CMinimalSkinkRenderer::GetPresentationClock(IMFPresentationClock** ppPresentationClock){

	TRACE_SINK((L"SinkRenderer::GetPresentationClock"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppPresentationClock == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	if(m_pClock == NULL){

		hr = MF_E_NO_CLOCK;
	}
	else{

		*ppPresentationClock = m_pClock;
		(*ppPresentationClock)->AddRef();
	}

	return hr;
}

HRESULT CMinimalSkinkRenderer::Shutdown(){

	TRACE_SINK((L"SinkRenderer::Shutdown"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	if(m_pStreamSkinkRenderer){
		IF_FAILED_RETURN(hr = m_pStreamSkinkRenderer->Shutdown());
		SAFE_RELEASE(m_pStreamSkinkRenderer);
	}

	SAFE_RELEASE(m_pClock);

	m_bShutdown = TRUE;
	m_dwCurrentFrame = 0;

	return hr;
}