//----------------------------------------------------------------------------------------------
// Player.h
//----------------------------------------------------------------------------------------------
#ifndef PLAYER_H
#define PLAYER_H

enum PlayerState
{
	Closed = 0,     // No session.
	Ready,          // Session was created, ready to open a file. 
	OpenPending,    // Session is opening a file.
	Started,        // Session is playing a file.
	Paused,         // Session is paused.
	Stopped,        // Session is stopped (ready to play). 
	Closing         // Application has closed the session, but is waiting for MESessionClosed.
};

class CPlayer : public IMFAsyncCallback{

public:

	static HRESULT CreateInstance(const HWND, const int, CPlayer**);

	// IUnknown - Player.cpp
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFAsyncCallback - Player.cpp
	STDMETHODIMP  GetParameters(DWORD*, DWORD*){ return S_OK; }
	STDMETHODIMP  Invoke(IMFAsyncResult*);

	HRESULT OpenUrl(LPCWSTR);
	HRESULT Shutdown();

private:

	CPlayer(const HWND hwndVideo, const int iIndex) : m_lRefCount(1), m_hwndVideo(hwndVideo), m_iIndex(iIndex){}
	virtual ~CPlayer(){}

	volatile long m_lRefCount;
	HWND m_hwndVideo = NULL;
	int m_iIndex = 0;
	PlayerState m_state = Closed;
	std::mutex m_mtx;
	std::condition_variable m_cv_open;
	std::condition_variable m_cv_close;
	CComPtr<IMFMediaSource> m_pSource = NULL;
	CComPtr<IMFTransform> m_pDecoder = NULL;
	IMFMediaSession* m_pSession = NULL;

	HRESULT HandleEvent(IMFMediaEvent*);
	HRESULT OnTopologyStatus(IMFMediaEvent*);
	HRESULT OnPresentationEnded(IMFMediaEvent*);
	HRESULT OnNewPresentation(IMFMediaEvent*);
	HRESULT StartPlayback();
	HRESULT CreateSession();
	HRESULT CreateMediaSource(LPCWSTR, IMFMediaSource**);
	HRESULT CreatePlaybackTopology(IMFMediaSource*, IMFPresentationDescriptor*, const HWND, IMFTopology**, IMFTransform**);
	HRESULT AddBranchToPartialTopology(IMFTopology*, IMFMediaSource*, IMFPresentationDescriptor*, const DWORD, const HWND, IMFTransform**, const bool withOverlay = false);
	HRESULT AddOutputNode(IMFTopology*, IMFActivate*, const DWORD, IMFTopologyNode**);
	HRESULT AddSourceNode(IMFTopology*, IMFMediaSource*, IMFPresentationDescriptor*, IMFStreamDescriptor*, IMFTopologyNode**);
	HRESULT CreateVideoDecoderTopology(IMFTopology*, IMFTopologyNode*, IMFTopologyNode*, IMFTransform**);
	HRESULT AddTransformNode(IMFTopology*, const CLSID&, IMFTopologyNode**, IMFTransform**);
};

#endif