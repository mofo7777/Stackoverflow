//----------------------------------------------------------------------------------------------
// Dxva2Manager.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

CDxva2Manager::CDxva2Manager() :
	m_pVideoProcessor(NULL),
	m_pRefSurface9(NULL),
	m_pSubSurface9(NULL),
	m_llDuration(0LL),
	m_llTime(0LL),
	m_uiRefWidth(0),
	m_uiRefHeight(0),
	m_uiRefLine(0),
	m_uiSubWidth(0),
	m_uiSubHeight(0),
	m_uiSubLine(0)
{
}

HRESULT CDxva2Manager::InitDxva2(IDirect3DDeviceManager9* pDeviceManager, IMFMediaType* pOutputType, IMFMediaType* pRefInputType, IMFMediaType* pSubInputType) {

	assert(m_pVideoProcessor == NULL);
	assert(m_pRefSurface9 == NULL);
	assert(m_pSubSurface9 == NULL);

	HRESULT hr;

	IF_FAILED_RETURN(hr = (pDeviceManager == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pOutputType == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pRefInputType == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pSubInputType == NULL ? E_POINTER : S_OK));

	IDirectXVideoProcessorService* pVideoProcessorService = NULL;
	HANDLE hD3d9Device = INVALID_HANDLE_VALUE;

	GUID subtype = GUID_NULL;
	UINT32 uiWidth = 0;
	UINT32 uiHeight = 0;
	D3DFORMAT D3DFormat = D3DFMT_UNKNOWN;

	DXVA2_VideoDesc dxva2VideoDesc = { 0 };
	UINT uiCount = 0;
	UINT uiStreamCount = 1;
	GUID* guids = NULL;

	try {

		IF_FAILED_THROW(hr = pDeviceManager->OpenDeviceHandle(&hD3d9Device));
		IF_FAILED_THROW(hr = pDeviceManager->GetVideoService(hD3d9Device, IID_PPV_ARGS(&pVideoProcessorService)));

		IF_FAILED_THROW(hr = GetDxva2VideoDesc(&dxva2VideoDesc, pRefInputType));
		IF_FAILED_THROW(hr = pVideoProcessorService->GetVideoProcessorDeviceGuids(&dxva2VideoDesc, &uiCount, &guids));
		IF_FAILED_THROW(hr = pVideoProcessorService->CreateVideoProcessor(guids[0], &dxva2VideoDesc, D3DFMT_X8R8G8B8, uiStreamCount, &m_pVideoProcessor));

		IF_FAILED_THROW(hr = pRefInputType->GetGUID(MF_MT_SUBTYPE, &subtype));
		IF_FAILED_THROW(hr = MFGetAttributeSize(pRefInputType, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight));

		if (subtype == MFVideoFormat_NV12)
			D3DFormat = (D3DFORMAT)D3DFMT_NV12;
		else
			IF_FAILED_THROW(hr = E_FAIL);

		IF_FAILED_THROW(hr = pVideoProcessorService->CreateSurface(uiWidth, uiHeight, 0, D3DFormat, D3DPOOL_DEFAULT, 0, DXVA2_VideoProcessorRenderTarget, &m_pRefSurface9, NULL));

		m_uiRefWidth = uiWidth;
		m_uiRefHeight = uiHeight;
		m_uiRefLine = m_uiRefHeight + (m_uiRefHeight / 2);

		IF_FAILED_THROW(hr = pSubInputType->GetGUID(MF_MT_SUBTYPE, &subtype));
		IF_FAILED_THROW(hr = MFGetAttributeSize(pSubInputType, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight));

		/*if (subtype == MFVideoFormat_AYUV)
			D3DFormat = (D3DFORMAT)D3DFMT_AYUV;
		else
			IF_FAILED_THROW(hr = E_FAIL);*/

		m_uiSubWidth = uiWidth;
		m_uiSubHeight = uiHeight;
		m_uiSubLine = m_uiSubHeight + (m_uiSubHeight / 2);

		IF_FAILED_THROW(hr = pVideoProcessorService->CreateSurface(uiWidth, uiHeight, 0, D3DFormat, D3DPOOL_DEFAULT, 0, DXVA2_VideoProcessorRenderTarget, &m_pSubSurface9, NULL));
	}
	catch (HRESULT) {}

	CoTaskMemFree(guids);
	
	if (hD3d9Device != INVALID_HANDLE_VALUE) {

		LOG_HRESULT(pDeviceManager->CloseDeviceHandle(hD3d9Device));
	}

	SAFE_RELEASE(pVideoProcessorService);

	return hr;
}

