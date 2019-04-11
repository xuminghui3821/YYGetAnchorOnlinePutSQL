#ifndef __COM_LIBRARY__
#define __COM_LIBRARY__

typedef HRESULT (WINAPI* PFN_Func)(void);// DllRegisterServer(void)

class CComLibrary {
	HMODULE  hModule;
	PFN_Func pRegister;
	PFN_Func pUnRegister;
public:
	explicit CComLibrary(LPCSTR lpszLib) 
		: hModule(NULL), pRegister(NULL), pUnRegister(NULL)
	{
		hModule = ::LoadLibrary(lpszLib);
		if (hModule) {
			pRegister = (PFN_Func)::GetProcAddress(hModule,"DllRegisterServer");
			pUnRegister = (PFN_Func)::GetProcAddress(hModule,"DllUnregisterServer");
		}
	}
	~CComLibrary() 
	{
		if (pRegister) pRegister = NULL;
		if (pUnRegister) pUnRegister = NULL;
		if (hModule) {
			::FreeLibrary(hModule);
			hModule = NULL;
		}
	}
private:
	CComLibrary(const CComLibrary&) {}
public:
	bool IsCOMLibrary() 
	{
		return (hModule && pRegister);
	}
	bool Register()
	{
		if (pRegister) {
			return SUCCEEDED(pRegister()) ? true : false;
		}
		return false;
	}
	bool UnRegister()
	{
		if (pUnRegister) {
			return SUCCEEDED(pUnRegister()) ? true : false;
		}
		return false;
	}
};

#endif