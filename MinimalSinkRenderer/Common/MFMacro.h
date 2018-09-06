//----------------------------------------------------------------------------------------------
// MFMacro.h
//----------------------------------------------------------------------------------------------
#ifndef MFMACRO_H
#define MFMACRO_H

#ifndef MF_SAFE_RELEASE
#define MF_SAFE_RELEASE
template <class T> inline void SAFE_RELEASE(T*& p){
	
	if(p){
		p->Release();
		p = NULL;
	}
}
#endif

#ifndef MF_SAFE_DELETE
#define MF_SAFE_DELETE
template<class T> inline void SAFE_DELETE(T*& p){
	
	if(p){
		delete p;
		p = NULL;
	}
}
#endif

#ifndef MF_SAFE_DELETE_ARRAY
#define MF_SAFE_DELETE_ARRAY
template<class T> inline void SAFE_DELETE_ARRAY(T*& p){
	
	if(p){
		delete[] p;
		p = NULL;
	}
}
#endif

// Need to use other macro. When error, multiple call.
#ifndef IF_FAILED_RETURN
#if(_DEBUG && MF_USE_LOGGING)
#define IF_FAILED_RETURN(hr) if(FAILED(hr)){ LOG_HRESULT(hr); return hr; }
#else
#define IF_FAILED_RETURN(hr) if(FAILED(hr)){ return hr; }
#endif
#endif

#ifndef IF_FAILED_THROW
#if(_DEBUG && MF_USE_LOGGING)
#define IF_FAILED_THROW(hr) if(FAILED(hr)){ LOG_HRESULT(hr); throw hr; }
#else
#define IF_FAILED_THROW(hr) if(FAILED(hr)){ throw hr; }
#endif
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]) )
#endif

#ifndef RETURN_STRING
#define RETURN_STRING(x) case x: return L#x
#endif

#ifndef IF_EQUAL_RETURN
#define IF_EQUAL_RETURN(param, val) if(val == param) return L#val
#endif

#endif