void CDxva2Manager::ReleaseDxva2() {

	SAFE_RELEASE(m_pVideoProcessor);
	SAFE_RELEASE(m_pRefSurface9);
	SAFE_RELEASE(m_pSubSurface9);

	m_llDuration = 0LL;
	m_llTime = 0LL;
	m_uiRefWidth = 0;
	m_uiRefHeight = 0;
	m_uiRefLine = 0;
	m_uiSubWidth = 0;
	m_uiSubHeight = 0;
	m_uiSubLine = 0;
}

HRESULT CDxva2Manager::ProcessInput(IMFSample* pSample, const DWORD dwStreamId) {

	HRESULT hr = S_OK;

	IMFMediaBuffer* pBuffer = NULL;
	BYTE* pData = NULL;

	IDirect3DSurface9* pSurface9 = NULL;
	D3DLOCKED_RECT d3dRect;
	LONG lStride = 0;
	UINT32 uiWidth = 0;
	UINT32 uiLine = 0;

	IMF2DBuffer* p2DBuffer = NULL;

	try {

		if (dwStreamId == 0) {

			IF_FAILED_THROW(hr = pSample->GetSampleTime(&m_llTime));
			IF_FAILED_THROW(hr = pSample->GetSampleDuration(&m_llDuration));
		}

		IF_FAILED_THROW(hr = pSample->GetBufferByIndex(0, &pBuffer));
		IF_FAILED_THROW(hr = pBuffer->QueryInterface(IID_PPV_ARGS(&p2DBuffer)));

		IF_FAILED_THROW(hr = p2DBuffer->Lock2D(&pData, &lStride));

		if (dwStreamId == 0) {

			pSurface9 = m_pRefSurface9;
			uiWidth = m_uiRefWidth;
			uiLine = m_uiRefLine;
		}
		else if (dwStreamId == 1) {

			pSurface9 = m_pSubSurface9;
			//uiWidth = m_uiSubWidth * 4;
			//uiLine = m_uiSubHeight;
			uiWidth = m_uiSubWidth;
			uiLine = m_uiSubLine;
		}

		IF_FAILED_THROW(hr = pSurface9->LockRect(&d3dRect, NULL, 0));

		IF_FAILED_THROW(hr = MFCopyImage((BYTE*)d3dRect.pBits, d3dRect.Pitch, pData, lStride, uiWidth, uiLine));

		IF_FAILED_THROW(hr = pSurface9->UnlockRect());
	}
	catch (HRESULT) {}

	if (pBuffer && pData) {
		LOG_HRESULT(p2DBuffer->Unlock2D());
	}

	SAFE_RELEASE(p2DBuffer);
	SAFE_RELEASE(pBuffer);

	return hr;
}

