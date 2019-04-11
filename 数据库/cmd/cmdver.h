#ifndef __VERSION_H__
#define __VERSION_H__
#pragma once
 
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0501
#else
#if _WIN32_WINNT < 0x501
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#endif

#if _WIN32_WINNT < 0x0501
	#undef _WIN32_WINNT
	#define _WIN32_WINNT 0x0501
#endif

#ifndef __ATLBASE_H__
#include <atlbase.h>
#endif

#include <windows.h>
#include <winuser.h>
#pragma comment(lib,"Version.lib")

class CSystemVersion
{
public:
	CSystemVersion() {}
public:
	// ÅÐ¶Ï²Ù×÷Æ½Ì¨X86/X64/IA64
	bool IsX86() const
	{
		return GetSystemInfo()->wProcessorArchitecture == 0;
	}
	bool IsAMD64() const
	{
		return GetSystemInfo()->wProcessorArchitecture == 9;
	}
	bool IsIA64() const
	{
		return GetSystemInfo()->wProcessorArchitecture == 6;
	}

	bool IsWindowsServer() const
	{
		return IsServer2003() || GetVersion()->wProductType != VER_NT_WORKSTATION;
	}
	bool IsWindowWorkstation() const
	{
		return GetVersion()->wProductType == VER_NT_WORKSTATION;
	}

	bool IsWindows8() const
	{
		return VersionVerify(6, 2);
	}
	bool IsWindows81() const
	{
		return VersionVerify(6, 3);
	}
	bool IsWindows10() const
	{
		return VersionVerifyEx(10, 0);
	}

	bool IsWindows8OrLast()
	{
		return IsWindows8() || IsWindows81() || IsWindows10(); 
	}

