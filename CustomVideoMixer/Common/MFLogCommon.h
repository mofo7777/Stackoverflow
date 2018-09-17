//----------------------------------------------------------------------------------------------
// MFLogCommon.h
//----------------------------------------------------------------------------------------------
#ifndef MFLOGCOMMON_H
#define MFLOGCOMMON_H

#if (_DEBUG && MF_USE_LOGGING)

typedef LPCWSTR(*GetGUIDStringName)(const GUID&);

inline void LogGuidHexa(const GUID& guid, const BOOL bFirst){

	int iSize = 37;
	WCHAR pBuffer[37];
	WCHAR* ppBuffer = pBuffer;
	// todo : swprintf_s return -1 if error

	ppBuffer += swprintf_s(ppBuffer, iSize, L"%.8lX-%.4hX-%.4hX-", guid.Data1, guid.Data2, guid.Data3);

	iSize -= 19;

	for(int i = 0; i < sizeof(guid.Data4); ++i){

		ppBuffer += swprintf_s(ppBuffer, iSize, L"%.2hhX", guid.Data4[i]);

		iSize -= 2;

		if(i == 1){

			*(ppBuffer++) = '-';
			iSize--;
		}
	}

	if(bFirst)
		TRACE_NO_END_LINE((L"\t{%s}\t", pBuffer));
	else
		TRACE((L"{%s}", pBuffer));
}

inline void LogUINT32AsUINT64(const PROPVARIANT& var){

	UINT32 uHigh = 0, uLow = 0;

	Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &uHigh, &uLow);

	TRACE((L"%u x %u", uHigh, uLow));
}

inline HRESULT SpecialCaseAttributeValue(GUID guid, const PROPVARIANT& var){

	if(guid == MF_MT_FRAME_RATE){
		LogUINT32AsUINT64(var);
	}
	else if(guid == MF_MT_FRAME_SIZE){
		LogUINT32AsUINT64(var);
	}
	else if(guid == MF_MT_PIXEL_ASPECT_RATIO){
		LogUINT32AsUINT64(var);
	}
	else{
		return S_FALSE;
	}

	return S_OK;
}

inline void LogPropertyVariant(const GUID* pGuid, const PROPVARIANT& var, GetGUIDStringName pGetGUIDString){

	HRESULT hr = S_FALSE;

	if(pGuid != NULL)
		hr = SpecialCaseAttributeValue(*pGuid, var);

	if(hr == S_FALSE){

		switch(var.vt){

		case VT_UI4:
			TRACE((L"%lu", var.ulVal));
			break;

		case VT_I4:
			TRACE((L"%ld", var.lVal));
			break;

		case VT_UI8:
			TRACE((L"%I64u", var.uhVal));
			break;

		case VT_BOOL:
			TRACE((L"%s", var.boolVal == -1 ? "true" : "false"));
			break;

		case VT_R8:
			TRACE((L"%f", var.dblVal));
			break;

		case VT_CLSID:
		{
			LPCWSTR pwszGuidName = pGetGUIDString(*var.puuid);

			if(pwszGuidName != NULL){
				TRACE((L"%s", pwszGuidName));
			}
			else{
				LogGuidHexa(*var.puuid, FALSE);
			}
		}
		break;

		case VT_LPWSTR:
			// Log  : var.pwszVal
			TRACE((L"VT_LPWSTR = todo"));
			break;

		case VT_VECTOR | VT_UI1:
			TRACE((L"(VT_VECTOR|VT_UI1) = <<byte array>>"));
			break;

		case VT_UNKNOWN:
			TRACE((L"VT_UNKNOWN = IUnknown"));
			break;

		case VT_EMPTY:
			TRACE((L"VT_EMPTY"));
			break;

		case VT_R4:
			TRACE((L"VT_R4 = %f", var.fltVal));
			break;

			// 8195 = VT_ARRAY | VT_I4

		default:
			TRACE((L"Unexpected attribute type (vt = %hu)", var.vt));
			break;
		}
	}
}

inline void LogAttributeValueByIndex(IMFAttributes* pAttributes, UINT32 uiIndex, GetGUIDStringName pGetGUIDString){

	HRESULT hr;
	PROPVARIANT var;
	GUID guid = GUID_NULL;

	PropVariantInit(&var);

	hr = pAttributes->GetItemByIndex(uiIndex, &guid, &var);

	if(SUCCEEDED(hr)){

		LPCWSTR pwszGUIDNameAttributes = pGetGUIDString(guid);

		if(pwszGUIDNameAttributes != NULL){
			TRACE_NO_END_LINE((L"\t%s\t", pwszGUIDNameAttributes));
		}
		else{

			LogGuidHexa(guid, TRUE);
		}

		LogPropertyVariant(&guid, var, pGetGUIDString);
	}
	else{
		TRACE((L"\tGetItemByIndex (Index = %u)  hr = %s", uiIndex, MFErrorString(hr)));
	}

	PropVariantClear(&var);
}

#endif

#endif