HRESULT CDxva2Manager::ProcessOutput(IMFSample* pSample) {

	HRESULT hr = S_OK;
	IMFMediaBuffer* pBuffer = NULL;
	IDirect3DSurface9* pSurface = NULL;

	DXVA2_VideoProcessBltParams blt = { 0 };
	RECT rc = { 0, 0, (LONG)m_uiRefWidth, (LONG)m_uiRefHeight };
	DXVA2_AYUVSample16 color;
	color.Cr = 0x0000;
	color.Cb = 0xFFFF;
	color.Y = 0x0000;
	color.Alpha = 0xFFFF;

	const UINT EX_COLOR_INFO[][2] =
	{
		// SDTV ITU-R BT.601 YCbCr to driver's optimal RGB range
		{ DXVA2_VideoTransferMatrix_BT601, DXVA2_NominalRange_Unknown },
		// SDTV ITU-R BT.601 YCbCr to studio RGB [16...235]
		{ DXVA2_VideoTransferMatrix_BT601, DXVA2_NominalRange_16_235 },
		// SDTV ITU-R BT.601 YCbCr to computer RGB [0...255]
		{ DXVA2_VideoTransferMatrix_BT601, DXVA2_NominalRange_0_255 },
		// HDTV ITU-R BT.709 YCbCr to driver's optimal RGB range
		{ DXVA2_VideoTransferMatrix_BT709, DXVA2_NominalRange_Unknown },
		// HDTV ITU-R BT.709 YCbCr to studio RGB [16...235]
		{ DXVA2_VideoTransferMatrix_BT709, DXVA2_NominalRange_16_235 },
		// HDTV ITU-R BT.709 YCbCr to computer RGB [0...255]
		{ DXVA2_VideoTransferMatrix_BT709, DXVA2_NominalRange_0_255 }
	};

	DXVA2_Fixed32 ProcAmpValues[4] = { 0 };
	DXVA2_Fixed32 NFilterValues[6] = { 0 };
	DXVA2_Fixed32 DFilterValues[6] = { 0 };

	DXVA2_VideoSample samples[2] = { 0 };
	UINT uiStreamCount = 2;

	blt.TargetFrame = m_llTime;
	blt.TargetRect = rc;
	blt.ConstrictionSize.cx = rc.right;
	blt.ConstrictionSize.cy = rc.bottom;
	blt.BackgroundColor = color;
	blt.DestFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_Unknown;
	blt.DestFormat.NominalRange = EX_COLOR_INFO[0][1];
	blt.DestFormat.VideoTransferMatrix = DXVA2_VideoTransferMatrix_Unknown;
	blt.DestFormat.VideoLighting = DXVA2_VideoLighting_dim;
	blt.DestFormat.VideoPrimaries = DXVA2_VideoPrimaries_BT709;
	blt.DestFormat.VideoTransferFunction = DXVA2_VideoTransFunc_709;
	blt.DestFormat.SampleFormat = DXVA2_SampleProgressiveFrame;
	blt.ProcAmpValues.Brightness = ProcAmpValues[0];
	blt.ProcAmpValues.Contrast.Value = 1;
	blt.ProcAmpValues.Hue = ProcAmpValues[2];
	blt.ProcAmpValues.Saturation.Value = 1;
	blt.Alpha = DXVA2_Fixed32OpaqueAlpha();
	blt.NoiseFilterLuma.Level = NFilterValues[0];
	blt.NoiseFilterLuma.Threshold = NFilterValues[1];
	blt.NoiseFilterLuma.Radius = NFilterValues[2];
	blt.NoiseFilterChroma.Level = NFilterValues[3];
	blt.NoiseFilterChroma.Threshold = NFilterValues[4];
	blt.NoiseFilterChroma.Radius = NFilterValues[5];
	blt.DetailFilterLuma.Level = DFilterValues[0];
	blt.DetailFilterLuma.Threshold = DFilterValues[1];
	blt.DetailFilterLuma.Radius = DFilterValues[2];
	blt.DetailFilterChroma.Level = DFilterValues[3];
	blt.DetailFilterChroma.Threshold = DFilterValues[4];
	blt.DetailFilterChroma.Radius = DFilterValues[5];

	samples[0].Start = m_llTime;
	samples[0].End = m_llTime + m_llDuration;
	samples[0].SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
	samples[0].SampleFormat.NominalRange = DXVA2_NominalRange_16_235;
	samples[0].SampleFormat.VideoTransferMatrix = EX_COLOR_INFO[0][0];
	samples[0].SampleFormat.VideoLighting = DXVA2_VideoLighting_dim;
	samples[0].SampleFormat.VideoPrimaries = DXVA2_VideoPrimaries_BT709;
	samples[0].SampleFormat.VideoTransferFunction = DXVA2_VideoTransFunc_709;
	samples[0].SampleFormat.SampleFormat = DXVA2_SampleProgressiveFrame;
	samples[0].SrcSurface = m_pRefSurface9;
	samples[0].SrcRect = rc;
	rc.bottom = m_uiRefHeight / 2;
	samples[0].DstRect = rc;
	samples[0].PlanarAlpha = DXVA2FloatToFixed(float(0xFF) / 0xFF);

	rc.right = m_uiSubWidth;
	rc.bottom = m_uiSubHeight;

	samples[1] = samples[0];
	samples[1].SampleFormat = samples[0].SampleFormat;
	samples[1].SampleFormat.SampleFormat = DXVA2_SampleSubStream;
	samples[1].SrcSurface = m_pSubSurface9;
	samples[1].SrcRect = rc;
	rc.top = m_uiSubHeight / 2;
	samples[1].DstRect = rc;

	try {

		IF_FAILED_THROW(hr = pSample->ConvertToContiguousBuffer(&pBuffer));
		IF_FAILED_THROW(hr = MFGetService(pBuffer, MR_BUFFER_SERVICE, __uuidof(IDirect3DSurface9), (void**)&pSurface));

		IF_FAILED_THROW(hr = m_pVideoProcessor->VideoProcessBlt(pSurface, &blt, samples, uiStreamCount, NULL));
	}
	catch (HRESULT) {}

	SAFE_RELEASE(pBuffer);
	SAFE_RELEASE(pSurface);

	return hr;
}

