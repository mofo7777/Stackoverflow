//----------------------------------------------------------------------------------------------
// DllMain.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HMODULE g_hModule;

DEFINE_CLASSFACTORY_SERVER_LOCK;

ClassFactoryData g_ClassFactories[] = { { &CLSID_CustomVideoMixer, CCustomVideoMixer::CreateInstance } };
      
const DWORD g_numClassFactories = ARRAY_SIZE(g_ClassFactories);

const WCHAR SZ_DECODER_NAME[] = L"Custom Video Mixer";

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/){

	switch(ul_reason_for_call){

		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls((HMODULE)hModule);
			g_hModule = (HMODULE)hModule;
			break;

		case DLL_PROCESS_DETACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	}

	return TRUE;
}

STDAPI DllCanUnloadNow(){

	if(!ClassFactory::IsLocked()){
		return S_OK;
	}
	else{
		return S_FALSE;
	}
}

STDAPI DllRegisterServer(){

	HRESULT hr = S_OK;

	if(SUCCEEDED(hr = RegisterObject(g_hModule, CLSID_CustomVideoMixer, SZ_DECODER_NAME, TEXT("Both")))){

		MFT_REGISTER_TYPE_INFO InputTypesInfo[] = { { MFMediaType_Video, MFVideoFormat_NV12 }, { MFMediaType_Video, MFVideoFormat_AYUV } };
		MFT_REGISTER_TYPE_INFO OutputTypesInfo[] = { { MFMediaType_Video, MFVideoFormat_RGB32 } };

		hr = MFTRegister(CLSID_CustomVideoMixer, MFT_CATEGORY_VIDEO_DECODER, const_cast<LPWSTR>(SZ_DECODER_NAME), 0, ARRAY_SIZE(InputTypesInfo),
			InputTypesInfo, ARRAY_SIZE(OutputTypesInfo), OutputTypesInfo, NULL);
	}

	return hr;
}

STDAPI DllUnregisterServer(){

	UnregisterObject(CLSID_CustomVideoMixer);
	MFTUnregister(CLSID_CustomVideoMixer);

	return S_OK;
}

STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void** ppv){

	ClassFactory* pFactory = NULL;
	
	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
	
	for(DWORD index = 0; index < g_numClassFactories; index++){

		if(*g_ClassFactories[index].pclsid == clsid){

			pFactory = new (std::nothrow)ClassFactory(g_ClassFactories[index].pfnCreate);

			if(pFactory){
				hr = S_OK;
			}
			else{
				hr = E_OUTOFMEMORY;
			}
			break;
		}
	}

	if(SUCCEEDED(hr)){
		hr = pFactory->QueryInterface(riid, ppv);
	}

	SAFE_RELEASE(pFactory);
	
	return hr;
}