	bool VersionVerify(DWORD Major, DWORD Minor) const
	{
		OSVERSIONINFOEX osvi;
		DWORDLONG dwlConditionMask = 0;
		int op = VER_EQUAL;

		// Initialize the OSVERSIONINFOEX structure.
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		osvi.dwMajorVersion = Major;
		osvi.dwMinorVersion = Minor;

		// Initialize the condition mask.

		VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, op);
		VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, op);

		// Perform the test.
		return VerifyVersionInfo(
			&osvi,
			VER_MAJORVERSION | VER_MINORVERSION,
			dwlConditionMask) ? true : false;
	}

	bool VersionVerifyEx(DWORD Major, DWORD Minor) const
	{
		typedef LONG(__stdcall* PFN_RtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation);
		RTL_OSVERSIONINFOEXW verInfo = { 0 };
		verInfo.dwOSVersionInfoSize = sizeof(verInfo);

		PFN_RtlGetVersion FN_RtlGetVersion = (PFN_RtlGetVersion)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");

		if (FN_RtlGetVersion != 0 && FN_RtlGetVersion((PRTL_OSVERSIONINFOW)&verInfo) == 0) {
			return verInfo.dwMajorVersion == Major && verInfo.dwMinorVersion >= Minor;
		}
		return false;
	}

	bool IsWindows2012() const
	{
		POSVERSIONINFOEX p = GetVersion();
		return p->dwMajorVersion == 6 && p->dwMinorVersion == 2 && p->wProductType != VER_NT_WORKSTATION;
	}
	bool IsWindows2012R2() const
	{
		POSVERSIONINFOEX p = GetVersion();
		return p->dwMajorVersion == 6 && p->dwMinorVersion == 3 && p->wProductType != VER_NT_WORKSTATION;
	}
	bool IsWindows7() const
	{
		POSVERSIONINFOEX p = GetVersion();
		return p->dwMajorVersion == 6 && p->dwMinorVersion == 1 && p->wProductType == VER_NT_WORKSTATION;
	}
	bool IsVista() const
	{
		POSVERSIONINFOEX p = GetVersion();
		return p->dwMajorVersion == 6 && p->dwMinorVersion == 0 && p->wProductType == VER_NT_WORKSTATION;
	}
	bool IsWindows2008() const
	{
		POSVERSIONINFOEX p = GetVersion();
		return p->dwMajorVersion == 6 && p->dwMinorVersion == 0 && p->wProductType != VER_NT_WORKSTATION;
	}
	bool IsWindows2008R2() const
	{
		POSVERSIONINFOEX p = GetVersion();
		return p->dwMajorVersion == 6 && p->dwMinorVersion == 1 && p->wProductType != VER_NT_WORKSTATION;
	}
	bool IsWindows2003() const
	{
		POSVERSIONINFOEX p = GetVersion();
		return p->dwMajorVersion == 5 && p->dwMinorVersion == 2 && GetSystemMetrics(89 /*SM_SERVERR2*/) == 0;
	}
	bool IsWindows2003R2() const
	{
		POSVERSIONINFOEX p = GetVersion();
		return p->dwMajorVersion == 5 && p->dwMinorVersion == 2 && GetSystemMetrics(89 /*SM_SERVERR2*/) != 0;
	}

	bool IsWindowsXP() const
	{
		POSVERSIONINFOEX p = GetVersion();
		return p->dwMajorVersion == 5 && p->dwMinorVersion == 1;
	}
	bool IsWindowsXP64() const
	{
		POSVERSIONINFOEX p = GetVersion();
		return p->dwMajorVersion == 5 && p->dwMinorVersion == 1 && p->wProductType == VER_NT_WORKSTATION && IsAMD64();
	}
	bool IsWindows2000() const
	{
		POSVERSIONINFOEX p = GetVersion();
		return p->dwMajorVersion == 5 && p->dwMinorVersion == 0;
	}

	bool IsServer2003() const
	{
		return IsWindows2003() || IsWindows2003R2();
	}
	bool IsServer2008() const
	{
		return IsWindows2008() || IsWindows2008R2();
	}
	bool IsServer2012() const
	{
		return IsWindows2012();
	}

	bool IsWindows2000ORLast() const
	{
		return GetVersion()->dwMajorVersion >= 5;
	}
	bool IsWindowsXPORLast() const
	{
		POSVERSIONINFOEX p = GetVersion();
		return p->dwMajorVersion > 5 || (p->dwMajorVersion == 5 && p->dwMinorVersion >= 1);
	}
	bool IsVistaORLast() const
	{
		return GetVersion()->dwMajorVersion >= 6;
	}
	bool IsWindow7ORLast() const
	{
		POSVERSIONINFOEX p = GetVersion();
		return p->dwMajorVersion > 6 || (p->dwMajorVersion == 6 && p->dwMinorVersion >= 1);
	}
	WORD GetServicePack(WORD* pdwMinor = NULL)
	{
		POSVERSIONINFOEX p = GetVersion();
		if (pdwMinor) {
			*pdwMinor = p->wServicePackMinor;
		}
		return p->wServicePackMajor;
	}
	void GetOSVersino(DWORD& Major, DWORD& Minor, WORD& spMajor, WORD& spMinor, LPTSTR lpszServicePack = 0)
	{
		POSVERSIONINFOEX p = GetVersion();

		Major = p->dwMajorVersion;
		Minor = p->dwMinorVersion;
		spMajor = p->wServicePackMajor;
		spMinor = p->wServicePackMinor;
		if (lpszServicePack) {
			lstrcpyn(lpszServicePack, p->szCSDVersion, 128);
		}
	}

	int GetVersionIndex()
	{
		if (IsWindows2000())
			return 1;
		else if (IsWindowsXP())
			return 2;
		else if (IsVista())
			return 3;
		else if (IsWindows7())
			return 4;
		else if (IsWindows10())
			return 13;
		else if (IsWindows81())
			return 6;
		else if (IsWindows2012R2())
			return 12;
		else if (IsWindows8())
			return 5;
		else if (IsWindows2012())
			return 11;
		else if (IsWindows2003())
			return 7;
		else if (IsWindows2003R2())
			return 8;
		else if (IsWindows2008())
			return 9;
		else if (IsWindows2008R2())
			return 10;

		return 0;
	}

	LPCTSTR GetVersionString(int nIndex, LPTSTR pszVersion, DWORD cbSize)
	{
		static LPCTSTR lpszVer[] = {
			{ _T("Windows 98/ME") },
			{ _T("Windows 2000") },
			{ _T("Windows XP") },
			{ _T("Windows Vista") },
			{ _T("Windows 7") },
			{ _T("Windows 8") },
			{ _T("Windows 8.1") },
			{ _T("Windows Server 2003") },
			{ _T("Windows Server 2003 R2") },
			{ _T("Windows Server 2008") },
			{ _T("Windows Server 2008 R2") },
			{ _T("Windows Server 2012") },
			{ _T("Windows Server 2012 R2") },
			{ _T("Windows 10") }
		};

		if (nIndex < 0 || nIndex > 13)
			nIndex = GetVersionIndex();

		if (pszVersion) {
			lstrcpyn(pszVersion, lpszVer[nIndex], cbSize);
			return pszVersion;
		}
		else return lpszVer[nIndex];
	}

	LPCTSTR GetCurrentVersionString()
	{
		return GetVersionString(-1, NULL, NULL);
	}


	int GetX86Index()
	{
		if (IsAMD64())
			return 1;
		else if (IsIA64())
			return 2;
		return 0;
	}

	LPCTSTR GetX86String(int nIndex, LPTSTR pszX86, DWORD cbSize)
	{
		static LPCTSTR pszXPLAT[] = {
			{ _T("X86") },
			{ _T("X64") },
			{ _T("IA64") }
		};

		if (nIndex < 0 || nIndex > 2)
			nIndex = GetX86Index();

		if (pszX86) {
			lstrcpyn(pszX86, pszXPLAT[nIndex], cbSize);
			return pszX86;
		}
		return pszXPLAT[nIndex];
	}

	LPCTSTR GetCurrentX86String()
	{
		return GetX86String(-1, NULL, NULL);
	}