HRESULT CDxva2Manager::GetDxva2VideoDesc(DXVA2_VideoDesc* dxva2VideoDesc, IMFMediaType* pRefInputType) {

	HRESULT hr;

	IF_FAILED_RETURN(hr = (dxva2VideoDesc == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pRefInputType == NULL ? E_POINTER : S_OK));

	D3DFORMAT D3DFormat = D3DFMT_UNKNOWN;
	GUID subtype = { 0 };
	UINT32 uiWidth = 0;
	UINT32 uiHeight = 0;
	UINT32 uiNumerator = 0;
	UINT32 uiDenominator = 0;

	const UINT EX_COLOR_INFO[][2] =
	{
		// SDTV ITU-R BT.601 YCbCr to driver's optimal RGB range
		{ DXVA2_VideoTransferMatrix_BT601, DXVA2_NominalRange_Unknown },
		// SDTV ITU-R BT.601 YCbCr to studio RGB [16...235]
		{ DXVA2_VideoTransferMatrix_BT601, DXVA2_NominalRange_16_235 },
		// SDTV ITU-R BT.601 YCbCr to computer RGB [0...255]
		{ DXVA2_VideoTransferMatrix_BT601, DXVA2_NominalRange_0_255 },
		// HDTV ITU-R BT.709 YCbCr to driver's optimal RGB range
		{ DXVA2_VideoTransferMatrix_BT709, DXVA2_NominalRange_Unknown },
		// HDTV ITU-R BT.709 YCbCr to studio RGB [16...235]
		{ DXVA2_VideoTransferMatrix_BT709, DXVA2_NominalRange_16_235 },
		// HDTV ITU-R BT.709 YCbCr to computer RGB [0...255]
		{ DXVA2_VideoTransferMatrix_BT709, DXVA2_NominalRange_0_255 }
	};
	
	IF_FAILED_RETURN(hr = pRefInputType->GetGUID(MF_MT_SUBTYPE, &subtype));
	IF_FAILED_RETURN(hr = MFGetAttributeSize(pRefInputType, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight));
	IF_FAILED_RETURN(hr = MFGetAttributeRatio(pRefInputType, MF_MT_FRAME_RATE, &uiNumerator, &uiDenominator));

	if (subtype == MFVideoFormat_NV12)
		D3DFormat = (D3DFORMAT)D3DFMT_NV12;
	else
		IF_FAILED_RETURN(hr = E_FAIL);

	dxva2VideoDesc->SampleWidth = uiWidth;
	dxva2VideoDesc->SampleHeight = uiHeight;
	dxva2VideoDesc->SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
	dxva2VideoDesc->SampleFormat.NominalRange = DXVA2_NominalRange_16_235;
	dxva2VideoDesc->SampleFormat.VideoTransferMatrix = EX_COLOR_INFO[0][0];
	dxva2VideoDesc->SampleFormat.VideoLighting = DXVA2_VideoLighting_dim;
	dxva2VideoDesc->SampleFormat.VideoPrimaries = DXVA2_VideoPrimaries_BT709;
	dxva2VideoDesc->SampleFormat.VideoTransferFunction = DXVA2_VideoTransFunc_709;
	dxva2VideoDesc->SampleFormat.SampleFormat = DXVA2_SampleProgressiveFrame;
	dxva2VideoDesc->Format = D3DFormat;
	dxva2VideoDesc->InputSampleFreq.Numerator = uiNumerator;
	dxva2VideoDesc->InputSampleFreq.Denominator = uiDenominator;
	dxva2VideoDesc->OutputFrameFreq.Numerator = uiNumerator;
	dxva2VideoDesc->OutputFrameFreq.Denominator = uiDenominator;

	return hr;
}

/*DWORD CDxva2Manager::GetFrameNumber() {

	DWORD currentTime;
	DWORD currentSysTime = timeGetTime();
	//LONGLONG llTimeTest = m_llTime * 1000;

	if (m_llTime > currentSysTime){

		currentTime = currentSysTime + (0xFFFFFFFF - m_llTime);
	}
	else{

		currentTime = currentSysTime - m_llTime;
	}

	const UINT VIDEO_MSPF = (1000 + VIDEO_FPS / 2) / VIDEO_FPS;
	DWORD frame = currentTime / VIDEO_MSPF;
	DWORD delta = (currentTime - m_dwPreviousTime) / VIDEO_MSPF;

	if (delta > 1){

		TRACE((L"Frame dropped %u frame(s)", delta - 1));
	}

	if (delta > 0){

		m_dwPreviousTime = currentTime;
	}

	return frame;
}*/

