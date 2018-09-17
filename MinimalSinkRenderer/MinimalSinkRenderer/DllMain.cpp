//----------------------------------------------------------------------------------------------
// DllMain.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HMODULE g_hModule;

DEFINE_CLASSFACTORY_SERVER_LOCK

WCHAR SZ_RENDERER_NAME[] = L"Minimal Sink Renderer";

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

	HRESULT hr;

	hr = RegisterObject(g_hModule, CLSID_MinimalSinkRenderer, SZ_RENDERER_NAME, TEXT("Both"));

	if(SUCCEEDED(hr)){

		MFT_REGISTER_TYPE_INFO tInInfo [] = { { MFMediaType_Video, MFVideoFormat_RGB32 } };

		hr = MFTRegister(CLSID_MinimalSinkRenderer, MFT_CATEGORY_OTHER, SZ_RENDERER_NAME, 0, ARRAY_SIZE(tInInfo), tInInfo, 0, NULL, NULL);
	}

	return hr;
}

STDAPI DllUnregisterServer(){

	UnregisterObject(CLSID_MinimalSinkRenderer);
	MFTUnregister(CLSID_MinimalSinkRenderer);
	return S_OK;
}

STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void** ppv){

	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

	if(clsid == CLSID_MinimalSinkRenderer){

		ClassFactory* pFactory = NULL;

		pFactory = new (std::nothrow)ClassFactory(CMinimalSkinkRenderer::CreateInstance);

		if(pFactory){

			hr = pFactory->QueryInterface(riid, ppv);

			SAFE_RELEASE(pFactory);
		}
		else{

			hr = E_OUTOFMEMORY;
		}
	}

	return hr;
}