//----------------------------------------------------------------------------------------------
// MFRegistry.h
//----------------------------------------------------------------------------------------------
#ifndef MFREGISTRY_H
#define MFREGISTRY_H

#ifndef CHARS_IN_GUID
  const DWORD CHARS_IN_GUID = 39;
#endif

inline HRESULT CreateObjectKeyName(const GUID& guid, TCHAR* sName, DWORD cchMax){
		
		// convert CLSID uuid to string 
		OLECHAR szCLSID[CHARS_IN_GUID];
		HRESULT hr = StringFromGUID2(guid, szCLSID, CHARS_IN_GUID);
		
		if(FAILED(hr)){
				return hr;
		}

		// Create a string of the form "CLSID\{clsid}"
		return StringCchPrintf(sName, cchMax, TEXT("CLSID\\%ls"), szCLSID);
}

inline HRESULT CreateRegistryKey(HKEY hKey, LPCTSTR subkey, HKEY* phKey){

		LONG lreturn = RegCreateKeyEx(
				hKey,                 // parent key
				subkey,               // name of subkey
				0,                    // reserved
				NULL,                 // class string (can be NULL)
				REG_OPTION_NON_VOLATILE,
				KEY_ALL_ACCESS,
				NULL,                 // security attributes
				phKey,
				NULL                  // receives the "disposition" (is it a new or existing key)
				);

		return HRESULT_FROM_WIN32(lreturn);
}

inline HRESULT SetKeyValue(HKEY hKey, const TCHAR* sName, const TCHAR* sValue){
		
		size_t cch = 0;

		HRESULT hr = StringCchLength(sValue, MAXLONG, &cch);
		
		if(SUCCEEDED(hr)){
				
				// Size must include NULL terminator, which is not counted in StringCchLength
				DWORD  cbData = ((DWORD)cch + 1) * sizeof(TCHAR);

				// set description string
				LONG ret = RegSetValueEx(hKey, sName, 0, REG_SZ, (BYTE*)sValue, cbData);
				
				if(ret == ERROR_SUCCESS){
						hr = S_OK;
				}
				else{
						hr = HRESULT_FROM_WIN32(ret);
				}
		}
		
		return hr;
}

inline HRESULT RegisterObject(HMODULE hModule, const GUID& guid, const TCHAR* sDescription, const TCHAR* sThreadingModel){
		
		HKEY hKey = NULL;
		HKEY hSubkey = NULL;

		TCHAR achTemp[MAX_PATH];

		// Create the name of the key from the object's CLSID
		HRESULT hr = CreateObjectKeyName(guid, achTemp, MAX_PATH);

		// Create the new key.
		if(SUCCEEDED(hr)){
				
				LONG lreturn = RegCreateKeyEx(
						HKEY_CLASSES_ROOT,
						(LPCTSTR)achTemp,     // subkey
						0,                    // reserved
						NULL,                 // class string (can be NULL)
						REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS,
						NULL,                 // security attributes
						&hKey,
						NULL                  // receives the "disposition" (is it a new or existing key)
						);

				hr = __HRESULT_FROM_WIN32(lreturn);
		}

		// The default key value is a description of the object.
		if(SUCCEEDED(hr)){
				hr = SetKeyValue(hKey, NULL, sDescription);
		}

		// Create the "InprocServer32" subkey
		if(SUCCEEDED(hr)){
				
				const TCHAR* sServer = TEXT("InprocServer32");

				LONG lreturn = RegCreateKeyEx(hKey, sServer, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubkey, NULL);

				hr = __HRESULT_FROM_WIN32(lreturn);
		}

		// The default value for this subkey is the path to the DLL. Get the name of the module ...
		if(SUCCEEDED(hr)){
				
				DWORD res = GetModuleFileName(hModule, achTemp, MAX_PATH);
				
				if(res == 0){
						hr = __HRESULT_FROM_WIN32(GetLastError());
				}
				if(res == MAX_PATH){
						hr = E_FAIL; // buffer too small
				}
		}

		// ... and set the default key value.
		if(SUCCEEDED(hr)){
				hr = SetKeyValue(hSubkey, NULL, achTemp);
		}

		// Add a new value to the subkey, for "ThreadingModel" = <threading model>
		if(SUCCEEDED(hr)){
				hr = SetKeyValue(hSubkey, TEXT("ThreadingModel"), sThreadingModel);
		}

		// close hkeys
		if(hSubkey != NULL){
				RegCloseKey(hSubkey);
		}

		if(hKey != NULL){
				RegCloseKey(hKey);
		}

		return hr;
}

inline HRESULT UnregisterObject(const GUID& guid){
		
		TCHAR achTemp[MAX_PATH];

		HRESULT hr = CreateObjectKeyName(guid, achTemp, MAX_PATH);

		if(SUCCEEDED(hr)){
				
				// Delete the key recursively.
				DWORD res = RegDeleteTree(HKEY_CLASSES_ROOT, achTemp);

				if(res == ERROR_SUCCESS){
						hr = S_OK;
				}
				else{
						hr = __HRESULT_FROM_WIN32(res);
				}
		}

		return hr;
}