void CDxva2Manager::LogVideoProcessBltParams(DXVA2_VideoProcessBltParams& blt) {

	// REFERENCE_TIME       TargetFrame;
	// RECT                 TargetRect;
	// SIZE                 ConstrictionSize;
	// UINT                 StreamingFlags;
	// DXVA2_AYUVSample16   BackgroundColor;
	// DXVA2_ExtendedFormat DestFormat;
	// DXVA2_ProcAmpValues  ProcAmpValues;
	// DXVA2_Fixed32        Alpha;
	// DXVA2_FilterValues   NoiseFilterLuma;
	// DXVA2_FilterValues   NoiseFilterChroma;
	// DXVA2_FilterValues   DetailFilterLuma;
	// DXVA2_FilterValues   DetailFilterChroma;
	// DWORD                DestData;

	TRACE((L"DXVA2_VideoProcessBltParams :"));
	TRACE((L"TargetFrame\t\t\t: %I64d", blt.TargetFrame));
	TRACE((L"TargetRect\t\t\t: left = %d - top = %d - right = %d - bottom = %d", blt.TargetRect.left, blt.TargetRect.top, blt.TargetRect.right, blt.TargetRect.bottom));
	TRACE((L"ConstrictionSize\t: cx = %d - cy = %d", blt.ConstrictionSize.cx, blt.ConstrictionSize.cy));
	TRACE((L"StreamingFlags\t\t: %d", blt.StreamingFlags));
	TRACE((L"BackgroundColor\t\t: Y = 0x%x - Cb = 0x%x - Cr = 0x%x - Alpha = 0x%x", blt.BackgroundColor.Y, blt.BackgroundColor.Cb, blt.BackgroundColor.Cr, blt.BackgroundColor.Alpha));
	TRACE((L"DestFormat\t\t\t: 0x%x", blt.DestFormat));
	TRACE((L"ProcAmpValues\t\t: Brightness = %d - Contrast = %d - Hue = %d - Saturation = %d", blt.ProcAmpValues.Brightness, blt.ProcAmpValues.Contrast, blt.ProcAmpValues.Hue, blt.ProcAmpValues.Saturation));
	TRACE((L"Alpha\t\t\t\t: %d", blt.Alpha));
	TRACE((L"NoiseFilterLuma\t\t: Level = %d - Radius = %d - Threshold = %d", blt.NoiseFilterLuma.Level, blt.NoiseFilterLuma.Radius, blt.NoiseFilterLuma.Threshold));
	TRACE((L"NoiseFilterChroma\t: Level = %d - Radius = %d - Threshold = %d", blt.NoiseFilterChroma.Level, blt.NoiseFilterChroma.Radius, blt.NoiseFilterChroma.Threshold));
	TRACE((L"DetailFilterLuma\t: Level = %d - Radius = %d - Threshold = %d", blt.DetailFilterLuma.Level, blt.DetailFilterLuma.Radius, blt.DetailFilterLuma.Threshold));
	TRACE((L"DetailFilterChroma\t: Level = %d - Radius = %d - Threshold = %d", blt.DetailFilterChroma.Level, blt.DetailFilterChroma.Radius, blt.DetailFilterChroma.Threshold));
	TRACE((L"DestData\t\t\t: %d", blt.DestData));
}

void CDxva2Manager::LogVideoSample(DXVA2_VideoSample& sample) {

	// REFERENCE_TIME       Start;
	// REFERENCE_TIME       End;
	// DXVA2_ExtendedFormat SampleFormat;
	// IDirect3DSurface9*   SrcSurface;
	// RECT                 SrcRect;
	// RECT                 DstRect;
	// DXVA2_AYUVSample8    Pal[16];
	// DXVA2_Fixed32        PlanarAlpha;
	// DWORD                SampleData;

	TRACE((L"DXVA2_VideoSample :"));
	TRACE((L"Start\t\t\t: %I64d", sample.Start));
	TRACE((L"End\t\t\t\t: %I64d", sample.End));
	TRACE((L"SampleFormat\t: 0x%x", sample.SampleFormat));
	TRACE((L"SrcRect\t\t\t: left = %d - top = %d - right = %d - bottom = %d", sample.SrcRect.left, sample.SrcRect.top, sample.SrcRect.right, sample.SrcRect.bottom));
	TRACE((L"DstRect\t\t\t: left = %d - top = %d - right = %d - bottom = %d", sample.DstRect.left, sample.DstRect.top, sample.DstRect.right, sample.DstRect.bottom));
	TRACE((L"PlanarAlpha\t\t: %d", sample.PlanarAlpha));
	TRACE((L"SampleData\t\t: %d", sample.SampleData));
}