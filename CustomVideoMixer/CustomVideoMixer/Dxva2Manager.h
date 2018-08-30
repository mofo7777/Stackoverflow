//----------------------------------------------------------------------------------------------
// Dxva2Manager.h
//----------------------------------------------------------------------------------------------
#ifndef DXVA2MANAGER_H
#define DXVA2MANAGER_H

class CDxva2Manager {

	public:

		CDxva2Manager();
		~CDxva2Manager() { ReleaseDxva2(); }

		HRESULT InitDxva2(IDirect3DDeviceManager9*, IMFMediaType*, IMFMediaType*, IMFMediaType*);
		void ReleaseDxva2();
		HRESULT ProcessInput(IMFSample*, const DWORD);
		HRESULT ProcessOutput(IMFSample*);

	private:

		IDirectXVideoProcessor* m_pVideoProcessor;
		IDirect3DSurface9* m_pRefSurface9;
		IDirect3DSurface9* m_pSubSurface9;

		LONGLONG m_llDuration;
		LONGLONG m_llTime;

		UINT32 m_uiRefWidth;
		UINT32 m_uiRefHeight;
		UINT32 m_uiRefLine;
		UINT32 m_uiSubWidth;
		UINT32 m_uiSubHeight;
		UINT32 m_uiSubLine;

		HRESULT GetDxva2VideoDesc(DXVA2_VideoDesc*, IMFMediaType*);

		void LogVideoProcessBltParams(DXVA2_VideoProcessBltParams&);
		void LogVideoSample(DXVA2_VideoSample&);
};

#endif