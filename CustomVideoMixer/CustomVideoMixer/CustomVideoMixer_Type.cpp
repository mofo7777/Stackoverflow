//----------------------------------------------------------------------------------------------
// CustomVideoMixer_Type.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HRESULT CCustomVideoMixer::GetOutputType(IMFMediaType** ppType) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetOutputType"));

	HRESULT hr = S_OK;
	IMFMediaType* pOutputType = NULL;

	try {

		IF_FAILED_THROW(hr = MFCreateMediaType(&pOutputType));

		IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		// MFVideoFormat_ARGB32 MFVideoFormat_RGB32
		IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
		IF_FAILED_THROW(hr = MFSetAttributeRatio(pOutputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1));

		*ppType = pOutputType;
		(*ppType)->AddRef();
	}
	catch (HRESULT) {}

	SAFE_RELEASE(pOutputType);
	return hr;
}

/*HRESULT CCustomVideoMixer::GetOutputType(IMFMediaType** ppType) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetOutputType"));
	
	HRESULT hr = S_OK;
	IMFMediaType* pOutputType = NULL;
	
	try {

		IF_FAILED_THROW(hr = MFCreateMediaType(&pOutputType));

		IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
		//IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_SAMPLE_SIZE, m_dwRefSampleSize));
	
		//IF_FAILED_THROW(hr = MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, m_uiRefWidthInPixels, m_uiRefHeightInPixels));
		IF_FAILED_THROW(hr = MFSetAttributeRatio(pOutputType, MF_MT_FRAME_RATE, m_RefFrameRate.Numerator, m_RefFrameRate.Denominator));
		//-->IF_FAILED_THROW(hr = MFSetAttributeRatio(pOutputType, MF_MT_PIXEL_ASPECT_RATIO, m_RefPixelRatio.Numerator, m_RefPixelRatio.Denominator));
	
		//-->MFVideoArea area;
		//-->area.OffsetX = MakeOffset(0.0f);
		//-->area.OffsetY = MakeOffset(0.0f);
		//-->area.Area.cx = m_uiRefWidthInPixels;
		//-->area.Area.cy = m_uiRefHeightInPixels;
	
		//-->IF_FAILED_THROW(hr = pOutputType->SetBlob(MF_MT_GEOMETRIC_APERTURE, (UINT8*)&area, sizeof(MFVideoArea)));
		//-->IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_PAN_SCAN_ENABLED, 0));

		*ppType = pOutputType;
		(*ppType)->AddRef();

		// MF_MT_MAJOR_TYPE					MFMediaType_Video
		// MF_MT_SUBTYPE					MFVideoFormat_RGB32
		// MF_MT_FRAME_SIZE					1024 x 768
		// MF_MT_FRAME_RATE					15 x 1
		// MF_MT_GEOMETRIC_APERTURE			<< byte array >>
		// MF_MT_MINIMUM_DISPLAY_APERTURE	<< byte array >>
		// MF_MT_PAN_SCAN_APERTURE			<< byte array >>
		// MF_MT_PIXEL_ASPECT_RATIO			1310720 x 1310720
		// MF_MT_VIDEO_PRIMARIES			2
		// MF_MT_INTERLACE_MODE				2
		// MF_MT_DEFAULT_STRIDE				4096
		// MF_MT_ALL_SAMPLES_INDEPENDENT	1
		// MF_MT_FIXED_SIZE_SAMPLES			1
		// MF_MT_SAMPLE_SIZE				3145728
	}
	catch (HRESULT) {}
	
	SAFE_RELEASE(pOutputType);
	return hr;
}*/

