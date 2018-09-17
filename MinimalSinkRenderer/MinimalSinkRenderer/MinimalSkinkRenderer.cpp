//----------------------------------------------------------------------------------------------
// MinimalSkinkRenderer.cpp
//----------------------------------------------------------------------------------------------
#include "Stdafx.h"

CMinimalSkinkRenderer::CMinimalSkinkRenderer(HRESULT& hr)
	: m_nRefCount(1),
	m_bShutdown(FALSE),
	m_pStreamSkinkRenderer(NULL),
	m_pClock(NULL),
	m_dwCurrentFrame(0)
{
	TRACE_SINK((L"SinkRenderer::CTOR"));

	CStreamSkinkRenderer* pStream = NULL;
	hr = CStreamSkinkRenderer::CreateInstance(this, &pStream, hr);

	if(SUCCEEDED(hr)){

		m_pStreamSkinkRenderer = pStream;
		m_pStreamSkinkRenderer->AddRef();
	}
	else{
		m_bShutdown = TRUE;
	}

	SAFE_RELEASE(pStream);
}

CMinimalSkinkRenderer::~CMinimalSkinkRenderer(){

	TRACE_SINK((L"SinkRenderer::DTOR"));
	Shutdown();
}

HRESULT CMinimalSkinkRenderer::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

	TRACE_SINK((L"SinkRenderer::CreateInstance"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

	CMinimalSkinkRenderer* pMFSk = new (std::nothrow)CMinimalSkinkRenderer(hr);

	IF_FAILED_RETURN(pMFSk == NULL ? E_OUTOFMEMORY : S_OK);
	IF_FAILED_RETURN(FAILED(hr) ? hr : S_OK);

	LOG_HRESULT(hr = pMFSk->QueryInterface(iid, ppv));

	SAFE_RELEASE(pMFSk);

	return hr;
}

HRESULT CMinimalSkinkRenderer::QueryInterface(REFIID riid, void** ppv){

	TRACE_SINK((L"SinkRenderer::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
		QITABENT(CMinimalSkinkRenderer, IMFMediaSink),
		QITABENT(CMinimalSkinkRenderer, IMFClockStateSink),
	{0}
	};

	return QISearch(this, qit, riid, ppv);
}

ULONG CMinimalSkinkRenderer::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"SinkRenderer::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CMinimalSkinkRenderer::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"SinkRenderer::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}

HRESULT CMinimalSkinkRenderer::ProcessSample(IMFSample* pSample){

	TRACE_SINK((L"SinkRenderer::ProcessSample"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pSample == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	// if m_dwCurrentFrame == ULONG_MAX, we should need to handle
	m_dwCurrentFrame++;

	LONGLONG llTime = 0;
	hr = pSample->GetSampleTime(&llTime);

	// Log every 2 seconds
	// With big_buck_bunny_720p_50mb.mp4 : 25 fps (25 * 2 = 50) == 2 seconds
	if(SUCCEEDED(hr) && !(m_dwCurrentFrame % 50)){

		TRACE_NO_END_LINE((L"Frame at %u : ", m_dwCurrentFrame));
		MFTimeString(llTime);
	}

	// Save one picture, to check if this is a correct bmp frame.
	// If first frame is black, change TRACE_FRAME_NUMBER
	if(m_dwCurrentFrame == TRACE_FRAME_NUMBER)
		SaveSampleToBmpFile(pSample);

	// Here we should use a thread, instead of Sleep. It is just for simplicity and demonstration.
	// 1 second = 1000 ms
	// 1000 / 25 fps = 40
	// 26 instead of 40, to be relatively in time with sound
	Sleep(26);

	return hr;
}

void CMinimalSkinkRenderer::SaveSampleToBmpFile(IMFSample* pSample){

	if(m_pStreamSkinkRenderer == NULL)
		return;

	HRESULT hr = S_OK;
	IMFMediaType* pMediaType = NULL;
	UINT32 uiWidth = 0;
	UINT32 uiHeight = 0;
	UINT32 uiStride = 0;
	UINT32 uiSize = 0;

	IMFMediaBuffer* pBuffer = NULL;
	BYTE* pData = NULL;
	DWORD dwLenght = 0;

	try{

		IF_FAILED_THROW(hr = m_pStreamSkinkRenderer->GetCurrentMediaType(&pMediaType));
		IF_FAILED_THROW(hr = MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight));
		IF_FAILED_THROW(hr = pMediaType->GetUINT32(MF_MT_DEFAULT_STRIDE, &uiStride));
		IF_FAILED_THROW(hr = pMediaType->GetUINT32(MF_MT_SAMPLE_SIZE, &uiSize));

		IF_FAILED_THROW(hr = (uiSize == (uiStride * uiHeight) ? S_OK : E_UNEXPECTED));

		IF_FAILED_THROW(hr = pSample->ConvertToContiguousBuffer(&pBuffer));
		IF_FAILED_THROW(hr = pBuffer->Lock(&pData, NULL, &dwLenght));

		CreateBmpFile(BMP_IMAGE_FILE, pData, uiWidth, uiHeight, uiSize, uiStride);

		IF_FAILED_THROW(hr = pBuffer->Unlock());
	}
	catch(HRESULT){}

	SAFE_RELEASE(pBuffer);
	SAFE_RELEASE(pMediaType);
}

void CMinimalSkinkRenderer::CreateBmpFile(LPCWSTR wszBmpFile, BYTE* pData, const UINT32 uiWidth, const UINT32 uiHeight, const UINT32 uiSize, const UINT32 uiStride){

	HRESULT hr;

	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwWritten;

	BYTE header24[54] = {0x42, 0x4d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00,
		0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	DWORD dwSizeFile = uiWidth * uiHeight * 3;
	dwSizeFile += 54;
	header24[2] = dwSizeFile & 0x000000ff;// 0x36
	header24[3] = static_cast<BYTE>((dwSizeFile & 0x0000ff00) >> 8);// 0x10
	header24[4] = static_cast<BYTE>((dwSizeFile & 0x00ff0000) >> 16);// 0x0e
	header24[5] = (dwSizeFile & 0xff000000) >> 24;// 0x00
	dwSizeFile -= 54;
	header24[18] = uiWidth & 0x000000ff;// 0x80
	header24[19] = (uiWidth & 0x0000ff00) >> 8;// 0x02
	header24[20] = static_cast<BYTE>((uiWidth & 0x00ff0000) >> 16);// 0x00
	header24[21] = (uiWidth & 0xff000000) >> 24;// 0x00

	header24[22] = uiHeight & 0x000000ff;// 0x0e
	header24[23] = (uiHeight & 0x0000ff00) >> 8;// 0x01
	header24[24] = static_cast<BYTE>((uiHeight & 0x00ff0000) >> 16);// 0x00
	header24[25] = (uiHeight & 0xff000000) >> 24;// 0x00

												 // 000E1036
	header24[34] = dwSizeFile & 0x000000ff;// 0x00
	header24[35] = (dwSizeFile & 0x0000ff00) >> 8;// 0x10
	header24[36] = static_cast<BYTE>((dwSizeFile & 0x00ff0000) >> 16);// 0x0e
	header24[37] = static_cast<BYTE>((dwSizeFile & 0xff000000) >> 24);// 0x00

	try{

		hFile = CreateFile(wszBmpFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

		IF_FAILED_THROW(hr = (hFile == INVALID_HANDLE_VALUE ? E_FAIL : S_OK));

		IF_FAILED_THROW(hr = (WriteFile(hFile, (LPCVOID)header24, 54, &dwWritten, 0) == FALSE));
		IF_FAILED_THROW(hr = (dwWritten == 0 ? E_FAIL : S_OK));

		BYTE* bSourceBuffer = pData + (uiSize - uiStride);
		BYTE* bPixelBuffer = bSourceBuffer;
		BYTE bRGB[3];

		for(UINT32 ui = 0; ui < uiHeight; ui++){

			for(UINT32 uiPixel = 0; uiPixel < uiWidth; uiPixel++){

				bRGB[0] = *bPixelBuffer++;
				bRGB[1] = *bPixelBuffer++;
				bRGB[2] = *bPixelBuffer++;
				bPixelBuffer++;

				IF_FAILED_THROW(hr = (WriteFile(hFile, (LPCVOID)bRGB, 3, &dwWritten, 0) == FALSE));
				IF_FAILED_THROW(hr = (dwWritten == 0 ? E_FAIL : S_OK));
			}

			bSourceBuffer -= uiStride;
			bPixelBuffer = bSourceBuffer;
		}
	}
	catch(HRESULT){}

	if(hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
}