static const TCHAR* REGKEY_MF_SCHEME_HANDLERS = TEXT("Software\\Microsoft\\Windows Media Foundation\\SchemeHandlers");

inline HRESULT RegisterSchemeHandler(const GUID& guid, const TCHAR* szSchemeExtension, const TCHAR* szDescription){
		
		HRESULT hr = S_OK;

		HKEY    hKey = NULL;
		HKEY    hSubKey = NULL;

		OLECHAR szCLSID[CHARS_IN_GUID];

		size_t  cchDescription = 0;
		
		hr = StringCchLength(szDescription, STRSAFE_MAX_CCH, &cchDescription);

		if(SUCCEEDED(hr)){
				hr = StringFromGUID2(guid, szCLSID, CHARS_IN_GUID);
		}

		if(SUCCEEDED(hr)){
				hr = CreateRegistryKey(HKEY_LOCAL_MACHINE, REGKEY_MF_SCHEME_HANDLERS, &hKey);
		}

		if(SUCCEEDED(hr)){ 
				hr = CreateRegistryKey(hKey, szSchemeExtension, &hSubKey);
		}

		if(SUCCEEDED(hr)){				
				hr = RegSetValueEx(hSubKey, szCLSID, 0, REG_SZ, (BYTE*)szDescription, static_cast<DWORD>((cchDescription + 1) * sizeof(TCHAR)));
		}

		if(hSubKey != NULL){
				RegCloseKey( hSubKey );
		}

		if(hKey != NULL){
				RegCloseKey( hKey );
		}

		return hr;
}

inline HRESULT UnregisterSchemeHandler(const GUID& guid, const TCHAR* szSchemeExtension){
		
		TCHAR szKey[MAX_PATH];
		OLECHAR szCLSID[CHARS_IN_GUID];

		DWORD result = 0;
		HRESULT hr = S_OK;

		if(SUCCEEDED(hr = StringCchPrintf(szKey, MAX_PATH, TEXT("%s\\%s"), REGKEY_MF_SCHEME_HANDLERS, szSchemeExtension))){

				if(SUCCEEDED(hr = StringFromGUID2(guid, szCLSID, CHARS_IN_GUID))){

						result = RegDeleteKeyValue(HKEY_LOCAL_MACHINE, szKey, szCLSID);
						
						if(result != ERROR_SUCCESS){
								hr = HRESULT_FROM_WIN32(result);
						}
				}
		}

		return hr;
}

static const TCHAR* REGKEY_MF_BYTESTREAM_HANDLERS = TEXT("Software\\Microsoft\\Windows Media Foundation\\ByteStreamHandlers");

inline HRESULT RegisterByteStreamHandler(const GUID& guid, const TCHAR* sFileExtension, const TCHAR* sDescription){
		
		HRESULT hr = S_OK;

		HKEY hKey = NULL;
		HKEY hSubKey = NULL;

		OLECHAR szCLSID[CHARS_IN_GUID];

		size_t  cchDescription = 0;
		
		hr = StringCchLength(sDescription, STRSAFE_MAX_CCH, &cchDescription);

		if(SUCCEEDED(hr)){
				hr = StringFromGUID2(guid, szCLSID, CHARS_IN_GUID);
		}

		if(SUCCEEDED(hr)){
				hr = CreateRegistryKey(HKEY_LOCAL_MACHINE, REGKEY_MF_BYTESTREAM_HANDLERS, &hKey);
		}

		if(SUCCEEDED(hr)){
				hr = CreateRegistryKey(hKey, sFileExtension, &hSubKey);
		}

		if(SUCCEEDED(hr)){				
				hr = RegSetValueEx(hSubKey, szCLSID, 0, REG_SZ, (BYTE*)sDescription, static_cast<DWORD>((cchDescription + 1) * sizeof(TCHAR)));
		}

		if(hSubKey != NULL){
				RegCloseKey(hSubKey);
		}

		if(hKey != NULL){
				RegCloseKey(hKey);
		}

		return hr;
}

inline HRESULT UnregisterByteStreamHandler(const GUID& guid, const TCHAR* sFileExtension){
		
		TCHAR szKey[MAX_PATH];
		OLECHAR szCLSID[CHARS_IN_GUID];

		DWORD result = 0;
		HRESULT hr = S_OK;

		hr = StringCchPrintf(szKey, MAX_PATH, TEXT("%s\\%s"), REGKEY_MF_BYTESTREAM_HANDLERS, sFileExtension);

		if(SUCCEEDED(hr = StringFromGUID2(guid, szCLSID, CHARS_IN_GUID))){

				result = RegDeleteKeyValue(HKEY_LOCAL_MACHINE, szKey, szCLSID);
				
				if(result != ERROR_SUCCESS){
						hr = HRESULT_FROM_WIN32(result);
				}
		}

		return hr;
}

#endif
