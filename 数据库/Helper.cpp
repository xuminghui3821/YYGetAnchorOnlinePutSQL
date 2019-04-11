#include <string>
#include <tchar.h>
#include <vector>
#include <Windows.h>
#include "helper.h"
#include <winhttp.h>
#pragma comment(lib, "Winhttp.lib")
#pragma warning(disable: 4996) 
Helper::Helper(void)
{
}


Helper::~Helper(void)
{
}

bool Helper::Bxp()
{
	OSVERSIONINFO osvi;                                                                        //定义OSVERSIONINFO数据结构对象
	memset(&osvi, 0, sizeof(OSVERSIONINFO));                                      //开空间 
	osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);                     //定义大小 
	GetVersionEx(&osvi);
	if (osvi.dwMajorVersion == 5)
		return TRUE;
	else
		return FALSE;
}

void Helper::PRINTF(const char *format, ...) {
	char buf[4096];
	va_list ap;
	va_start(ap, format);
	vsprintf(buf, format, ap);
	printf(buf);
#if _DEBUG
	printf(buf);
#else
	char debugbuf[4096];
	strcpy_s(debugbuf, "@xiaohuihui ");
	strcat(debugbuf, buf);
	OutputDebugStringA(debugbuf);
#endif
}

std::wstring Helper::UTF8ToUnicode(const string& str)
{
	int unicodeLen = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);  
	wchar_t* pUnicode;  
	pUnicode = new wchar_t[unicodeLen];  
	memset(pUnicode, 0, (unicodeLen) * sizeof(wchar_t));  
	::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, (LPWSTR)pUnicode, unicodeLen);  
	wstring  rt;  
	rt = (wchar_t*)pUnicode;
	::delete [] pUnicode; 
	return rt;  
}

std::string Helper::UnicodeToUTF8(const wstring& wStr)
{
	char* pElementText;
	int iTextLen;
	// wide char to multi char
	iTextLen = WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), -1, NULL, 0, NULL, NULL);

	pElementText = new char[iTextLen];
	memset((void*)pElementText, 0, sizeof(char) * (iTextLen));
	::WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), -1, pElementText, iTextLen, NULL, NULL);

	string strText;
	strText = pElementText;
	::delete[] pElementText;
	return strText;
}

std::string Helper::UnicodeToUTF8(const wchar_t* szWstr)
{
	char* pElementText;
	int iTextLen;
	// wide char to multi char
	iTextLen = WideCharToMultiByte(CP_UTF8, 0, szWstr, -1, NULL, 0,NULL, NULL);

	pElementText = new char[iTextLen];
	memset((void*)pElementText, 0, sizeof(char) * (iTextLen));
	::WideCharToMultiByte(CP_UTF8, 0, szWstr, -1, pElementText, iTextLen, NULL, NULL);

	string strText;
	strText = pElementText;
	::delete[] pElementText;
	return strText;
}

void Helper::UnicodeToUTF8(const wchar_t *wchar, char *chr, int length)
{
	memset(chr, 0, length);
	::WideCharToMultiByte(CP_UTF8, 0, wchar, -1, chr, length, NULL, NULL);
}

void Helper::UnicodeToAnsi(const wchar_t *wchar, char *chr, int length)
{
	memset(chr, 0, length);
	::WideCharToMultiByte(CP_ACP, 0, wchar, -1, chr, length, NULL, NULL);
}

std::string Helper::UnicodeToAnsi(const wstring wstr)
{
	char* pElementText;
	int iTextLen;
	// wide char to multi char
	iTextLen = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);

	pElementText = new char[iTextLen];
	memset((void*)pElementText, 0, sizeof(char) * (iTextLen));
	::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, pElementText, iTextLen, NULL, NULL);

	string strText;
	strText = pElementText;
	::delete[] pElementText;
	return strText;
}

std::string Helper::UTF8ToAnsi(const string& str)
{
	wstring strTmp = UTF8ToUnicode(str);
	return UnicodeToAnsi(strTmp);
}

std::wstring Helper::AnsiToUnicode(const string& str)
{
	int  unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);  
	wchar_t * pUnicode;  
	pUnicode = new wchar_t[unicodeLen];  
	memset(pUnicode, 0, (unicodeLen) * sizeof(wchar_t));  
	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, (LPWSTR)pUnicode, unicodeLen);  
	wstring  rt;  
	rt = (wchar_t *)pUnicode;
	::delete [] pUnicode; 
	return  rt;  
}



