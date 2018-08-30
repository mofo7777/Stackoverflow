//----------------------------------------------------------------------------------------------
// CustomVideoMixer.h
//----------------------------------------------------------------------------------------------
#ifndef MFTCUSTOMVIDEOMIXER_H
#define MFTCUSTOMVIDEOMIXER_H

class CCustomVideoMixer :
	BaseObject,
	public IMFVideoDeviceID,
	public IMFGetService,
	public IMFTopologyServiceLookupClient,
	public IMFTransform,
	public IMFVideoMixerControl,
	public IMFVideoProcessor,
	public IMFAttributes,
	public IMFVideoMixerBitmap,
	public IMFVideoPositionMapper
{

public:

	// CustomVideoMixer.cpp
	static HRESULT CreateInstance(IUnknown*, REFIID, void**);

	// IUnknown - CustomVideoMixer.cpp
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFVideoDeviceID - CustomVideoMixer.cpp
	STDMETHODIMP GetDeviceID(IID*);

	// IMFGetService - CustomVideoMixer.cpp
	STDMETHODIMP GetService(REFGUID, REFIID, LPVOID*);

	// IMFTopologyServiceLookupClient - CustomVideoMixer.cpp
	STDMETHODIMP InitServicePointers(IMFTopologyServiceLookup*);
	STDMETHODIMP ReleaseServicePointers();

	// IMFTransform - CustomVideoMixer_Transform.cpp
	STDMETHODIMP GetStreamLimits(DWORD*, DWORD*, DWORD*, DWORD*);
	STDMETHODIMP GetStreamCount(DWORD*, DWORD*);
	STDMETHODIMP GetStreamIDs(DWORD, DWORD*, DWORD, DWORD*);
	STDMETHODIMP GetInputStreamInfo(DWORD, MFT_INPUT_STREAM_INFO*);
	STDMETHODIMP GetOutputStreamInfo(DWORD, MFT_OUTPUT_STREAM_INFO*);
	STDMETHODIMP GetAttributes(IMFAttributes**);
	STDMETHODIMP GetInputStreamAttributes(DWORD, IMFAttributes**);
	STDMETHODIMP GetOutputStreamAttributes(DWORD, IMFAttributes**);
	STDMETHODIMP DeleteInputStream(DWORD);
	STDMETHODIMP AddInputStreams(DWORD, DWORD*);
	STDMETHODIMP GetInputAvailableType(DWORD, DWORD, IMFMediaType**);
	STDMETHODIMP GetOutputAvailableType(DWORD, DWORD, IMFMediaType**);
	STDMETHODIMP SetInputType(DWORD, IMFMediaType*, DWORD);
	STDMETHODIMP SetOutputType(DWORD, IMFMediaType*, DWORD);
	STDMETHODIMP GetInputCurrentType(DWORD, IMFMediaType**);
	STDMETHODIMP GetOutputCurrentType(DWORD, IMFMediaType**);
	STDMETHODIMP GetInputStatus(DWORD, DWORD*);
	STDMETHODIMP GetOutputStatus(DWORD*);
	STDMETHODIMP SetOutputBounds(LONGLONG, LONGLONG);
	STDMETHODIMP ProcessEvent(DWORD, IMFMediaEvent*);
	STDMETHODIMP ProcessMessage(MFT_MESSAGE_TYPE, ULONG_PTR);
	STDMETHODIMP ProcessInput(DWORD, IMFSample*, DWORD);
	STDMETHODIMP ProcessOutput(DWORD, DWORD, MFT_OUTPUT_DATA_BUFFER*, DWORD*);

	// IMFVideoMixerControl - CustomVideoMixer_Mixer.cpp
	STDMETHODIMP GetStreamOutputRect(DWORD, MFVideoNormalizedRect*);
	STDMETHODIMP GetStreamZOrder(DWORD, DWORD*);
	STDMETHODIMP SetStreamOutputRect(DWORD, const MFVideoNormalizedRect*);
	STDMETHODIMP SetStreamZOrder(DWORD, DWORD);

	// IMFVideoProcessor - CustomVideoMixer_Mixer.cpp
	STDMETHODIMP GetAvailableVideoProcessorModes(UINT*, GUID**);
	STDMETHODIMP GetBackgroundColor(COLORREF*);
	STDMETHODIMP GetFilteringRange(DWORD, DXVA2_ValueRange*);
	STDMETHODIMP GetFilteringValue(DWORD, DXVA2_Fixed32*);
	STDMETHODIMP GetProcAmpRange(DWORD, DXVA2_ValueRange*);
	STDMETHODIMP GetProcAmpValues(DWORD, DXVA2_ProcAmpValues*);
	STDMETHODIMP GetVideoProcessorCaps(LPGUID, DXVA2_VideoProcessorCaps*);
	STDMETHODIMP GetVideoProcessorMode(LPGUID);
	STDMETHODIMP SetBackgroundColor(COLORREF);
	STDMETHODIMP SetFilteringValue(DWORD, DXVA2_Fixed32*);
	STDMETHODIMP SetProcAmpValues(DWORD, DXVA2_ProcAmpValues*);
	STDMETHODIMP SetVideoProcessorMode(LPGUID);

	// IMFAttributes - CustomVideoMixer_Attributes.cpp
	STDMETHODIMP Compare(IMFAttributes*, MF_ATTRIBUTES_MATCH_TYPE, BOOL*);
	STDMETHODIMP CompareItem(REFGUID, REFPROPVARIANT, BOOL*);
	STDMETHODIMP CopyAllItems(IMFAttributes*);
	STDMETHODIMP DeleteAllItems();
	STDMETHODIMP DeleteItem(REFGUID);
	STDMETHODIMP GetAllocatedBlob(REFGUID, UINT8**, UINT32*);
	STDMETHODIMP GetAllocatedString(REFGUID, LPWSTR*, UINT32*);
	STDMETHODIMP GetBlob(REFGUID, UINT8*, UINT32, UINT32*);
	STDMETHODIMP GetBlobSize(REFGUID, UINT32*);
	STDMETHODIMP GetCount(UINT32*);
	STDMETHODIMP GetDouble(REFGUID, double*);
	STDMETHODIMP GetGUID(REFGUID, GUID*);
	STDMETHODIMP GetItem(REFGUID, PROPVARIANT*);
	STDMETHODIMP GetItemByIndex(UINT32, GUID*, PROPVARIANT*);
	STDMETHODIMP GetItemType(REFGUID, MF_ATTRIBUTE_TYPE*);
	STDMETHODIMP GetString(REFGUID, LPWSTR, UINT32, UINT32*);
	STDMETHODIMP GetStringLength(REFGUID, UINT32*);
	STDMETHODIMP GetUINT32(REFGUID, UINT32*);
	STDMETHODIMP GetUINT64(REFGUID, UINT64*);
	STDMETHODIMP GetUnknown(REFGUID, REFIID, LPVOID*);
	STDMETHODIMP LockStore();
	STDMETHODIMP SetBlob(REFGUID, const UINT8*, UINT32);
	STDMETHODIMP SetDouble(REFGUID, double);
	STDMETHODIMP SetGUID(REFGUID, REFGUID);
	STDMETHODIMP SetItem(REFGUID, REFPROPVARIANT);
	STDMETHODIMP SetString(REFGUID, LPCWSTR);
	STDMETHODIMP SetUINT32(REFGUID, UINT32);
	STDMETHODIMP SetUINT64(REFGUID, UINT64);
	STDMETHODIMP SetUnknown(REFGUID, IUnknown*);
	STDMETHODIMP UnlockStore();

	// IMFVideoMixerBitmap - CustomVideoMixer_Bitmap.cpp
	STDMETHODIMP ClearAlphaBitmap();
	STDMETHODIMP GetAlphaBitmapParameters(MFVideoAlphaBitmapParams*);
	STDMETHODIMP SetAlphaBitmap(const MFVideoAlphaBitmap*);
	STDMETHODIMP UpdateAlphaBitmapParameters(const MFVideoAlphaBitmapParams*);

	// IMFVideoPositionMapper - CustomVideoMixer_Bitmap.cpp
	STDMETHODIMP MapOutputCoordinateToInputStream(float, float, DWORD, DWORD, float*, float*);


private:

	// CustomVideoMixer.cpp
	CCustomVideoMixer();
	virtual ~CCustomVideoMixer();

	CriticSection m_CriticSection;
	volatile long m_nRefCount;

	CDxva2Manager m_cDxva2Manager;

	IMediaEventSink* m_pMediaEventSink;

	IMFMediaType* m_pRefInputType;
	IMFMediaType* m_pSubInputType;
	IMFMediaType* m_pOutputType;

	BOOL m_bDraining;
	DWORD m_dwInputStreamCount;
	BOOL m_bHaveRefOuput;
	BOOL m_bHaveSubOuput;

	// CustomVideoMixer.cpp
	HRESULT SetD3DManager(IDirect3DDeviceManager9*);
	HRESULT BeginStreaming(ULONG_PTR);
	HRESULT Flush();

	// CustomVideoMixer_Type.cpp
	HRESULT GetOutputType(IMFMediaType**);
	HRESULT OnCheckInputType(IMFMediaType**, IMFMediaType*, const GUID);
	HRESULT OnSetInputType(IMFMediaType**, IMFMediaType*);
	HRESULT OnCheckOutputType(IMFMediaType*);
};

#endif