//----------------------------------------------------------------------------------------------
// MFClassFactory.h
//----------------------------------------------------------------------------------------------
#ifndef MFCLASSFACTORY_H
#define MFCLASSFACTORY_H

typedef HRESULT (*CreateInstanceFn)(IUnknown*, REFIID, void**);

struct ClassFactoryData{
		
		const GUID* pclsid;
		CreateInstanceFn pfnCreate;
};

class ClassFactory : public IClassFactory{

  private:
				
				volatile long           m_refCount;     // Reference count.
				static volatile long    m_serverLocks;  // Number of server locks

				CreateInstanceFn        m_pfnCreation;  // Function to create an instance of the object.

  public:

				ClassFactory(CreateInstanceFn pfnCreation) : m_pfnCreation(pfnCreation), m_refCount(1){}

				static bool IsLocked(){ return (m_serverLocks != 0); }

				STDMETHODIMP_(ULONG) AddRef(){ return InterlockedIncrement(&m_refCount); }
				STDMETHODIMP_(ULONG) Release(){
						
						assert(m_refCount >= 0);
						ULONG uCount = InterlockedDecrement(&m_refCount);
						
						if(uCount == 0){
								delete this;
						}
						
						return uCount;
				}
				
				STDMETHODIMP QueryInterface(REFIID riid, void** ppv){
						
						if(NULL == ppv){
								return E_POINTER;
						}
						else if(riid == __uuidof(IUnknown)){
								*ppv = static_cast<IUnknown*>(this);
						}
						else if(riid == __uuidof(IClassFactory)){
								*ppv = static_cast<IClassFactory*>(this);
						}
						else{
								*ppv = NULL;
								return E_NOINTERFACE;
						}
						
						AddRef();
						return S_OK;
				}

				STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv){
						
						if(pUnkOuter != NULL){
								
								if(riid != __uuidof(IUnknown)){
										return E_NOINTERFACE;
								}
						}

						return m_pfnCreation(pUnkOuter, riid, ppv);
				}

				STDMETHODIMP LockServer(BOOL lock){   
						
						if(lock){
								LockServer();
						}
						else{
								UnlockServer();
						}
						return S_OK;
				}

				static void LockServer(){ InterlockedIncrement(&m_serverLocks); }
				static void UnlockServer(){ InterlockedDecrement(&m_serverLocks); }
};

class BaseObject{

  public:
				
				BaseObject(){ ClassFactory::LockServer(); }
				virtual ~BaseObject(){ ClassFactory::UnlockServer(); }
};

class RefCountedObject{

  protected:
				
				volatile long   m_refCount;

  public:
				
				RefCountedObject() : m_refCount(1){}
				virtual ~RefCountedObject(){ assert(m_refCount == 0); }

				ULONG AddRef(){ return InterlockedIncrement(&m_refCount); }
				ULONG Release(){
						
						assert(m_refCount > 0);
						
						ULONG uCount = InterlockedDecrement(&m_refCount);
						
						if(uCount == 0){
								delete this;
						}
						return uCount;
				}
};

#define DEFINE_CLASSFACTORY_SERVER_LOCK  volatile long ClassFactory::m_serverLocks = 0;

#endif