std::string Helper::Post(string url,string file,string prm)
{
	//url = "127.0.0.1";//写死
	int prot = INTERNET_DEFAULT_HTTP_PORT;//默认端口为80
	int item = url.find(':');
	if (item != string::npos){//链接中存在端口参数
		prot = atoi(url.substr(item + 1, url.length() - item - 1).c_str());
		url = url.substr(0, item);
	}
	BOOL  bResults = FALSE;
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	string retStr="";

	HINTERNET hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;

	hSession = WinHttpOpen(  L"Download/1.0", 
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, 
		WINHTTP_NO_PROXY_BYPASS, 0);
	WinHttpSetTimeouts(hSession, 5000000, 5000000, 5000000, 5000000);
	if (hSession)
		hConnect = WinHttpConnect(hSession, 
		AnsiToUnicode(url).c_str(),
		prot, 0);

	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"POST", 
		AnsiToUnicode(file).c_str(), 
		NULL, 
		NULL, 
		WINHTTP_DEFAULT_ACCEPT_TYPES, 
		0);
	if (hRequest)
	{
		WinHttpAddRequestHeaders( hRequest, 
			L"Content-type:application/x-www-form-urlencoded\r\nAccept:*/*\r\nConnection:close\r\n",
			wcslen(L"Content-type:application/x-www-form-urlencoded\r\nAccept:*/*\r\nConnection:close\r\n"),
			WINHTTP_ADDREQ_FLAG_ADD);

		bResults = WinHttpSendRequest(hRequest, 
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0, 
			(prm=="")?0:(LPVOID)prm.c_str(), 
			prm.length(), 
			prm.length(), 
			0);
	}
	if (!bResults)
		return retStr;
	bResults = WinHttpReceiveResponse(hRequest, NULL);
	if (bResults)
	{
		do 
		{
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			{
				return retStr;
			}
			pszOutBuffer = new char[dwSize+1];
			if (!pszOutBuffer)
			{
				dwSize=0;
			}
			else
			{
				// 读取数据
				ZeroMemory(pszOutBuffer, dwSize + 1);
				if (!WinHttpReadData(hRequest, 
					(LPVOID)pszOutBuffer, 
					dwSize, &dwDownloaded))
				{  
					
					return retStr;
				}
				else
				{  
					retStr+=(char*)pszOutBuffer;
				}

				delete [] pszOutBuffer;
			}
		} while (dwSize > 0);
	}
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
	return retStr;
}




template<typename T>
void split(const basic_string<T>& s, T c,
	vector<basic_string<T> >& v) 
{
	basic_string<T>::size_type i = 0;
	basic_string<T>::size_type j = s.find(c);

	while (j != basic_string<T>::npos) 
	{
		v.push_back(s.substr(i, j-i));
		i = ++j;
		j = s.find(c, j);

		if (j == basic_string<T>::npos)
			v.push_back(s.substr(i, s.length( )));
	}
}



void Helper::splitstring(string& str,char c,vector<string>& vstr)
{
	split(str,c,vstr);
}

void Helper::splitstring(wstring& wstr,wchar_t wc,vector<wstring>& vwstr)
{
	split(wstr,wc,vwstr);
}




string Helper::Get( string url,string file )
{
	BOOL  bResults = FALSE;
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	string retStr="";

	HINTERNET hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;

	hSession = WinHttpOpen(  L"Download/1.0", 
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME, 
		WINHTTP_NO_PROXY_BYPASS, 0);
	WinHttpSetTimeouts(hSession, 5000, 5000, 5000, 5000);
	if (hSession)
		hConnect = WinHttpConnect(hSession, 
		AnsiToUnicode(url).c_str(),
		INTERNET_DEFAULT_HTTP_PORT, 0);

	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", 
		AnsiToUnicode(file).c_str(), 
		NULL, 
		NULL, 
		WINHTTP_DEFAULT_ACCEPT_TYPES, 
		0);
	if (hRequest)
	{
		WinHttpAddRequestHeaders( hRequest, 
			L"Content-type:application/x-www-form-urlencoded\r\nAccept:*/*\r\nConnection:close\r\nUser-Agent:Mozi11a\r\n",
			wcslen(L"Content-type:application/x-www-form-urlencoded\r\nAccept:*/*\r\nConnection:close\r\nUser-Agent:Mozi11a\r\n"),
			WINHTTP_ADDREQ_FLAG_ADD);

		bResults = WinHttpSendRequest(hRequest, 
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0, 
			nullptr, 
			0, 
			0, 
			0);
	}
	if (!bResults)
		goto _END_OF_HTTP_GET;

	bResults = WinHttpReceiveResponse(hRequest, NULL);
	if (bResults)
	{
		do 
		{
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			{
				goto _END_OF_HTTP_GET;
				return retStr;
			}
			pszOutBuffer = new char[dwSize+1];
			if (!pszOutBuffer)
			{
				dwSize=0;
			}
			else
			{
				// 读取数据
				ZeroMemory(pszOutBuffer, dwSize + 1);
				if (!WinHttpReadData(hRequest, 
					(LPVOID)pszOutBuffer, 
					dwSize, &dwDownloaded))
				{  
					
					//return retStr;
					goto _END_OF_HTTP_GET;
				}
				else
				{  
					retStr+=(char*)pszOutBuffer;
				}

				delete [] pszOutBuffer;
			}
		} while (dwSize > 0);
	}

_END_OF_HTTP_GET:

	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	return retStr;
}

VOID SafeGetNativeSystemInfo(__out LPSYSTEM_INFO lpSystemInfo)
{
	if (NULL == lpSystemInfo) return;
	typedef VOID(WINAPI *LPFN_GetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);
	LPFN_GetNativeSystemInfo fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo)GetProcAddress(GetModuleHandle(_T("kernel32")), "GetNativeSystemInfo");
	if (NULL != fnGetNativeSystemInfo)
	{
		fnGetNativeSystemInfo(lpSystemInfo);
	}
	else
	{
		GetSystemInfo(lpSystemInfo);
	}
}

int Helper::GetSystemBits()
{
	SYSTEM_INFO si;
	SafeGetNativeSystemInfo(&si);
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
		si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
	{
		return 64;
	}
	return 32;
}