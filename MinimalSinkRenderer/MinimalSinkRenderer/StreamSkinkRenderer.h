//----------------------------------------------------------------------------------------------
// StreamSkinkRenderer.h
//----------------------------------------------------------------------------------------------
#ifndef STREAMSINKRENDERER_H
#define STREAMSINKRENDERER_H

class CStreamSkinkRenderer : BaseObject, public IMFStreamSink, public IMFMediaTypeHandler{

public:

	// StreamSkinkRenderer.cpp
	static HRESULT CreateInstance(CMinimalSkinkRenderer*, CStreamSkinkRenderer**, HRESULT&);

	// IUnknown - StreamSkinkRenderer.cpp
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFStreamSink - StreamSkinkRenderer_Sink.cpp
	STDMETHODIMP GetMediaSink(IMFMediaSink**);
	STDMETHODIMP GetIdentifier(DWORD*);
	STDMETHODIMP GetMediaTypeHandler(IMFMediaTypeHandler**);
	STDMETHODIMP ProcessSample(IMFSample*);
	STDMETHODIMP PlaceMarker(MFSTREAMSINK_MARKER_TYPE, const PROPVARIANT*, const PROPVARIANT*);
	STDMETHODIMP Flush();

	// IMFMediaEventGenerator - StreamSkinkRenderer_Event.cpp
	STDMETHODIMP GetEvent(DWORD, IMFMediaEvent**);
	STDMETHODIMP BeginGetEvent(IMFAsyncCallback*, IUnknown*);
	STDMETHODIMP EndGetEvent(IMFAsyncResult*, IMFMediaEvent**);
	STDMETHODIMP QueueEvent(MediaEventType, REFGUID, HRESULT, const PROPVARIANT*);

	// IMFMediaTypeHandler - StreamSkinkRenderer_Type.cpp
	STDMETHODIMP IsMediaTypeSupported(IMFMediaType*, IMFMediaType**);
	STDMETHODIMP GetMediaTypeCount(DWORD*);
	STDMETHODIMP GetMediaTypeByIndex(DWORD, IMFMediaType**);
	STDMETHODIMP SetCurrentMediaType(IMFMediaType*);
	STDMETHODIMP GetCurrentMediaType(IMFMediaType**);
	STDMETHODIMP GetMajorType(GUID*);

	// StreamSkinkRenderer.cpp
	HRESULT Start(MFTIME);
	HRESULT Stop();
	HRESULT Pause();
	HRESULT Restart();
	HRESULT Shutdown();

private:

	// StreamSkinkRenderer.cpp
	CStreamSkinkRenderer(CMinimalSkinkRenderer*, HRESULT&);
	virtual ~CStreamSkinkRenderer();

	CriticSection m_CriticSection;
	volatile long m_nRefCount;
	StreamState m_State;

	IMFMediaEventQueue* m_pEventQueue;
	IMFMediaType* m_pMediaType;
	CMinimalSkinkRenderer* m_pMinimalSkinkRenderer;

	// Inline
	HRESULT CheckShutdown() const{ return (m_State == StreamFinalized ? MF_E_SHUTDOWN : S_OK); }
};

#endif