public:
	static LPSYSTEM_INFO GetSystemInfo()
	{
		static SYSTEM_INFO si = { 0 };
		if (!si.dwOemId) {
			GetNativeSystemInfo(&si);
		}
		return &si;
	}
	static POSVERSIONINFOEX GetVersion()
	{
		static OSVERSIONINFOEX osvi = { 0 };
		if (!osvi.dwOSVersionInfoSize) {
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
			GetVersionEx((OSVERSIONINFO*)&osvi);
		}
		return &osvi;
	}
};


namespace VER {
	class CCustomVersion {
	public:
		CCustomVersion()
			: Major(Ver[0])
			, Minor(Ver[1])
			, Build(Ver[2])
			, Revision(Ver[3])
		{}
		
		CCustomVersion(DWORD v1, DWORD v2, DWORD v3 = 0, DWORD v4 = 0)
			: CCustomVersion()
		{
			Ver[0] = v1; Ver[1] = v2; Ver[2] = v2; Ver[3] = v4;
		}

		CCustomVersion(const char* strVer)
			: CCustomVersion()
		{
			ParserVersion(strVer, Major, Minor, Build, Revision); 
		}

		CCustomVersion(const CCustomVersion& obj) 
			: CCustomVersion()
		{
			if (this == &obj)
				return; 
			for (int i = 0; i < 4; i++) {
				Ver[i] = obj.Ver[i];
			}
		}

		CCustomVersion& operator=(const char* strVer)
		{
			ParserVersion(strVer, Major, Minor, Build, Revision);
			return *this; 
		}

		int Compare(const CCustomVersion& obj)
		{
 			for (int i = 0; i < 4; i++) {
				if (Ver[i] > obj.Ver[i])
					return 1;
				else if (Ver[i] < obj.Ver[i])
					return -1;
			}
			return 0; 
		}

		const char* ToString(LPSTR pszOut) {
			wsprintfA(pszOut, "%d.%d.%d.%d", Ver[0], Ver[1], Ver[2], Ver[3]); 
			return pszOut; 
		}

		const char* toString()
		{
			static char _ver[64] = { 0 }; 
			return ToString(_ver); 
		}

 	public:
		bool operator==(const CCustomVersion& obj) { return Compare(obj) == 0; }
		bool operator>(const CCustomVersion& obj) { return Compare(obj) > 0; }
		bool operator<(const CCustomVersion& obj) { return Compare(obj) < 0; }
		bool operator>=(const CCustomVersion& obj) { return Compare(obj) >= 0; }
		bool operator<=(const CCustomVersion& obj) { return Compare(obj) <= 0; }
 	public:
		static void ParserVersion(const char* strVer, DWORD& v1, DWORD& v2, DWORD& v3, DWORD& v4)
		{
			if (!strVer) {
				v1 = v2 = v3 = v4 = 0;
 				return; 
			}

			DWORD s[4] = { 0 }; 
			int n = 0;
			for (const char* p = strVer; *p; p++) {
				if (*p >= '0' && *p <= '9') {
					s[n] = s[n] * 10 + *p - '0';
				}
				else if (*p == '.') {
					if (++n > 3)
						break;
				}
			}
			v1 = s[0]; v2 = s[1]; v3 = s[2]; v4 = s[3];
 		}
	public:
		DWORD& Major;
		DWORD& Minor;
		DWORD& Build;
		DWORD& Revision;
	protected:
		DWORD Ver[4]; 
	};
}

class CFileVersion : public VER::CCustomVersion {
public:
	CFileVersion() 
		: VER::CCustomVersion()
	{}

	CFileVersion(LPCTSTR lpszFile)
		: CFileVersion()
	{
		VER_GetFileVersion(lpszFile, &Major, &Minor, &Build, &Revision);
	}
	CFileVersion(HMODULE hModule)
		: CFileVersion()
	{
		GetModuleVersion(hModule); 
	}
public:
	BOOL GetModuleVersion(HMODULE hModule)
	{
		TCHAR szModule[1024] = { 0 }; 
		GetModuleFileName(hModule, szModule, 1024); 
		return VER_GetFileVersion(szModule, &Major, &Minor, &Build, &Revision);
	}
public:
	static BOOL VER_GetFileVersion(LPCTSTR lpcszFile, DWORD *pMajor, DWORD *pMinor, DWORD *pBuild, DWORD *pRevision)
	{
		DWORD dwHandle = 0;
		DWORD dwVerInfoSize = GetFileVersionInfoSize(lpcszFile, &dwHandle);
		BOOL fResult = FALSE;
		if (dwVerInfoSize) {
			LPVOID lpBuffer = LocalAlloc(LPTR, dwVerInfoSize);
			if (lpBuffer) {
				if (GetFileVersionInfo(lpcszFile, dwHandle, dwVerInfoSize, lpBuffer))
				{
					VS_FIXEDFILEINFO * pFixedInfo = NULL;
					UINT nFixedSize = 0;
					fResult = VerQueryValue(lpBuffer, TEXT("\\"), (LPVOID*)&pFixedInfo, &nFixedSize);
					if (fResult && nFixedSize)
					{
						*pMajor = HIWORD(pFixedInfo->dwFileVersionMS);
						*pMinor = LOWORD(pFixedInfo->dwFileVersionMS);
						*pBuild = HIWORD(pFixedInfo->dwFileVersionLS);
						*pRevision = LOWORD(pFixedInfo->dwFileVersionLS);
						fResult = TRUE;
					}
				}
				LocalFree(lpBuffer);
			}
		}
		return fResult;
	}
}; 