HRESULT CCustomVideoMixer::OnCheckInputType(IMFMediaType** ppInptuType, IMFMediaType* pmt, const GUID gFormatSubType) {

	TRACE_TRANSFORM((L"CustomVideoMixer::OnCheckInputType"));

	HRESULT hr = S_OK;

	if (ppInptuType && *ppInptuType) {

		DWORD dwFlags = 0;

		if ((*ppInptuType)->IsEqual(pmt, &dwFlags) == S_OK) {
			return hr;
		}
		else {
			IF_FAILED_RETURN(hr = MF_E_INVALIDTYPE);
			// Todo
			//SAFE_RELEASE(pInptuType);
			//SAFE_RELEASE(m_pOutputType);
			//Flush();
		}
	}

	GUID majortype = { 0 };
	GUID subtype = { 0 };
	UINT32 width = 0;
	UINT32 height = 0;

	IF_FAILED_RETURN(hr = pmt->GetMajorType(&majortype));
	IF_FAILED_RETURN(hr = pmt->GetGUID(MF_MT_SUBTYPE, &subtype));
	IF_FAILED_RETURN(hr = MFGetAttributeSize(pmt, MF_MT_FRAME_SIZE, &width, &height));

	IF_FAILED_RETURN(hr = (majortype != MFMediaType_Video ? MF_E_INVALIDTYPE : S_OK));
	IF_FAILED_RETURN(hr = (subtype != gFormatSubType ? MF_E_INVALIDTYPE : S_OK));

	// Todo : check width = 0/height = 0
	IF_FAILED_RETURN(hr = ((width > MAX_VIDEO_WIDTH_HEIGHT || height > MAX_VIDEO_WIDTH_HEIGHT) ? MF_E_INVALIDTYPE : S_OK));

	return hr;
}

HRESULT CCustomVideoMixer::OnSetInputType(IMFMediaType** ppInptuType, IMFMediaType* pType) {

	TRACE_TRANSFORM((L"CustomVideoMixer::OnSetInputType"));

	HRESULT hr = S_OK;
	GUID subtype = { 0 };

	SAFE_RELEASE(*ppInptuType);

	IF_FAILED_RETURN(hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype));
	//IF_FAILED_RETURN(hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &m_uiRefWidthInPixels, &m_uiRefHeightInPixels));
	//IF_FAILED_RETURN(hr = MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, (UINT32*)&m_RefFrameRate.Numerator, (UINT32*)&m_RefFrameRate.Denominator));
	//IF_FAILED_RETURN(hr = MFGetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, (UINT32*)&m_RefPixelRatio.Numerator, (UINT32*)&m_RefPixelRatio.Denominator));

	// Todo check width = 0/height = 0/m_FrameRate = 0/m_PixelRatio = 0
	//IF_FAILED_RETURN(hr = MFFrameRateToAverageTimePerFrame(m_FrameRate.Numerator, m_FrameRate.Denominator, &m_rtAvgPerFrameInput));

	//m_dwRefSampleSize = (m_uiRefHeightInPixels + (m_uiRefHeightInPixels / 2)) * m_uiRefWidthInPixels;

	//m_bIsMpeg1 = (subtype != MFVideoFormat_MPEG2);
	//m_bMpegHeaderEx = m_bIsMpeg1;

	//IF_FAILED_RETURN(hr = ConfigureDecoder());

	*ppInptuType = pType;
	(*ppInptuType)->AddRef();

	return hr;
}

HRESULT CCustomVideoMixer::OnCheckOutputType(IMFMediaType* pType) {

	TRACE_TRANSFORM((L"CustomVideoMixer::OnCheckOutputType"));

	LogMediaType(pType);

	HRESULT hr = S_OK;

	if (m_pOutputType) {

		DWORD dwFlags = 0;

		if (m_pOutputType->IsEqual(pType, &dwFlags) == S_OK) {
			return hr;
		}
		else {
			IF_FAILED_RETURN(hr = MF_E_INVALIDTYPE);
			// Todo
			//SAFE_RELEASE(m_pInputType);
			//Flush();
		}
	}

	// Todo : not here, but before call to OnCheckOutputType
	if (m_pRefInputType == NULL) {
		return MF_E_TRANSFORM_TYPE_NOT_SET;
	}

	IMFMediaType* pOurType = NULL;
	BOOL bMatch = FALSE;

	try {

		IF_FAILED_THROW(hr = GetOutputType(&pOurType));

		IF_FAILED_THROW(hr = pOurType->Compare(pType, MF_ATTRIBUTES_MATCH_OUR_ITEMS, &bMatch));

		IF_FAILED_THROW(hr = (!bMatch ? MF_E_INVALIDTYPE : S_OK));
	}
	catch (HRESULT) {}

	SAFE_RELEASE(pOurType);
	return hr;
}