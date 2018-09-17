//----------------------------------------------------------------------------------------------
// CustomVideoMixer_Attributes.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HRESULT CCustomVideoMixer::Compare(IMFAttributes* /*pTheirs*/, MF_ATTRIBUTES_MATCH_TYPE /*MatchType*/, BOOL* /*pbResult*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::Compare"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::CompareItem(REFGUID /*guidKey*/, REFPROPVARIANT /*Value*/, BOOL* /*pbResult*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::CompareItem"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::CopyAllItems(IMFAttributes* /*pDest*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::CopyAllItems"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::DeleteAllItems() {

	TRACE_TRANSFORM((L"CustomVideoMixer::DeleteAllItems"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::DeleteItem(REFGUID /*guidKey*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::DeleteItem"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::GetAllocatedBlob(REFGUID /*guidKey*/, UINT8** /*ppBuf*/, UINT32* /*pcbSize*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetAllocatedBlob"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::GetAllocatedString(REFGUID /*guidKey*/, LPWSTR* /*ppwszValue*/, UINT32* /*pcchLength*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetAllocatedString"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::GetBlob(REFGUID /*guidKey*/, UINT8* /*pBuf*/, UINT32 /*cbBufSize*/, UINT32* /*pcbBlobSize*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetBlob"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::GetBlobSize(REFGUID /*guidKey*/, UINT32* /*pcbBlobSize*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetBlobSize"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::GetCount(UINT32* /*pcItems*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetCount"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::GetDouble(REFGUID /*guidKey*/, double* /*pfValue*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetDouble"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::GetGUID(REFGUID /*guidKey*/, GUID* /*pguidValue*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetGUID"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::GetItem(REFGUID /*guidKey*/, PROPVARIANT* /*pValue*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetItem"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::GetItemByIndex(UINT32 /*unIndex*/, GUID* /*pguidKey*/, PROPVARIANT* /*pValue*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetItemByIndex"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::GetItemType(REFGUID /*guidKey*/, MF_ATTRIBUTE_TYPE* /*pType*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetItemType"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::GetString(REFGUID /*guidKey*/, LPWSTR /*pwszValue*/, UINT32 /*cchBufSize*/, UINT32* /*pcchLength*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetString"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::GetStringLength(REFGUID /*guidKey*/, UINT32* /*pcchLength*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetStringLength"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::GetUINT32(REFGUID guidKey, UINT32* punValue) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetUINT32"));

	if (punValue == NULL)
		return E_POINTER;

	if (guidKey == MF_SA_D3D_AWARE) {

		TRACE_TRANSFORM((L"MF_SA_D3D_AWARE"));

		*punValue = TRUE;
		return S_OK;
	}
	else if(guidKey == MF_SA_REQUIRED_SAMPLE_COUNT) {

		TRACE_TRANSFORM((L"MF_SA_REQUIRED_SAMPLE_COUNT"));

		*punValue = 1;
		return S_OK;
	}
	else {

		TRACE_TRANSFORM((L"ERROR : MF_E_ATTRIBUTENOTFOUND"));
	}

	return MF_E_ATTRIBUTENOTFOUND;
}

HRESULT CCustomVideoMixer::GetUINT64(REFGUID /*guidKey*/, UINT64* /*punValue*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetUINT64"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::GetUnknown(REFGUID /*guidKey*/, REFIID /*riid*/, LPVOID* /*ppv*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::GetUnknown"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::LockStore() {

	TRACE_TRANSFORM((L"CustomVideoMixer::LockStore"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::SetBlob(REFGUID guidKey, const UINT8* /*pBuf*/, UINT32 /*cbBufSize*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetBlob"));

	if (guidKey == VIDEO_ZOOM_RECT) {

		TRACE_TRANSFORM((L"VIDEO_ZOOM_RECT"));
		return S_OK;
	}
	else {

		TRACE_TRANSFORM((L"ERROR : MF_E_ATTRIBUTENOTFOUND"));
	}

	return MF_E_ATTRIBUTENOTFOUND;
}

HRESULT CCustomVideoMixer::SetDouble(REFGUID /*guidKey*/, double /*fValue*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetDouble"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::SetGUID(REFGUID /*guidKey*/, REFGUID /*guidValue*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetGUID"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::SetItem(REFGUID /*guidKey*/, REFPROPVARIANT /*Value*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetItem"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::SetString(REFGUID /*guidKey*/, LPCWSTR /*wszValue*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetString"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::SetUINT32(REFGUID /*guidKey*/, UINT32 /*unValue*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetUINT32"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::SetUINT64(REFGUID /*guidKey*/, UINT64 /*unValue*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetUINT64"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::SetUnknown(REFGUID /*guidKey*/, IUnknown* /*pUnknown*/) {

	TRACE_TRANSFORM((L"CustomVideoMixer::SetUnknown"));

	return E_NOTIMPL;
}

HRESULT CCustomVideoMixer::UnlockStore() {

	TRACE_TRANSFORM((L"CustomVideoMixer::UnlockStore"));

	return E_NOTIMPL;
}