class CIEVersion : public CFileVersion
{
public:
	CIEVersion() : CFileVersion()
	{
		VER_GetCurrentIEVersion(&Major,&Minor,&Build,&Revision);
	}
public:
	bool IsSucceeded() const { return Major > 0; }
	bool IsIE55() const {return Major == 5 && Minor == 5; }
	bool IsIE6() const { return Major == 6; }
	bool IsIE7() const { return Major == 7; }
	bool IsIE8() const { return Major == 8; }
	bool IsIE9() const { return Major == 9; }
	bool IsIE10() const { return Major == 10; }
	bool IsIE11() const { return Major == 11; }
	bool IsIE6OrLast() const { return Major >= 6; }
	bool IsIE7OrLast() const { return Major >= 7; }
	bool IsIE8OrLast() const { return Major >= 8; }
	bool IsIE9OrLast() const { return Major >= 9; }
public:
 	static BOOL VER_GetCurrentIEVersion(DWORD *pMajor,DWORD *pMinor, DWORD *pBuild, DWORD *pRevision)
	{
		return VER_GetFileVersion(_T("mshtml.dll"),pMajor,pMinor,pBuild,pRevision);
	}
};

#if 1
#define IE_COMPATIBLE_KEY	_T("SOFTWARE\\Microsoft\\Internet Explorer\\MAIN\\FeatureControl\\FEATURE_BROWSER_EMULATION")
#define IE64_COMPATIBLE_KEY _T("SOFTWARE\\Wow6432Node\\Microsoft\\Internet Explorer\\MAIN\\FeatureControl\\FEATURE_BROWSER_EMULATION") 
#include <Urlmon.h>
#pragma comment(lib,"Urlmon.lib")



namespace Misc {
	ATL_NOINLINE inline BOOL  IsValidIECompatibleVersion(DWORD nVersion) 
	{
		static DWORD vars[] = {7000,8000,8888,9000,9999,10000,10001,11000,11001,12000,12001,0}; 
		for (int i = 0; vars[i] > 0 ; i++ ) {
			if (vars[i] == nVersion)
				return TRUE; 
		}
		return FALSE; 
	}

	ATL_NOINLINE inline VOID  RefreshRegisterTable()
	{
		WinExec("ASSOC .AV=AVFILE>NUL 2>NUL", SW_HIDE);
	}

	ATL_NOINLINE inline DWORD CurrentIECompatibleVersion()
	{
		CIEVersion ver; 
		if (ver.IsIE8OrLast()) {
			switch(ver.Major) {
			case 8: return 8000; 
			case 9: return 9000; 
			default: return ver.Major * 1000 + 1; 
			}; 
		}
		return 0; 
	}

	ATL_NOINLINE inline DWORD GetIECompatibleVersion(LPCTSTR lpszModule)
	{
		if (!lpszModule) {
			static TCHAR szModule[1024] = {0}; 
			::GetModuleFileName(NULL, szModule, 1024); 
			lpszModule = szModule; 
		}

		LPCTSTR pszExe = ::PathFindFileName(lpszModule); 
		DWORD cbVersion = 0; 
		CRegKey regKey;
		if (ERROR_SUCCESS == regKey.Create(HKEY_LOCAL_MACHINE,IE_COMPATIBLE_KEY)) {
			regKey.QueryDWORDValue(pszExe, cbVersion);
		}

		return cbVersion; 
	}

	ATL_NOINLINE inline BOOL  IsIECompatible(LPCTSTR lpszModule, DWORD cbVersion )
	{
		return GetIECompatibleVersion(lpszModule) == cbVersion ? TRUE : FALSE;
	}


