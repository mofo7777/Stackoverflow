//----------------------------------------------------------------------------------------------
// StreamSkinkRenderer_Sink.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HRESULT CStreamSkinkRenderer::GetMediaSink(IMFMediaSink** ppMediaSink){

	TRACE_STREAM((L"StreamRenderer::GetMediaSink"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppMediaSink == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*ppMediaSink = m_pMinimalSkinkRenderer;
	(*ppMediaSink)->AddRef();

	return hr;
}

HRESULT CStreamSkinkRenderer::GetIdentifier(DWORD* pdwIdentifier){

	TRACE_STREAM((L"StreamRenderer::GetIdentifier"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pdwIdentifier == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*pdwIdentifier = 0;

	return hr;
}

HRESULT CStreamSkinkRenderer::GetMediaTypeHandler(IMFMediaTypeHandler** ppHandler){

	TRACE_STREAM((L"StreamRenderer::GetMediaTypeHandler"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppHandler == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = this->QueryInterface(IID_IMFMediaTypeHandler, reinterpret_cast<void**>(ppHandler)));

	return hr;
}

HRESULT CStreamSkinkRenderer::ProcessSample(IMFSample* pSample){

	TRACE_STREAM((L"StreamRenderer::ProcessSample"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pSample == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_pMinimalSkinkRenderer->ProcessSample(pSample));

	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL));

	return hr;
}

HRESULT CStreamSkinkRenderer::PlaceMarker(MFSTREAMSINK_MARKER_TYPE /*eMarkerType*/, const PROPVARIANT* /*pvarMarkerValue*/, const PROPVARIANT* /*pvarContextValue*/){

	TRACE_STREAM((L"StreamRenderer::PlaceMarker"));

	// Todo check marker.
	return E_NOTIMPL;
}

HRESULT CStreamSkinkRenderer::Flush(){

	TRACE_STREAM((L"StreamRenderer::Flush"));

	// if CStreamSkinkRenderer::PlaceMarker is implemented, see :
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms701626(v=vs.85).aspx

	return S_OK;
}