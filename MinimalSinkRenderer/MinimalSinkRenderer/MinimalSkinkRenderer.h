//----------------------------------------------------------------------------------------------
// MinimalSkinkRenderer.h
//----------------------------------------------------------------------------------------------
#ifndef MINIMALSINKRENDERER_H
#define MINIMALSINKRENDERER_H

class CMinimalSkinkRenderer : BaseObject, public IMFMediaSink, public IMFClockStateSink{

public:

	// MinimalSkinkRenderer.cpp
	static HRESULT CreateInstance(IUnknown*, REFIID, void**);

	// IUnknown - MinimalSkinkRenderer.cpp
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFMediaSink - MinimalSkinkRenderer_Sink.cpp
	STDMETHODIMP GetCharacteristics(DWORD*);
	STDMETHODIMP AddStreamSink(DWORD, IMFMediaType*, IMFStreamSink**);
	STDMETHODIMP RemoveStreamSink(DWORD);
	STDMETHODIMP GetStreamSinkCount(DWORD*);
	STDMETHODIMP GetStreamSinkByIndex(DWORD, IMFStreamSink**);
	STDMETHODIMP GetStreamSinkById(DWORD, IMFStreamSink**);
	STDMETHODIMP SetPresentationClock(IMFPresentationClock*);
	STDMETHODIMP GetPresentationClock(IMFPresentationClock**);
	STDMETHODIMP Shutdown();

	// IMFClockStateSink - MinimalSkinkRenderer_Clock.cpp
	STDMETHODIMP OnClockStart(MFTIME, LONGLONG);
	STDMETHODIMP OnClockStop(MFTIME);
	STDMETHODIMP OnClockPause(MFTIME);
	STDMETHODIMP OnClockRestart(MFTIME);
	STDMETHODIMP OnClockSetRate(MFTIME, float);

	// MinimalSkinkRenderer.cpp
	HRESULT ProcessSample(IMFSample*);

private:

	// MinimalSkinkRenderer.cpp
	CMinimalSkinkRenderer(HRESULT&);
	virtual ~CMinimalSkinkRenderer();

	CriticSection m_CriticSection;
	volatile long m_nRefCount;
	BOOL m_bShutdown;
	DWORD m_dwCurrentFrame;

	CStreamSkinkRenderer* m_pStreamSkinkRenderer;
	IMFPresentationClock* m_pClock;

	// Inline
	HRESULT CheckShutdown() const{ return (m_bShutdown ? MF_E_SHUTDOWN : S_OK); }
};

#endif