	ATL_NOINLINE inline BOOL  SetIECompatibleVersion(LPCTSTR lpszModule, DWORD cbVersion )
	{
		if (!lpszModule) {
			static TCHAR szModule[1024] = {0}; 
			if (!(*szModule)) {
				::GetModuleFileName(NULL, szModule, 1024); 
			}
			lpszModule = szModule; 
		}
		if (!cbVersion) {
			cbVersion = CurrentIECompatibleVersion(); 
		}

		LPCTSTR pszExe = ::PathFindFileName(lpszModule); 
		if (IsIECompatible(pszExe, cbVersion))
			return TRUE; 

		CRegKey regKey;
		LONG lResult = regKey.Create(HKEY_LOCAL_MACHINE, IE_COMPATIBLE_KEY);
		if (lResult != ERROR_SUCCESS)
			return FALSE;

		lResult = regKey.SetDWORDValue(pszExe, cbVersion);
		if (lResult != ERROR_SUCCESS)
			return FALSE;

		typedef void (WINAPI *LPFN_PGNSI)(LPSYSTEM_INFO);
		static LPFN_PGNSI pGNSI = NULL;
		if (!pGNSI) {
			pGNSI = (LPFN_PGNSI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
		}
		if (!pGNSI)
			return TRUE;

		SYSTEM_INFO si = { 0 };
		pGNSI(&si);
		if (si.wProcessorArchitecture != 0) { // X64 OR IA64
			CRegKey regKey;
			if (ERROR_SUCCESS == regKey.Open(HKEY_LOCAL_MACHINE, IE64_COMPATIBLE_KEY)) {
				regKey.SetDWORDValue(pszExe, cbVersion);
			}
		}

		WinExec("ASSOC .AV=AVFILE>NUL 2>NUL", SW_HIDE);
		return TRUE;

	}
	ATL_NOINLINE inline BOOL  SetCurrentModuleIECompatibleVersion(DWORD cbVersion)
	{
		return SetIECompatibleVersion(NULL, cbVersion);
	}

	inline bool UpdateCurrentProcessIEMode()
	{
		return SetCurrentModuleIECompatibleVersion(CurrentIECompatibleVersion()) ? true : false;
	}

	ATL_NOINLINE inline BOOL  SetUserAgent(LPCTSTR pszUserAgent)
	{
		LPCSTR lpszUserAgent = CT2A(pszUserAgent);
		return SUCCEEDED(::UrlMkSetSessionOption(URLMON_OPTION_USERAGENT, (LPVOID)lpszUserAgent, lstrlenA(lpszUserAgent), 0));
	}

	ATL_NOINLINE inline DWORD GetUserAgent(LPTSTR pszUserAgent, DWORD cbSize)
	{
		CHAR szBuffer[1024] = { 0 };
		DWORD cbBuffer = 0;
		HRESULT hr = ::UrlMkGetSessionOption(URLMON_OPTION_USERAGENT, (LPVOID)szBuffer, 1024, &cbBuffer, 0);
		if (SUCCEEDED(hr)) {
			lstrcpyn(pszUserAgent, CA2T(szBuffer), cbSize);
			cbBuffer = lstrlen(pszUserAgent);
		}
		return cbBuffer;
	}


	struct RegKeyMap {
		HKEY RootKey;
		LPCTSTR SubKey;
		LPCTSTR ValueName;
	};

	struct RegKeyExMap {
		HKEY RootKey;
		LPCTSTR SubKey;
		LPCTSTR ValueName;
		LPCTSTR Value;
	};

	class CBrowserRegister {
	public:
		CBrowserRegister(LPTSTR lpszProc = NULL)
			: sProc(lpszProc)
		{
			sExe = ::PathFindFileName(sProc); 
			sAppName = sExe; 
			::PathRemoveExtension(sAppName.GetBuffer()); 
			sAppName.ReleaseBuffer(); 
		}
 
		bool IsDefaultBrowser()
		{
			if (sProgID == "") {
				ATLASSERT(FALSE); 
				return false; 
			}
			CRegKey regKey; 
			if (ERROR_SUCCESS == regKey.Open(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice"), KEY_READ)) {
				CString sID; 
				DWORD cbSize = 128;
				regKey.QueryStringValue(_T("ProgId"), sID.GetBufferSetLength(128), &cbSize);
				sID.ReleaseBuffer(); 
				return sID == sProgID; 
			}
			return false; 
 		}

		bool SetDefaultBrowser()
		{
			if (sProc == _T("") || sProgID == _T("") || sAppName == _T("")) {
				ATLASSERT(FALSE); 
				return false; 
			}

			bool result = false;
			CSystemVersion sysVer;
			if (sysVer.IsWindows8OrLast())
			{
				result = RegisterProgID();
				if (result) {
					result = RegisterApp();
				}
				if (result) {
					result = RegsiterProtcol();
				}
			}
			else {
				result = RegisterProgID();
				if (result) {
					result = RegisterApp();
				}
				RegisterWin7();
			}
			RefreshRegisterTable();
			return result; 
		}

		CString GetDefaultBrowser()
		{
			CString sProcess, sPID;
			CRegKey regKey;
 			if (ERROR_SUCCESS == regKey.Open(HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice"))) {
				DWORD cbSize = 128;
				regKey.QueryStringValue(_T("ProgId"), sPID.GetBufferSetLength(128), &cbSize);
				sPID.ReleaseBuffer(); 
			};

			if (sPID == _T("")) {
				return _T("");
			}

			if (sPID.Left(1) == _T("\"")) {
				sProcess = sPID; 
			}
			else {
				CString sKey; 
				sKey.Format(_T("%s\\Shell\\open\\command"), sProgID);
				CRegKey regkey;

				if (ERROR_SUCCESS == regkey.Open(HKEY_CLASSES_ROOT, sKey, KEY_READ)) {
					DWORD cbSize(1024); 
					regkey.QueryStringValue(_T(""), sProcess.GetBufferSetLength(1024), &cbSize);
					sProcess.ReleaseBuffer(); 
				}
			}
			if (sProcess.Left(1) == _T("\"")) {
				int cbLength = sProcess.GetLength(); 
				TCHAR szBuffer[1024] = { 0 }; 
				LPTSTR szBrowser = sProcess.GetBuffer(); 
				int index = 0, count = 0;
				for (int i = 0; i < cbLength; i++){
					if (szBrowser[i] ==_T('"')){
						count++;
						if (count == 2){
							break;
						}
						continue;
					}
					szBuffer[index++] = szBrowser[i];
				}
				sProcess.ReleaseBuffer(); 
				sProcess = szBuffer; 
			}
			return sProcess; 
		}

	protected:
 		bool RegisterApp()
		{
			LPCTSTR Keys[] = {
				_T("SOFTWARE\\Clients\\StartMenuInternet\\%s"),
				_T("SOFTWARE\\Clients\\StartMenuInternet\\%s\\Capabilities"),
				_T("SOFTWARE\\Clients\\StartMenuInternet\\%s\\Capabilities\\FileAssociations"),
				_T("SOFTWARE\\Clients\\StartMenuInternet\\%s\\Capabilities\\Startmenu"),
				_T("SOFTWARE\\Clients\\StartMenuInternet\\%s\\Capabilities\\UrlAssociations"),
				_T("SOFTWARE\\Clients\\StartMenuInternet\\%s\\Shell"),
				_T("SOFTWARE\\Clients\\StartMenuInternet\\%s\\Shell\\open"),
				_T("SOFTWARE\\Clients\\StartMenuInternet\\%s\\Shell\\open\\command"),
				_T("SOFTWARE\\Clients\\StartMenuInternet\\%s\\DefaultIcon")
			};

			CRegKey regKeys[sizeof(Keys) / sizeof(LPCSTR)];

			for (int i = 0; i < sizeof(Keys) / sizeof(LPCSTR); i++) {
				CString sKey;
				sKey.Format(Keys[i], sExe);
				regKeys[i].Create(HKEY_LOCAL_MACHINE, sKey);
			}

			regKeys[0].SetStringValue(NULL, sAppDesc); 
			regKeys[0].SetStringValue(_T("ApplicationDescription"), sAppDesc);
			regKeys[0].SetStringValue(_T("ApplicationName"), sAppName);
			regKeys[1].SetStringValue(_T("ApplicationDescription"), sAppDesc);
			regKeys[1].SetStringValue(_T("ApplicationName"), sAppDesc);

			LPCTSTR cap_names[] = { _T(".htm"), _T(".html"), _T(".mht"), _T(".mhtml") };
			for (int i = 0; i < sizeof(cap_names) / sizeof(LPCSTR); i++) {
				regKeys[2].SetStringValue(cap_names[i], sProgID);
			}

			regKeys[3].SetStringValue(_T("StartmenuInternet"), sAppName);

			LPCTSTR url_ass[] = { _T("http"), _T("https") };
			for (int i = 0; i < sizeof(url_ass) / sizeof(LPCSTR); i++) {
				regKeys[4].SetStringValue(url_ass[i], sProgID);
			}

			regKeys[5].SetStringValue(NULL, sAppName);
			regKeys[7].SetStringValue(NULL, sProc);

			CString icon = sProc; 
			icon += _T(",0");
			regKeys[8].SetStringValue(NULL, icon);

			bool result = ((HKEY)regKeys[7]) != NULL;
			{
				CRegKey regKey;
				regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Clients\\StartMenuInternet"));
				regKey.SetStringValue(NULL, sAppName);
			}
			if (result) {
				CRegKey regKey;
				DWORD cbResult = regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\RegisteredApplications"));
				if (result = (cbResult == ERROR_SUCCESS)) {
					TCHAR szValue[1024] = { 0 };
					wnsprintf(szValue, 1024, _T("SOFTWARE\\Clients\\StartMenuInternet\\%s\\Capabilities"), sExe);
					result = (ERROR_SUCCESS == regKey.SetStringValue(sAppName, szValue));
				}
			}
			return result; 
		}

		bool RegisterProgID()
		{
			LPCTSTR Keys[] = {
				_T("%s"),
				_T("%s\\DefaultIcon"),
				_T("%s\\Shell"),
				_T("%s\\Shell\\open"),
				_T("%s\\Shell\\open\\command")
 			};
			CRegKey regKeys[sizeof(Keys) / sizeof(LPCTSTR)];

			for (int i = 0; i < sizeof(Keys) / sizeof(LPCSTR); i++) {
				CString sKey;
				sKey.Format(Keys[i], sProgID);
				regKeys[i].Create(HKEY_CLASSES_ROOT, sKey);
			}

			CString icon = sProc;
			icon += _T(",0");
			regKeys[1].SetStringValue(NULL, icon);
			regKeys[2].SetStringValue(NULL, _T("open"));

			CString sCmd; 
			sCmd.Format(_T("\"%s\" \"%%1\""), sProc);
			regKeys[4].SetStringValue(NULL, sCmd); 
			return ((HKEY)regKeys[4]) != NULL;
		}

		bool RegsiterProtcol()
		{
			RegKeyMap Maps[] = {
				{ HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice"), NULL },
				{ HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https\\UserChoice"), NULL },
				{ HKEY_CURRENT_USER, _T("Software\\Classes\\HTTP\\shell\\open\\command"), NULL },
				{ HKEY_CURRENT_USER, _T("Software\\Classes\\HTTPS\\shell\\open\\command"), NULL },
				{ HKEY_CURRENT_USER, _T("Software\\Classes\\IE.HTTP\\shell\\open\\command"), NULL },
				{ HKEY_CURRENT_USER, _T("Software\\Classes\\IE.HTTPS\\shell\\open\\command"), NULL },
				{ HKEY_CURRENT_USER, _T("Software\\Classes\\htmlfile\\shell\\open\\command"), NULL },
				{ HKEY_CURRENT_USER, _T("Software\\Classes\\mhtmlfile\\shell\\open\\command"), NULL },
				{ HKEY_CLASSES_ROOT, _T("IE.AssocFile.HTM\\shell\\open\\command"), NULL },
				{ HKEY_CLASSES_ROOT, _T("IE.AssocFile.HTML\\shell\\open\\command"), NULL },
				{ HKEY_CLASSES_ROOT, _T("http\\shell\\open\\command"), NULL },
				{ HKEY_CLASSES_ROOT, _T("htmlfile\\shell\\open\\command"), NULL },
				{ HKEY_CLASSES_ROOT, _T("mhtmlfile\\shell\\open\\command"), NULL },
				{ HKEY_CLASSES_ROOT, _T("InternetShortcut\\shell\\open\\command"), NULL }

			};
			
			RegKeyExMap Maps1[] = {
				{ HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice"), _T("ProgId"), sProgID },
				{ HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https\\UserChoice"), _T("ProgId"), sProgID },
				{ HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\Roaming\\OpenWith\\UrlAssociations\\http\\UserChoice"), _T("ProgId"), sProgID },
				{ HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\Roaming\\OpenWith\\UrlAssociations\\https\\UserChoice"), _T("ProgId"), sProgID }
			};

			CString sCmd;
			sCmd.Format(_T("\"%s\" \"%%1\""), sProc);

			for (int i = 0; i < sizeof(Maps) / sizeof(RegKeyMap); i++) {
				CRegKey regKey;
				DWORD cbResult = regKey.Open(Maps[i].RootKey, Maps[i].SubKey);
				if (ERROR_SUCCESS == cbResult) {
					cbResult = regKey.SetStringValue(Maps[i].ValueName, sCmd);
				}
			}
			for (int i = 0; i < sizeof(Maps1) / sizeof(RegKeyExMap); i++) {
				CRegKey regKey;
				DWORD cbResult = regKey.Open(Maps1[i].RootKey, Maps1[i].SubKey);
				if (ERROR_SUCCESS == cbResult) {
					cbResult = regKey.SetStringValue(Maps1[i].ValueName, Maps1[i].Value);
				}
 			}
			return true; 
		}

		bool RegisterWin7()
		{
			RegKeyMap Maps[] = {
				{ HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice"), NULL },
				{ HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https\\UserChoice"), NULL },
				{ HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\Roaming\\OpenWith\\UrlAssociations\\http\\UserChoice"), NULL },
				{ HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Windows\\Roaming\\OpenWith\\UrlAssociations\\https\\UserChoice"), NULL },
				{ HKEY_CLASSES_ROOT, _T("http\\shell\\open\\command"), NULL },
				{ HKEY_CLASSES_ROOT, _T("htmlfile\\shell\\open\\command"), NULL },
				{ HKEY_CLASSES_ROOT, _T("mhtmlfile\\shell\\open\\command"), NULL },
				{ HKEY_CLASSES_ROOT, _T("InternetShortcut\\shell\\open\\command"), NULL }
			};

	 		CString sCmd;
			sCmd.Format(_T("\"%s\" \"%%1\""), sProc);

			for (int i = 0; i < sizeof(Maps) / sizeof(RegKeyMap); i++) {
				CRegKey regKey;
				DWORD cbResult = regKey.Open(Maps[i].RootKey, Maps[i].SubKey);
				if (ERROR_SUCCESS == cbResult) {
					cbResult = regKey.SetStringValue(Maps[i].ValueName, sCmd);
				}
			}

 			return true; 
		}
		
	public:
		CString sProc; 
		CString sExe; 
		CString sAppDesc;
		CString sAppName; 
		CString sProgID; 
	};

	ATL_NOINLINE inline BOOL SetDefaultBrowser(LPCTSTR lpszAppName, LPCTSTR lpszProc = NULL)
	{
		TCHAR szProc[MAX_PATH] = { 0 }; 
		if (!lstrlen(lpszProc)) {
			::GetModuleFileName(NULL, szProc, MAX_PATH);
		}
		else {
			lstrcpyn(szProc, lpszProc, MAX_PATH);
		}
 
		CBrowserRegister reg(szProc); 
		reg.sAppDesc = lpszAppName;
		reg.sProgID = "SLBrowserHTML";
 		return reg.SetDefaultBrowser(); 
	}
	
	ATL_NOINLINE inline BOOL DefaultBrowserIsCurrentProcess()
	{
		TCHAR szModule[MAX_PATH] = { 0 };
		::GetModuleFileName(NULL, szModule, MAX_PATH); 
		CBrowserRegister reg(NULL);
		reg.sProgID = "SLBrowserHTML";
		CString sBrowser = reg.GetDefaultBrowser(); 
		return StrCmpI(szModule, sBrowser) == 0 ? TRUE : FALSE; 
	}
};
#else


namespace Misc
{
	#define FEATURE_BROWSER_KEY  	_T("SOFTWARE\\Microsoft\\Internet Explorer\\MAIN\\FeatureControl\\FEATURE_BROWSER_EMULATION") 
	#define FEATURE_BROWSER64_KEY  	_T("SOFTWARE\\Wow6432Node\\Microsoft\\Internet Explorer\\MAIN\\FeatureControl\\FEATURE_BROWSER_EMULATION") 

	inline void FlushFileReal(LPCSTR lpszExt , LPCSTR lpszExe)
	{
		CHAR szRef[1024] = {0};
		wsprintfA(szRef, "ASSOC %s=\"%s\">NUL 2>NUL",lpszExt, lpszExe);
		WinExec(szRef,SW_HIDE);
	}

	inline void FlushRegTable()
	{
		FlushFileReal(".AV","AVFILE");
	}

	inline bool IsIECompatible(LPCTSTR lpszModule)
	{
		CIEVersion ver;
		if (ver.IsIE8OrLast()) {
			CRegKey regKey;
			if (ERROR_SUCCESS != regKey.Open(HKEY_LOCAL_MACHINE,FEATURE_BROWSER_KEY)) {
				return false;
			}
			DWORD cbValue = 0;
			LRESULT lResult = regKey.QueryDWORDValue(lpszModule,cbValue);
			return (lResult == ERROR_SUCCESS) && (cbValue != 0);
		}
		return true;
	}

	inline bool UpdateIEMode(LPCTSTR lpszModule, bool fUpdateReg = true)
	{
		if (IsIECompatible(lpszModule))
			return true;

		CRegKey regKey;
		LONG lResult = regKey.Create(HKEY_LOCAL_MACHINE,FEATURE_BROWSER_KEY);
		if (lResult != ERROR_SUCCESS)
			return false;

		lResult = regKey.SetDWORDValue(lpszModule,9000);
		if (lResult != ERROR_SUCCESS)
			return false;

		CSystemVersion ver;
		if (!ver.IsX86()) {
			CRegKey regKey;
			if (ERROR_SUCCESS != regKey.Open(HKEY_LOCAL_MACHINE,FEATURE_BROWSER64_KEY)) {
				return false;
			}
			return (ERROR_SUCCESS == regKey.SetDWORDValue(lpszModule,9000));
		}

		if (fUpdateReg) {
			FlushRegTable();
		}
		return true;
	}

	inline bool UpdateCurrentProcessIEMode()
	{
		TCHAR szModule[1024] = {0}, szExe[1024] = {0};
		::GetModuleFileName(NULL,szModule,1024);
		lstrcpy(szExe,::PathFindFileName(szModule));
		::CharLowerBuff(szExe,1024);
		return UpdateIEMode(szExe);
	}
};
#endif

#endif // end of define __VERSION