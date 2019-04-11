#ifndef __CMD_ENC__
#define __CMD_ENC__

#pragma once
#pragma warning( disable : 4018)
#pragma warning( disable : 4267)
#pragma warning( disable : 4819)
#pragma warning( disable : 4995)

#include <tchar.h>
#include <atlfile.h>
#include <string>
#include <atlconv.h>
#include <winioctl.h>
#include <Wininet.h>
#include <shlwapi.h>
#include <STRSAFE.H>
#ifdef _STRING_
#include <vector>
#include <algorithm> 
#endif

#include <atlenc.h>
#include <atomic>
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib, "wininet.lib")


#define COMMAND_FLAGS_BINDMAC		1
#define COMMAND_FLAGS_SIGNMD5		2

#ifndef __SAFE_MEMORY_INC__
#define __SAFE_MEMORY_INC__
#define __SAFE_FREE_PTR(ptr)			if (ptr) { free(ptr); ptr = NULL; }
#define __SAFE_DELETE_PTR(ptr)			if (ptr) { delete ptr; ptr = NULL; }
#define __SAFE_DELETE_ARRAY_PTR(ptr)	if (ptr) { delete [] ptr; ptr = NULL; }
#endif // __SAFE_MEMORY_INC__

#define POS_MD5	5

#ifndef __POS_CALLBACK__
#define __POS_CALLBACK__
struct IPosCallback {
	virtual void PosChanging(int type, __int64 iPos,__int64 iCount, LPCSTR lpszFile, bool& fCancel) = 0;
	virtual void PosChanged(int type, __int64 iPos,__int64 iCount, LPCSTR lpszFile) = 0;
};
#endif

struct IPosCallbackEx : IPosCallback {
	virtual void OnMessage(int nCode, LPCSTR pszMessage) = 0; 
	virtual bool IsCancel() { return m_fCancel; }
public:
	IPosCallbackEx() : m_fCancel(false) {}
	bool m_fCancel; 
};

class CSafeNonePosCallback : public IPosCallback {
public:
	explicit CSafeNonePosCallback(std::atomic<bool>& running)
		: is_running(running)
	{}
	void PosChanging(int type, __int64 iPos, __int64 iCount, LPCSTR lpszFile, bool& fCancel) {
		fCancel = !is_running; 
	}
	void PosChanged(int type, __int64 iPos, __int64 iCount, LPCSTR lpszFile) {}
	std::atomic<bool>& is_running; 
};

#ifndef __CMD_DUMP_MSG__
#define __CMD_DUMP_MSG__
struct IDumpMessage {
	virtual void Dump(int nCode, LPCSTR lpszFMT, ...) {
		char szBuffer[2048] = {0}; 
		va_list args;
		va_start(args,lpszFMT);
 		StringCchPrintfA(szBuffer, sizeof(szBuffer)/sizeof(CHAR)-1, lpszFMT, args); 
		va_end(args);
		DumpMessage(nCode, szBuffer); 
	} 
	virtual void DumpMessage(int nCode, LPCSTR pszMsg) {OutputDebugStringA(pszMsg);}
}; 
#endif

#ifndef CMD_BUFF_SIZE
#define CMD_BUFF_SIZE 128
#endif

enum CmdBufferType
{
	CmdNone		= 0,
	CmdUTF8		= 1,
	CmdBase64   = 2,
	CmdHex		= 4,
	CmdEncrypt	= 8
};

struct ICmdStyleCallback {
	virtual void OnCmdStyleChanging(DWORD cbNew, bool& fCancel, void* pContext = NULL) {}
	virtual void OnCmdStyleChanged(DWORD cbNew, DWORD cbOld, void* pContext = NULL) {}
};

#define DECLARE_CMD_STYLE_PROPERTY(name, expr) \
	__declspec(property(get = Is##name, put = Set##name)) bool name; \
	bool Is##name() { return IsHave(expr); } \
	void Set##name(bool rhs) { _Modify( (bool)rhs,(DWORD)expr); }

template <typename T>
struct CCmdStyleImpl {
	CCmdStyleImpl(DWORD cbStyle = 0) : m_Style(cbStyle), m_pCallback(NULL) {}
	CCmdStyleImpl(const CCmdStyleImpl<T>& obj) { if (&obj != this) m_Style = obj.m_Style; }
public:
	bool IsHave(DWORD cbStyle) { return (m_Style & cbStyle) == cbStyle; }
	bool IsEmpty() { return m_Style == 0; }
	void Empty() { _Set(0); }
	CCmdStyleImpl<T>& _Add(DWORD cbStyle) { DWORD _s = m_Style | cbStyle; _Set(_s); return *this; }
	CCmdStyleImpl<T>& _Remove(DWORD cbStyle) { DWORD _s = m_Style;  _s &= ~cbStyle; _Set(_s); return *this; }
	CCmdStyleImpl<T>& _Modify(bool rhs, DWORD cbStyle) { return rhs ? _Add(cbStyle) : _Remove(cbStyle); }
	bool operator==(DWORD cbStyle) { return m_Style == cbStyle; }
	bool operator!=(DWORD cbStyle) { return m_Style != cbstyle; }
	CCmdStyleImpl<T>& operator=(DWORD cbStyle) { _Set(cbStyle);  return *this }
public:
	void SetCallback(ICmdStyleCallback* p) { m_pCallback = p; }
	void _Set(DWORD cbNew) {
		if (cbNew != m_Style) {
			if (m_pCallback) {
				bool fCancel = false; 
				m_pCallback->OnCmdStyleChanging(cbNew, fCancel, this);
				if (fCancel) return; 
			}
			DWORD cbOld = m_Style; 
			m_Style = cbNew; 
			if (m_pCallback) {
				m_pCallback->OnCmdStyleChanged(cbNew, cbOld, this);
			}
		}
	}
public:
	DWORD m_Style;

	ICmdStyleCallback* m_pCallback; 
};


template <typename T>
struct CCmdRefStyleImpl {
	CCmdRefStyleImpl(unsigned int& cbStyle) : m_Style(cbStyle), m_pCallback(NULL) {}
private:
	CCmdRefStyleImpl(const CCmdStyleImpl<T>& obj) : m_Style(obj.m_Style) { }
public:
	bool IsHave(unsigned int cbStyle) { return (m_Style & cbStyle) == cbStyle; }
	bool NotHave(unsigned int cbStyle) { return (m_Style & cbStyle) != cbStyle; }
	bool IsEmpty() { return m_Style == 0; }
	void Empty() { m_Style = 0; }
	void _Add(unsigned int cbStyle) { unsigned int _s = m_Style | cbStyle; _Set(_s); }
	void _Remove(unsigned int cbStyle) { unsigned int _s = m_Style;  _s &= ~cbStyle; _Set(_s); }
	void _Modify(bool rhs , unsigned int cbStyle) {rhs ? _Add(cbStyle) : _Remove(cbStyle); }
 
	bool operator==(unsigned int cbStyle) { return m_Style == cbStyle; }
	bool operator!=(unsigned int cbStyle) { return m_Style != cbstyle; }
	CCmdStyleImpl<T>& operator=(unsigned int cbStyle) { _Set(cbStyle);  return *this }
public:
	void SetCallback(ICmdStyleCallback* p) { m_pCallback = p;  }
public:
	void _Set(DWORD cbNew) {
		if (cbNew != m_Style) {
			if (m_pCallback) {
				bool fCancel = false;
				m_pCallback->OnCmdStyleChanging(cbNew, fCancel, this);
				if (fCancel) return;
			}
			DWORD cbOld = m_Style;
			m_Style = cbNew;
			if (m_pCallback) {
				m_pCallback->OnCmdStyleChanged(cbNew, cbOld, this);
			}
		}
	}
public:
	unsigned int& m_Style;
	ICmdStyleCallback* m_pCallback;
};

class CCmdStyle : public CCmdStyleImpl<CCmdStyle> {
public:
	CCmdStyle(DWORD cbStyle = 0) : CCmdStyleImpl<CCmdStyle>(cbStyle) {}
};

namespace FLAGS {
	inline bool _Have(DWORD Style, DWORD cbStyle) {
		return (Style & cbStyle) == cbStyle;
	}
	inline DWORD _Add(DWORD& Style, DWORD cbStyle) {
		Style |= cbStyle; return Style;
	}
	inline DWORD _Remove(DWORD &Style, DWORD cbStyle) {
		Style &= ~cbStyle ; return Style;
	}
	inline DWORD _Modify(DWORD &Style, DWORD cbAdd, DWORD cbRemove = 0) {
		Style |= cbAdd; Style &= ~cbRemove; return Style;
	}
	inline DWORD _Modify(bool rhs , DWORD& Style, DWORD cbStyle) {
		return (rhs ? _Add(Style, cbStyle) : _Remove(Style, cbStyle)); 
	}
};

class CCmdBuffer {
public:
	CCmdBuffer();
	CCmdBuffer(const char* str,CmdBufferType = CmdNone);
	CCmdBuffer(unsigned long size);
	CCmdBuffer(const CCmdBuffer& obj);
	~CCmdBuffer() ;
public:
	char* Detach();
	CCmdBuffer* Detach(CCmdBuffer* obj);
	void Attach(char* p, unsigned long cbSize = 0);
	void Attach(CCmdBuffer* obj);
public:
	bool IsNull();
	void Zero();
	void Empty(); 
public:
	operator char*();
	operator const char*();
	CCmdBuffer& operator+(const CCmdBuffer& obj);
	CCmdBuffer& operator+(const char* pszBuffer);
	CCmdBuffer& operator+=(const CCmdBuffer& obj);
	CCmdBuffer& operator+=(const char* pszBuffer);
	CCmdBuffer& operator=(const CCmdBuffer& obj);
	CCmdBuffer& operator=(const char* pszBuffer);
public:	
	const char* GetBuffer()	;
	char* GetBufferPtr() ;
	unsigned long GetLength();
	unsigned long GetAllocSize();
	CmdBufferType GetBufferType();
	void SetBufferType(CmdBufferType type);
public:
	char* SetBuffer(CCmdBuffer& obj);
	char* SetBuffer(const char* p, unsigned long size,CmdBufferType type = CmdNone);
	char* SetBuffer(const char* pszText,CmdBufferType type = CmdNone);
	char* Append(const char* pBuffer, unsigned long cbSize);
	char* Append(const char* pBuffer);
	char* AppendFormat(const char* pszFMT,...);
	char* ToBuffer(char** ppBuffer, unsigned long *pcbSize, CmdBufferType type = CmdBase64);
	char* ToBuffer(CCmdBuffer& obj,CmdBufferType type=CmdBase64);
	bool Clone(CCmdBuffer* p);
public:
	CCmdBuffer& Encrypt(char* pszKey, CmdBufferType type = CmdHex);
	CCmdBuffer& Decrypt(char* pszKey, CmdBufferType type = CmdHex);
public:
	char* Alloc(unsigned long size);
	void Free()	;
	static char* CmdAlloc(char* ptr,unsigned long size);
	static void CmdFree(char* ptr);
public:
	char* m_pBuffer;
	size_t m_Alloced;
	size_t m_Length;
	CmdBufferType m_BufferType;
};

class CCmdTempBuffer {
public:
	CCmdTempBuffer(char* p = 0) : ptr(p), size(0) {}
	~CCmdTempBuffer() { Free(); }
public:
	void Attach(char* p,unsigned int sz = 0) { Free(); ptr = p; size = sz; }
	void Attach(CCmdTempBuffer &obj) { Free(); size = obj.GetLength(); ptr = obj.Detach(); }
	void Attach(CCmdBuffer &obj);
	char* Detach() { char* temp = ptr; ptr = 0; size = 0; return temp; }
	operator char*() { return ptr; }
	operator const char*() { return ptr; }
	char** operator&() { return &ptr; }
	bool operator!() { return ptr == 0; }
	bool IsNull() { return ptr == 0; }
	unsigned long GetLength() { return size; }
	template <typename Q> Q* GetPtr() { return (Q*)ptr; }
public:
	char* Alloc(unsigned long sz) {
		if (ptr) ptr = (char*)realloc(ptr,sz);
 		else ptr = (char*)calloc(sz, 1);
		size = sz;
		return ptr;
	}
	void Free() { if (ptr) { free(ptr); ptr = 0;} size = 0; }
public:
	char* ptr;
	unsigned int size;
};

class CGlobalMemory {
public:
	CGlobalMemory(); 
	CGlobalMemory(size_t size); 
	CGlobalMemory(IStream* pStream); 
private:
	CGlobalMemory(const CGlobalMemory&) {}
public:
	bool IsNull() const { return m_h == NULL;  }
	operator HGLOBAL() { return m_h; }
	size_t GetSize() { return m_size ; }
	HGLOBAL GetHandle() { return m_h; }
public:
	HRESULT Alloc(const size_t size, UINT uFlags = GHND	);
	HRESULT Free(); 
	HRESULT ReAlloc(const size_t size); 
	HRESULT ToStream(IStream** ppStream, BOOL fDeleteOnRelease = TRUE);
	HRESULT FromStream(IStream* pStream); 
public:
	void* Lock(); 
	void  Unlock(); 
	size_t Write(const unsigned char* buffer, const size_t size);
	size_t Write(const unsigned int pos, const unsigned char* buffer, const size_t size);
	size_t Read(unsigned char* buffer, const size_t size);
	size_t Read(const unsigned int pos, unsigned char* buffer, const size_t size);
private:
	HGLOBAL m_h; 
	size_t  m_size; 
};

/*
	字串工具帮助函数
*/
namespace StringTool
{
	LPCSTR  NewGuidString(LPSTR lpszBuffer, DWORD cbSize);
	LPCWSTR NewGuidString(LPWSTR lpszBuffer, DWORD cbSize);
	LPCSTR  MacString(LPSTR lpszBuffer, DWORD cbSize);
	LPCWSTR MacString(LPWSTR lpszBuffer, DWORD cbSize);
	LPCSTR	FormatNumeric(LONGLONG Number,LPSTR pszBuffer);
	LPCWSTR FormatNumeric(LONGLONG Number,LPWSTR pszBuffer);
	LPCTSTR FormatNumeric(LONGLONG Number);
	LPCSTR	OleFormatNumeric(LONGLONG Number,LPSTR pszBuffer,  LPCSTR lpszFMT = "#,##0");
	LPCWSTR OleFormatNumeric(LONGLONG Number,LPWSTR pszBuffer, LPWSTR lpszFMT = L"#,##0");
	LPCTSTR OleFormatNumeric(LONGLONG Number,LPCTSTR lpszFMT = _T("#,##0"));

	LPCSTR  LongIntToString(LONGLONG Number, LPSTR pszBuffer, DWORD cbSize);
	LPCWSTR  LongIntToString(LONGLONG Number, LPWSTR pszBuffer, DWORD cbSize);
#ifdef __ATLSTR_H__
	CStringA NewGuidStringA();
	CStringW NewGuidStringW();
	CStringA& NewGuidString(CStringA& sGuid);
	CStringW& NewGuidString(CStringW& sGuid);

	CStringA MacStringA();
	CStringW MacStringW();
	CStringA& MacString(CStringA& sMac);
	CStringW& MacString(CStringW& sMac);
	CStringA  LongIntToStringA(LONGLONG Number); 
	CStringW  LongIntToStringW(LONGLONG Number); 
#endif
#ifdef _STRING_
	std::string& NewGuidString(std::string &str);
	std::string& MacString(std::string& str);
	std::string  LongIntToString(LONGLONG Number);
#endif
};

namespace PathHelper 
{
	HMODULE GetCurrentModuleHandle();
	LPCSTR  GetModulePath(HMODULE hModule, LPSTR lpszPath, DWORD cbSize) ;
	LPCWSTR GetModulePath(HMODULE hModule, LPWSTR lpszPath, DWORD cbSize) ;

	LPCTSTR CurrentModulePath();
	LPCSTR	CurrentModulePath(LPSTR pszPath, DWORD cbSize);
	LPCWSTR	CurrentModulePath(LPWSTR pszPath, DWORD cbSize);

	LPCSTR  ApplicationPath(LPSTR lpszBuffer, DWORD cbSize);
	LPCWSTR ApplicationPath(LPWSTR lpszBuffer, DWORD cbSize);
	LPCSTR  ApplicationPathA();
	LPCWSTR ApplicationPathW();
	LPCSTR  ApplicationFile(LPCSTR lpszFile, LPSTR lpszBuffer, DWORD cbSize);
	LPCWSTR ApplicationFile(LPCWSTR lpszFile, LPWSTR lpszBuffer, DWORD cbSize);
	BOOL	MakeDirectory(LPCSTR lpszDirectory);
	BOOL	MakeDirectory(LPCWSTR lpszDirectory);
	BOOL	MakeFileDirectory(LPCSTR lpszFile);
	BOOL	MakeFileDirectory(LPCWSTR lpszFile);
	
	CHAR GetMaxFreeSpaceDriver();
#ifdef __ATLSTR_H__
	CString ApplicationPath();
	CString ApplicationFile(LPCTSTR lpszFile);
#endif

#ifdef _STRING_
	std::string StdApplicationPath(); 
	std::string StdApplicationFile(LPCSTR pszFile); 
#endif
};

namespace StreamHelper
{
	HRESULT BufferToStream(const BYTE* pBuffer, DWORD cbSize, IStream** ppStream);
	HRESULT ResourceToBuffer(UINT nResID,PBYTE* ppBuffer, DWORD *pcbSize, LPCTSTR pszResType = _T("PNG"),HINSTANCE hInstance = NULL);
	HRESULT ResourceToStream(UINT nResID,IStream** ppStream,LPCTSTR pszResType = _T("PNG"),HINSTANCE hInstance = NULL);
};

namespace IniHelper
{
 	BOOL SetIniKey(LPCTSTR sSection,LPCTSTR sKey, LPCTSTR sValue, LPCTSTR sIniFile);
	BOOL SetIniKey(LPCTSTR sSection, LPCTSTR sKey, LONGLONG nValue, LPCTSTR sIniFile);
	LPCTSTR GetIniKey(LPCTSTR sSection,LPCTSTR sKey,LPCTSTR sIniFile);
	LPCTSTR GetStringKey(LPCTSTR sSection,LPCTSTR sKey ,LPCTSTR sIniFile,LPCTSTR lpszDefault = _T("") );
	LONGLONG GetIntKey(LPCTSTR sSection,LPCTSTR sKey ,LPCTSTR sIniFile,LONGLONG nDefaultValue = 0);
}

namespace FileHelper
{
	HRESULT GetFileLength(LPCTSTR lpszFile,LONGLONG& FileLength);
	HRESULT ReadToBuffer(LPCTSTR lpszFile,PBYTE* ppBytes, DWORD* pcbSize);
	HRESULT ReadToStream(LPCTSTR lpszFile,IStream** ppStream);
#ifdef _STRING_
	std::string StdReadToString(LPCTSTR lpszFile);
#endif
#ifdef __ATLSTR_H__
	CStringA ReadToString(LPCTSTR lpszFile);
#endif	
	HRESULT WriteToFile(LPCTSTR lpszFile, LPVOID lpBuffer, DWORD cbSize);
	HRESULT AppendFile(LPCTSTR lpszFile, const BYTE* pBuffer, DWORD cbSize);
	HANDLE  CreateProcess(LPCTSTR lpszProcess, WORD wShowWindow = SW_SHOW);
	BOOL    CreateProcess(LPCTSTR lpszProcess, LPCTSTR lpszDir, DWORD dwWait, WORD wShowWindow = SW_SHOW);
	HANDLE  CreateProcessEx(LPCTSTR lpszProcess, LPCTSTR lpszArgs, LPCTSTR lpszDir, WORD wShowWindow  = SW_SHOW);
	HANDLE  CreateDOSProcess(LPCTSTR lpszProcess);
};

namespace ProcHelper 
{
	BOOL GetAutoStart(LPCTSTR pszKey, LPCTSTR pszProc = NULL); 
	BOOL SetAutoStart(LPCTSTR pszKey, LPCTSTR pszProc); 
	BOOL IsAutoStart(LPCTSTR pszProc); 

	BOOL EnableDebugPrivilege();   
 
}; 


namespace HttpHelper
{
	BOOL ParserUrl(LPCTSTR lpszUrl, LPTSTR pszHost, LPTSTR pszUrl, WORD& port);
#ifdef __ATLSTR_H__
	BOOL ParserUrl(LPCTSTR lpszUrl, CString& sHost, CString& sUrl, WORD& port);
#endif
#ifdef __CMD_HTTP__	
	DWORD HttpGet(LPCTSTR lpszUrl, PBYTE* pOutBuffer, DWORD* pcbSize);
	int HttpPost(LPCTSTR lpszUrl, const BYTE* pPostData, const DWORD cbData, LPCTSTR lpszHeader , PBYTE* pOutBuffer, DWORD* pcbSize);
#endif
};

namespace RegHelperX
{
	LONG GetRegDWORDValue(LPCTSTR lpszPath, LPCTSTR lpszKey, DWORD& dwValue, HKEY hRootKey = HKEY_LOCAL_MACHINE);
	LONG GetRegStringValue(LPCTSTR lpszPath, LPCTSTR lpszKey, LPTSTR pBuffer, DWORD* pcbSize, HKEY hRootKey = HKEY_LOCAL_MACHINE);
	LONG WriteRegDWORDValue(LPCTSTR lpszPath, LPCTSTR lpszKey, DWORD dwValue, HKEY hRootKey = HKEY_LOCAL_MACHINE);
	LONG WriteRegStringValue(LPCTSTR lpszPath, LPCTSTR lpszKey, LPCTSTR pBuffer, HKEY hRootKey = HKEY_LOCAL_MACHINE);
#ifdef __ATLSTR_H__
	LONG GetRegStringValue(LPCTSTR lpszPath, LPCTSTR lpszKey, CString& StringBuffer, HKEY hRootKey = HKEY_LOCAL_MACHINE);
#endif
};

namespace Clipbord
{
	BOOL SaveClipbord(LPCSTR lpszText);
}

namespace RES
{
	HRESULT BytesToStream(const BYTE* pBuffer,DWORD cbSize, IStream** ppStream);
	HRESULT GetResourceBuffer(UINT uResID, PBYTE* ppOutBuffer, DWORD* pcbSize, LPCTSTR pszType = _T("PNG"), HINSTANCE hResDLL = NULL);
	HRESULT GetResourceStream(UINT uResID, IStream** ppStream, LPCTSTR pszType = _T("PNG"), HINSTANCE hResDLL = NULL);
}

namespace MAC
{
	#define NCF_VIRTUAL				0x00000001
	#define NCF_SOFTWARE_ENUMERATED	0x00000002
	#define NCF_PHYSICAL			0x00000004
	#define NCF_HIDDEN				0x00000008
	#define NCF_NO_SERVICE			0x00000010
	#define NCF_NOT_USER_REMOVABLE	0x00000020
	#define NCF_MULTIPORT_INSTANCED_ADAPTER		0x00000080
	#define NCF_HAS_UI				0x00000100
	#define NCF_FILTER				0x00000400

	typedef struct tagNETAPATER_INFO
	{
		DWORD Characteristics;
		CHAR ServiceName[MAX_PATH];
		CHAR Description[MAX_PATH];
		CHAR DeviceInstanceID[MAX_PATH];
		CHAR MacAddress[32];
		CHAR CurrentMacAddress[32];
		CHAR IPv4[32];
		CHAR IPv6[32];
	}NETAPATER_INFO,*LPNETAPATER_INFO;

	HANDLE WINAPI FindApaters(DWORD cbFlags);
	LPVOID WINAPI FirstApater(HANDLE hFind);
	LPVOID WINAPI NextApater(HANDLE hFind);
	VOID WINAPI CloseApaterHandle(HANDLE hFind); 
};

/*
	数据包签名处理帮助库
*/
namespace Sign
{
	//========================================================================
	// CRC32 哈希算法
	//
	long	CRC_Hash(const PBYTE pBuffer, DWORD cbSize);
	long	CRC_Hash(LPCSTR lpszText);
	long	CRC_Hash(LPCWSTR lpszText);
	long	CRC_HashFile(LPCSTR lpszFile);
	long	CRC_HashFile(LPCWSTR lpszFile);
	long	CRC_HashModule(HINSTANCE hInstance = NULL);
	//========================================================================
	// MD5 哈希加密算法帮助函数
	//
	PBYTE	MD5_Hash(const PBYTE pBuffer, DWORD cbInSize, PBYTE pOutBytes, DWORD cbBufferSize);
	LPCSTR	MD5_Hash(const PBYTE pBuffer, DWORD cbInSize, LPSTR pOutMD5);
	LPCWSTR	MD5_Hash(const PBYTE pBuffer, DWORD cbInSize, LPWSTR pOutMD5);
	LPCSTR	MD5_Hash(LPCSTR pszText,LPSTR pOutMD5);
	LPCWSTR	MD5_Hash(LPCWSTR pszText, LPWSTR pOutMD5);
	LPCSTR	MD5_HashFile(LPCSTR lpszFile,LPSTR pOutMD5);
	LPCWSTR MD5_HashFile(LPCWSTR lpszfile,LPWSTR pOutMD5);
	LPCSTR  MD5_HashModule(LPSTR pOutMD5,HINSTANCE hInstance = NULL);
	LPCWSTR MD5_HashModule(LPWSTR pOutMD5,HINSTANCE hInstance = NULL);

	HRESULT MD5_HashHugeFile(LPCTSTR lpszFile, LPTSTR pOutMD5, IPosCallback *Callback = NULL);
#ifdef __ATLSTR_H__
	long	CRC_Hash(CStringA &buffer);
	CStringA MD5_Hash(CStringA &buffer);
#endif
#ifdef _STRING_
	long	CRC_Hash(std::string& buffer);
	std::string	MD5_Hash(std::string& buffer);
#endif
	// ---- MAC地址哈希帮助函数------
	long MAC_Hash();
	long MAC_Hash(LPCSTR lpszMac);
	long MAC_Hash(LPCWSTR lpszMac);
};

/*
	BASE64 编码帮助库函数
*/
namespace Base64
{
	#define MAX_BASE64_LINE_LENGTH   57
	//=======================================================================
	// Base64Encode/Base64Decode
	// compliant with RFC 2045
	//=======================================================================
	//
	#define BASE64_FLAG_NONE	0
	#define BASE64_FLAG_NOPAD	1
	#define BASE64_FLAG_NOCRLF  2
	//=======================================================================
	int Base64EncodeGetRequiredLength(int nSrcLen, DWORD dwFlags = BASE64_FLAG_NOPAD | BASE64_FLAG_NOCRLF);
	int Base64DecodeGetRequiredLength(int nSrcLen) throw();
	LPSTR Base64Encode(const BYTE *pbData, int nSrcLen, LPSTR szDest, int *pnDestLen,DWORD dwFlags = BASE64_FLAG_NOPAD | BASE64_FLAG_NOCRLF) throw();
	BYTE* Base64Decode(LPCSTR szSrc, int nSrcLen, BYTE *pbDest, int *pnDestLen) throw();

	// 内部调用alloc 自动分配缓存，调用方需显示free销毁内存
	LPSTR Base64Encode(const BYTE* pbSrcData, int nSrcLen, LPSTR* ppDest,/* int* pnDestLen,*/ DWORD dwFlags = BASE64_FLAG_NOPAD | BASE64_FLAG_NOCRLF);
	LPSTR Base64Encode(const BYTE* pbSrcData,int nSrcLen, DWORD dwFlags = BASE64_FLAG_NOPAD | BASE64_FLAG_NOCRLF);
	BYTE* Base64Decode(LPCSTR szSrc, BYTE** ppDest, int* pnDestLen);
	BYTE* Base64Decode(LPCSTR szSrc, int* pnDestLen);

#ifdef __ATLSTR_H__
 	CStringA& Base64Encode(const BYTE* pbSrcData, int nSrcLen, CStringA& sDest,DWORD dwFlags = BASE64_FLAG_NOPAD | BASE64_FLAG_NOCRLF);
	CStringA  Base64Encode(const CStringA& sText, DWORD dwFlags = BASE64_FLAG_NOPAD | BASE64_FLAG_NOCRLF);
	CStringA& Base64Encode(const CStringA& sText, CStringA& sDest,DWORD dwFlags = BASE64_FLAG_NOPAD | BASE64_FLAG_NOCRLF);
	CStringA  Base64Decode(const CStringA& sText);
	CStringA& Base64Decode(const CStringA& sText, CStringA& sDest);
#endif
#ifdef _STRING_
	std::string& Base64Encode(const BYTE* pbSrcData, int nSrcLen, std::string& sDest,DWORD dwFlags = BASE64_FLAG_NOPAD | BASE64_FLAG_NOCRLF);
	std::string  Base64Encode(const std::string& sText,DWORD dwFlags = BASE64_FLAG_NOPAD | BASE64_FLAG_NOCRLF);
	std::string& Base64Encode(const std::string& sText, std::string& sDest, DWORD dwFlags = BASE64_FLAG_NOPAD | BASE64_FLAG_NOCRLF);
	std::string  Base64Decode(const std::string& sText);
	std::string& Base64Decode(const std::string& sText, std::string& sDest);
#endif
};

/*HEX 编码帮助库*/
namespace HEX
{
	#ifdef _CHAR_UNSIGNED
	#define HEX_INVALID CHAR_MAX
	#else
	/* In pre-V8 this was always the value, which meant we didn't compile clean with /J */
	#define HEX_INVALID ((char)(-1))
	#endif	//=======================================================================
	// HexEncode, HexDecode
	//
	// Support for encoding/decoding binary XML datatypes with hex encoding
	//=======================================================================
	//
	int HexEncodeGetRequiredLength(int nSrcLen);
	int HexDecodeGetRequiredLength(int nSrcLen) throw();

	LPSTR HexEncode(const BYTE *pbSrcData, int nSrcLen, LPSTR szDest, int *pnDestLen, bool upper = false) throw();
	PBYTE HexDecode(LPCSTR pSrcData, int nSrcLen, LPBYTE pbDest, int* pnDestLen) throw();

	// 内置分配缓存
	LPSTR HexEncode(const BYTE* pbSrcData, int nSrcLen, LPSTR* ppDest);
	LPSTR HexEncode(const BYTE* pbSrcData, int nSrcLen);
	PBYTE HexDecode(LPCSTR szSrc, BYTE** ppDest, int* pnDestLen);
	PBYTE HexDecode(LPCSTR szSrc, int* pnDestLen);

#ifdef __ATLSTR_H__
	PBYTE HexDecode(const CStringA& sSrc, PBYTE* ppDest, int* pnDestLen);

	CStringA& HexEncode(const BYTE* pbSrcData, int nSrcLen, CStringA& sDest);
	CStringA& HexEncode(const CStringA& sSrc, CStringA& sDest);
	CStringA  HexEncode(const CStringA& sSrc);
	CStringA& HexDecode(const CStringA& sSrc, CStringA& sDest);
	CStringA  HexDecode(const CStringA& sSrc);
#endif
#ifdef _STRING_
	PBYTE HexDecode(const std::string& sSrc, PBYTE* ppDest, int* pnDestLen);

	std::string& HexEncode(const BYTE* pbSrcData, int nSrcLen, std::string& sDest);
	std::string& HexEncode(const std::string& sSrc, std::string& sDest);
	std::string  HexEncode(const std::string& sSrc, bool upper = false);
	std::string& HexDecode(const std::string& sSrc, std::string& sDest);
	std::string  HexDecode(const std::string& sSrc);
#endif

};

/*
	随机散列+表置换XOR加密算法
*/
namespace Encrypt
{
	enum eCoder
	{
		BASE64 = 0,
		HEX	   = 1,
		UTF8   = 2,
	};

	/* 随机散列XOR加密 */
	namespace RandXOR
	{
		long GetHashKey(LPCSTR lpszKey);
		long GetHashKey(LPCWSTR lpszKey);
		const BYTE* EncryptBuffer(PBYTE pBuffer,DWORD cbSize, LONG lKey);
		const BYTE* EncryptBuffer(const BYTE* pBuffer, DWORD cbSize, BYTE** ppBuffer, LONG lKey);
		const BYTE* DecryptBuffer(PBYTE pBuffer,DWORD cbSize, LONG lKey);
		const BYTE* DecryptBuffer(const BYTE* pBuffer, DWORD cbSize, BYTE** ppBuffer, LONG lKey);

		LPCSTR EncryptString(LPCSTR _SrcText,LPSTR* _EnText, LONG lKey, int nCoder = BASE64);
		LPCWSTR EncryptString(LPCSTR _SrcText,LPWSTR* _EnText, LONG lKey, int nCoder = BASE64);
		LPCSTR DecryptString(LPCSTR _EnText,LPSTR* _SrcText, LONG lKey, int nCoder = BASE64);
		LPCWSTR DecryptString(LPCSTR _EnText,LPWSTR* _SrcText, LONG lKey, int nCoder = BASE64);

		LPCSTR EncryptString(LPCSTR _SrcText, LPSTR* _EnText, LPCSTR _Key, int nCoder = BASE64);
		LPCWSTR EncryptString(LPCSTR _SrcText, LPWSTR* _EnText, LPCWSTR _lKey, int nCoder = BASE64);
		LPCSTR DecryptString(LPCSTR _EnText, LPSTR* _SrcText, LPCSTR _Key, int nCoder = BASE64);
		LPCWSTR DecryptString(LPCSTR _EnText, LPWSTR* _SrcText, LPCWSTR _Key, int nCoder = BASE64);

#ifdef __ATLSTR_H__
		CStringA  EncryptString(CStringA _SrcText,CStringA _Key, int nCoder = HEX);
		CStringA  DecryptString(CStringA _EnText, CStringA _Key, int nCoder = HEX);
		CStringA& EncryptString(LPCSTR _SrcText, CStringA& _EnText, LONG lKey, int nCoder = BASE64);
		CStringW& EncryptString(LPCSTR _SrcText, CStringW& _EnText, LONG lKey, int nCoder = BASE64);
		CStringA& DecryptString(LPCSTR _EnText, CStringA& _SrcText, LONG lKey, int nCoder = BASE64);
		CStringW& DecryptString(LPCSTR _EnText ,CStringW& _SrcText, LONG lKey, int nCoder = BASE64);

		CStringA& EncryptString(LPCSTR _SrcText, CStringA& _EnText, LPCSTR _Key, int nCoder = BASE64);
		CStringW& EncryptString(LPCSTR _SrcText, CStringW& _EnText, LPCWSTR _lKey, int nCoder = BASE64);
		CStringA& DecryptString(LPCSTR _EnText, CStringA& _SrcText, LPCSTR _Key, int nCoder = BASE64);
		CStringW& DecryptString(LPCSTR _EnText, CStringW& _SrcText, LPCWSTR _Key, int nCoder = BASE64);
#endif

#ifdef _STRING_
		std::string& EncryptString(LPCSTR _SrcText, std::string& _EnText, LONG lKey, int nCoder = BASE64);
		std::string& DecryptString(LPCSTR _EnText, std::string& _SrcText, LONG lKey, int nCoder = BASE64);
		std::string& EncryptString(LPCSTR _SrcText, std::string& _EnText, const std::string& _Key, int nCoder = BASE64);
		std::string& DecryptString(LPCSTR _EnText, std::string& _SrcText, const std::string& _Key, int nCoder = BASE64);
#endif
	};

	/*随机散列表置换加密*/
	namespace TXOR
	{
		typedef struct _crypt_handle {
			int _seed;
			unsigned char _t1[256];
			unsigned char _t2[256];
		}CryptHandle,*CryptHandlePtr;
		typedef const CryptHandle* CryptHandleCPtr;

		// ------ 密钥管理 -----------------
 		unsigned int GetPublicKey(LPCSTR lpszKey);
		unsigned int GetPublicKey(LPCWSTR lpszKey);
		unsigned int GetPrivateKey(LPCSTR lpszKey);
		unsigned int GetPrivateKey(LPCWSTR lpszKey);

		// ------ 加密/解密核心算法 --------------------
		CryptHandleCPtr InitCryptHandle(CryptHandlePtr _HandlePtr, unsigned int _PublicKey);
 		void* EncryptBuffer(CryptHandleCPtr _HandlePtr, unsigned int _PrivateKey, unsigned char* _EnBuffer, unsigned int _InSize);
		void* DecryptBuffer(CryptHandleCPtr _HandlePtr, unsigned int _PrivateKey, unsigned char* _EnBuffer, unsigned int _InSize);
		
		// ------ 加密/解密核心辅助函数 ----------------
		void* EncryptBuffer(CryptHandleCPtr _HandlePtr, unsigned int _PrivateKey, const unsigned char* _SrcBuffer, unsigned int _InSize, unsigned char** ppEnBuffer);
		void* DecryptBuffer(CryptHandleCPtr _HandlePtr, unsigned int _PrivateKey, const unsigned char* _SrcBuffer, unsigned int _InSize, unsigned char** ppEnBuffer);
		//------ 加密/解密编码输出字串 ------------------------
		LPSTR EncryptBuffer(CryptHandleCPtr _HandlePtr, unsigned int _PrivateKey, const unsigned char* _SrcBuffer, unsigned int _InSize, PSTR* ppEnBuffer, int* pLength, int nCoder = BASE64);
		BYTE* DecryptBuffer(CryptHandleCPtr _HandlePtr, unsigned int _PrivateKey, LPCSTR _EnText, BYTE** ppSrcBuffer, int* pLength, int nCoder = BASE64);
		
		void* EncryptBuffer(const unsigned int _PublicKey, unsigned int _PrivateKey, unsigned char* _SrcBuffer, unsigned int _InSize);
		void* EncryptBuffer(const unsigned int _PublicKey, unsigned int _PrivateKey, const unsigned char* _SrcBuffer, unsigned int _InSize, unsigned char** ppEnBuffer);
		void* EncryptBuffer(LPCSTR _PublicKey, LPCSTR _PrivateKey, unsigned char* _SrcBuffer, unsigned int _InSize);
		void* EncryptBuffer(LPCSTR _PublicKey, LPCSTR _PrivateKey, const unsigned char* _SrcBuffer, unsigned int _InSize, unsigned char** ppEnBuffer);
		void* EncryptBuffer(LPCWSTR _PublicKey, LPCWSTR _PrivateKey, unsigned char* _SrcBuffer, unsigned int _InSize);
		void* EncryptBuffer(LPCWSTR _PublicKey, LPCWSTR _PrivateKey, const unsigned char* _SrcBuffer, unsigned int _InSize, unsigned char** ppEnBuffer);
		
		void* DecryptBuffer(const unsigned int _PublicKey, unsigned int _PrivateKey, unsigned char* _SrcBuffer, unsigned int _InSize);
		void* DecryptBuffer(const unsigned int _PublicKey, unsigned int _PrivateKey, const unsigned char* _EnBuffer, unsigned int _InSize, unsigned char** ppSrcBuffer);
		void* DecryptBuffer(LPCSTR _PublicKey, LPCSTR _PrivateKey, unsigned char* _SrcBuffer, unsigned int _InSize);
		void* DecryptBuffer(LPCSTR _PublicKey, LPCSTR _PrivateKey, const unsigned char* _SrcBuffer, unsigned int _InSize, unsigned char** ppEnBuffer);
		void* DecryptBuffer(LPCWSTR _PublicKey, LPCWSTR _PrivateKey, unsigned char* _SrcBuffer, unsigned int _InSize);
		void* DecryptBuffer(LPCWSTR _PublicKey, LPCWSTR _PrivateKey, const unsigned char* _SrcBuffer, unsigned int _InSize, unsigned char** ppEnBuffer);

		//------ 字串加解密帮助处理接口 ----------------
		LPCSTR EncryptString(LPCSTR pszSrcText,LPSTR* ppEnString, unsigned int _PrivateKey, unsigned int _PublicKey = 0, int nCoder = BASE64);
		LPCSTR EncryptString(LPCSTR pszSrcText,LPSTR* ppEnString, LPCSTR _PrivateKey, unsigned int _PublicKey = 0, int nCoder = BASE64);
		LPCSTR EncryptString(LPCSTR pszSrcText,LPSTR* ppEnString, LPCSTR _PrivateKey, LPCSTR _PublicKey , int nCoder = BASE64);
		LPCSTR DecryptString(LPCSTR pszEnText, LPSTR* ppSrcString, unsigned int _PrivateKey, unsigned int _PublicKey = 0, int nCoder = BASE64);
		LPCSTR DecryptString(LPCSTR pszEnText, LPSTR* ppSrcString, LPCSTR _PrivateKey, unsigned int _PublicKey = 0, int nCoder = BASE64);
		LPCSTR DecryptString(LPCSTR pszEnText, LPSTR* ppSrcString, LPCSTR _PrivateKey, LPCSTR _PublicKey , int nCoder = BASE64);
		
		LPCWSTR EncryptString(LPCWSTR pszSrcText,LPWSTR* ppEnString, unsigned int _PrivateKey, unsigned int _PublicKey = 0, int nCoder = BASE64);
		LPCWSTR EncryptString(LPCWSTR pszSrcText,LPWSTR* ppEnString, LPCWSTR _PrivateKey, unsigned int _PublicKey = 0, int nCoder = BASE64);
		LPCWSTR EncryptString(LPCWSTR pszSrcText,LPWSTR* ppEnString, LPCWSTR _PrivateKey, LPCWSTR _PublicKey , int nCoder = BASE64);
		LPCWSTR DecryptString(LPCWSTR pszEnText, LPWSTR* ppSrcString, unsigned int _PrivateKey, unsigned int _PublicKey = 0, int nCoder = BASE64);
		LPCWSTR DecryptString(LPCWSTR pszEnText, LPWSTR* ppSrcString, LPCWSTR _PrivateKey, unsigned int _PublicKey = 0, int nCoder = BASE64);
		LPCWSTR DecryptString(LPCWSTR pszEnText, LPWSTR* ppSrcString, LPCWSTR _PrivateKey, LPCWSTR _PublicKey , int nCoder = BASE64);

#ifdef __ATLSTR_H__
		CStringA EncryptStringA(const CStringA& SrcString, const CStringA& lpszKey, int nCode = BASE64);
		CStringA DecryptStringA(const CStringA& EnString, const CStringA& lpszKey, int nCode = BASE64);
		CStringW EncryptStringW(const CStringW& SrcString, const CStringW& lpszKey, int nCode = BASE64);
		CStringW DecryptStringW(const CStringW& EnString, const CStringW& lpszKey, int nCode = BASE64);

		LPCSTR EncryptString(const CStringA& pszSrcText,CStringA& sEnString, unsigned int _PrivateKey, unsigned int _PublicKey, int nCoder = BASE64);
		LPCSTR EncryptString(const CStringA& pszSrcText,CStringA& sEnString, LPCSTR _PrivateKey, unsigned int _PublicKey , int nCoder = BASE64);
		LPCSTR EncryptString(const CStringA& pszSrcText,CStringA& sEnString, LPCSTR _PrivateKey, LPCSTR _PublicKey , int nCoder = BASE64);
		LPCSTR DecryptString(const CStringA& pszEnText, CStringA& sSrcString, unsigned int _PrivateKey, unsigned int _PublicKey , int nCoder = BASE64);
		LPCSTR DecryptString(const CStringA& pszEnText, CStringA& sSrcString, LPCSTR _PrivateKey, unsigned int _PublicKey , int nCoder = BASE64);
		LPCSTR DecryptString(const CStringA& pszEnText, CStringA& sSrcString, LPCSTR _PrivateKey, LPCSTR _PublicKey , int nCoder = BASE64);
		
		LPCWSTR EncryptString(const CStringW& pszSrcText,CStringW& sEnString, unsigned int _PrivateKey, unsigned int _PublicKey, int nCoder = BASE64);
		LPCWSTR EncryptString(const CStringW& pszSrcText,CStringW& sEnString, LPCWSTR _PrivateKey, unsigned int _PublicKey , int nCoder = BASE64);
		LPCWSTR EncryptString(const CStringW& pszSrcText,CStringW& sEnString, LPCWSTR _PrivateKey, LPCWSTR _PublicKey , int nCoder = BASE64);
		LPCWSTR DecryptString(const CStringW& pszEnText, CStringW& sSrcString, unsigned int _PrivateKey, unsigned int _PublicKey, int nCoder = BASE64);
		LPCWSTR DecryptString(const CStringW& pszEnText, CStringW& sSrcString, LPCWSTR _PrivateKey, unsigned int _PublicKey, int nCoder = BASE64);
		LPCWSTR DecryptString(const CStringW& pszEnText, CStringW& sSrcString, LPCWSTR _PrivateKey, LPCWSTR _PublicKey, int nCoder = BASE64);
#endif

#ifdef _STRING_
 		LPCSTR EncryptString(const std::string& pszSrcText,std::string& sEnString, unsigned int _PrivateKey, unsigned int _PublicKey, int nCoder = BASE64);
		LPCSTR EncryptString(const std::string& pszSrcText,std::string& sEnString, LPCSTR _PrivateKey, unsigned int _PublicKey, int nCoder = BASE64);
		LPCSTR EncryptString(const std::string& pszSrcText,std::string& sEnString, LPCSTR _PrivateKey, LPCSTR _PublicKey , int nCoder = BASE64);
		LPCSTR DecryptString(const std::string& pszEnText, std::string& sSrcString, unsigned int _PrivateKey, unsigned int _PublicKey , int nCoder = BASE64);
		LPCSTR DecryptString(const std::string& pszEnText, std::string& sSrcString, LPCSTR _PrivateKey, unsigned int _PublicKey , int nCoder = BASE64);
		LPCSTR DecryptString(const std::string& pszEnText, std::string& sSrcString, LPCSTR _PrivateKey, LPCSTR _PublicKey , int nCoder = BASE64);
#endif
	};
};

#ifndef __STRING_CONVERT_H__
#define __STRING_CONVERT_H__
/*
	字串转换+UTF8编码
*/
namespace StringHelper
{
	template <typename T, typename _Type> 
	class StringHelperImpl {
	public:
 		StringHelperImpl() : m_pStr(NULL) {}
		~StringHelperImpl() { Destroy(); }
	public:
 		void   Destroy() { __SAFE_FREE_PTR(m_pStr); }
		void   Attach(_Type* ptr) { Destroy(); m_pStr = ptr; }
		_Type* Detach() { _Type* Temp = m_pStr; m_pStr = NULL; return Temp; }
		operator _Type*() { return m_pStr; }
		operator const _Type*() { return m_pStr; }
	public:
 		_Type* m_pStr;
	};

	class CWString : public StringHelperImpl<CWString,WCHAR>{
	public:
		explicit CWString(LPCSTR lpszText,UINT acp = GetACP())  
		{
			int nLengthA = lstrlenA(lpszText) ;
			int nLengthW = nLengthA;
 			nLengthW = MultiByteToWideChar(acp,0,lpszText,-1,NULL,0); 
			m_pStr = (LPWSTR)calloc(nLengthW +2,sizeof(WCHAR));
			MultiByteToWideChar(acp,0,lpszText,-1,m_pStr,nLengthW); 
			//m_pStr[nBufSize] = 0x00;
 		}
 	private:
		CWString(const CWString&) {}
	};

	class CAString : public StringHelperImpl<CAString,CHAR> {
	public:
		explicit CAString(LPCWSTR lpszText)  
		{
 			int nBufSize = WideCharToMultiByte(GetACP(),0,lpszText,-1,NULL,0,NULL,NULL); 
			m_pStr = (LPSTR)calloc(nBufSize+1,sizeof(CHAR));
			WideCharToMultiByte(GetACP(),0,lpszText,-1,m_pStr,nBufSize,NULL,NULL); 
			m_pStr[nBufSize] = 0x00;
 		}
	};

	class CUTF8Encoder : public StringHelperImpl<CUTF8Encoder,CHAR>	{
	public:
		explicit CUTF8Encoder(const BYTE* pBuffer , DWORD cbSize) : Length(0)
		{
			int TempSize = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pBuffer, cbSize, NULL, 0);
			PBYTE TempBuffer = (PBYTE)calloc(TempSize+1,sizeof(WCHAR));
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pBuffer, cbSize, (LPWSTR)TempBuffer, TempSize);
 
			Length = WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)TempBuffer,TempSize,NULL,0,NULL,NULL); 
			m_pStr = (LPSTR)calloc(Length+1,sizeof(CHAR));
			WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)TempBuffer,TempSize,m_pStr,Length,NULL,NULL); 
 			free(TempBuffer);
		}
 		explicit CUTF8Encoder(LPCWSTR lpszText) :  Length(0)
		{
 			Length = WideCharToMultiByte(CP_UTF8,0,lpszText,-1,NULL,0,NULL,NULL); 
			m_pStr = (LPSTR)calloc(Length+1,sizeof(CHAR));
			WideCharToMultiByte(CP_UTF8,0,lpszText,-1,m_pStr,Length,NULL,NULL); 
			//m_pStr[nBufSize] = 0x00;
 		}
#if 1
		explicit CUTF8Encoder(LPCSTR lpszText) 
			: CUTF8Encoder((const BYTE*)lpszText, lstrlenA(lpszText))
		{}
#else
		explicit CUTF8Encoder(LPCSTR lpszText) :  Length(0)
		{
 			CWString wString(lpszText);
			int nLengthW = lstrlenW(wString.m_pStr);
			int nLengthA = nLengthW;
 			nLengthA = WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)wString.m_pStr,-1,NULL,0,NULL,NULL) +2 ; 
			m_pStr = (LPSTR)calloc(nLengthA+20,sizeof(CHAR));
			WideCharToMultiByte(CP_UTF8,0,(LPCWSTR)wString.m_pStr,-1,m_pStr,nLengthA ,NULL,NULL) ; 
			char s = m_pStr[nLengthA];
			char s1 = m_pStr[nLengthA-1];
			
			if (m_pStr[nLengthA-1] == -28) {
				m_pStr[nLengthA-1] = 0x00;
			}
  			//m_pStr[nLengthA] = 0x00;
		}
#endif
	private:
		CUTF8Encoder(const CUTF8Encoder&) {}
	public:
		int Length;
	};

	class CUTF8Decoder : public StringHelperImpl<CUTF8Decoder,CHAR>	{
	public:
		explicit CUTF8Decoder(LPCWSTR lpszText) :  Length(0)
		{
			Length = WideCharToMultiByte(CP_ACP, 0, lpszText, -1, NULL, 0, NULL, NULL) + 2;
			m_pStr = (LPSTR)calloc(Length+1,sizeof(CHAR));
			WideCharToMultiByte(CP_ACP, 0, lpszText, -1, m_pStr, Length, NULL, NULL);
			//m_pStr[Length] = 0x00;
 		}
		explicit CUTF8Decoder(LPCSTR lpszText): Length(0) 
		{
			CWString wString(lpszText,CP_UTF8);
			//LPCWSTR wString = CA2W(lpszText,CP_UTF8);
			Length = WideCharToMultiByte(CP_ACP, 0, wString, -1, NULL, 0, NULL, NULL) + 2;
			m_pStr = (LPSTR)calloc(Length+1,sizeof(CHAR));
			WideCharToMultiByte(CP_ACP, 0, wString, -1, m_pStr, Length, NULL, NULL);
		}
	private:
		CUTF8Decoder(const CUTF8Decoder&) {}
	public:
		int Length;
	};

#ifdef _STRING_
	inline std::string& trim(std::string &s)   
	{  
		if (s.empty()) {  
			return s;  
		}  
		s.erase(0,s.find_first_not_of(" "));  
		s.erase(s.find_last_not_of(" ") + 1);  
		return s;  
	}  

	inline void split(std::string& s, std::string& delim,std::vector< std::string >* ret)  
	{  
		size_t last = 0;  
		size_t index=s.find_first_of(delim,last);  
		while (index!=std::string::npos)  
		{  
			ret->push_back(s.substr(last,index-last));  
			last=index+1;  
			index=s.find_first_of(delim,last);  
		}  
		if (index-last>0)  
		{  
			ret->push_back(s.substr(last,index-last));  
		}  
	}  

	inline std::string& tolower(std::string& s) {
		transform(s.begin(), s.end(), s.begin(), ::tolower);  
		return s; 
	}
	inline std::string& toupper(std::string& s) {
		transform(s.begin(), s.end(), s.begin(), ::toupper);  
		return s; 
	}

#endif
};
#endif

// CRC32 哈希帮助函数库
namespace Sign
{
	namespace CRCHelper	
	{
		inline unsigned long* GetHashTable() 
		{
			static bool fInited = false;
			static unsigned long HashTable[256] = {0};
			if (!fInited) {
				unsigned long i,j, v;
 				for (i = 0; i < 256; i++) {
					v = i;
					for (j = 0; j < 8; j++)	{
						if (v & 1) v = (v >> 1) ^ 0xEDB88320;
						else v >>= 1;
					}
					HashTable[i] = v;
				}
				fInited = true;
			}
			return HashTable;
		}
	};
	using namespace StringHelper;

	inline long	CRC_Hash(const PBYTE pBuffer, DWORD cbSize)
	{
		unsigned long  v(0xffffffff);
		unsigned long* HashTable = CRCHelper::GetHashTable();
		int len = cbSize;
		PBYTE buffer = pBuffer;
		while(len--)
			v = (v >> 8) ^ HashTable[(v & 0xFF) ^ *buffer++];
		return (long)(v^0xffffffff);
	}

	inline long	CRC_Hash(LPCSTR lpszText)
	{
		return CRC_Hash((PBYTE)lpszText,lstrlenA(lpszText));
	}

	inline long	CRC_Hash(LPCWSTR lpszText)
	{
		CAString s(lpszText);
		return CRC_Hash((LPCSTR)s);
	}

#ifdef __ATLSTR_H__
	inline long CRC_Hash(CStringA &sText)
	{
		return CRC_Hash((LPCSTR)sText);
	}
#endif

#ifdef _STRING_
	inline long CRC_Hash(std::string& str)
	{
		return CRC_Hash(str.c_str());
	}
#endif
	inline long MAC_Hash()
	{
		static long _Hash = 0;
		if (!_Hash) {
			CHAR _Mac[64] = {0};
			if( StringTool::MacString(_Mac,64) ) {
				_Hash = CRC_Hash(_Mac);
			}
		}
		return _Hash;
	}
	inline long MAC_Hash(LPCSTR lpszMac)
	{
		return CRC_Hash(lpszMac);
	}
	inline long MAC_Hash(LPCWSTR lpszMac)
	{
		return CRC_Hash(lpszMac);
	}

	class CRC32Encoder {
	public:
		explicit CRC32Encoder(const PBYTE pBuffer, size_t nLength)
			: HashValue(0)
		{
			unsigned long  v(0xffffffff);
			unsigned long* HashTable = CRCHelper::GetHashTable();
			size_t len = nLength;
			PBYTE buffer = pBuffer;
			while (len--)
				v = (v >> 8) ^ HashTable[(v & 0xFF) ^ *buffer++];
			HashValue = (v ^ 0xffffffff);
		}
		explicit CRC32Encoder(LPCSTR lpszText)
			: CRC32Encoder((PBYTE)lpszText, lstrlenA(lpszText))
		{}
		explicit CRC32Encoder(LPCWSTR lpszText)
			: CRC32Encoder(CW2A(lpszText))
		{}
	public:
		unsigned long HashValue;
	};
};
 

/*
	MD5 哈希帮助函数库
*/
namespace Sign
{
	namespace MD5Helper
	{
		typedef struct {  
			ULONG i[2];  
			ULONG buf[4];  
			unsigned char in[64];  
			unsigned char digest[16];
		} MD5_CTX;

		typedef VOID (WINAPI *PFN_MD5Init)(MD5_CTX *pContext);
		typedef VOID (WINAPI *PFN_MD5Update)(MD5_CTX* context,PBYTE pInput, UINT cbSize);
		typedef VOID (WINAPI *PFN_MD5Final)(MD5_CTX* context);

		struct MD5Entry {
			PFN_MD5Init		MD5Init;
			PFN_MD5Update	MD5Update;
			PFN_MD5Final	MD5Final;
		};

		inline MD5Entry* GetMD5Entry()
		{
			static MD5Entry _Entry = {0};
			if (_Entry.MD5Init == NULL) {
				HMODULE hModule = ::LoadLibraryA("Cryptdll.dll");
				if (hModule) {
					_Entry.MD5Init = (PFN_MD5Init)(FARPROC)::GetProcAddress(hModule,"MD5Init");
					_Entry.MD5Update = (PFN_MD5Update)(FARPROC)::GetProcAddress(hModule,"MD5Update");
					_Entry.MD5Final = (PFN_MD5Final)(FARPROC)::GetProcAddress(hModule,"MD5Final");
				}
			}
			return &_Entry;
		}
	};
	using namespace MD5Helper;
	inline PBYTE MD5_Hash(const PBYTE pBuffer, DWORD cbInSize, PBYTE pOutBytes, DWORD cbBufferSize)
	{
		ATLASSERT(cbBufferSize >= 16);
 		MD5Entry* _Entry = GetMD5Entry();
		if (_Entry->MD5Init) {
			MD5_CTX ctx = {0};
			_Entry->MD5Init(&ctx);
			_Entry->MD5Update(&ctx, pBuffer,cbInSize);
			_Entry->MD5Final(&ctx);
			memcpy(pOutBytes,ctx.digest,16);
		}
		return pOutBytes;
	}

	inline LPCSTR MD5_Hash(const PBYTE pBuffer, DWORD cbInSize, LPSTR pOutMD5)
	{
		BYTE Code[32] = {0};
		MD5_Hash(pBuffer,cbInSize,Code,32);
		for (int i = 0; i < 16; i++) {
			wsprintfA(&pOutMD5[i*2],"%02x",Code[i]);
		}
		pOutMD5[32] = 0x00;
		return pOutMD5;
	}

	inline LPCWSTR MD5_Hash(const PBYTE pBuffer, DWORD cbInSize, LPWSTR pOutMD5)
	{
		BYTE Code[32] = {0};
		MD5_Hash(pBuffer,cbInSize,Code,32);
		for (int i = 0; i < 16; i++) {
			wsprintfW(&pOutMD5[i*2],L"%02x",Code[i]);
		}
		pOutMD5[32] = 0x00;
		return pOutMD5;
	}

	inline LPCSTR MD5_Hash(LPCSTR pszText,LPSTR pOutMD5)
	{
		BYTE Code[32] = {0};
		MD5_Hash((PBYTE)pszText,lstrlenA(pszText),Code,32);
		for (int i = 0; i < 16; i++) {
			wsprintfA(&pOutMD5[i*2],"%02x",Code[i]);
		}
		pOutMD5[32] = 0x00;
		return pOutMD5;
	}

	inline LPCWSTR MD5_Hash(LPCWSTR pszText, LPWSTR pOutMD5)
	{
		CAString str(pszText);
		BYTE Code[32] = {0};
		MD5_Hash((PBYTE)str.m_pStr,lstrlenA(str.m_pStr),Code,32);
		for (int i = 0; i < 16; i++) {
			wsprintfW(&pOutMD5[i*2],L"%02x",Code[i]);
		}
		pOutMD5[32] = 0x00;
		return pOutMD5;
 	}

#ifdef __ATLSTR_H__
 	inline CStringA MD5_Hash(CStringA &buffer)
	{
		CStringA MD5Hash = "";
		MD5Entry* _Entry = GetMD5Entry();
		if (_Entry->MD5Init) {
			MD5_CTX ctx = {0};
			_Entry->MD5Init(&ctx);
			_Entry->MD5Update(&ctx, (PBYTE)(LPCSTR)buffer,buffer.GetLength());
			_Entry->MD5Final(&ctx);
			buffer.ReleaseBuffer();
			
			for (int i = 0; i < 16; i++) {
 				MD5Hash.AppendFormat("%02x",ctx.digest[i]);
 			}
		}
		return MD5Hash;
	}
#endif

#ifdef _STRING_
	inline std::string MD5_Hash(std::string& str) {
		std::string MD5Hash = "";
		MD5Entry* _Entry = GetMD5Entry();
		if (_Entry->MD5Init) {
			MD5_CTX ctx = {0};
			_Entry->MD5Init(&ctx);
			_Entry->MD5Update(&ctx, (PBYTE)str.c_str(),str.length());
			_Entry->MD5Final(&ctx);
 			
			for (int i = 0; i < 16; i++) {
				char sTemp[3] = {0};
				wsprintfA(sTemp,"%02x",ctx.digest[i]);
				MD5Hash += sTemp;
			}
		}
		return MD5Hash;
	}
#endif
	class CMD5Encoder {
	public:
		explicit CMD5Encoder(BYTE* pBuffer, size_t nLength)
		{
			memset(MD5, 0, 16);
			MD5Entry* _Entry = GetMD5Entry();
			if (_Entry->MD5Init) {
				MD5_CTX ctx = { 0 };
				_Entry->MD5Init(&ctx);
				_Entry->MD5Update(&ctx, pBuffer, nLength);
				_Entry->MD5Final(&ctx);
				memcpy(MD5, ctx.digest, 16);
			}
		}
		explicit CMD5Encoder(LPCSTR lpszText)
			: CMD5Encoder((PBYTE)lpszText, lstrlenA(lpszText))
		{}
		explicit CMD5Encoder(LPCWSTR lpszText)
			: CMD5Encoder(CW2A(lpszText))
		{}
	private:
		CMD5Encoder(const CMD5Encoder&) {}
	public:
		LPCSTR ToString(LPSTR pszOut, bool upper)
		{
			for (int i = 0; i < 16; i++) {
				wsprintfA(&pszOut[i * 2], upper ? "%02X" : "%02x", MD5[i]);
			}
			pszOut[32] = 0x00;
			return pszOut;
		}
		LPCWSTR ToString(LPWSTR pszOut, bool upper)
		{
			for (int i = 0; i < 16; i++) {
				wsprintfW(&pszOut[i * 2], upper ? L"%02X" : L"%02x", MD5[i]);
			}
			pszOut[32] = 0x00;
			return pszOut;
		}
#ifdef _STRING_
		std::string ToStdString(bool upper = false)
		{
			CHAR szMD5[33];
			return ToString(szMD5, upper);
		}
		std::wstring ToStdStringW(bool upper = false)
		{
			wchar_t szMD5[33];
			return ToString(szMD5, upper);
		}
		static std::string StaticToStdString(CHAR* MD5, bool upper = false)
		{
			CHAR szMD5[64];;
			for (int i = 0; i < 16; i++) {
				wsprintfA(&szMD5[i * 2], upper ? "%02X" : "%02x", MD5[i]);
			}
			szMD5[32] = 0x00; 
 			std::string _md5 = szMD5; 
			return _md5; 
		}
#endif
#ifdef __ATLSTR_H__
		CStringA ToStringA(bool upper = false)
		{
			CStringA sRet;
			for (int i = 0; i < 16; i++) {
				sRet.AppendFormat(upper ? "%02X" : "%02x", MD5[i]);
			}
			return sRet;
		}
		CStringW ToStringW(bool upper = false)
		{
			CStringW sRet;
			for (int i = 0; i < 16; i++) {
				sRet.AppendFormat(upper ? L"%02X" : L"%02x", MD5[i]);
			}
			return sRet;
		}
#endif
	public:
		unsigned char MD5[16];
	};
};

namespace Base64
{
	inline int Base64EncodeGetRequiredLength(int nSrcLen, DWORD dwFlags)
	{
		__int64 nSrcLen4 = static_cast<__int64>(nSrcLen)* 4;
		int nRet = static_cast<int>(nSrcLen4 / 3);

		if ((dwFlags & BASE64_FLAG_NOPAD) == 0)
			nRet += nSrcLen % 3;

		int nCRLFs = nRet / 76 + 1;
		int nOnLastLine = nRet % 76;

		if (nOnLastLine) {
			if (nOnLastLine % 4)
				nRet += 4 - (nOnLastLine % 4);
		}
		nCRLFs *= 2;
		if ((dwFlags & BASE64_FLAG_NOCRLF) == 0)
			nRet += nCRLFs;
		return nRet;
	}

	inline int Base64DecodeGetRequiredLength(int nSrcLen) throw()
	{
		return nSrcLen;
	}

	inline LPSTR Base64Encode(const BYTE *pbSrcData, int nSrcLen, LPSTR szOutBuffer, int *pnDestLen, DWORD dwFlags) throw()
	{
		static const char s_chBase64EncodingTable[64] = {
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q',
			'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
			'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
			'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

		LPSTR szDest = szOutBuffer;
		if (!pbSrcData || !szDest || !pnDestLen) {
			return NULL;
		}
		if (*pnDestLen < Base64EncodeGetRequiredLength(nSrcLen, dwFlags)) {
			return NULL;
		}

		int nWritten(0);
		int nLen1((nSrcLen / 3) * 4);
		int nLen2(nLen1 / 76);
		int nLen3(19);

		for (int i = 0; i <= nLen2; i++) {
			if (i == nLen2)
				nLen3 = (nLen1 % 76) / 4;
			for (int j = 0; j<nLen3; j++)	{
				DWORD dwCurr(0);
				for (int n = 0; n<3; n++)	{
					dwCurr |= *pbSrcData++;
					dwCurr <<= 8;
				}
				for (int k = 0; k<4; k++)	{
					BYTE b = (BYTE)(dwCurr >> 26);
					*szDest++ = s_chBase64EncodingTable[b];
					dwCurr <<= 6;
				}
			}
			nWritten += nLen3 * 4;
			if ((dwFlags & BASE64_FLAG_NOCRLF) == 0)	{
				*szDest++ = '\r'; *szDest++ = '\n';
				nWritten += 2;
			}
		}
		if (nWritten && (dwFlags & BASE64_FLAG_NOCRLF) == 0) {
			szDest -= 2;
			nWritten -= 2;
		}

		nLen2 = (nSrcLen % 3) ? (nSrcLen % 3 + 1) : 0;
		if (nLen2) {
			DWORD dwCurr(0);
			for (int n = 0; n<3; n++)	{
				if (n<(nSrcLen % 3))
					dwCurr |= *pbSrcData++;
				dwCurr <<= 8;
			}
			for (int k = 0; k<nLen2; k++)	{
				BYTE b = (BYTE)(dwCurr >> 26);
				*szDest++ = s_chBase64EncodingTable[b];
				dwCurr <<= 6;
			}
			nWritten += nLen2;
			if ((dwFlags & BASE64_FLAG_NOPAD) == 0) {
				nLen3 = nLen2 ? 4 - nLen2 : 0;
				for (int j = 0; j<nLen3; j++) {
					*szDest++ = '=';
				}
				nWritten += nLen3;
			}
		}
		*pnDestLen = nWritten;
		return szOutBuffer;
	}

	inline int DecodeBase64Char(unsigned int ch) throw()
	{
		// returns -1 if the character is invalid or should be skipped
		// otherwise, returns the 6-bit code for the character from the encoding table
		if (ch >= 'A' && ch <= 'Z')	return ch - 'A' + 0;	// 0 range starts at 'A'
		if (ch >= 'a' && ch <= 'z')	return ch - 'a' + 26;	// 26 range starts at 'a'
		if (ch >= '0' && ch <= '9')	return ch - '0' + 52;	// 52 range starts at '0'
		if (ch == '+')	return 62;
		if (ch == '/')	return 63;
		return -1;
	}

	inline BYTE* Base64Decode(LPCSTR szSrc, int nSrcLen, BYTE *pbDest, int *pnDestLen) throw()
	{
		// walk the source buffer
		// each four character sequence is converted to 3 bytes
		// CRLFs and =, and any characters not in the encoding table 
		// are skiped
		if (szSrc == NULL || pnDestLen == NULL)	{
			return NULL;
		}
		LPCSTR szSrcEnd = szSrc + nSrcLen;
		int nWritten = 0;

		BOOL bOverflow = (pbDest == NULL) ? TRUE : FALSE;
		while (szSrc < szSrcEnd && (*szSrc) != 0) {
			DWORD dwCurr = 0;
			int i;
			int nBits = 0;
			for (i = 0; i<4; i++)	{
				if (szSrc >= szSrcEnd)
					break;
				int nCh = DecodeBase64Char(*szSrc);
				szSrc++;
				if (nCh == -1) {
					// skip this char
					i--;
					continue;
				}
				dwCurr <<= 6;
				dwCurr |= nCh;
				nBits += 6;
			}
			if (!bOverflow && nWritten + (nBits / 8) > (*pnDestLen))
				bOverflow = TRUE;

			// dwCurr has the 3 bytes to write to the output buffer
			// left to right
			dwCurr <<= 24 - nBits;
			for (i = 0; i<nBits / 8; i++) {
				if (!bOverflow) {
					*pbDest = (BYTE)((dwCurr & 0x00ff0000) >> 16);
					pbDest++;
				}
				dwCurr <<= 8;
				nWritten++;
			}
		}
		*pnDestLen = nWritten;

		if (bOverflow) {
			if (pbDest != NULL) {
			}
			return NULL;
		}
		return pbDest;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	inline LPSTR Base64Encode(const BYTE* pbSrcData, int nSrcLen, LPSTR* ppDest, DWORD dwFlags)
	{
		int nLength = Base64EncodeGetRequiredLength(nSrcLen, dwFlags);
		LPSTR pBuffer = (LPSTR)calloc(nLength + 1, 1);
		int nOutLen = nLength;
		if (!Base64Encode(pbSrcData, nSrcLen, pBuffer, &nOutLen, dwFlags)) {
			__SAFE_FREE_PTR(pBuffer);
		}
		else {
			pBuffer[nOutLen] = 0x00;
			*ppDest = pBuffer;
		}
		return pBuffer;
	}

	inline LPSTR Base64Encode(const BYTE* pbSrcData, int nSrcLen, DWORD dwFlags)
	{
		LPSTR pBuffer = NULL;
		return Base64Encode(pbSrcData, nSrcLen, &pBuffer, dwFlags);
	}

	inline BYTE* Base64Decode(LPCSTR szSrc, BYTE** ppDest, int* pnDestLen)
	{
		int nLength = lstrlenA(szSrc);
		PBYTE pBuffer = (PBYTE)calloc(nLength, 1);
		int nOutLength = nLength;
		if (Base64Decode(szSrc, nLength, pBuffer, &nOutLength)) {
			pBuffer[nOutLength] = 0x00;
			*ppDest = pBuffer;
			*pnDestLen = nOutLength;
			return pBuffer;
		}
		else {
			*ppDest = NULL;
			__SAFE_FREE_PTR(pBuffer);
			*pnDestLen = 0;
			return NULL;
		}
	}

	inline BYTE* Base64Decode(LPCSTR szSrc, int* pnDestLen)
	{
		BYTE* pBuffer = NULL;
		return Base64Decode(szSrc, &pBuffer, pnDestLen);
	}

#ifdef __ATLSTR_H__
	inline CStringA& Base64Encode(const BYTE* pbSrcData, int nSrcLen, CStringA& sDest, DWORD dwFlags)
	{
		int nLength = Base64EncodeGetRequiredLength(nSrcLen, dwFlags);
		LPSTR pszDest = sDest.GetBufferSetLength(nLength);
		ZeroMemory(pszDest, nLength);
		int nOutLength = nLength;
		if (Base64Encode(pbSrcData, nSrcLen, pszDest, &nOutLength, dwFlags)) {
			pszDest[nOutLength] = 0x00;
			sDest.ReleaseBuffer();
		}
		else {
			sDest.ReleaseBuffer();
			sDest = "";
		}
		return sDest;
	}

	inline CStringA& Base64Encode(const CStringA& sText, CStringA& sDest, DWORD dwFlags)
	{
		return Base64Encode((const BYTE*)(LPCSTR)sText, sText.GetLength(), sDest, dwFlags);
	}

	inline CStringA Base64Encode(const CStringA& sText, DWORD dwFlags)
	{
		CStringA sTemp;
		return Base64Encode(sText, sTemp, dwFlags);
	}

	inline CStringA& Base64Decode(const CStringA& sText, CStringA& sDest)
	{
		int nLength = sText.GetLength();
		LPSTR p = sDest.GetBufferSetLength(nLength);
		ZeroMemory(p, nLength);
		int nOutLength = nLength;
		if (!Base64Decode((LPCSTR)sText, nLength, (PBYTE)p, &nOutLength)) {
			sDest.ReleaseBuffer();
			sDest = "";
		}
		else {
			p[nOutLength] = 0x00;
			sDest.ReleaseBuffer();
		}
		return sDest;
	}

	inline CStringA Base64Decode(const CStringA& sText)
	{
		CStringA sTemp;
		return Base64Decode(sText, sTemp);
	}

#endif
#ifdef _STRING_
	inline std::string& Base64Encode(const BYTE* pbSrcData, int nSrcLen, std::string& sDest, DWORD dwFlags)
	{
		int nLength = Base64EncodeGetRequiredLength(nSrcLen, dwFlags);
		sDest.resize(nLength);
		LPSTR pszDest = &sDest[0];
		int nOutLength = nLength;
		if (!Base64Encode(pbSrcData, nSrcLen, pszDest, &nOutLength, dwFlags)) {
			sDest = "";
		}
		return sDest;
	}

	inline std::string& Base64Encode(const std::string& sText, std::string& sDest, DWORD dwFlags)
	{
		return Base64Encode((const BYTE*)sText.c_str(), sText.length(), sDest, dwFlags);
	}

	inline std::string  Base64Encode(const std::string& sText, DWORD dwFlags)
	{
		std::string temp;
		return Base64Encode(sText, temp, dwFlags);
	}

	inline std::string& Base64Decode(const std::string& sText, std::string& sDest)
	{
		int len = sText.length();
		int outlen = len;
		sDest.resize(len);
		LPSTR p = &sDest[0];
		if (Base64Decode(sText.c_str(), len, (PBYTE)p, &outlen)) {
			sDest.resize(outlen);
		}
		else sDest = "";
		return sDest;
	}

	inline std::string  Base64Decode(const std::string& sText)
	{
		std::string temp;
		return Base64Decode(sText, temp);
	}
#endif

	class CBase64Encoder : public StringHelper::StringHelperImpl < CBase64Encoder, CHAR> {
	public:
		explicit CBase64Encoder(const BYTE* pBuffer, size_t cbSize) {
			Base64Encode(pBuffer, cbSize, &m_pStr, (DWORD)(BASE64_FLAG_NOPAD | BASE64_FLAG_NOCRLF));
		}
		explicit CBase64Encoder(LPCSTR pszText)
			: CBase64Encoder((BYTE*)pszText, lstrlenA(pszText))
		{}
		explicit CBase64Encoder(LPCWSTR pszText)
			: CBase64Encoder(CW2A(pszText))
		{}
	private:
		CBase64Encoder(const CBase64Encoder&) {}
 	};

	class CBase64Decoder : public StringHelper::StringHelperImpl < CBase64Decoder, CHAR > {
	public:
		explicit CBase64Decoder() {}
	private:
		CBase64Decoder(const CBase64Decoder&) {}
	public:
		size_t Length;
	};
};

namespace StringHelper {
	typedef Base64::CBase64Encoder CBase64Encoder;
	typedef Base64::CBase64Decoder CBase64Decoder;
}

/*HEX 编码帮助库*/
namespace HEX
{
	inline int HexEncodeGetRequiredLength(int nSrcLen)
	{
		__int64 nRet64 = 2 * static_cast<__int64>(nSrcLen)+1;
		ATLENSURE(nRet64 <= INT_MAX && nRet64 >= INT_MIN);
		int nRet = static_cast<int>(nRet64);
		return nRet;
	}

	inline int HexDecodeGetRequiredLength(int nSrcLen) throw()
	{
		return nSrcLen / 2;
	}

	inline LPSTR HexEncode(const BYTE *pbSrcData, int nSrcLen, LPSTR szDest, int *pnDestLen, bool upper) throw()
	{
		static const char s_chHexChars_0[16] =
		{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

		static const char s_chHexChars_1[16] =
		{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

		const char* s_chHexChars = upper ? s_chHexChars_0 : s_chHexChars_1;

		if (!pbSrcData || !szDest || !pnDestLen){
			return NULL;
		}
		if (*pnDestLen < HexEncodeGetRequiredLength(nSrcLen))
		{
			ATLASSERT(FALSE);
			return NULL;
		}
		int nRead = 0, nWritten = 0;
		BYTE ch;
		const BYTE* p = pbSrcData;
		LPSTR p1 = szDest;
		while (nRead < nSrcLen)	{
			ch = *p++;
			nRead++;
			*p1++ = s_chHexChars[(ch >> 4) & 0x0F];
			*p1++ = s_chHexChars[ch & 0x0F];
			nWritten += 2;
		}

		*pnDestLen = nWritten;
		return szDest;
	}

	inline char GetHexValue(char ch) throw()
	{
		if (ch >= '0' && ch <= '9')	return (ch - '0');
		if (ch >= 'A' && ch <= 'F')	return (ch - 'A' + 10);
		if (ch >= 'a' && ch <= 'f')	return (ch - 'a' + 10);
		return HEX_INVALID;
	}

	inline PBYTE HexDecode(LPCSTR pSrcData, int nSrcLen, LPBYTE pbOut, int* pnDestLen) throw()
	{
		LPBYTE pbDest = pbOut;
		if (!pSrcData || !pbDest || !pnDestLen)	{
			return NULL;
		}
		if (*pnDestLen < HexDecodeGetRequiredLength(nSrcLen))	{
			ATLASSERT(FALSE);
			return NULL;
		}
		int nRead = 0;
		int nWritten = 0;
		while (nRead < nSrcLen)
		{
			char ch1 = GetHexValue((char)*pSrcData++);
			char ch2 = GetHexValue((char)*pSrcData++);
			if ((ch1 == HEX_INVALID) || (ch2 == HEX_INVALID))
			{
				return FALSE;
			}
			*pbDest++ = (BYTE)(16 * ch1 + ch2);
			nWritten++;
			nRead += 2;
		}
		*pnDestLen = nWritten;
		return pbOut;
	}

	// 内置分配缓存
	inline LPSTR HexEncode(const BYTE* pbSrcData, int nSrcLen, LPSTR* ppDest, bool upper = true)
	{
		if (!pbSrcData || nSrcLen <= 0 || !ppDest)  {
			ATLASSERT(FALSE);
			return NULL;
		}
		int nLength = HexEncodeGetRequiredLength(nSrcLen);
		*ppDest = (LPSTR)calloc(nLength, 1);

		if (!HexEncode(pbSrcData, nSrcLen, *ppDest, &nLength, upper))	{
			__SAFE_FREE_PTR(*ppDest);
		}
		else { (*ppDest)[nLength] = 0x00; }
		return *ppDest;
	}

	inline LPSTR HexEncode(const BYTE* pbSrcData, int nSrcLen, bool upper = true)
	{
		LPSTR pBuffer = NULL;
		return HexEncode(pbSrcData, nSrcLen, &pBuffer, upper);
	}

	inline PBYTE HexDecode(LPCSTR szSrc, BYTE** ppDest, int* pnDestLen)
	{
		if (!szSrc || !ppDest || !pnDestLen) {
			ATLASSERT(FALSE);
			return NULL;
		}
		int nLength = lstrlenA(szSrc);
		if (nLength <= 0) {
			ATLASSERT(FALSE);
			return NULL;
		}
		*pnDestLen = HexDecodeGetRequiredLength(nLength);
		*ppDest = (PBYTE)calloc(nLength, 1);
		if (!HexDecode(szSrc, nLength, *ppDest, pnDestLen))
		{
			__SAFE_FREE_PTR(*ppDest);
			*pnDestLen = 0;
		}
		return *ppDest;
	}

	inline PBYTE HexDecode(LPCSTR szSrc, int* pnDestLen)
	{
		PBYTE pBuffer = NULL;
		return HexDecode(szSrc, &pBuffer, pnDestLen);
	}
#ifdef __ATLSTR_H__
	inline CStringA& HexEncode(const BYTE* pbSrcData, int nSrcLen, CStringA& sDest)
	{
		if (!pbSrcData || nSrcLen <= 0)  {
			sDest = "";
			return sDest;
		}
		int nLength = HexEncodeGetRequiredLength(nSrcLen);
		LPSTR pBuffer = sDest.GetBufferSetLength(nLength);
		memset(pBuffer, 0, nLength);
		if (!HexEncode(pbSrcData, nSrcLen, pBuffer, &nLength))	{
			sDest.ReleaseBuffer();
			sDest = "";
		}
		else {
			pBuffer[nLength] = 0x00;
			sDest.ReleaseBuffer();
		}
		return sDest;
	}

	inline PBYTE HexDecode(const CStringA& sSrc, PBYTE* ppDest, int* pnDestLen)
	{
		return HexDecode((LPCSTR)sSrc, ppDest, pnDestLen);
	}

	inline CStringA& HexEncode(const CStringA& sSrc, CStringA& sDest)
	{
		return HexEncode((const BYTE*)(LPCSTR)sSrc, sSrc.GetLength(), sDest);
	}
	inline CStringA HexEncode(const CStringA& sSrc)
	{
		CStringA sTemp;
		return HexEncode(sSrc, sTemp);
	}

	inline CStringA& HexDecode(const CStringA& sSrc, CStringA& sDest)
	{
		int nLength = sSrc.GetLength();
		if (nLength <= 0) {
			sDest = "";
			return sDest;
		}
		int nDestLength = HexDecodeGetRequiredLength(nLength);
		PBYTE pBuffer = (PBYTE)sDest.GetBufferSetLength(nDestLength + 1);
		memset(pBuffer, 0, nDestLength + 1);
		if (!HexDecode(sSrc, nLength, pBuffer, &nLength))
		{
			sDest.ReleaseBuffer();
			sDest = "";
		}
		else {
			pBuffer[nDestLength] = 0x00;
			sDest.ReleaseBuffer();
		}
		return sDest;
	}

	inline CStringA HexDecode(const CStringA& sSrc)
	{
		CStringA sTemp;
		return HexDecode(sSrc, sTemp);
	}
#endif
#ifdef _STRING_
	inline std::string& HexEncode(const BYTE* pbSrcData, int nSrcLen, std::string& sDest, bool upper = false)
	{
		int nLength = HexEncodeGetRequiredLength(nSrcLen);
		sDest.resize(nLength + 1);
		if (HexEncode(pbSrcData, nSrcLen, (LPSTR)&sDest[0], &nLength, upper)) {
			sDest[nLength] = 0x00;
		}
		else { sDest = ""; }
		return sDest;
	}

	inline PBYTE HexDecode(const std::string& sSrc, PBYTE* ppDest, int* pnDestLen)
	{
		return HexDecode((LPCSTR)sSrc.c_str(), ppDest, pnDestLen);
	}

	inline std::string& HexEncode(const std::string& sSrc, std::string& sDest, bool upper = false)
	{
		return HexEncode((const BYTE*)sSrc.c_str(), sSrc.length(), sDest, upper);
	}

	inline std::string  HexEncode(const std::string& sSrc, bool upper)
	{
		std::string sTemp;
		return HexEncode(sSrc, sTemp, upper);
	}

	inline std::string& HexDecode(const std::string& sSrc, std::string& sDest)
	{
		int nLength = sSrc.length();
		if (nLength <= 0) {
			ATLASSERT(FALSE);
			sDest = "";
			return sDest;
		}
		int nDestLength = HexDecodeGetRequiredLength(nLength);
		sDest.resize(nDestLength + 1);
		if (!HexDecode(sSrc.c_str(), sSrc.length(), (BYTE*)&sDest[0], &nDestLength))
		{
			sDest = "";
		}
		else {
			sDest[nDestLength] = 0x00;
		}
		return sDest;
	}
	inline std::string  HexDecode(const std::string& sSrc)
	{
		std::string sTemp;
		return HexDecode(sSrc, sTemp);
	}
#endif

	class CHexEncoder : public StringHelper::StringHelperImpl<CHexEncoder, CHAR> {
	public:
		explicit CHexEncoder(const BYTE* pBuffer, size_t cbLength, bool upper = false)
			: Length(0)
		{
			Length = HexEncodeGetRequiredLength(cbLength);
			m_pStr = (CHAR*)calloc(Length + 1, 1);
			HexEncode(pBuffer, cbLength, m_pStr, (int*)&Length, upper);
		}
		explicit CHexEncoder(LPCSTR pszText, bool upper = false)
			: CHexEncoder((BYTE*)pszText, lstrlenA(pszText), upper)
		{}
		explicit CHexEncoder(LPCWSTR pszText, bool upper = false)
			: CHexEncoder(CW2A(pszText), upper)
		{}
	public:
		int Length;
	private:
		CHexEncoder(const CHexEncoder&) {}
	};

	class CHexDecoder : public StringHelper::StringHelperImpl < CHexDecoder, CHAR > {
	public:
		explicit CHexDecoder(LPCSTR pszText)
			: Length(0)
		{
			int nLength = lstrlenA(pszText);
			Length = HexDecodeGetRequiredLength(nLength);
			m_pStr = (CHAR*)calloc(Length+1, 1);
			HexDecode(pszText, nLength, (LPBYTE)m_pStr, &Length);
		}
		explicit CHexDecoder(LPCWSTR pszText)
			: CHexDecoder(CW2A(pszText))
		{}
	private:
		CHexDecoder(const CHexDecoder&) {}
	public:
		int Length;
	};
};

namespace StringHelper {
	typedef Sign::CRC32Encoder CRC32Encoder;
	typedef Sign::CMD5Encoder  CMD5Encoder;
	typedef HEX::CHexEncoder CHexEncoder;
	typedef HEX::CHexDecoder CHexDecoder;
};
namespace Encrypt
{
namespace RandXOR
	{
		inline long GetHashKey(LPCSTR lpszKey)
		{
			return Sign::CRC_Hash(lpszKey);
		}
		inline long GetHashKey(LPCWSTR lpszKey)
		{
			StringHelper::CAString sKey(lpszKey);
			return Sign::CRC_Hash(sKey.m_pStr);
		}
		//_Seed = (-1 * (_Seed % 1000));

		//int nCount = 1;
		//while (true){
		//int _Rand = ((_Seed = _Seed * 214013L + 2531011L) >> 16) & 0x7fff;
		//unsigned char c = _Rand % 256;

		inline const BYTE* EncryptBuffer(PBYTE pBuffer,DWORD cbSize, LONG lKey) 
		{
			if (!pBuffer || cbSize <= 0) {
				ATLASSERT(FALSE);
				return NULL;
			}
			LONG lCode = lKey % 0xFF;
			if (lCode > 0) lCode *= -1;

			//srand(lCode);
			int _Seed = lCode;
			for (DWORD i = 0; i < cbSize; i++) {
				int _Rand = ((_Seed = _Seed * 214013L + 2531011L) >> 16) & 0x7fff;
				pBuffer[i] = pBuffer[i] ^ (_Rand % 256);
			}
			return pBuffer;
		}
		inline const BYTE* EncryptBuffer(const BYTE* pBuffer, DWORD cbSize, BYTE** ppBuffer, LONG lKey) 
		{
			if (!pBuffer || !cbSize || !ppBuffer) {
				ATLASSERT(FALSE);
				return NULL;
			}
			*ppBuffer = (PBYTE)malloc(cbSize);
			memcpy(*ppBuffer, pBuffer, cbSize);
			return EncryptBuffer(*ppBuffer,cbSize,lKey);
		}
 		inline const BYTE* DecryptBuffer(PBYTE pBuffer,DWORD cbSize, LONG lKey)
		{
			return EncryptBuffer(pBuffer,cbSize,lKey);
		}
		inline const BYTE* DecryptBuffer(const BYTE* pBuffer, DWORD cbSize, BYTE** ppBuffer, LONG lKey)
		{
			return EncryptBuffer(pBuffer,cbSize,ppBuffer,lKey);
		}

		inline LPCSTR EncryptString(LPCSTR _SrcText,LPSTR* _EnText, LONG lKey, int nCoder ) 
		{
			if (nCoder != BASE64 && nCoder != HEX && nCoder != UTF8) {
				ATLASSERT(FALSE);
				return NULL;
			}

			int nLength = lstrlenA(_SrcText);
			if (!nLength || !_EnText) {
 				return "";
			}
			PSTR EnText = NULL;
			bool fSuccssed = false;
 
			PBYTE EnBuffer = (PBYTE)calloc(nLength,1);
			memcpy(EnBuffer,_SrcText,nLength);
			if (!EncryptBuffer(EnBuffer,nLength,lKey))
				goto _Exit;

			if (nCoder == BASE64) {
				EnText = Base64::Base64Encode(EnBuffer,nLength);
			}
			else if (nCoder == HEX) {
				EnText = HEX::HexEncode(EnBuffer,nLength,true);
			}
			else if (nCoder == UTF8) {
				StringHelper::CUTF8Encoder _Coder((const BYTE*)EnBuffer,nLength);
				EnText = _Coder.Detach();
			}

			fSuccssed = true;
		_Exit:
			__SAFE_FREE_PTR(EnBuffer);
			if (!fSuccssed && EnText) {
				__SAFE_FREE_PTR(EnText);
			}
			*_EnText = EnText;
			return *_EnText;
		}

		inline LPCWSTR EncryptString(LPCSTR _SrcText,LPWSTR* _EnText, LONG lKey, int nCoder)
		{
			LPSTR pBuffer = NULL;
			if (EncryptString(_SrcText,&pBuffer,lKey,nCoder)) {
				StringHelper::CWString s(pBuffer);
				*_EnText = s.Detach();
			}
			__SAFE_FREE_PTR(pBuffer);
			return *_EnText;
		}

		inline LPCSTR DecryptString(LPCSTR _EnText,LPSTR* _SrcText, LONG lKey, int nCoder)
		{
			unsigned char* EnBuffer	= NULL;
			BYTE* SrcBuffer = NULL;
			bool fSuccssed = false;
			int nLength = 0;

			if (!_EnText || !_SrcText) {
				ATLASSERT(FALSE);
				goto _Exit;
			}
			if (nCoder != BASE64 && nCoder != HEX && nCoder != UTF8) {
				ATLASSERT(FALSE);
				goto _Exit;
			}

			if (nCoder == BASE64){
				EnBuffer = Base64::Base64Decode(_EnText,&nLength);
			}
			else if (nCoder == HEX)	{
				EnBuffer = HEX::HexDecode(_EnText,&nLength);
			}
			else if (nCoder == UTF8) {
				StringHelper::CUTF8Decoder _Coder(_EnText);
				nLength = _Coder.Length;
				EnBuffer = (BYTE*)_Coder.Detach();
			}

			if (!EnBuffer || nLength <= 0) {
				ATLASSERT(FALSE);
				goto _Exit;
			}
			SrcBuffer = (BYTE*)calloc(nLength+1,1);
			memcpy(SrcBuffer, EnBuffer, nLength);
			if (!DecryptBuffer(SrcBuffer,nLength,lKey))
			{
				ATLASSERT(FALSE);
				goto _Exit;
			}
			SrcBuffer[nLength] = 0x00;
			fSuccssed = true;
		_Exit:
			__SAFE_FREE_PTR(EnBuffer);

			if (!fSuccssed && SrcBuffer) {
				__SAFE_FREE_PTR(SrcBuffer);
			}
			*_SrcText = (LPSTR)SrcBuffer;
			return (LPCSTR)SrcBuffer;
		}

		inline LPCWSTR DecryptString(LPCSTR _EnText,LPWSTR* _SrcText, LONG lKey, int nCoder)
		{
			LPSTR pBuffer = NULL;
			if (DecryptString(_EnText,&pBuffer,lKey,nCoder)) {
				StringHelper::CWString s(pBuffer);
				*_SrcText = s.Detach();
			}
			__SAFE_FREE_PTR(pBuffer);
			return *_SrcText;
		}

		inline LPCSTR EncryptString(LPCSTR _SrcText, LPSTR* _EnText, LPCSTR _Key, int nCoder)
		{
			return EncryptString(_SrcText,_EnText,GetHashKey(_Key),nCoder);
		}
		inline LPCWSTR EncryptString(LPCSTR _SrcText, LPWSTR* _EnText, LPCWSTR _Key, int nCoder)
		{
			return EncryptString(_SrcText,_EnText,GetHashKey(_Key),nCoder);
		}
		inline LPCSTR DecryptString(LPCSTR _EnText, LPSTR* _SrcText, LPCSTR _Key, int nCoder)
		{
			return DecryptString(_EnText,_SrcText,GetHashKey(_Key),nCoder);
		}
		inline LPCWSTR DecryptString(LPCSTR _EnText, LPWSTR* _SrcText, LPCWSTR _Key, int nCoder)
		{
			return DecryptString(_EnText,_SrcText,GetHashKey(_Key),nCoder);
		}

#ifdef __ATLSTR_H__
		inline CStringA  EncryptString(CStringA _SrcText,CStringA _Key, int nCoder)
		{
			CStringA _EnText ="" ;
			LPSTR pBuffer = NULL;
			if (EncryptString(_SrcText,&pBuffer,Sign::CRC_Hash(_Key),nCoder)) {
				_EnText = pBuffer;
			}
			__SAFE_FREE_PTR(pBuffer);
			return _EnText;

		}

		inline CStringA  DecryptString(CStringA _EnText, CStringA _Key, int nCoder)
		{
			CStringA _SrcText = "";
			LPSTR pBuffer = NULL;
			if (DecryptString(_EnText,&pBuffer,Sign::CRC_Hash(_Key),nCoder)) {
				_SrcText = pBuffer;
			}
			__SAFE_FREE_PTR(pBuffer);
			return _SrcText;
		}

		inline CStringA& EncryptString(LPCSTR _SrcText, CStringA& _EnText, LONG lKey, int nCoder)
		{
			LPSTR pBuffer = NULL;
			if (EncryptString(_SrcText,&pBuffer,lKey,nCoder)) {
				_EnText = pBuffer;
			}
			__SAFE_FREE_PTR(pBuffer);
			return _EnText;
		}

		inline CStringW& EncryptString(LPCSTR _SrcText, CStringW& _EnText, LONG lKey, int nCoder)
		{
			LPWSTR pBuffer = NULL;
			if (EncryptString(_SrcText,&pBuffer,lKey,nCoder)) {
				_EnText = pBuffer;
			}
			__SAFE_FREE_PTR(pBuffer);
			return _EnText;
		}
		inline CStringA& DecryptString(LPCSTR _EnText, CStringA& _SrcText, LONG lKey, int nCoder)
		{
			LPSTR pBuffer = NULL;
			if (DecryptString(_EnText,&pBuffer,lKey,nCoder)) {
				_SrcText = pBuffer;
			}
			__SAFE_FREE_PTR(pBuffer);
			return _SrcText;
		}
		inline CStringW& DecryptString(LPCSTR _EnText ,CStringW& _SrcText, LONG lKey, int nCoder)
		{
			LPWSTR pBuffer = NULL;
			if (DecryptString(_EnText,&pBuffer,lKey,nCoder)) {
				_SrcText = pBuffer;
			}
			__SAFE_FREE_PTR(pBuffer);
			return _SrcText;
		}

		inline CStringA& EncryptString(LPCSTR _SrcText, CStringA& _EnText, LPCSTR _Key, int nCoder)
		{
			return EncryptString(_SrcText,_EnText,GetHashKey(_Key),nCoder);
		}
		inline CStringW& EncryptString(LPCSTR _SrcText, CStringW& _EnText, LPCWSTR _Key, int nCoder)
		{
			return EncryptString(_SrcText,_EnText,GetHashKey(_Key),nCoder);
		}
		inline CStringA& DecryptString(LPCSTR _EnText, CStringA& _SrcText, LPCSTR _Key, int nCoder)
		{
			return DecryptString(_EnText,_SrcText,GetHashKey(_Key),nCoder);
		}
		inline CStringW& DecryptString(LPCSTR _EnText, CStringW& _SrcText, LPCWSTR _Key, int nCoder)
		{
			return DecryptString(_EnText,_SrcText,GetHashKey(_Key),nCoder);
		}
#endif

#ifdef _STRING_
		inline std::string& EncryptString(LPCSTR _SrcText, std::string& _EnText, LONG lKey, int nCoder)
		{
			LPSTR pBuffer = NULL;
			if (EncryptString(_SrcText,&pBuffer,lKey,nCoder)) {
				_EnText = pBuffer;
			}
			__SAFE_FREE_PTR(pBuffer);
			return _EnText;
		}
		inline std::string& DecryptString(LPCSTR _EnText, std::string& _SrcText, LONG lKey, int nCoder)
		{
			LPSTR pBuffer = NULL;
			if (DecryptString(_EnText,&pBuffer,lKey,nCoder)) {
				_SrcText = pBuffer;
			}
			__SAFE_FREE_PTR(pBuffer);
			return _SrcText;
		}
		inline std::string& EncryptString(LPCSTR _SrcText, std::string& _EnText, const std::string& _Key, int nCoder)
		{
			return EncryptString(_SrcText,_EnText,GetHashKey(_Key.c_str()),nCoder);
		}
		inline std::string& DecryptString(LPCSTR _EnText, std::string& _SrcText, const std::string& _Key, int nCoder)
		{
			return DecryptString(_EnText,_SrcText,GetHashKey(_Key.c_str()),nCoder);
		}
#endif
		struct CEncryptString : public StringHelper::StringHelperImpl<CEncryptString, CHAR> {
		public:
			explicit CEncryptString(LPCSTR pszText, LPCSTR pszKey, int nCode = Encrypt::HEX) {
				long nHash = GetHashKey(pszKey); 
				EncryptString(pszText, &m_pStr, nHash, nCode); 
			}
			explicit CEncryptString(LPCSTR pszText, long nHash, int nCode = Encrypt::HEX) {
				EncryptString(pszText, &m_pStr, nHash, nCode); 
			}
			explicit CEncryptString(LPCWSTR pszText, LPCWSTR pszKey, int nCode = Encrypt::HEX) {
				StringHelper::CAString src(pszText); 
				EncryptString(src.m_pStr,  &m_pStr, GetHashKey(pszKey),nCode); 
			}
			explicit CEncryptString(LPCWSTR pszText, long nHash, int nCode = Encrypt::HEX) {
				StringHelper::CAString src(pszText); 
				EncryptString(src.m_pStr,  &m_pStr, nHash,nCode); 
			}
		};

		struct CDecryptString : public StringHelper::StringHelperImpl<CDecryptString, CHAR> {
		public:
			explicit CDecryptString(LPCSTR pszText, LPCSTR pszKey, int nCode = Encrypt::HEX) {
				long nHash = GetHashKey(pszKey); 
				DecryptString(pszText, &m_pStr, nHash, nCode); 
			}
			explicit CDecryptString(LPCSTR pszText, long nHash, int nCode = Encrypt::HEX) {
				DecryptString(pszText, &m_pStr, nHash, nCode); 
			}
			explicit CDecryptString(LPCWSTR pszText, LPCWSTR pszKey, int nCode = Encrypt::HEX) {
				StringHelper::CAString src(pszText); 
				DecryptString(src.m_pStr,  &m_pStr, GetHashKey(pszKey),nCode); 
			}
			explicit CDecryptString(LPCWSTR pszText, long nHash, int nCode = Encrypt::HEX) {
				StringHelper::CAString src(pszText); 
				DecryptString(src.m_pStr,  &m_pStr, nHash,nCode); 
			}
		};
	};
};

namespace Encrypt
{
	namespace TXOR
	{
 		inline unsigned int GetPublicKey(LPCSTR lpszKey)
		{
			return (unsigned int)Sign::CRC_Hash(lpszKey);
		}

		inline unsigned int GetPublicKey(LPCWSTR lpszKey)
		{
			return (unsigned int)Sign::CRC_Hash(lpszKey);
		}

		inline unsigned int GetPrivateKey(LPCSTR lpszKey)
		{
			return (unsigned int)Sign::CRC_Hash(lpszKey);
		}

		inline unsigned int GetPrivateKey(LPCWSTR lpszKey)
		{
			return (unsigned int)Sign::CRC_Hash(lpszKey);
		}

		inline CryptHandleCPtr InitCryptHandle(CryptHandlePtr _HandlePtr, unsigned int _Seed)
		{
			if (!_HandlePtr)
				return NULL;

			ZeroMemory(_HandlePtr, sizeof(CryptHandle));
 		
 			if (_Seed == 0) {
				_Seed = 0x775407;
			}
			_HandlePtr->_seed = _Seed;
			_Seed = (-1 * (_Seed % 1000));
		
			int nCount = 1;
			while (true){
				int _Rand = ((_Seed = _Seed * 214013L + 2531011L) >> 16) & 0x7fff;
				unsigned char c = _Rand % 256;
				// fix by: 2013-07-16
				// if (_HandlePtr->_t2[c] == 0)	{
				if (_HandlePtr->_t2[c] == 0 && c != 0)	{
					_HandlePtr->_t2[c] = nCount;
					_HandlePtr->_t1[nCount] = c;
					nCount++;
				}
				if (nCount > 0xFF) {
					break;
				}
			}
			return _HandlePtr;
		}
		
		inline void* EncryptBuffer(CryptHandleCPtr _HandlePtr, unsigned int _PrivateKey, unsigned char* _EnBuffer, unsigned int _InSize)
		{
			if (!_HandlePtr || !_EnBuffer)
				return NULL;
 		
			int _Seed = (int)_PrivateKey % 0x100000;
			if (_Seed > 0) {
				_Seed = -1 * _Seed;
			}
			for (int i = 0; i < (int)_InSize; i++)
			{
				int _Rand = ((_Seed = _Seed * 214013L + 2531011L) >> 16) & 0x7fff;
				unsigned char c = _EnBuffer[i] ^ (_Rand % 256);
				_EnBuffer[i] = _HandlePtr->_t1[c];
			}
			return _EnBuffer;
		}
		
		inline void* DecryptBuffer(CryptHandleCPtr _HandlePtr, unsigned int _PrivateKey, unsigned char* _EnBuffer, unsigned int _InSize)
		{
			if (!_HandlePtr || !_EnBuffer)
				return NULL;
 			int _Seed = (int)_PrivateKey % 0x100000;
			if (_Seed > 0) {
				_Seed = -1 * _Seed;
			}
			for (int i = 0; i < (int)_InSize; i++) {
				int _Rand = ((_Seed = _Seed * 214013L + 2531011L) >> 16) & 0x7fff;
				_EnBuffer[i] = _HandlePtr->_t2[ _EnBuffer[i] ] ^ (_Rand % 256);
			}
			return _EnBuffer;
		}

		inline void* EncryptBuffer(CryptHandleCPtr _HandlePtr, unsigned int _PrivateKey, const unsigned char* _SrcBuffer, unsigned int _InSize, unsigned char** ppEnBuffer)
		{
			if (!_HandlePtr || !_SrcBuffer || !ppEnBuffer) {
				ATLASSERT(FALSE);
				return NULL;
			}
			unsigned char* _Buffer = (unsigned char*)malloc(_InSize);
			memcpy(_Buffer, _SrcBuffer, _InSize);
			EncryptBuffer(_HandlePtr, _PrivateKey, _Buffer, _InSize);
			*ppEnBuffer = _Buffer;
			return _Buffer;
		}

		inline void* DecryptBuffer(CryptHandleCPtr _HandlePtr, unsigned int _PrivateKey, const unsigned char* _EnBuffer, unsigned int _InSize, unsigned char** ppDeBuffer)
		{
			if (!_HandlePtr || !_EnBuffer || !ppDeBuffer) {
				ATLASSERT(FALSE);
				return NULL;
			}
			unsigned char* _Buffer = (unsigned char*)malloc(_InSize);
			memcpy(_Buffer, _EnBuffer, _InSize);
			DecryptBuffer(_HandlePtr, _PrivateKey, _Buffer, _InSize);
			*ppDeBuffer = _Buffer;
			return _Buffer;
		}

		inline void* EncryptBuffer(const unsigned int _PublicKey, unsigned int _PrivateKey, unsigned char* _SrcBuffer, unsigned int _InSize)
		{
			CryptHandle Handle;
			return EncryptBuffer(InitCryptHandle(&Handle,_PublicKey), _PrivateKey, _SrcBuffer, _InSize);
		}

		inline void* DecryptBuffer(const unsigned int _PublicKey, unsigned int _PrivateKey, unsigned char* _EnBuffer, unsigned int _InSize) 
		{
			CryptHandle Handle;
			return DecryptBuffer(InitCryptHandle(&Handle,_PublicKey), _PrivateKey, _EnBuffer, _InSize);
		}

		inline void* EncryptBuffer(const unsigned int _PublicKey, unsigned int _PrivateKey, const unsigned char* _SrcBuffer, unsigned int _InSize, unsigned char** ppEnBuffer)
		{
			CryptHandle Handle;
			return EncryptBuffer(InitCryptHandle(&Handle,_PublicKey), _PrivateKey, _SrcBuffer, _InSize, ppEnBuffer);
 		}

		inline void* DecryptBuffer(const unsigned int _PublicKey, unsigned int _PrivateKey, const unsigned char* _EnBuffer, unsigned int _InSize, unsigned char** ppDeBuffer)
		{
			CryptHandle Handle;
			return DecryptBuffer(InitCryptHandle(&Handle,_PublicKey), _PrivateKey, _EnBuffer, _InSize, ppDeBuffer);
 		}

		inline void* EncryptBuffer(LPCSTR _PublicKey, LPCSTR _PrivateKey, unsigned char* _SrcBuffer, unsigned int _InSize)
		{
			return EncryptBuffer(GetPublicKey(_PublicKey),GetPrivateKey(_PrivateKey),_SrcBuffer,_InSize);
		}
		
		inline void* DecryptBuffer(LPCSTR _PublicKey, LPCSTR _PrivateKey, unsigned char* _EnBuffer, unsigned int _InSize)
		{
			return DecryptBuffer(GetPublicKey(_PublicKey),GetPrivateKey(_PrivateKey),_EnBuffer,_InSize);
		}

		inline void* EncryptBuffer(LPCSTR _PublicKey, LPCSTR _PrivateKey, const unsigned char* _SrcBuffer, unsigned int _InSize, unsigned char** ppEnBuffer)
		{
			return EncryptBuffer(GetPublicKey(_PublicKey),GetPrivateKey(_PrivateKey),_SrcBuffer,_InSize,ppEnBuffer);
		}

		inline void* DecryptBuffer(LPCSTR _PublicKey, LPCSTR _PrivateKey, const unsigned char* _EnBuffer, unsigned int _InSize, unsigned char** ppDeBuffer)
		{
			return DecryptBuffer(GetPublicKey(_PublicKey),GetPrivateKey(_PrivateKey),_EnBuffer,_InSize,ppDeBuffer);
		}

		inline void* EncryptBuffer(LPCWSTR _PublicKey, LPCWSTR _PrivateKey, unsigned char* _SrcBuffer, unsigned int _InSize)
		{
			return EncryptBuffer(GetPublicKey(_PublicKey),GetPrivateKey(_PrivateKey),_SrcBuffer,_InSize);
		}
		
		inline void* DecryptBuffer(LPCWSTR _PublicKey, LPCWSTR _PrivateKey, unsigned char* _EnBuffer, unsigned int _InSize)
		{
			return DecryptBuffer(GetPublicKey(_PublicKey),GetPrivateKey(_PrivateKey),_EnBuffer,_InSize);
		}

		inline void* EncryptBuffer(LPCWSTR _PublicKey, LPCWSTR _PrivateKey, const unsigned char* _SrcBuffer, unsigned int _InSize, unsigned char** ppEnBuffer)
		{
			return EncryptBuffer(GetPublicKey(_PublicKey),GetPrivateKey(_PrivateKey),_SrcBuffer,_InSize,ppEnBuffer);
		}

		inline void* DecryptBuffer(LPCWSTR _PublicKey, LPCWSTR _PrivateKey, const unsigned char* _EnBuffer, unsigned int _InSize, unsigned char** ppDeBuffer)
		{
			return DecryptBuffer(GetPublicKey(_PublicKey),GetPrivateKey(_PrivateKey),_EnBuffer,_InSize,ppDeBuffer);
		}

		//------ 加密/解密编码输出字串 ------------------------
		inline LPSTR EncryptBuffer(CryptHandleCPtr _HandlePtr, unsigned int _PrivateKey, const unsigned char* _SrcBuffer, unsigned int _InSize, PSTR* ppEnBuffer, int* pLength, int nCoder)
		{
			PBYTE EnBuffer	= NULL;
			PSTR EnText = NULL;
			bool fSuccssed = false;
			int nLength = 0;

			if (!_HandlePtr || !_SrcBuffer || !_InSize) {
				ATLASSERT(FALSE);
				goto _Exit;
			}
			if (nCoder != BASE64 && nCoder != HEX && nCoder != UTF8) {
				ATLASSERT(FALSE);
				goto _Exit;
			}
			EnBuffer = (unsigned char*)malloc(_InSize);
			if (!EnBuffer) {
				ATLASSERT(FALSE);
				goto _Exit;
			}
			memcpy(EnBuffer, _SrcBuffer, _InSize);
			if (!EncryptBuffer(_HandlePtr,_PrivateKey,EnBuffer,_InSize))
			{
				ATLASSERT(FALSE);
				goto _Exit;
			}
			
			if (nCoder == BASE64){
 				Base64::Base64Encode(EnBuffer,_InSize,&EnText);
				nLength = lstrlenA(EnText);
			}
			else if (nCoder == HEX)	{
				HEX::HexEncode(EnBuffer,(int)_InSize,&EnText, true);
				nLength = lstrlenA(EnText);
			}
			else if (nCoder == UTF8) {
				StringHelper::CUTF8Encoder _Coder((const BYTE*)EnBuffer,_InSize);
				nLength = _Coder.Length;
				EnText = _Coder.Detach();
			}

			fSuccssed = true;
		_Exit:
			__SAFE_FREE_PTR(EnBuffer);

			if (!fSuccssed && EnText) {
				__SAFE_FREE_PTR(EnText);
 			}
			*ppEnBuffer = EnText;
			if (pLength) {
				*pLength = nLength;
			}
			return EnText;
		}

		inline BYTE* DecryptBuffer(CryptHandleCPtr _HandlePtr, unsigned int _PrivateKey, LPCSTR _EnText, BYTE** ppSrcBuffer, int* pLength, int nCoder)
		{
			unsigned char* EnBuffer	= NULL;
			BYTE* SrcBuffer = NULL;
			bool fSuccssed = false;
			int nLength = 0;

			if (!_HandlePtr || !_EnText || !ppSrcBuffer) {
				ATLASSERT(FALSE);
				goto _Exit;
			}
			if (nCoder != BASE64 && nCoder != HEX && nCoder != UTF8) {
				ATLASSERT(FALSE);
				goto _Exit;
			}

			if (nCoder == BASE64){
				EnBuffer = Base64::Base64Decode(_EnText,&nLength);
			}
			else if (nCoder == HEX)	{
				EnBuffer = HEX::HexDecode(_EnText,&nLength);
			}
			else if (nCoder == UTF8) {
				StringHelper::CUTF8Decoder _Coder(_EnText);
				nLength = _Coder.Length;
				EnBuffer = (BYTE*)_Coder.Detach();
			}

			if (!EnBuffer || nLength <= 0) {
				ATLASSERT(FALSE);
				goto _Exit;
			}
			SrcBuffer = (BYTE*)calloc(nLength+1,1);
			memcpy(SrcBuffer, EnBuffer, nLength);
			if (!DecryptBuffer(_HandlePtr,_PrivateKey,SrcBuffer,nLength))
			{
				ATLASSERT(FALSE);
				goto _Exit;
			}
 			fSuccssed = true;
		_Exit:
			__SAFE_FREE_PTR(EnBuffer);

			if (!fSuccssed && SrcBuffer) {
				__SAFE_FREE_PTR(SrcBuffer);
			}
			*ppSrcBuffer = SrcBuffer;
			if (pLength) {
				*pLength = nLength;
			}
			return SrcBuffer;
		}
		
		//------ 字串加解密帮助处理接口 ----------------
		inline LPCSTR EncryptString(LPCSTR pszSrcText,LPSTR* ppEnString, unsigned int _PrivateKey, unsigned int _PublicKey, int nCoder)
		{
   			int nLength = lstrlenA(pszSrcText),nOutLength = 0;
			CryptHandle Handle;
 			return EncryptBuffer(InitCryptHandle(&Handle,_PublicKey),_PrivateKey,(const unsigned char*)pszSrcText,nLength,ppEnString,&nOutLength,nCoder);
		}

		inline LPCSTR EncryptString(LPCSTR pszSrcText,LPSTR* ppEnString, LPCSTR _PrivateKey, unsigned int _PublicKey , int nCoder) 
		{
			return EncryptString(pszSrcText,ppEnString,GetPrivateKey(_PrivateKey),_PublicKey,nCoder);
		}
		
		inline LPCSTR EncryptString(LPCSTR pszSrcText,LPSTR* ppEnString, LPCSTR _PrivateKey, LPCSTR _PublicKey , int nCoder)
		{
			return EncryptString(pszSrcText,ppEnString,_PrivateKey,GetPublicKey(_PublicKey),nCoder);
		}
		
		inline LPCSTR DecryptString(LPCSTR pszEnText, LPSTR* ppSrcString, unsigned int _PrivateKey, unsigned int _PublicKey , int nCoder)
		{
   			int nLength = 0;
			CryptHandle Handle;
 			return (LPCSTR)DecryptBuffer(InitCryptHandle(&Handle,_PublicKey),_PrivateKey,pszEnText,(BYTE**)ppSrcString,&nLength,nCoder);
		}
		
		inline LPCSTR DecryptString(LPCSTR pszEnText, LPSTR* ppSrcString, LPCSTR _PrivateKey, unsigned int _PublicKey, int nCoder)
		{
			return DecryptString(pszEnText,ppSrcString,GetPrivateKey(_PrivateKey),_PublicKey,nCoder);
		}

		inline LPCSTR DecryptString(LPCSTR pszEnText, LPSTR* ppSrcString, LPCSTR _PrivateKey, LPCSTR _PublicKey , int nCoder)
		{
			return DecryptString(pszEnText,ppSrcString,GetPrivateKey(_PrivateKey),GetPublicKey(_PublicKey),nCoder);
		}
		
		inline LPCWSTR EncryptString(LPCWSTR pszSrcText,LPWSTR* ppEnString, unsigned int _PrivateKey, unsigned int _PublicKey , int nCoder)
		{
			LPSTR pEnBuffer = NULL;
			if (EncryptString(CW2A(pszSrcText),&pEnBuffer,_PrivateKey,_PublicKey,nCoder)) {
				using namespace StringHelper;
				CWString wString(pEnBuffer);
				*ppEnString = wString.Detach();
			}
			__SAFE_FREE_PTR(pEnBuffer) ;
			return *ppEnString;
		}

		inline LPCWSTR EncryptString(LPCWSTR pszSrcText,LPWSTR* ppEnString, LPCWSTR _PrivateKey, unsigned int _PublicKey, int nCoder )
		{
			return EncryptString(pszSrcText,ppEnString,GetPrivateKey(_PrivateKey),_PublicKey,nCoder);
		}
		
		inline LPCWSTR EncryptString(LPCWSTR pszSrcText,LPWSTR* ppEnString, LPCWSTR _PrivateKey, LPCWSTR _PublicKey , int nCoder)
		{
			return EncryptString(pszSrcText,ppEnString,_PrivateKey,GetPublicKey(_PublicKey),nCoder);
		}
		
		inline LPCWSTR DecryptString(LPCWSTR pszEnText, LPWSTR* ppSrcString, unsigned int _PrivateKey, unsigned int _PublicKey, int nCoder)
		{
 			LPSTR pSrcBuffer = NULL;
 			if (DecryptString(CW2A(pszEnText),&pSrcBuffer,_PrivateKey,_PublicKey,nCoder)) {
				using namespace StringHelper;
				CWString wString(pSrcBuffer);
				*ppSrcString = wString.Detach();
			}
			__SAFE_FREE_PTR(pSrcBuffer);
			return (*ppSrcString);
		}

		inline LPCWSTR DecryptString(LPCWSTR pszEnText, LPWSTR* ppSrcString, LPCWSTR _PrivateKey, unsigned int _PublicKey, int nCoder)
		{
			return DecryptString(pszEnText,ppSrcString,GetPrivateKey(_PrivateKey),_PublicKey,nCoder);
		}

		inline LPCWSTR DecryptString(LPCWSTR pszEnText, LPWSTR* ppSrcString, LPCWSTR _PrivateKey, LPCSTR _PublicKey , int nCoder)
		{
			return DecryptString(pszEnText,ppSrcString,_PrivateKey,GetPublicKey(_PublicKey),nCoder);
		}

#ifdef __ATLSTR_H__
		inline LPCSTR EncryptString(const CStringA& pszSrcText,CStringA& sEnString, unsigned int _PrivateKey, unsigned int _PublicKey , int nCoder)
		{
			LPSTR lpszBuffer = NULL;
			if (EncryptString((LPCSTR)pszSrcText,&lpszBuffer,_PrivateKey,_PublicKey,nCoder)) {
				sEnString = lpszBuffer;
			}
			__SAFE_FREE_PTR(lpszBuffer);
			return (LPCSTR)sEnString;
		}

		inline LPCSTR EncryptString(const CStringA& pszSrcText,CStringA& sEnString, LPCSTR _PrivateKey, unsigned int _PublicKey, int nCoder)
		{
			return EncryptString(pszSrcText,sEnString,GetPrivateKey(_PrivateKey),_PublicKey,nCoder);
		}
		
		inline LPCSTR EncryptString(const CStringA& pszSrcText,CStringA& sEnString, LPCSTR _PrivateKey, LPCSTR _PublicKey , int nCoder)
		{
			return EncryptString(pszSrcText,sEnString,_PrivateKey,GetPublicKey(_PublicKey),nCoder);
		}
		
		inline LPCSTR DecryptString(const CStringA& pszEnText, CStringA& sSrcString, unsigned int _PrivateKey, unsigned int _PublicKey , int nCoder )
		{
			LPSTR lpszBuffer = NULL;
 			if (DecryptString((LPCSTR)pszEnText,&lpszBuffer,_PrivateKey,_PublicKey,nCoder)) {
				sSrcString=lpszBuffer;
			}
			__SAFE_FREE_PTR(lpszBuffer);
			return (LPCSTR)sSrcString;
		}
		
		inline LPCSTR DecryptString(const CStringA& pszEnText, CStringA& sSrcString, LPCSTR _PrivateKey, unsigned int _PublicKey, int nCoder )
		{
			return DecryptString(pszEnText,sSrcString,GetPrivateKey(_PrivateKey),_PublicKey,nCoder);
		}
		
		inline LPCSTR DecryptString(const CStringA& pszEnText, CStringA& sSrcString, LPCSTR _PrivateKey, LPCSTR _PublicKey , int nCoder )
		{
			return DecryptString(pszEnText, sSrcString,_PrivateKey,GetPublicKey(_PublicKey),nCoder);
		}
		
		inline LPCWSTR EncryptString(const CStringW& pszSrcText,CStringW& sEnString, unsigned int _PrivateKey, unsigned int _PublicKey , int nCoder)
		{
			LPWSTR lpszBuffer = NULL;
			if (EncryptString((LPCWSTR)pszSrcText,&lpszBuffer,_PrivateKey,_PublicKey,nCoder)) {
				sEnString = lpszBuffer;
			}
			__SAFE_FREE_PTR(lpszBuffer);
			return (LPCWSTR)sEnString;
		}
		
		inline LPCWSTR EncryptString(const CStringW& pszSrcText,CStringW& sEnString, LPCSTR _PrivateKey, unsigned int _PublicKey , int nCoder )
		{
			return EncryptString(pszSrcText,sEnString,GetPrivateKey(_PrivateKey),_PublicKey,nCoder);
		}
		
		inline LPCWSTR EncryptString(const CStringW& pszSrcText,CStringW& sEnString, LPCSTR _PrivateKey, LPCSTR _PublicKey , int nCoder)
		{
			return EncryptString(pszSrcText,sEnString,_PrivateKey,GetPublicKey(_PublicKey),nCoder);
		}
		
		inline LPCWSTR DecryptString(const CStringW& pszEnText, CStringW& sSrcString, unsigned int _PrivateKey, unsigned int _PublicKey, int nCoder)
		{
			LPWSTR lpszBuffer = NULL;
 			if (DecryptString((LPCWSTR)pszEnText,&lpszBuffer,_PrivateKey,_PublicKey,nCoder)) {
				sSrcString = lpszBuffer;
			}
			__SAFE_FREE_PTR(lpszBuffer);
			return (LPCWSTR)sSrcString;
		}

		inline LPCWSTR DecryptString(const CStringW& pszEnText, CStringW& sSrcString, LPCSTR _PrivateKey, unsigned int _PublicKey , int nCoder )
		{
			return DecryptString(pszEnText,sSrcString,GetPrivateKey(_PrivateKey),_PublicKey,nCoder);
		}

		inline LPCWSTR DecryptString(const CStringW& pszEnText, CStringW& sSrcString, LPCSTR _PrivateKey, LPCSTR _PublicKey, int nCoder )
		{
			return DecryptString(pszEnText,sSrcString,_PrivateKey,GetPublicKey(_PublicKey),nCoder);
		}

		inline CStringA EncryptStringA(const CStringA& SrcString, const CStringA& lpszKey, int nCoder)
		{
			CStringA EnString;
			LPSTR lpszBuffer = NULL;
			if (EncryptString((LPCSTR)SrcString,&lpszBuffer,(unsigned int)GetPrivateKey(lpszKey),(unsigned int)0,nCoder)) {
				EnString = lpszBuffer;
			}
			__SAFE_FREE_PTR(lpszBuffer);
			return EnString;
		}

		inline CStringA DecryptStringA(const CStringA& EnString, const CStringA& lpszKey, int nCoder)
		{
			CStringA SrcString;
			LPSTR lpszBuffer = NULL;
 			if (DecryptString((LPCSTR)EnString,&lpszBuffer,(unsigned int)GetPrivateKey(lpszKey),(unsigned int)0,nCoder)) {
				SrcString = lpszBuffer;
			}
			__SAFE_FREE_PTR(lpszBuffer);
			return SrcString;
		}

		inline CStringW EncryptStringW(const CStringW& SrcString, const CStringW& lpszKey, int nCode) 
		{
			CStringW EnString;
			EncryptString((LPCWSTR)SrcString,(CStringW&) EnString, (unsigned int)GetPrivateKey(lpszKey), (unsigned int)0, nCode);
			return EnString;
		}
		inline CStringW DecryptStringW(const  CStringW& EnString, const CStringW& sKey, int nCode) 
		{
			CStringW SrcString;
			DecryptString((LPCWSTR)EnString, (CStringW&)SrcString, (unsigned int)GetPrivateKey(sKey), (unsigned int)0, nCode);
			return SrcString;
		}

#endif
#ifdef _STRING_
 		inline LPCSTR EncryptString(const std::string& pszSrcText,std::string& sEnString, unsigned int _PrivateKey, unsigned int _PublicKey , int nCoder)
		{
			LPSTR lpszBuffer = NULL;
			if (EncryptString(pszSrcText.c_str(),&lpszBuffer,_PrivateKey,_PublicKey,nCoder)) {
				sEnString = lpszBuffer;
			}
			__SAFE_FREE_PTR(lpszBuffer);
			return (LPCSTR)sEnString.c_str();
		}

		inline LPCSTR EncryptString(const std::string& pszSrcText,std::string& sEnString, LPCSTR _PrivateKey, unsigned int _PublicKey, int nCoder)
		{
			return EncryptString(pszSrcText.c_str(),sEnString,GetPrivateKey(_PrivateKey),_PublicKey,nCoder);
		}
		
		inline LPCSTR EncryptString(const std::string& pszSrcText,std::string& sEnString, LPCSTR _PrivateKey, LPCSTR _PublicKey , int nCoder)
		{
			return EncryptString(pszSrcText.c_str(),sEnString,_PrivateKey,GetPublicKey(_PublicKey),nCoder);
		}
		
		inline LPCSTR DecryptString(const std::string& pszEnText, std::string& sSrcString, unsigned int _PrivateKey, unsigned int _PublicKey , int nCoder )
		{
			LPSTR lpszBuffer = NULL;
 			if (DecryptString(pszEnText.c_str(),&lpszBuffer,_PrivateKey,_PublicKey,nCoder)) {
 				sSrcString = lpszBuffer;
			}
 			__SAFE_FREE_PTR(lpszBuffer);
 			return sSrcString.c_str();
		}
		
		inline LPCSTR DecryptString(const std::string& pszEnText, std::string& sSrcString, LPCSTR _PrivateKey, unsigned int _PublicKey, int nCoder )
		{
			return DecryptString(pszEnText.c_str(),sSrcString,GetPrivateKey(_PrivateKey),_PublicKey,nCoder);
		}
		
		inline LPCSTR DecryptString(const std::string& pszEnText, std::string& sSrcString, LPCSTR _PrivateKey, LPCSTR _PublicKey , int nCoder )
		{
			return DecryptString(pszEnText.c_str(), sSrcString,_PrivateKey,GetPublicKey(_PublicKey),nCoder);
		}
#endif

		struct CEncryptString : public StringHelper::StringHelperImpl<CEncryptString, CHAR> {
		public:
			explicit CEncryptString(LPCSTR pszText, LPCSTR pszKey, LPSTR publicKey, int nCode = Encrypt::HEX) 
				: CEncryptString(pszText, GetPrivateKey(pszKey), GetPublicKey(publicKey), nCode)
			{}
 			explicit CEncryptString(LPCSTR pszText, LPCSTR pszKey, unsigned int publicKey, int nCode = Encrypt::HEX) 
				: CEncryptString(pszText, GetPrivateKey(pszKey), publicKey, nCode)
			{}
			explicit CEncryptString(LPCSTR pszText, unsigned int privateKey, unsigned int publicKey, int nCode = Encrypt::HEX) {
				EncryptString(pszText, &m_pStr, privateKey, publicKey, nCode);
			}
			explicit CEncryptString(LPCWSTR pszText, LPCWSTR pszKey, LPCWSTR publicKey, int nCode = Encrypt::HEX)
				: CEncryptString(pszText, GetPrivateKey(pszKey), GetPublicKey(publicKey), nCode)
			{}
			explicit CEncryptString(LPCWSTR pszText, LPCWSTR pszKey, unsigned publicKey, int nCode = Encrypt::HEX)
				: CEncryptString(pszText, GetPrivateKey(pszKey), publicKey, nCode)
			{}
			explicit CEncryptString(LPCWSTR pszText, unsigned privateKey, unsigned publicKey, int nCode = Encrypt::HEX)
				: CEncryptString(CW2A(pszText), privateKey, publicKey, nCode)
			{}
 		};

		struct CDecryptString : public StringHelper::StringHelperImpl<CDecryptString, CHAR> {
		public:
			explicit CDecryptString(LPCSTR pszText, LPCSTR pszKey, LPCSTR publicKey, int nCode = Encrypt::HEX)
				: CDecryptString(pszText, GetPrivateKey(pszKey), GetPublicKey(publicKey), nCode)
			{}
			explicit CDecryptString(LPCSTR pszText, LPCSTR pszKey, unsigned int publicKey, int nCode = Encrypt::HEX) 
				: CDecryptString(pszText, GetPrivateKey(pszKey), publicKey, nCode)
			{}
			explicit CDecryptString(LPCSTR pszText, unsigned int privateKey, unsigned int publicKey, int nCode = Encrypt::HEX) {
				DecryptString(pszText, &m_pStr, privateKey, publicKey, nCode); 
			}
			explicit CDecryptString(LPCWSTR pszText, LPCWSTR pszKey, LPCWSTR publicKey, int nCode = Encrypt::HEX)
				: CDecryptString(pszText, GetPrivateKey(pszKey), GetPublicKey(publicKey), nCode)
			{}
 			explicit CDecryptString(LPCWSTR pszText, LPCWSTR pszKey, unsigned int publicKey, int nCode = Encrypt::HEX)
				: CDecryptString(pszText, GetPrivateKey(pszKey), publicKey, nCode)
			{}
			explicit CDecryptString(LPCWSTR pszText, unsigned int privateKey, unsigned int publicKey, int nCode = Encrypt::HEX)
				: CDecryptString(CW2A(pszText), privateKey, publicKey, nCode)
			{}
		};
	};
};

namespace StringTool
{
	inline LPCSTR  NewGuidString(LPSTR lpszBuffer, DWORD cbSize)
	{
		if (!lpszBuffer || cbSize < 38) {
			ATLASSERT(FALSE);
			return NULL;
		}
		GUID guid;
		HRESULT hr = CoCreateGuid(&guid);
		if (SUCCEEDED(hr)) {
			WCHAR szGuid[64] = {0};
			StringFromGUID2(guid,szGuid,64);
			StringHelper::CAString sGuid(szGuid);
			lstrcpynA(lpszBuffer,sGuid.m_pStr,cbSize);
			StrTrimA(lpszBuffer,"{}");
			return lpszBuffer;
		}
		else return NULL;
 	}
	inline LPCWSTR NewGuidString(LPWSTR lpszBuffer, DWORD cbSize)
	{
		if (!lpszBuffer || cbSize < 38) {
			ATLASSERT(FALSE);
			return NULL;
		}
		GUID guid;
		HRESULT hr = CoCreateGuid(&guid);
		if (SUCCEEDED(hr)) {
 			StringFromGUID2(guid,lpszBuffer,cbSize);
 			StrTrimW(lpszBuffer,L"{}");
			return lpszBuffer;
		}
		else return NULL;
	}

	inline LPCSTR FormatNumeric(LONGLONG Number,LPSTR pszBuffer) 
	{
		char out[64] = {0};
		char buffer[64] = {0};
		
		_ui64toa_s(Number,buffer,64, 10);
		
		int len = 0, n = 0;
		char* s = buffer;
		while(*s && !(*s >= '0' && *s <= '9')) s++;
 		while(*s++) len++; s--;
		
		if (len > 3) {
			char* s1 = out;
			while(--s >= buffer) {
				*s1++ = *s;
				if (++n % 3 == 0 && n < len) *s1++ = ','; 
			}
			s = buffer;
			while(--s1 >= out) *s++ = *s1;
			*s++ = 0x00;
		}
		lstrcpyA(pszBuffer, out);
		return pszBuffer;
	}

	inline LPCWSTR FormatNumeric(LONGLONG Number,LPWSTR pszBuffer)
	{
		wchar_t out[64] = {0};
		wchar_t buffer[64] = {0};
		
		_ui64tow_s(Number,buffer,64,10);
		
		int len = 0, n = 0;
		wchar_t* s = buffer;
		while(*s && !(*s >= L'0' && *s <= L'9')) s++;
 		while(*s++) len++; s--;
		
		if (len > 3) {
			wchar_t* s1 = out;
			while(--s >= buffer) {
				*s1++ = *s;
				if (++n % 3 == 0 && n < len) *s1++ = L','; 
			}
			s = buffer;
			while(--s1 >= out) *s++ = *s1;
			*s++ = 0x00;
		}
		lstrcpyW(pszBuffer, out);
		return pszBuffer;
	}

	inline LPCTSTR FormatNumeric(LONGLONG Number)
	{
		static TCHAR szBuffer[64] = {0};
		return FormatNumeric(Number, szBuffer);
	}
	
	inline LPCSTR	OleFormatNumeric(LONGLONG Number,LPSTR pszBuffer,  LPCSTR lpszFMT)
	{
		CComVariant var(Number);
		CComBSTR bsOut;

		::VarFormat(&var,CA2W(lpszFMT),0,0,VAR_FORMAT_NOSUBSTITUTE,&bsOut);
		lstrcpyA(pszBuffer, CW2A(bsOut));
		return pszBuffer;
	}

	inline LPCWSTR OleFormatNumeric(LONGLONG Number,LPWSTR pszBuffer, LPWSTR lpszFMT)
	{
		CComVariant var(Number);
		CComBSTR bsOut;

		::VarFormat(&var,lpszFMT,0,0,VAR_FORMAT_NOSUBSTITUTE,&bsOut);
		lstrcpyW(pszBuffer, (LPCWSTR)bsOut);
		return pszBuffer;
	}

	inline LPCTSTR OleFormatNumeric(LONGLONG Number,LPCTSTR lpszFMT)
	{
		static TCHAR szBuffer[128] = {0};
		CComVariant var(Number);
		CComBSTR bsOut;

		::VarFormat(&var,CT2W(lpszFMT),0,0,VAR_FORMAT_NOSUBSTITUTE,&bsOut);
		lstrcpyn(szBuffer, CW2T(bsOut), 128);
		return szBuffer;
	}

	inline LPCSTR MacString(LPSTR lpszBuffer, DWORD cbSize)
	{
		if (!lpszBuffer || cbSize < 32) {
			ATLASSERT(FALSE);
			return NULL;
		}
		HANDLE Handle = MAC::FindApaters(0);
		if (Handle) {
			MAC::LPNETAPATER_INFO pApater = NULL;
			MAC::LPNETAPATER_INFO p = (MAC::LPNETAPATER_INFO)MAC::FirstApater(Handle);
			
			while( p != NULL) {
				bool fValid = false;
				for (int i = 0; i < 6; i++) {
					if (p->MacAddress[i]) {
						fValid = true; 
						break;
					}
				}
				if (fValid && ((p->Characteristics & NCF_PHYSICAL) || memcmp(p->DeviceInstanceID,"PCI",3) == 0)) {
					pApater = p;
					break;
				}
				p = (MAC::LPNETAPATER_INFO)MAC::NextApater(Handle);
			};

			if (pApater) {
 				wsprintfA(lpszBuffer,"%02X-%02X-%02X-%02X-%02X-%02X",
					(BYTE)pApater->MacAddress[0],
					(BYTE)pApater->MacAddress[1],
					(BYTE)pApater->MacAddress[2],
					(BYTE)pApater->MacAddress[3],
					(BYTE)pApater->MacAddress[4],
					(BYTE)pApater->MacAddress[5]
				);
			}
 			MAC::CloseApaterHandle(Handle);
			return lpszBuffer;
		}
		return NULL;
	}

	inline LPCWSTR MacString(LPWSTR lpszBuffer, DWORD cbSize)
	{
		if (!lpszBuffer || cbSize < 32) {
			ATLASSERT(FALSE);
			return NULL;
		}
		CHAR szMac[MAX_PATH] = {0};
		if (MacString(szMac,MAX_PATH)) {
			StringHelper::CWString sMac(szMac);
			lstrcpynW(lpszBuffer,sMac.m_pStr,cbSize);
			return lpszBuffer;
		}
		return NULL;
	}

	inline LPCSTR  LongIntToString(LONGLONG Number, LPSTR pszBuffer, DWORD cbSize)
	{
		ATLASSERT(pszBuffer && cbSize); 
		_i64toa_s(Number,pszBuffer,cbSize,10);
		return pszBuffer; 
	}

	inline LPCWSTR  LongIntToString(LONGLONG Number, LPWSTR pszBuffer, DWORD cbSize) 
	{
		ATLASSERT(pszBuffer && cbSize); 
		_i64tow_s(Number, pszBuffer, cbSize, 10); 
		return pszBuffer; 
	}

#ifdef __ATLSTR_H__
	inline CStringA& NewGuidString(CStringA& sGuid)
	{
		LPSTR p = sGuid.GetBufferSetLength(64);
		NewGuidString(p,64);
		sGuid.ReleaseBuffer();
		return sGuid;
	}

	inline CStringW& NewGuidString(CStringW& sGuid)
	{
 		LPWSTR p = sGuid.GetBufferSetLength(64);
		NewGuidString(p,64);
		sGuid.ReleaseBuffer();
		return sGuid;
	}	 
	
	inline CStringA NewGuidStringA() 
	{
		CStringA sGuid;
		return NewGuidString(sGuid);
	}

	inline CStringW NewGuidStringW() 
	{
		CStringW sGuid;
		return NewGuidString(sGuid);
	}

	inline CStringA& MacString(CStringA& sMac)
	{
		MacString(sMac.GetBufferSetLength(MAX_PATH),MAX_PATH);
		sMac.ReleaseBuffer();
		return sMac;
	}
	inline CStringW& MacString(CStringW& sMac)
	{
		MacString(sMac.GetBufferSetLength(MAX_PATH),MAX_PATH);
		sMac.ReleaseBuffer();
		return sMac;
	}
	inline CStringA MacStringA()
	{
		CStringA sMac;
		return MacString(sMac);
	}
	inline CStringW MacStringW()
	{
		CStringW sMac;
		return MacString(sMac);
	}

	inline CStringA LongIntToStringA(LONGLONG Number)
	{
		CHAR buff[MAX_PATH] = {0}; 
		return LongIntToString(Number, buff, MAX_PATH); 
	}

	inline CStringW LongIntToStringW(LONGLONG Number)
	{
		WCHAR buff[MAX_PATH] = {0}; 
		return LongIntToString(Number, buff, MAX_PATH); 
	}
#endif
#ifdef _STRING_
	inline std::string& NewGuidString(std::string &str)
	{
		CHAR szGuid[64] = {0};
		if (NewGuidString(szGuid,64)) {
			str = szGuid;
		} else str = "";
		return str;
	}
	inline std::string& MacString(std::string& str)
	{
		CHAR szMac[64] = {0};
		if (MacString(szMac,64)) {
			str = szMac;
		} else str = "";
		return str;
	}
	inline std::string LongIntToString(LONGLONG Number)
 	{
		CHAR buff[MAX_PATH] = {0}; 
		return LongIntToString(Number, buff, MAX_PATH); 
	}
#endif
};

namespace MAC
{
#pragma comment(lib, "advapi32.lib")
	typedef struct tagNETAPATER_HEADER
	{
		UINT cbSize;
		UINT ApaterCount;
		UINT ApaterIndex;
	}NETAPATER_HEADER,*LPNETAPATER_HEADER;

	#define NET_CARDS_KEY 			"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards"
	#define NET_CARDS_ROOT			"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}"

	#define NETHEADER_LENGTH 		sizeof(NETAPATER_HEADER)
	#define NETINFO_LENGTH 			sizeof(NETAPATER_INFO)

	#define IOCTL_NDIS_QUERY_GLOBAL_STATS	0x00170002 
	#define OID_802_3_PERMANENT_ADDRESS		0x01010101
	#define OID_802_3_CURRENT_ADDRESS       0x01010102

	#define SAFE_CLOSE_REGKEY(key)	if (key) { RegCloseKey(key); key = NULL; }

	ATL_NOINLINE inline void WINAPI GetNetApaterInfo(LPNETAPATER_INFO pApater)
	{
		HKEY hKey = NULL, hSubKey = NULL;
		CHAR szName[MAX_PATH], szValue[MAX_PATH];
		LONG nIndex;
		DWORD cbType, cbSize;
		
		nIndex 	= 0;
		hKey 	= NULL;

		if (ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE, NET_CARDS_ROOT,NULL,KEY_READ,&hKey)) {
			for (nIndex = 0; !RegEnumKeyA(hKey,nIndex,szName,MAX_PATH); nIndex++ ) {
				hSubKey = NULL;
				if (ERROR_SUCCESS != RegOpenKeyExA(hKey,szName,0,KEY_READ,&hSubKey)) {
					continue;
				}
				//
				// 比对安装网卡ServiceName，如果为指定网卡，则取网卡实际信息
				//
				cbType = REG_SZ;
				cbSize = MAX_PATH;
				if (ERROR_SUCCESS == RegQueryValueExA(hSubKey,"NetCfgInstanceId",NULL,&cbType,(PBYTE)szValue,&cbSize)) 	{
	 				if (lstrcmpiA(szValue,pApater->ServiceName) == 0) {
	 					cbType = REG_DWORD;
	 					cbSize = 4;
		   				RegQueryValueExA(hSubKey,"Characteristics",NULL,&cbType,(PBYTE)&pApater->Characteristics,&cbSize); 
		   				cbType = REG_SZ;
		   				cbSize = MAX_PATH;
		   				RegQueryValueExA(hSubKey,"DeviceInstanceID",NULL,&cbType,(PBYTE)&pApater->DeviceInstanceID,&cbSize); 
						break;
	 				}
				}
				SAFE_CLOSE_REGKEY(hSubKey);
			} 
		}
		SAFE_CLOSE_REGKEY(hSubKey);
		SAFE_CLOSE_REGKEY(hKey);
	}

	ATL_NOINLINE inline void WINAPI GetNetApaterMacAddress(LPNETAPATER_INFO pApater)
	{
		CHAR szName[MAX_PATH];
		CHAR szBuffer[MAX_PATH];
		DWORD cbType;
		DWORD cbSize;
		HANDLE hDevice;
		
		//
		// 创建与网卡NDIS通信连接
		//
		wsprintfA(szName,"\\\\.\\%s",pApater->ServiceName);
		hDevice = CreateFileA(szName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
		if (hDevice != INVALID_HANDLE_VALUE) {
			//
			// 获取当前网卡固件中的物理MAC地址
			//
			cbSize = MAX_PATH;
			cbType = OID_802_3_PERMANENT_ADDRESS;
			ZeroMemory(szBuffer,MAX_PATH);
			if(DeviceIoControl(hDevice,IOCTL_NDIS_QUERY_GLOBAL_STATS, &cbType,4,szBuffer,256,&cbSize, NULL)) {
				memcpy(pApater->MacAddress,szBuffer,6);
			}
			
			//
			// 获取网卡当前使用中的MAC 地址
			//
			cbSize = MAX_PATH;
			cbType = OID_802_3_CURRENT_ADDRESS;
			ZeroMemory(szBuffer,MAX_PATH);
			if(DeviceIoControl(hDevice,IOCTL_NDIS_QUERY_GLOBAL_STATS, &cbType,4,szBuffer,256,&cbSize, NULL)) {
				memcpy(pApater->CurrentMacAddress,szBuffer,6);
			}
			CloseHandle(hDevice);
		}
	}

	ATL_NOINLINE inline HANDLE WINAPI FindApaters(DWORD cbFlags)
	{
		HKEY hKey, hSubKey;
		DWORD nIndex, nItem, cbName, cbValue, cbType;
 		CHAR szName[MAX_PATH], szValue[MAX_PATH], szTemp[MAX_PATH];
		ULONG lResult ;
		LPNETAPATER_HEADER pHandle;
		LPNETAPATER_INFO   pApater;
		//HANDLE hDevice;
		BOOL fResult ;
		
		pHandle = NULL;
		fResult = FALSE;
		hKey 	= NULL;
		hSubKey	= NULL;
		
		do {
			//
			// 打开系统网卡设置注册表项
			//
 			lResult = RegOpenKeyExA( HKEY_LOCAL_MACHINE,
 				NET_CARDS_KEY,NULL,KEY_READ,&hKey 
 			);
	 		
			if (lResult != ERROR_SUCCESS) {
				break;
			}
			
			//
			// 初始化网卡列表内存使用信息
			//
			pHandle = (LPNETAPATER_HEADER)GlobalAlloc(GPTR, NETHEADER_LENGTH);
			if (!pHandle) {
				break;
			}
			pHandle->cbSize = NETHEADER_LENGTH;
			pHandle->ApaterCount = 0;
			pHandle->ApaterIndex = 0;

 			//
			// 遍历并枚举所有系统安装的网卡列表
			//
			for (nIndex = 0; !RegEnumKeyA(hKey,nIndex,szTemp,MAX_PATH); nIndex++ ) {
 				lResult = RegOpenKeyExA(hKey,szTemp,0,KEY_READ,&hSubKey);
 				if (lResult != ERROR_SUCCESS) {
 					continue;
 				}
	 			
 				//
 				// 查找到新网卡，重新分配网卡列表缓存区
 				//
 				pHandle->ApaterCount++;
 				pHandle->cbSize = NETHEADER_LENGTH + NETINFO_LENGTH * pHandle->ApaterCount;
 				pHandle = (LPNETAPATER_HEADER)GlobalReAlloc(pHandle,pHandle->cbSize,GHND);
 				if (!pHandle) {
 					goto _Exit;
 				}
 				pApater = (LPNETAPATER_INFO)((PBYTE)pHandle + NETHEADER_LENGTH + pHandle->ApaterIndex * NETINFO_LENGTH);
 				pHandle->ApaterIndex++;
	 			
 				//
 				// 枚举网卡注册信息
 				//
				for ( nItem = 0, cbName = MAX_PATH, cbValue = MAX_PATH; 
						!RegEnumValueA(hSubKey,nItem,szName,&cbName,0,&cbType,(LPBYTE)szValue,&cbValue);
						nItem++, cbName = MAX_PATH, cbValue = MAX_PATH
					)
 				{
 					if (lstrcmpiA(szName,"ServiceName")==0)
 					{
 						lstrcpynA(pApater->ServiceName,szValue,MAX_PATH);
 					}
 					else if (lstrcmpiA(szName,"Description") == 0)
 					{
						lstrcpynA(pApater->Description,szValue,MAX_PATH); 					
 					}
  				}
 				SAFE_CLOSE_REGKEY(hSubKey);
	 			
  				//
 				// 如果网卡ServiceName 项存在，则调用驱动获取网卡物理MAC
 				//
 				if (*pApater->ServiceName)
 				{
 					GetNetApaterMacAddress(pApater);
  					GetNetApaterInfo(pApater);
 				}
			}
			pHandle->ApaterIndex = 0;
			fResult = pHandle->ApaterCount ;
		} while(FALSE);
		
		//
		// 检查执行结果，如果返回错误，则清理内存
		//
	_Exit:
		SAFE_CLOSE_REGKEY(hSubKey);
		SAFE_CLOSE_REGKEY(hKey);
		
 		if (!fResult) {
			if (pHandle) {
				GlobalFree(pHandle);
				pHandle = NULL;
			}
		}
		
		//
		// 返回网卡信息获取列表句柄
		//	
		return (HANDLE) pHandle;
	}

	ATL_NOINLINE inline LPVOID WINAPI FirstApater(HANDLE hFind)
	{
 		LPNETAPATER_HEADER pHeader;
 		if (hFind)	{
			pHeader = (LPNETAPATER_HEADER)hFind;
			if (pHeader->ApaterCount > 0 && pHeader->cbSize > NETHEADER_LENGTH)
			{
				pHeader->ApaterIndex = 0;
				return (PBYTE)pHeader + NETHEADER_LENGTH;
			}
		}
		return NULL;
	}

	ATL_NOINLINE inline LPVOID WINAPI NextApater(HANDLE hFind)
	{
 		LPNETAPATER_HEADER pHeader;
		
 		if (hFind) {
			pHeader = (LPNETAPATER_HEADER)hFind;
			if (pHeader->ApaterCount > 0 && pHeader->cbSize > NETHEADER_LENGTH)	{
				pHeader->ApaterIndex ++;
				if (pHeader->ApaterIndex < pHeader->ApaterCount) {
					return (PBYTE)pHeader + NETHEADER_LENGTH + pHeader->ApaterIndex * NETINFO_LENGTH;
				}
			}
		}
		return NULL;
	}

	ATL_NOINLINE inline VOID WINAPI CloseApaterHandle(HANDLE hFind)
	{
		if (hFind) {
			GlobalFree(hFind);
			hFind = NULL;
		}
	}

	ATL_NOINLINE inline VOID WINAPI FormatMacAddress(LPNETAPATER_INFO pInfo, CHAR* szBuffer)
	{
		if (!pInfo || !szBuffer)
			return; 
		wsprintfA(szBuffer, "%02X-%02X-%02X-%02X-%02X-%02X", 
			(BYTE)pInfo->MacAddress[0],
			(BYTE)pInfo->MacAddress[1],
			(BYTE)pInfo->MacAddress[2],
			(BYTE)pInfo->MacAddress[3],
			(BYTE)pInfo->MacAddress[4],
			(BYTE)pInfo->MacAddress[5]
			);
	}
}

namespace PathHelper 
{
	inline HMODULE GetCurrentModuleHandle()
	{
		MEMORY_BASIC_INFORMATION mbi;
		HMODULE hModule = (VirtualQuery(GetCurrentModuleHandle, &mbi, sizeof(mbi)) != 0) 
			? (HMODULE) mbi.AllocationBase : NULL ;
		return hModule;
	}

	inline LPCSTR GetModulePath(HMODULE hModule, LPSTR lpszPath, DWORD cbSize) 
	{
		if (::GetModuleFileNameA(hModule,lpszPath,cbSize)) {
			LPSTR p = lpszPath;
			while(*p++);
			while(--p != lpszPath && *p != '\\') *p = 0x00;
			return lpszPath;
		}
		return NULL;
	}

	inline LPCWSTR GetModulePath(HMODULE hModule, LPWSTR lpszPath, DWORD cbSize) 
	{
		if (::GetModuleFileNameW(hModule,lpszPath,cbSize)) {
			LPWSTR p = lpszPath;
			while(*p++);
			while(--p != lpszPath && *p != L'\\') *p = 0x00;
			return lpszPath;
		}
		return NULL;
	}

	inline LPCTSTR CurrentModulePath()
	{
		static TCHAR szModule[MAX_PATH] = {0};
		if (!*szModule)
		{
 			::GetModuleFileName(GetCurrentModuleHandle(),szModule,MAX_PATH);
			LPTSTR p = szModule;
			while(*p++);
			while(--p != szModule && *p != _T('\\')) *p = 0x00;
		}
		return *szModule == _T('\\') ? &szModule[4] : szModule;
	}
 	
	inline LPCSTR CurrentModulePath(LPSTR pszPath, DWORD cbSize)
	{
		lstrcpynA(pszPath,CT2A(CurrentModulePath()),cbSize);
		return pszPath;
	}

	inline LPCWSTR CurrentModulePath(LPWSTR pszPath, DWORD cbSize)
	{
		lstrcpynW(pszPath,CT2W(CurrentModulePath()),cbSize);
		return pszPath;
	}

	inline LPCSTR  ApplicationPath(LPSTR lpszBuffer, DWORD cbSize)
	{
		return GetModulePath(NULL,lpszBuffer,cbSize);
	}

	inline LPCWSTR ApplicationPath(LPWSTR lpszBuffer, DWORD cbSize)
	{
		return GetModulePath(NULL,lpszBuffer,cbSize);
	}

	inline LPCSTR ApplicationPathA()
	{
		static CHAR szPath[1024] = {0};
		if (!(*szPath)) {
			GetModulePath(NULL,szPath,1024);
		}
		return szPath;
	}

	inline LPCWSTR ApplicationPathW()
	{
		static WCHAR szPath[1024] = {0};
		if (!(*szPath)) {
			GetModulePath(NULL,szPath,1024);
		}
		return szPath;
	}

	inline LPCSTR  ApplicationFile(LPCSTR lpszFile, LPSTR lpszBuffer, DWORD cbSize)
	{
		ApplicationPath(lpszBuffer,cbSize);
		LPCSTR p = lpszFile;
		while(*p && (*p == ' ' || *p == '\\')) p++;
		lstrcatA(lpszBuffer,p);
		return lpszBuffer;
	}

	inline LPCWSTR ApplicationFile(LPCWSTR lpszFile, LPWSTR lpszBuffer, DWORD cbSize)
	{
		ApplicationPath(lpszBuffer,cbSize);
		LPCWSTR p = lpszFile;
		while(*p && (*p == L' ' || *p == L'\\')) p++;
		lstrcatW(lpszBuffer,p);
		return lpszBuffer;
	}

	inline BOOL	MakeDirectory(LPCSTR lpszDirectory)
	{
		if (!lpszDirectory) {
			ATLASSERT(FALSE);
			return FALSE;
		}
  		if (PathFileExistsA(lpszDirectory)) {
			return PathIsDirectoryA(lpszDirectory);
		}
		else {
			CHAR szPath[MAX_PATH] = {0};
			if (StrStrIA(lpszDirectory,":") == NULL) {// 不是绝对路径
				CHAR szAppPath[MAX_PATH];
				if (CurrentModulePath(szAppPath,MAX_PATH)) {
					 PathCombineA(szPath,szAppPath,lpszDirectory);
				}
			}
			else {
				StrCpyNA(szPath,lpszDirectory,MAX_PATH);
			}
			int nLen = lstrlenA(szPath);
			if (szPath[nLen] != '\\') {
				lstrcatA(szPath, "\\");
				nLen++;
			}
 			CHAR szDir[MAX_PATH] = {0};
			if (nLen ) {
				for (int i = 0; i < nLen; i++) {
					if (szPath[i] == '\\') {
						StrCpyNA(szDir,szPath,i+2);
						if (PathFileExistsA(szDir)) {
							if (!PathIsDirectoryA(szDir))
								return FALSE; // 目录中存在非法路径
						}
						else {
							if (!CreateDirectoryA(szDir,NULL))
								return FALSE; 
						}
					}
				}
				return TRUE;
			}
		}
 		return FALSE;
	}

	inline BOOL	MakeDirectory(LPCWSTR lpszDirectory)
	{
		if (!lpszDirectory) {
			ATLASSERT(FALSE);
			return FALSE;
		}
  		if (PathFileExistsW(lpszDirectory)) {
			return PathIsDirectoryW(lpszDirectory);
		}
		else {
			WCHAR szPath[MAX_PATH] = {0};
			if (StrStrIW(lpszDirectory,L":") == NULL) {// 不是绝对路径
				WCHAR szAppPath[MAX_PATH];
				if (CurrentModulePath(szAppPath,MAX_PATH)) {
					 PathCombineW(szPath,szAppPath,lpszDirectory);
				}
			}
			else {
				StrCpyNW(szPath,lpszDirectory,MAX_PATH);
			}
			int nLen = lstrlenW(szPath);
			if (szPath[nLen] != L'\\') {
				lstrcatW(szPath, L"\\");
				nLen++;
			}
 			WCHAR szDir[MAX_PATH] = {0};
			if (nLen ) {
				for (int i = 0; i < nLen; i++) {
					if (szPath[i] == L'\\') {
						StrCpyNW(szDir,szPath,i+2);
						if (PathFileExistsW(szDir)) {
							if (!PathIsDirectoryW(szDir))
								return FALSE; // 目录中存在非法路径
						}
						else {
							if (!CreateDirectoryW(szDir,NULL))
								return FALSE; 
						}
					}
				}
				return TRUE;
			}
		}
 		return FALSE;
	}
	inline BOOL	MakeFileDirectory(LPCSTR lpszFile) 
	{
		CHAR szDir[1024] = {0};
		lstrcpynA(szDir,lpszFile,1024);
		::PathRemoveFileSpecA(szDir);
		return MakeDirectory(szDir);
	}
	inline BOOL	MakeFileDirectory(LPCWSTR lpszFile)
	{
		WCHAR szDir[1024] = {0};
		lstrcpynW(szDir,lpszFile,1024);
		::PathRemoveFileSpecW(szDir);
		return MakeDirectory(szDir);
	}

	inline CHAR GetMaxFreeSpaceDriver()
	{
		CHAR szSpace[MAX_PATH] = {0}; 
		__int64 DriverSpaceArray[26] = {0}; 
		for (int i = 0; i < 26; i++) {
			wsprintfA(szSpace, "%c:", 'A' + i); 
			if (::GetDriveTypeA(szSpace) == DRIVE_FIXED ) {
				lstrcatA(szSpace, "\\");
				::GetDiskFreeSpaceExA(szSpace, NULL, NULL, (PULARGE_INTEGER)&DriverSpaceArray[i]); 
			}
		}

		char nIndex = 0; 
		__int64 iMax = DriverSpaceArray[0]; 

		for (char i = 0; i < 26; i++) {
			if (iMax < DriverSpaceArray[i]) {
				iMax = DriverSpaceArray[i]; 
				nIndex = i; 
			}
		}

		return ('A' + nIndex); 
	}
#ifdef __ATLSTR_H__
	inline CString ApplicationPath()
	{
		TCHAR szPath[1024] = {0};
		return ApplicationPath(szPath,1024);
 	}
	inline CString ApplicationFile(LPCTSTR lpszFile) 
	{
		TCHAR szPath[1024] = {0};
		return (LPCTSTR)ApplicationFile(lpszFile,szPath,1024);
	}
#endif

#ifdef _STRING_
	inline std::string StdApplicationPath()
	{
		char szPath[1024] = { 0 }; 
		return ApplicationPath(szPath, 1024); 
	}
	inline std::string StdApplicationFile(LPCSTR pszFile)
	{
		CHAR szPath[1024] = { 0 };
		return (LPCSTR)ApplicationFile(pszFile, szPath, 1024);
	}
#endif
};

namespace StreamHelper
{
	ATL_NOINLINE inline HRESULT BufferToStream(const BYTE* pBuffer, DWORD cbSize, IStream** ppStream)
	{
		if (!pBuffer || !ppStream || !cbSize) {
			return E_POINTER;
		}
		HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, cbSize);
		if (!hGlobal) {
			return E_OUTOFMEMORY;
		}
		LPVOID lpBase = ::GlobalLock(hGlobal);
		if (!lpBase) {
			GlobalFree(hGlobal);
			return E_OUTOFMEMORY;
		}
		memcpy(lpBase, pBuffer, cbSize);
		GlobalUnlock(hGlobal);
		return ::CreateStreamOnHGlobal(hGlobal,TRUE,ppStream);
	}

	ATL_NOINLINE inline HRESULT ResourceToBuffer(UINT nResID,PBYTE* ppBuffer, DWORD* pcbSize,LPCTSTR pszResType,HINSTANCE hInstance)
	{
		ATLASSERT(ppBuffer && pcbSize);
 		HGLOBAL  hGlobal = NULL;
		HRSRC    hSource = NULL;
		LPVOID   pBuffer  = NULL;
		PBYTE	 pOutBuffer = NULL;
		DWORD	 cbSize = 0;
		HRESULT  hr = E_OUTOFMEMORY;
		do {
			if (!(hSource = FindResource(hInstance, MAKEINTRESOURCE(nResID),pszResType))) {
				hr = E_FAIL; break;
			}
  			if(!(hGlobal = LoadResource(hInstance, hSource))){    
 				break;
			}
 			if (!(pBuffer = LockResource(hGlobal))) {
				break;
			}

			hr = E_OUTOFMEMORY;
			cbSize = (UINT)SizeofResource(hInstance, hSource);
			if (cbSize > 0)
			{
				pOutBuffer = (BYTE*)malloc(cbSize);
				if (pOutBuffer) {
					memcpy(pOutBuffer, pBuffer, cbSize);
					hr = S_OK;
				}
			}
  		} while(false);

		if (SUCCEEDED(hr)) {
			*ppBuffer = pOutBuffer;
			*pcbSize = cbSize;
		}
		else {
			if (pOutBuffer) {
				free(pOutBuffer);
			}
		}

		if (pBuffer){
 			UnlockResource(hGlobal);
		}
		if (hGlobal) {
			FreeResource(hGlobal); 
		}
 		return hr;
	}

	ATL_NOINLINE inline HRESULT ResourceToStream(UINT nResID,IStream** ppStream,LPCTSTR pszResType,HINSTANCE hInstance)
	{
 		HGLOBAL  hGlobal = NULL;
		HRSRC    hSource = NULL;
		LPVOID   pBuffer  = NULL;
 
		HRESULT  hr = E_OUTOFMEMORY;
		do {
			if (hInstance == NULL){
				hInstance = _pModule->GetResourceInstance();
				ATLASSERT(hInstance);
			}
			if (!(hSource = FindResource(hInstance, MAKEINTRESOURCE(nResID),pszResType))) {
				hr = E_FAIL; break;
			}
  			if(!(hGlobal = LoadResource(hInstance, hSource))){    
 				break;
			}
 			if (!(pBuffer = LockResource(hGlobal))) {
				break;
			}
 			DWORD cbSize = (UINT)SizeofResource(hInstance, hSource);
			hr = BufferToStream((PBYTE)pBuffer,cbSize,ppStream);

		} while(false);
		
		if (pBuffer){
 			UnlockResource(hGlobal);
		}
		if (hGlobal) {
			FreeResource(hGlobal); 
		}
 		return hr;
	}
};

namespace IniHelper
{
 	inline BOOL SetIniKey(LPCTSTR sSection,LPCTSTR sKey, LPCTSTR sValue, LPCTSTR sIniFile)
	{
		return ::WritePrivateProfileString(sSection,sKey,sValue,sIniFile);
	}

	inline BOOL SetIniKey(LPCTSTR sSection, LPCTSTR sKey, LONGLONG nValue, LPCTSTR sIniFile)
	{
		TCHAR szValue[64] = {0};
		_i64tot_s(nValue, szValue, 64, 10);
		return SetIniKey(sSection, sKey, szValue, sIniFile);
	}

	inline LPCTSTR GetIniKey(LPCTSTR sSection,LPCTSTR sKey,LPCTSTR sIniFile)
	{
 		static TCHAR TempBuffer[MAX_PATH] = {0};
		DWORD Result = GetPrivateProfileString(sSection,sKey,_T(""),TempBuffer,MAX_PATH,sIniFile);
		return Result ? TempBuffer : NULL;
	}

	inline LPCTSTR GetStringKey(LPCTSTR sSection,LPCTSTR sKey ,LPCTSTR sIniFile,LPCTSTR lpszDefault)
	{
		LPCTSTR lpszResult = GetIniKey(sSection, sKey, sIniFile);
		return (lpszResult == NULL ? lpszDefault : lpszResult);
	}

	inline LONGLONG GetIntKey(LPCTSTR sSection,LPCTSTR sKey ,LPCTSTR sIniFile,LONGLONG nDefaultValue)
	{
		LONGLONG lResult = nDefaultValue;
		LPCTSTR lpszValue = GetIniKey(sSection, sKey, sIniFile);
		if (lpszValue) {
#if _WIN32_IE >= 0x0600
			DWORD cbFlags = STIF_DEFAULT;
			if (StrStrI(lpszValue,_T("0x"))) {
				cbFlags = STIF_SUPPORT_HEX;
			}
			if (!StrToInt64Ex(lpszValue,cbFlags,&lResult)) {
				lResult = nDefaultValue;
			}
#else
			lResult = _tstoi64(lpszValue);
#endif
		}
		return lResult;
	}
}

namespace FileHelper
{
	inline HRESULT GetFileLength(LPCTSTR lpszFile,ULONGLONG& Length) 
 	{
 		HANDLE hFile = CreateFile(lpszFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			Length = 0; 
			return HRESULT_FROM_WIN32(GetLastError());
		}
 		HRESULT hr = S_OK;
 		LARGE_INTEGER FileLength;
		if (::GetFileSizeEx(hFile,&FileLength))  
			Length = FileLength.QuadPart ;
		else  {
			Length = 0; 
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
 		CloseHandle(hFile);
		return hr;
	}

	inline ULONGLONG GetFileLength(LPCSTR lpszFile) 
	{
		ULONGLONG nLength = 0; 
		GetFileLength(CA2T(lpszFile), nLength); 
		return nLength; 
	}
	inline ULONGLONG GetFileLength(LPCWSTR lpszFile) 
	{
		ULONGLONG nLength = 0; 
		GetFileLength(CW2T(lpszFile), nLength); 
		return nLength; 
	}

	inline HRESULT GetFileTime(LPCWSTR pszFile, FILETIME* pcTime,  FILETIME* pmTime)
	{
		if (!pcTime || !pmTime) {
			ATLASSERT(FALSE);
			return E_POINTER; 
		}
		
		HRESULT hr = S_OK; 
		DWORD cbStyle = 0; 
		if (::PathIsDirectoryW(pszFile)) {
			cbStyle =  FILE_FLAG_BACKUP_SEMANTICS;
		}

		HANDLE hFile = ::CreateFileW(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,cbStyle, NULL); 
		if (hFile != INVALID_HANDLE_VALUE) {
			FILETIME accessTime = {0}, createTime = {0}, modifyTime = {0};
			if (::GetFileTime(hFile, &createTime, &accessTime, &modifyTime)) {
				::FileTimeToLocalFileTime(&createTime, pcTime); 
				::FileTimeToLocalFileTime(&modifyTime, pmTime);
			}
			else {
				hr = HRESULT_FROM_WIN32(GetLastError()); 
			}
			CloseHandle(hFile);
		}
		else {
			hr = HRESULT_FROM_WIN32(GetLastError()); 
		}
		return HRESULT_FROM_WIN32(GetLastError());
	}

	inline HRESULT GetFileTime(LPCSTR pszFile, FILETIME* pcTime,  FILETIME* pmTime)
	{
		return GetFileTime(CA2W(pszFile), pcTime, pmTime);
	}

	inline HRESULT ReadToBuffer(LPCTSTR lpszFile,PBYTE* ppBytes, DWORD* pcbSize)
	{
		if (!ppBytes && !pcbSize) {
			return E_POINTER;
		}
		HRESULT hr = S_OK;
		HANDLE hFile = INVALID_HANDLE_VALUE;
		bool fAlloced = false;
		do {
			hFile = CreateFile(lpszFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
			if (hFile == INVALID_HANDLE_VALUE) {
				hr = HRESULT_FROM_WIN32(GetLastError());
				break;
			}
			DWORD cbSize = ::GetFileSize(hFile,NULL);
 			// 未设定读取缓存，只获取文件长度
			if (!ppBytes && pcbSize) {
				*pcbSize = cbSize;
				break;
 			}

			if (!(*ppBytes)) {
				*ppBytes = (BYTE*)calloc(cbSize+1,1);
				if (!(*ppBytes)) {
					hr = E_OUTOFMEMORY;
					break;
				}
				fAlloced = true;
			}
 			DWORD cbReadBytes = 0;
			if (!ReadFile(hFile,*ppBytes,cbSize,&cbReadBytes,NULL)) {
				hr = HRESULT_FROM_WIN32(GetLastError());
				if (fAlloced) {
					__SAFE_FREE_PTR(*ppBytes);
				}
				break;
			}
			else {
				ATLASSERT(cbReadBytes == cbSize);
				if (pcbSize) {
					*pcbSize = cbReadBytes;
				}
			}
		} while(false);
		if (hFile != INVALID_HANDLE_VALUE) {
			CloseHandle(hFile);
		}
		return hr;
	}
	
	inline HRESULT ReadToStream(LPCTSTR lpszFile,IStream** ppStream)
	{
		if (!ppStream) {
			return E_POINTER;
		}
		HRESULT hr = S_OK;
		HANDLE  hFile = INVALID_HANDLE_VALUE;
		HGLOBAL hGlobal = NULL;
		LPVOID  lpBase = NULL;
		do {
			hFile = CreateFile(lpszFile,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
			if (hFile == INVALID_HANDLE_VALUE) {
				hr = HRESULT_FROM_WIN32(GetLastError());
				break;
			}
			DWORD cbSize = ::GetFileSize(hFile,NULL);
 			hGlobal = GlobalAlloc(GMEM_MOVEABLE, cbSize);
			if (hGlobal == NULL) {
				hr = E_OUTOFMEMORY;
				break;
			}
			lpBase = GlobalLock(hGlobal);
			if (lpBase == NULL) {
				hr = E_OUTOFMEMORY;
				break;
			}
 			DWORD cbReadBytes = 0;
			if (!ReadFile(hFile,lpBase,cbSize,&cbReadBytes,NULL)) {
				hr = HRESULT_FROM_WIN32(GetLastError());
				break;
			}
			else {
				 hr = CreateStreamOnHGlobal(hGlobal,TRUE,ppStream);
			}
		} while(false);

		if (FAILED(hr)) {
			if (lpBase) {
				::GlobalUnlock(lpBase);
			}
			if (hGlobal) {
				::GlobalFree(hGlobal);
			}
		}
		if (hFile != INVALID_HANDLE_VALUE) {
			CloseHandle(hFile);
		}
		return hr;
	}

#ifdef _STRING_
	inline std::string StdReadToString(LPCSTR lpszFile)
	{
		char* buffer = 0; 
		DWORD cbSize = 0; 
 		ReadToBuffer(CA2T(lpszFile), (PBYTE*)&buffer, &cbSize);
		std::string result = "";
		if (buffer) {
			result = buffer;
			free(buffer);
		}
		return result; 
	}
#endif
#ifdef __ATLSTR_H__
	inline CStringA ReadToString(LPCSTR lpszFile)
	{
		char* buffer = 0;
		DWORD cbSize = 0;
		ReadToBuffer(CA2T(lpszFile), (PBYTE*)&buffer, &cbSize);
		CStringA result = buffer;
		free(buffer);
		return result;
	}
#endif
	
	inline HRESULT WriteToFile(LPCSTR lpszFile, LPVOID lpBuffer, DWORD cbSize)
	{
		if (::PathFileExistsA(lpszFile)) {
			::DeleteFileA(lpszFile);
		}
		HANDLE hFile = CreateFileA(lpszFile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return HRESULT_FROM_WIN32(GetLastError());
		}

		HRESULT hr = S_OK;
		DWORD cbWriteBytes = 0;
		if (!WriteFile(hFile,lpBuffer,cbSize,&cbWriteBytes,NULL)) {
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
		CloseHandle(hFile);
		return hr;
	}

	inline HRESULT WriteToFile(LPCWSTR lpszFile, LPVOID lpBuffer, DWORD cbSize)
	{
		if (::PathFileExistsW(lpszFile)) {
			::DeleteFileW(lpszFile);
		}
		HANDLE hFile = CreateFileW(lpszFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return HRESULT_FROM_WIN32(GetLastError());
		}

		HRESULT hr = S_OK;
		DWORD cbWriteBytes = 0;
		if (!WriteFile(hFile, lpBuffer, cbSize, &cbWriteBytes, NULL)) {
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
		CloseHandle(hFile);
		return hr;
	}


	inline HRESULT WriteToFile(LPCSTR lpszFile, LPCSTR lpszString)
	{
		return WriteToFile(lpszFile, (LPVOID)lpszString, lstrlenA(lpszString)); 
	}

	
	inline HRESULT AppendFile(LPCTSTR lpszFile, const BYTE* pBuffer, DWORD cbSize)
	{
		DWORD cbFlags = OPEN_EXISTING;
		if (!PathFileExists(lpszFile)) {
			cbFlags = CREATE_NEW;
		}
		HANDLE hFile = CreateFile(lpszFile,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,cbFlags,0,NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return HRESULT_FROM_WIN32(GetLastError());
		}
		LONG lMoved = 0;
		::SetFilePointer(hFile,0,&lMoved,FILE_END);
		
		HRESULT hr = S_OK;
		DWORD cbWriteBytes = 0;
		if (!WriteFile(hFile,pBuffer,cbSize,&cbWriteBytes,NULL)) {
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
		CloseHandle(hFile);
		return hr;
	}

	inline HANDLE  CreateProcess(LPCTSTR lpszProcess, WORD wShowWindow)
	{
		TCHAR szProc[4096] = {0};
		lstrcpyn(szProc, lpszProcess, 2048);

 		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi= {0};
		si.cb = sizeof(si);
		si.wShowWindow = wShowWindow; 
		if (CreateProcess(NULL, szProc,NULL,NULL,FALSE,0,NULL,0,&si,&pi)) {
			return pi.hProcess ;
		}
 		return NULL;
	}

	inline HANDLE  CreateProcessByDesktop(LPCTSTR lpszDesktop, LPCTSTR lpszProcess, WORD wShowWindow)
	{
		TCHAR szProc[4096] = { 0 };
		TCHAR szDesk[1024] = { 0 };
		lstrcpyn(szProc, lpszProcess, 2048);
		lstrcpyn(szDesk, lpszDesktop, 1024); 
		STARTUPINFO si = { 0 };

		PROCESS_INFORMATION pi = { 0 };
		si.cb = sizeof(si);
		si.wShowWindow = wShowWindow;
		si.lpDesktop = szDesk; 
		if (CreateProcess(NULL, szProc, NULL, NULL, FALSE, 0, NULL, 0, &si, &pi)) {
			return pi.hProcess;
		}
		return NULL;
	}

	inline BOOL CreateProcess(LPCTSTR lpszProcess, LPCTSTR lpszDir, DWORD dwWait, WORD wShowWindow)
	{
		TCHAR szCurrentPath[1024] = {0};
		TCHAR szProc[2048] = {0};

		GetCurrentDirectory(1024, szCurrentPath);
		SetCurrentDirectory(lpszDir);

		lstrcpyn(szProc, lpszProcess, 2038);
 		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi= {0};
		si.cb = sizeof(si);
		si.wShowWindow = wShowWindow; 

		BOOL fResult = CreateProcess(NULL, szProc,NULL,NULL,FALSE,0,NULL,0,&si,&pi);
		if (fResult) {
			DWORD cbResult = ::WaitForSingleObject(pi.hProcess , dwWait) ;
			if (cbResult != WAIT_OBJECT_0) {
				::TerminateProcess(pi.hProcess,-2);
			}
		}
		SetCurrentDirectory(szCurrentPath);
		return fResult;
	}

	inline HANDLE CreateProcessEx(LPCTSTR lpszProcess, LPCTSTR lpszArgs, LPCTSTR lpszDir, WORD wShowWindow)
	{
		TCHAR szProc[2048] = {0};
		lstrcpyn(szProc, lpszProcess, 2048);

		if (lpszArgs) {
			lstrcat(szProc, _T(" "));
			lstrcat(szProc, lpszArgs);
		}

		TCHAR szCurrentPath[1024] = {0};
 		if (::PathIsDirectory(lpszDir)) {
			GetCurrentDirectory(1024, szCurrentPath);
			SetCurrentDirectory(lpszDir);
		}

 		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi= {0};
		si.cb = sizeof(si);
		si.wShowWindow = wShowWindow; 
 		if (CreateProcess(NULL, szProc,NULL,NULL,FALSE,0,NULL,lpszDir,&si,&pi)) {
			
		}
		else {
			/*CStringA sExt;
			sExt.Format("LastError: %d", GetLastError());
			MessageBox(NULL, sExt, 0, 0);*/
		}
		if (szCurrentPath[0]) {
			SetCurrentDirectory(szCurrentPath);
		}
		return pi.hProcess;
	}


	inline HANDLE CreateDOSProcess(LPCTSTR lpszProcess)
	{
		TCHAR szProc[4096] = { 0 };
		lstrcpyn(szProc, lpszProcess, 2048);

		STARTUPINFO StartupInfo = { 0 };
		GetStartupInfo(&StartupInfo);
		StartupInfo.lpReserved = NULL;
		StartupInfo.lpDesktop = NULL;
		StartupInfo.lpTitle = NULL;
		StartupInfo.dwX = 0;
		StartupInfo.dwY = 0;
		StartupInfo.dwXSize = 0;
		StartupInfo.dwYSize = 0;
		StartupInfo.dwXCountChars = 500;
		StartupInfo.dwYCountChars = 500;
		StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
		StartupInfo.wShowWindow = SW_HIDE;
		//说明进程将以隐藏的方式在后台执行    
		StartupInfo.cbReserved2 = 0;
		StartupInfo.lpReserved2 = NULL;
		StartupInfo.hStdInput = stdin;
		StartupInfo.hStdOutput = stdout;
		StartupInfo.hStdError = stderr;    

		PROCESS_INFORMATION pi = { 0 };
		if (CreateProcess(NULL, szProc, NULL, NULL, FALSE, 0, NULL, 0, &StartupInfo, &pi)) {
			return pi.hProcess;
		}
		return NULL;
	}
};

namespace HttpHelper
{
	ATL_NOINLINE inline BOOL ParserUrl(LPCTSTR lpszUrl, LPTSTR pszHost, LPTSTR pszUrl, WORD& port)
	{
		if (lstrlen(lpszUrl) <= 0)
			return FALSE;

		TCHAR szHost[MAX_PATH] = {0};
		TCHAR szUrl[MAX_PATH] = {0};
		TCHAR szExtraInfo[MAX_PATH] = {0};

		URL_COMPONENTS url_components = {0};
		url_components.dwStructSize = sizeof(URL_COMPONENTS);
		url_components.nPort = port;
		url_components.lpszHostName = szHost;
		url_components.dwHostNameLength = MAX_PATH;
		url_components.lpszUrlPath = szUrl;
		url_components.dwUrlPathLength = MAX_PATH;
		url_components.lpszExtraInfo = szExtraInfo;
		url_components.dwExtraInfoLength = MAX_PATH;

		BOOL fResult = ::InternetCrackUrl(lpszUrl,lstrlen(lpszUrl),ICU_DECODE,&url_components);
		if (fResult)
		{
			port = url_components.nPort ;

			StrCpyN(pszHost,szHost,MAX_PATH);
			StrCpyN(pszUrl,szUrl,MAX_PATH);
			if (StrStrI(szUrl,szExtraInfo) == NULL)
			{
				StrCat(pszUrl,szExtraInfo);
			}
		}
		else
		{
			TCHAR szError[MAX_PATH];
			wsprintf(szError,_T("Call InternetCrackUrl Failed! Error code: 0x%08x \n"),::GetLastError());
			OutputDebugString(szError);
		}
		return fResult;
	}
#ifdef __ATLSTR_H__
	ATL_NOINLINE inline BOOL ParserUrl(LPCTSTR lpszUrl, CString& sHost, CString& sUrl, WORD& port)
	{
		LPTSTR pszHost = sHost.GetBufferSetLength(MAX_PATH);
		LPTSTR pszUrl = sUrl.GetBufferSetLength(MAX_PATH);
		BOOL Result = ParserUrl(lpszUrl, pszHost, pszUrl, port);
		sHost.ReleaseBuffer();
		sUrl.ReleaseBuffer();
		return Result;
	}
#endif


};

#ifdef _UNICODE
#define URLCrack URL::URLCrackW
#else
#define URLCrack URL::URLCrackA
#endif

namespace URL
{


#ifdef _STRING_
	inline std::string _UrlEncode(const std::string& szToEncode)
	{
		std::string src = szToEncode;
		char hex[] = "0123456789ABCDEF";
		std::string dst;
		for (size_t i = 0; i < src.size(); ++i)
		{
			unsigned char cc = src[i];
			if (isascii(cc))
			{
				if (cc == ' ')
				{
					dst += "%20";
				}
				else if (cc == '%')
					dst += "%25";
				else
					dst += cc;
			}
			else
			{
				unsigned char c = static_cast<unsigned char>(src[i]);
				dst += '%';
				dst += hex[c / 16];
				dst += hex[c % 16];
			}
		}
		return dst;
	}

	inline std::string _UrlDecode(const std::string& szToDecode)
	{
		std::string result;
		int hex = 0;
		for (size_t i = 0; i < szToDecode.length(); ++i)
		{
			switch (szToDecode[i])
			{
			case '+':
				result += ' ';
				break;
			case '%':
				if (isxdigit(szToDecode[i + 1]) && isxdigit(szToDecode[i + 2]))
				{
					std::string hexStr = szToDecode.substr(i + 1, 2);
					hex = strtol(hexStr.c_str(), 0, 16);
					//字母和数字[0-9a-zA-Z]、一些特殊符号[$-_.+!*'(),] 、以及某些保留字[$&+,/:;=?@]
					//可以不经过编码直接用于URL
					if (!((hex >= 48 && hex <= 57) || //0-9
						(hex >=97 && hex <= 122) || //a-z
						(hex >=65 && hex <= 90) || //A-Z
						//一些特殊符号及保留字[$-_.+!*'(),] [$&+,/:;=?@]
						hex == 0x21 || hex == 0x24 || hex == 0x26 || hex == 0x27 || hex == 0x28 || hex == 0x29
						|| hex == 0x2a || hex == 0x2b|| hex == 0x2c || hex == 0x2d || hex == 0x2e || hex == 0x2f
						|| hex == 0x3A || hex == 0x3B|| hex == 0x3D || hex == 0x3f || hex == 0x40 || hex == 0x5f
						))
					{
						result += char(hex);
						i += 2;
					}
					else result += '%';
				}
				else {
					result += '%';
				}
				break;
			default:
				result += szToDecode[i];
				break;
			}; 
		}
		return result;
	}
#endif

	class CURLEncoder : public StringHelper::StringHelperImpl<CURLEncoder,CHAR> {
	public:
		explicit CURLEncoder(const char* url) {
			std::string utf8 = StringHelper::CUTF8Encoder(url).m_pStr ; 
			std::string en = _UrlEncode(utf8); 
			m_pStr = (char*)calloc(en.length() + 1, 1); 
			lstrcpyA(m_pStr, en.c_str()); 
		}
		explicit CURLEncoder(const wchar_t* url) {
			std::string utf8 = StringHelper::CUTF8Encoder(url).m_pStr ; 
			std::string en = _UrlEncode(utf8); 
			m_pStr = (char*)calloc(en.length() + 1, 1); 
			lstrcpyA(m_pStr, en.c_str()); 
		}
	}; 

	class CURLDecoder : public StringHelper::StringHelperImpl<CURLDecoder,CHAR> {
	public:
		explicit CURLDecoder(const char* url) {
			std::string s  = url; 
			std::string de = _UrlDecode(s); 
			StringHelper::CUTF8Decoder _Coder(de.c_str()); 
			m_pStr = _Coder.Detach(); 
		}
		explicit CURLDecoder(const wchar_t* url) {
			std::string s  = CW2A(url); 
			std::string de = _UrlDecode(s); 
			StringHelper::CUTF8Decoder _Coder(de.c_str()); 
			m_pStr = _Coder.Detach(); 
		}
	}; 

	class URLCrackA {
	public:
		explicit URLCrackA(const char* url) {
			m_succeeded = crack(url);
		}
	public:
		void empty() {
			m_succeeded = false;
			scheme_n = 0;
			port = 0;
			memset(host, 0, sizeof(host) * sizeof(char));
			memset(url, 0, sizeof(url) * sizeof(char));
			memset(extra_info, 0, sizeof(extra_info) * sizeof(char));
			memset(scheme, 0, sizeof(scheme) * sizeof(char));
			memset(user, 0, sizeof(user) * sizeof(char));
			memset(pass, 0, sizeof(pass) * sizeof(char));
		}
		bool crack(const char* surl) {
			empty();
			int length = lstrlenA(surl);
			if (length <= 0) {
				return false;
			}

			CURLEncoder _Coder(surl); 

			URL_COMPONENTSA url_components = { 0 };
			url_components.dwStructSize = sizeof(URL_COMPONENTS);
			url_components.lpszHostName = host;
			url_components.dwHostNameLength = _cch_size;
			url_components.lpszUrlPath = url;
			url_components.dwUrlPathLength = _cch_max;
			url_components.lpszExtraInfo = extra_info;
			url_components.dwExtraInfoLength = _cch_max;
			url_components.lpszScheme = scheme;
			url_components.dwSchemeLength = _cch_size;
			url_components.lpszUserName = user;
			url_components.dwUserNameLength = _cch_size;
			url_components.lpszPassword = pass;
			url_components.dwPasswordLength = _cch_size;

			m_succeeded = ::InternetCrackUrlA(_Coder.m_pStr, length, ICU_DECODE, &url_components) ? true : false;//用于分析url分析完毕之后会将各个参数放入url_components中
			if (m_succeeded) {
				port = url_components.nPort;
				scheme_n = url_components.nScheme;
			}
			else {
				ATLTRACE(_T("Call InternetCrackUrl Failed! Error code: 0x%08x \n"), ::GetLastError());
			}
			return m_succeeded;

		}
		bool succeeded() {
			return m_succeeded;
		};
	public:
		enum { _cch_max = 256, _cch_size = 128 };
		char host[_cch_size];
		char url[_cch_max];
		char extra_info[_cch_max];
		char scheme[_cch_size];
		char user[_cch_size];
		char pass[_cch_size];
		unsigned short port;
		short scheme_n;
	protected:
		bool m_succeeded;
	};

	class URLCrackW {
	public:
		explicit URLCrackW(const wchar_t* url) {
			m_succeeded = crack(url);
		}
	public:
		void empty() {
			m_succeeded = false;
			scheme_n = 0;
			port = 0;
			memset(host, 0, sizeof(host) * sizeof(wchar_t));
			memset(url, 0, sizeof(url) * sizeof(wchar_t));
			memset(extra_info, 0, sizeof(extra_info) * sizeof(wchar_t));
			memset(scheme, 0, sizeof(scheme) * sizeof(wchar_t));
			memset(user, 0, sizeof(user) * sizeof(wchar_t));
			memset(pass, 0, sizeof(pass) * sizeof(wchar_t));
		}
		bool crack(const wchar_t* surl) {
			empty();
			int length = lstrlenW(surl);
			if (length <= 0) {
				return false;
			}

			URL_COMPONENTSW url_components = { 0 };
			url_components.dwStructSize = sizeof(URL_COMPONENTS);
			url_components.lpszHostName = host;
			url_components.dwHostNameLength = _cch_size;
			url_components.lpszUrlPath = url;
			url_components.dwUrlPathLength = _cch_max;
			url_components.lpszExtraInfo = extra_info;
			url_components.dwExtraInfoLength = _cch_max;
			url_components.lpszScheme = scheme;
			url_components.dwSchemeLength = _cch_size;
			url_components.lpszUserName = user;
			url_components.dwUserNameLength = _cch_size;
			url_components.lpszPassword = pass;
			url_components.dwPasswordLength = _cch_size;

			m_succeeded = ::InternetCrackUrlW(surl, length, ICU_DECODE, &url_components) ? true : false;
			if (m_succeeded) {
				port = url_components.nPort;
				scheme_n = url_components.nScheme;
			}
			else {
				ATLTRACE(_T("Call InternetCrackUrl Failed! Error code: 0x%08x \n"), ::GetLastError());
			}
			return m_succeeded;

		}
		bool succeeded() {
			return m_succeeded;
		};
	public:
		enum { _cch_max = 256, _cch_size = 128 };
		wchar_t host[_cch_size];
		wchar_t url[_cch_max];
		wchar_t extra_info[_cch_max];
		wchar_t scheme[_cch_size];
		wchar_t user[_cch_size];
		wchar_t pass[_cch_size];
		unsigned short port;
		short scheme_n;
	protected:
		bool m_succeeded;
	};
}

#ifdef __CMD_HTTP__

#include <Inet/INet.h>
namespace HttpHelper
{
	ATL_NOINLINE inline DWORD HttpGet(LPCTSTR lpszUrl, PBYTE* pOutBuffer, DWORD* pcbSize)
	{
  		if (pOutBuffer == NULL || pcbSize == NULL) {
			ATLASSERT(FALSE); 
			return 0; 
		}

		URLCrack crack(lpszUrl); 
		CCmdBuffer Buffer; 

		try {
			CInternetSession session; 
			CHttpConnection http(session,crack.host, crack.port,crack.user,crack.pass);
			::CHttpFile file(http, _T("GET"), CA2T(crack.url), _T("1.1")); 
			file.SendRequest(); 

			DWORD cbSize = 0;
			try { cbSize = file.GetLength(); }	catch (...) {}
			if (cbSize <= 0){
				try {
					TCHAR Buffer[64] = { 0 };
					DWORD cbBufferLength = sizeof(Buffer);
					file.QueryInfo(HTTP_QUERY_CONTENT_LENGTH, Buffer, &cbBufferLength);
					cbSize = (DWORD)StrToInt(Buffer);
				}
				catch (...) {}
			}

			if (cbSize > 0) {
				Buffer.Alloc(cbSize+1); 
			}

			const unsigned int __cch_size = 8192; 
			BYTE ReadBuffer[__cch_size];
			for (DWORD dwRead; dwRead = file.Read(ReadBuffer, __cch_size);) {
				Buffer.Append((char*)ReadBuffer, dwRead); 
			}

			*pcbSize = Buffer.m_Length; 
			*pOutBuffer = (PBYTE)Buffer.m_pBuffer; 
			Buffer.Empty(); 

			return *pcbSize; 
		}
		catch (::CInternetException &err) {
			OutputDebugString(err.GetErrorMessage()); 
			*pOutBuffer = 0; 
			*pcbSize = 0; 
		}
		return (DWORD)0;
	}

	ATL_NOINLINE inline DWORD HttpGetEx(LPCTSTR lpszUrl, PBYTE* pOutBuffer, DWORD* pcbSize, std::atomic<bool>& running)
	{
		if (pOutBuffer == NULL || pcbSize == NULL || !running) {
			ATLASSERT(FALSE);
			return 0;
		}

		URLCrack crack(lpszUrl);
		CCmdBuffer Buffer;

		try {
			CInternetSession session;
			CHttpConnection http(session, crack.host, crack.port, crack.user, crack.pass);
			::CHttpFile file(http, _T("GET"), CA2T(crack.url), _T("1.1"));
			if (running) file.SendRequest();

			DWORD cbSize = 0;
			try { cbSize = file.GetLength(); }
			catch (...) {}
			if (cbSize <= 0 && running){
				try {
					TCHAR Buffer[64] = { 0 };
					DWORD cbBufferLength = sizeof(Buffer);
					file.QueryInfo(HTTP_QUERY_CONTENT_LENGTH, Buffer, &cbBufferLength);
					cbSize = (DWORD)StrToInt(Buffer);
				}
				catch (...) {}
			}

			if (cbSize > 0) {
				Buffer.Alloc(cbSize + 1);
			}

			*pcbSize = 0;
			const unsigned int __cch_size = 8192;
			BYTE ReadBuffer[__cch_size];
			if (running) {
				for (DWORD dwRead; dwRead = file.Read(ReadBuffer, __cch_size);) {
					if (!running) {
						throw CInternetException(file.GetInternetHandle(), _T("user canceled!"));
					}
					Buffer.Append((char*)ReadBuffer, dwRead);
				}

				*pcbSize = Buffer.m_Length;
				*pOutBuffer = (PBYTE)Buffer.m_pBuffer;
				Buffer.Empty();
			}

			return *pcbSize;
		}
		catch (::CInternetException &err) {
			OutputDebugString(err.GetErrorMessage());
			*pOutBuffer = 0;
			*pcbSize = 0;
		}
		return (DWORD)0;
	}

	ATL_NOINLINE inline int HttpPost(LPCTSTR lpszUrl, const BYTE* pPostData, const DWORD cbData, LPCTSTR lpszHeader, PBYTE* pOutBuffer, DWORD* pcbSize)
	{
		return 0;
	}

	inline int HttpDownload(LPCSTR lpszUrl, PBYTE* pOutBuffer, DWORD* pcbSize)
	{
		return HttpGet(lpszUrl, pOutBuffer, pcbSize); 
	}

	inline bool HttpDownload(LPCSTR lpszUrl, LPCTSTR lpszFile)
	{
		CCmdTempBuffer buffer;
		HttpGet(lpszUrl, (PBYTE*)&buffer.ptr, (DWORD*)&buffer.size); 
		if (buffer.size > 0) {
			return SUCCEEDED(FileHelper::WriteToFile(lpszFile, buffer.ptr, buffer.size)); 
		}
		return false; 
	}
}

#endif

namespace StringHelper {
	typedef URL::CURLEncoder CUrlEncoder; 
	typedef URL::CURLDecoder CUrlDecoder; 
}; 

namespace RegHelperX
{
	inline LONG GetRegDWORDValue(LPCTSTR lpszPath, LPCTSTR lpszKey, DWORD& dwValue, HKEY hRootKey)
	{
		CRegKey regKey;
		LONG lResult = regKey.Open(hRootKey,lpszPath);
		if (lResult == ERROR_SUCCESS) {
			lResult = regKey.QueryDWORDValue(lpszKey,dwValue);
		}
		return lResult;
	}

	inline LONG GetRegStringValue(LPCTSTR lpszPath, LPCTSTR lpszKey, LPTSTR pBuffer, DWORD* pcbSize, HKEY hRootKey)
	{
		CRegKey regKey;
		LONG lResult = regKey.Open(hRootKey,lpszPath);
		if (lResult == ERROR_SUCCESS) {
			lResult = regKey.QueryStringValue(lpszKey,pBuffer,pcbSize);
		}
		return lResult;
 	}

	inline LONG WriteRegDWORDValue(LPCTSTR lpszPath, LPCTSTR lpszKey, DWORD dwValue, HKEY hRootKey)
	{
		CRegKey regKey;
		LONG lResult = regKey.Create(hRootKey,lpszPath);
		if (lResult == ERROR_SUCCESS) {
			lResult = regKey.SetDWORDValue(lpszKey,dwValue);
		}
		return lResult;
	}

	inline LONG WriteRegStringValue(LPCTSTR lpszPath, LPCTSTR lpszKey, LPCTSTR pBuffer, HKEY hRootKey )
	{
		CRegKey regKey;
		LONG lResult = regKey.Create(hRootKey,lpszPath);
		if (lResult == ERROR_SUCCESS) {
			lResult = regKey.SetStringValue(lpszKey,pBuffer);
		}
		return lResult;
	}

#ifdef __ATLSTR_H__
	inline LONG GetRegStringValue(LPCTSTR lpszPath, LPCTSTR lpszKey, CString& StringBuffer, HKEY hRootKey)
	{
		CRegKey regKey;
		LONG lResult = regKey.Open(hRootKey,lpszPath);
		if (lResult == ERROR_SUCCESS) {
			DWORD cbLength = 0;
			lResult = regKey.QueryStringValue(lpszKey,NULL,&cbLength);
			if (lResult == ERROR_SUCCESS) {
				LPTSTR pBuffer = StringBuffer.GetBufferSetLength(cbLength + 1);
				lResult = regKey.QueryStringValue(lpszKey,pBuffer,&cbLength);
				StringBuffer.ReleaseBuffer();
			}
		}
		return lResult;
	}
#endif
};

namespace Sign
{
	inline long CRC_HashFile(LPCSTR lpszFile)
	{
		PBYTE pBuffer = NULL;
		DWORD cbSize = 0;
		LONG lResult = 0;
		FileHelper::ReadToBuffer(CA2T(lpszFile),&pBuffer,&cbSize);
		if (pBuffer && cbSize)
		{
			lResult = Sign::CRC_Hash(pBuffer,cbSize);
		}
		__SAFE_FREE_PTR(pBuffer);
		return lResult;
	}

	inline long CRC_HashFile(LPCWSTR lpszFile)
	{
		PBYTE pBuffer = NULL;
		DWORD cbSize = 0;
		LONG lResult = 0;
		FileHelper::ReadToBuffer(CW2T(lpszFile),&pBuffer,&cbSize);
		if (pBuffer && cbSize)
		{
			lResult = Sign::CRC_Hash(pBuffer,cbSize);
		}
		__SAFE_FREE_PTR(pBuffer);
		return lResult;
	}

	inline long CRC_HashModule(HINSTANCE hInstance)
	{
		TCHAR szModule[1024] = {0};
		::GetModuleFileName(hInstance,szModule,1024);
		return CRC_HashFile(szModule);
	}

	inline LPCSTR	MD5_HashFile(LPCSTR lpszFile,LPSTR pOutMD5)
	{
		PBYTE pBuffer = NULL;
		DWORD cbSize = 0;
 		FileHelper::ReadToBuffer(CA2T(lpszFile),&pBuffer,&cbSize);
		if (pBuffer && cbSize)
		{
			Sign::MD5_Hash(pBuffer,cbSize,pOutMD5);
		}
		__SAFE_FREE_PTR(pBuffer);
		return pOutMD5;
	}
	inline LPCWSTR MD5_HashFile(LPCWSTR lpszFile,LPWSTR pOutMD5) 
	{
		PBYTE pBuffer = NULL;
		DWORD cbSize = 0;
 		FileHelper::ReadToBuffer(CW2T(lpszFile),&pBuffer,&cbSize);
		if (pBuffer && cbSize)
		{
			Sign::MD5_Hash(pBuffer,cbSize,(LPWSTR)pOutMD5);
		}
		__SAFE_FREE_PTR(pBuffer);
		return pOutMD5;
	}
	inline LPCSTR  MD5_HashModule(LPSTR pOutMD5,HINSTANCE hInstance )
	{
		CHAR szModule[1024] = {0};
		::GetModuleFileNameA(hInstance,szModule,1024);
		return MD5_HashFile(szModule,pOutMD5);
	}
	inline LPCWSTR MD5_HashModule(LPWSTR pOutMD5,HINSTANCE hInstance)
	{
		WCHAR szModule[1024] = {0};
		::GetModuleFileNameW(hInstance,szModule,1024);
		return MD5_HashFile(szModule,(LPWSTR)pOutMD5);
	}

	inline HRESULT MD5_HashHugeFile(LPCTSTR lpszFile, LPTSTR pOutMD5, IPosCallback *Callback)
	{
 		if (!::PathFileExists(lpszFile) || ::PathIsDirectory(lpszFile)) {
 			return E_INVALIDARG;
		}
		//
		// 初始化并动态加载MD5 API 
		//
		MD5Entry* _Entry = GetMD5Entry();
		if (!_Entry || !_Entry->MD5Init) {
			return E_FAIL;
		}

		bool fCancel = false;
 		//
		// 打开本地文件，并将启用文件映射模式读取文件MD5哈希
		// 算法描述：
		//		1、当文件小于默认读取设置值(10MB),则直接一次性将文件映射到缓存，一次性读取MD5哈希
		//		2、当文件大于默认读取设置值(10MB),则自动按照读取设置分块映射到文件缓存，分多次读取MD5
		//
		ATL::CAtlFile file;
		ATL::CAtlFileMapping<CHAR> fileMap;
		HRESULT hr = file.Create(lpszFile,GENERIC_READ,FILE_SHARE_READ,OPEN_EXISTING);
		if (FAILED(hr)) {
 			return hr;
		}
		
		ULONGLONG BufferSize = 10 * 1024 * 1024;
		ULONGLONG nFileSize = 0;
		file.GetSize(nFileSize);
		Sign::MD5_CTX ctx = { 0 };
		_Entry->MD5Init(&ctx);
		if (nFileSize > 0) {
			if (Callback) {
				Callback->PosChanging(POS_MD5, 0, nFileSize, CT2A(lpszFile), fCancel);
				if (fCancel) {
					return E_ABORT;
				}
			}
			if (nFileSize <= BufferSize) {
				//
				// 文件小于10MB，则直接一次性读取文件MD5哈希值
				//
				if (SUCCEEDED(fileMap.MapFile(file.m_h))) {
					_Entry->MD5Update(&ctx, (PBYTE)fileMap.GetData(), (DWORD)nFileSize);
					fileMap.Unmap();
				}
				_Entry->MD5Final(&ctx);
				if (Callback) {
					Callback->PosChanged(POS_MD5, nFileSize, nFileSize, CT2A(lpszFile));
				}
			}
			else {
				//
				// 文件大于10MB，则按照10MB大小分组读取文件内容，进行MD5哈希
				//
				ULONGLONG nStep = nFileSize / BufferSize;
				ULONGLONG nMod = nFileSize % BufferSize;
				for (ULONGLONG i = 0; i < nStep; i++)
				{
					if (Callback) {
						Callback->PosChanging(POS_MD5, i * BufferSize, nFileSize, NULL, fCancel);
						if (fCancel) {
							return E_ABORT;
						}
					}

					//
					// 分块映射文件内容到缓存去，并读取MD5哈希
					//
					__pragma(warning(push))
						__pragma(warning(disable: 4244))
						hr = fileMap.MapFile(file.m_h, BufferSize, i * BufferSize);
					if (SUCCEEDED(hr)) {
						_Entry->MD5Update(&ctx, (PBYTE)fileMap.GetData(), BufferSize);
					}
					__pragma(warning(pop))
						fileMap.Unmap();
					//
					// 用户未改变MD5读取状态，则向客户端激发传输速率事件
					//
					if (Callback) {
						LONGLONG nBytes = BufferSize * (i + 1);
						//ChangeProperty("trans",nBytes,true);
						Callback->PosChanged(POS_MD5, nBytes, nFileSize, NULL);
					}
				}
				//
				// 读取文件剩余部分，进行MD5哈希
				//
				if (nMod > 0)	{
					__pragma(warning(push))
						__pragma(warning(disable: 4244))
						hr = fileMap.MapFile(file.m_h, nMod, BufferSize * nStep);
					if (SUCCEEDED(hr)) {
						_Entry->MD5Update(&ctx, (PBYTE)fileMap.GetData(), nMod);
					}
					__pragma(warning(pop))
						fileMap.Unmap();
					if (Callback) {
						Callback->PosChanged(POS_MD5, nFileSize, nFileSize, NULL);
					}
				}
				_Entry->MD5Final(&ctx);
			}
		}
		//
		// 客户端未修改读取状态，则更新本地文件MD5值
		//
 		TCHAR szCode[64] = {0};
		TCHAR szTemp[3] = {0};
		for (int i = 0; i < 16; i++) {
			wsprintf(szTemp,_T("%02x"),ctx.digest[i]);
			StrCat(szCode,szTemp);
		}
		lstrcpy(pOutMD5,szCode);
		return S_OK;
 	}
}

////////////////////////////////////////////////////////////////////////////////
// CmdBuffer
inline CCmdBuffer::CCmdBuffer() 
	: m_pBuffer(0)
	, m_Alloced(0)
	, m_Length(0)
	, m_BufferType(CmdNone)
{}

inline CCmdBuffer::CCmdBuffer(unsigned long size)
	: m_pBuffer(0) 
	, m_Alloced(0)
	, m_Length(0)
	, m_BufferType(CmdNone)
{ 
	m_pBuffer = Alloc(size); 
}

inline CCmdBuffer::CCmdBuffer(const CCmdBuffer& obj) 
	: m_pBuffer(0) 
	, m_Alloced(0)
	, m_Length(0)
	, m_BufferType(CmdNone)
{
	if (&obj != this) {
		Free();
		SetBuffer(obj.m_pBuffer,obj.m_Length,obj.m_BufferType);
	}
}

inline CCmdBuffer::CCmdBuffer(const char* str,CmdBufferType type)
	: m_pBuffer(0) 
	, m_Alloced(0)
	, m_Length(0)
	, m_BufferType(CmdNone)
{
	Append(str);
	SetBufferType(type);
}

inline CCmdBuffer::~CCmdBuffer() 
{ 
	Free(); 
}

inline char* CCmdBuffer::Detach() 
{ 
	char* TempBuffer = m_pBuffer; 
	m_pBuffer = 0; 
	m_Alloced = 0;
	m_Length = 0;
	m_BufferType = CmdNone;
	return TempBuffer; 
}

inline CCmdBuffer* CCmdBuffer::Detach(CCmdBuffer* obj)
{
	if (obj == this) {
		Free();
		return this;
	}

	if (obj) {
		obj->Free();
		obj->m_pBuffer = m_pBuffer;
		obj->m_Alloced = m_Alloced;
		obj->m_Length = m_Length;
		obj->m_BufferType = m_BufferType;
	}
	m_pBuffer = 0;
	m_Alloced = m_Length = 0;
	m_BufferType = CmdNone;
	return obj;
}

inline void CCmdBuffer::Attach(char* p, unsigned long cbSize) 
{ 
	Free(); 
	m_pBuffer = p ; 
	if (cbSize <= 0) {
		cbSize = lstrlenA(p);
	}
	m_Alloced = m_Length = cbSize;
}

inline void CCmdBuffer::Attach(CCmdBuffer* obj)
{
	if (obj == this)
		return ;

	Free();
	if (obj) {
		m_pBuffer = obj->m_pBuffer;
		m_Alloced = obj->m_Alloced ;
		m_Length = obj->m_Length ;
		m_BufferType = obj->m_BufferType ;
		obj->m_pBuffer = 0;
		obj->m_Alloced = 0;
		obj->m_Length = 0;
		obj->m_BufferType = CmdNone;
	}
	
}

inline CCmdBuffer::operator char*()		
{ 
	return m_pBuffer;
}

inline CCmdBuffer::operator const char*()	
{ 
	return m_pBuffer;
}

inline CCmdBuffer& CCmdBuffer::operator+(const CCmdBuffer& obj)
{
	Append(obj.m_pBuffer,obj.m_Length);
	return *this;
}

inline CCmdBuffer& CCmdBuffer::operator+(const char* pszBuffer)
{
	Append(pszBuffer);
	return *this;
}

inline CCmdBuffer& CCmdBuffer::operator+=(const CCmdBuffer& obj)
{
	Append(obj.m_pBuffer,obj.m_Length);
	return *this;
}

inline CCmdBuffer& CCmdBuffer::operator+=(const char* pszBuffer)
{
	Append(pszBuffer);
	return *this;
}

inline CCmdBuffer& CCmdBuffer::operator=(const CCmdBuffer& obj)
{
	SetBuffer(obj.m_pBuffer,obj.m_Length,obj.m_BufferType);
	return *this;
}

inline CCmdBuffer& CCmdBuffer::operator=(const char* pszBuffer)
{
	SetBuffer(pszBuffer);
	return *this;
}

inline const char* CCmdBuffer::GetBuffer()		
{ 
	return m_pBuffer;
}

inline char* CCmdBuffer::GetBufferPtr() 
{
	return m_pBuffer;
}

inline unsigned long CCmdBuffer::GetLength()		
{ 
	return m_Length; 
}

inline unsigned long CCmdBuffer::GetAllocSize()	
{ 
	return m_Alloced;
}

inline CmdBufferType CCmdBuffer::GetBufferType()
{
	return m_BufferType;
}
inline void CCmdBuffer::SetBufferType(CmdBufferType type)
{
	m_BufferType = type;
}

inline bool CCmdBuffer::IsNull() 
{ 
	return m_pBuffer == 0; 
}

inline void CCmdBuffer::Zero()
{
	if (m_pBuffer) {
		memset(m_pBuffer,0,m_Alloced);
	}
	m_Length = 0;
}

inline void CCmdBuffer::Empty()
{
	m_pBuffer = NULL; 
	m_Alloced = 0;
	m_Length = 0;
	m_BufferType = CmdNone;
}

inline char* CCmdBuffer::SetBuffer(CCmdBuffer& obj)
{
	if (&obj != this) {
		Free();
		SetBuffer(obj.GetBuffer(),obj.GetLength(),obj.m_BufferType);
	}
	return m_pBuffer;
}

inline char* CCmdBuffer::SetBuffer(const char* p, unsigned long size,CmdBufferType type)
{
	if (p && size) {
		int newSize = size + CMD_BUFF_SIZE;
		if (Alloc(newSize))
		{
			memcpy(m_pBuffer,p,size);
			m_Length = size;
			m_BufferType = type;
			m_pBuffer[m_Length] = 0x00;
		}
		else
		{
			Free();
		}
	}
	else {
		Free();
	}
	return m_pBuffer;
}

inline char* CCmdBuffer::SetBuffer(const char* pszText,CmdBufferType type)
{
	return (char*)SetBuffer(pszText,(unsigned long)lstrlenA(pszText),type);
}

inline char* CCmdBuffer::Append(const char* pBuffer, unsigned long cbSize)
{
	if (!pBuffer || !cbSize)
		return (char*)m_pBuffer;

	unsigned long NewLength = m_Length + cbSize;
	if (NewLength > m_Alloced) {
		NewLength = m_Length + cbSize + CMD_BUFF_SIZE;
		if (Alloc(NewLength)) {
			m_Alloced = NewLength;
		}
		else {
			m_Alloced = 0;
			m_Length = 0;
			return NULL;
		}
	}
	memcpy(&m_pBuffer[m_Length],pBuffer,cbSize);
	m_Length += cbSize;
	m_pBuffer[m_Length] = 0x00;
	return (char*)m_pBuffer;
}

inline char* CCmdBuffer::Append(const char* pBuffer)
{
	return (char*)Append((char*)pBuffer, lstrlenA(pBuffer) );
}

inline char* CCmdBuffer::AppendFormat(const char* pszFMT,...)
{
	char buffer[2048] = {0};
	va_list args;
	va_start(args,pszFMT);
	wvnsprintfA(buffer,sizeof(buffer)/sizeof(char)-1,pszFMT,args);
	va_end(args);
	return Append(buffer);
}

inline char* CCmdBuffer::ToBuffer(char** ppBuffer, unsigned long *pcbSize, CmdBufferType type)
{
	if (!ppBuffer || !pcbSize)
		return NULL;
	if (!m_pBuffer || m_Length <= 0)
		return NULL;

	if (m_BufferType == type) {
		CCmdBuffer Temp(*this);
		*ppBuffer = Temp.Detach();
		*pcbSize = m_Length;
	}
	else {
		switch (m_BufferType)
		{
		case CmdNone:
			{
				switch(type)
				{
				case CmdUTF8:
					{
						StringHelper::CUTF8Encoder _Coder((PBYTE)m_pBuffer,m_Length);
						*pcbSize = _Coder.Length ;
						*ppBuffer = _Coder.Detach();
					}
					break;
				case CmdBase64:
					{
						Base64::Base64Encode((PBYTE)m_pBuffer,m_Length,ppBuffer,BASE64_FLAG_NONE);
						*pcbSize = lstrlenA(*ppBuffer);
					}
					break;
				};
			}
			break;
		case CmdUTF8:
			{
				switch(type)
				{
				case CmdNone:
					{
						StringHelper::CUTF8Decoder _Coder(m_pBuffer);
						*ppBuffer = _Coder.Detach();
						*pcbSize = _Coder.Length ;
					}
					break;
				case CmdBase64:
					{
						StringHelper::CUTF8Decoder _Coder(m_pBuffer);
						Base64::Base64Encode((PBYTE)_Coder.m_pStr,_Coder.Length,ppBuffer,BASE64_FLAG_NONE);
						*pcbSize = lstrlenA(*ppBuffer);
					}
					break;
				};
			}
			break;
		case CmdBase64:
			{
				switch(type)
				{
				case CmdNone:
					{
						Base64::Base64Decode(m_pBuffer,(PBYTE*)ppBuffer,(int*)pcbSize);
					}
					break;
				case CmdUTF8:
					{
						CCmdBuffer TempBuffer;
						Base64::Base64Decode(m_pBuffer,(PBYTE*)&TempBuffer.m_pBuffer,(int*)&TempBuffer.m_Length);
						StringHelper::CUTF8Encoder _Coder((PBYTE)TempBuffer.m_pBuffer,(DWORD)TempBuffer.m_Length);
						*pcbSize = _Coder.Length;
						*ppBuffer = _Coder.Detach();
					}
					break;
				};
			}
			break;
		};
	}
	return *ppBuffer;
}

inline CCmdBuffer& CCmdBuffer::Encrypt(char* pszKey, CmdBufferType type)
{
	return *this;
}

inline CCmdBuffer& CCmdBuffer::Decrypt(char* pszKey, CmdBufferType type)
{
	return *this;
}

inline char* CCmdBuffer::ToBuffer(CCmdBuffer& obj,CmdBufferType type)
{
	char* temp = 0;
	unsigned long size = 0;
	ToBuffer(&temp,&size,type);
	obj.Attach(temp,size);
	obj.SetBufferType(type);
	return temp;
}

inline bool CCmdBuffer::Clone(CCmdBuffer* p)
{
	if (p) {
		p->Free();
		p->SetBuffer(m_pBuffer,m_Length);
		p->m_BufferType = m_BufferType;
		return !p->IsNull();
	}
	return false;
}

inline char* CCmdBuffer::Alloc(unsigned long size) 
{ 
	if (m_pBuffer) {
		m_pBuffer = (char*)realloc(m_pBuffer,size);
		m_Alloced = size;
		if (m_Length >= m_Alloced) {
			m_Length = m_Alloced - 1;
		}
	}
	else {
		m_pBuffer = (char*)calloc(size,1);
		m_Alloced = size;
		m_Length = 0; 
	}
	return m_pBuffer;
}

inline void CCmdBuffer::Free()	
{ 
	if (m_pBuffer) 
	{
 		free(m_pBuffer);
	}
	m_Alloced = 0;
	m_Length = 0;
	m_BufferType = CmdNone;
}

inline void CCmdTempBuffer::Attach(CCmdBuffer &obj) 
{ 
	Free(); size = obj.GetLength(); ptr = obj.Detach(); 
}
	

namespace Clipbord
{
	ATL_NOINLINE inline BOOL SaveClipbord(LPCSTR lpszText)
	{
        if (OpenClipboard(NULL))
        {
            EmptyClipboard();                             
            HGLOBAL hClip = GlobalAlloc(GMEM_MOVEABLE, lstrlenA(lpszText) + 1 ); 
            CHAR* pszBuffer =(CHAR*)GlobalLock(hClip);
			lstrcpyA(pszBuffer, lpszText);
            GlobalUnlock(hClip);
            SetClipboardData(CF_TEXT,hClip);
            CloseClipboard();
			return TRUE;
         }
		 return FALSE;
	}
}

namespace RES
{
	ATL_NOINLINE inline HRESULT BytesToStream(const BYTE* pBuffer,DWORD cbSize, IStream** ppStream)
	{
		if (pBuffer == NULL || cbSize <=0 )
			return E_INVALIDARG;
		
		HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE,cbSize);
		if (hGlobal == NULL)
			return E_OUTOFMEMORY;
		PBYTE pv = (PBYTE)::GlobalLock(hGlobal);
		if (pv == NULL)	{
			GlobalFree(hGlobal);
			return E_OUTOFMEMORY;
		}
		memcpy(pv,pBuffer,cbSize);
		::GlobalUnlock(hGlobal);

 		HRESULT hr = ::CreateStreamOnHGlobal(hGlobal,TRUE,ppStream);
		if (FAILED(hr)){
			GlobalFree(hGlobal);
		}
		return hr;
	}

	ATL_NOINLINE inline HRESULT GetResourceBuffer(UINT nResID, PBYTE* ppOutBuffer, DWORD* pcbSize, LPCTSTR pszType, HINSTANCE hInstance)
	{
 		HGLOBAL  hGlobal = NULL;
		HRSRC    hSource = NULL;
		LPVOID   pBuffer  = NULL;
 
		HRESULT  hr = E_OUTOFMEMORY;
		do {
			if (hInstance == NULL){
				hInstance = _pModule->GetResourceInstance();
				ATLASSERT(hInstance);
			}
			if (!(hSource = FindResource(hInstance, MAKEINTRESOURCE(nResID),pszType))) {
				hr = E_FAIL; break;
			}
  			if(!(hGlobal = LoadResource(hInstance, hSource))){    
 				break;
			}
 			if (!(pBuffer = LockResource(hGlobal))) {
				break;
			}
 			DWORD cbSize = (UINT)SizeofResource(hInstance, hSource);
			//hr = BytesToStream((PBYTE)pBuffer,cbSize,ppStream);
			if (pcbSize) {
				*pcbSize = cbSize;
			}
			if (ppOutBuffer) {
				memcpy(*ppOutBuffer, pBuffer, cbSize);
			}
			hr = S_OK;

		} while(false);
		
		if (pBuffer){
 			UnlockResource(hGlobal);
		}
		if (hGlobal) {
			FreeResource(hGlobal); 
		}
 		return hr;
	}

	ATL_NOINLINE inline HRESULT GetResourceStream(UINT nResID, IStream** ppStream, LPCTSTR pszType, HINSTANCE hInstance)
	{
 		HGLOBAL  hGlobal = NULL;
		HRSRC    hSource = NULL;
		LPVOID   pBuffer  = NULL;
 
		HRESULT  hr = E_OUTOFMEMORY;
		do {
			if (hInstance == NULL){
				hInstance = _pModule->GetResourceInstance();
				ATLASSERT(hInstance);
			}
			if (!(hSource = FindResource(hInstance, MAKEINTRESOURCE(nResID),pszType))) {
				hr = E_FAIL; break;
			}
  			if(!(hGlobal = LoadResource(hInstance, hSource))){    
 				break;
			}
 			if (!(pBuffer = LockResource(hGlobal))) {
				break;
			}
 			DWORD cbSize = (UINT)SizeofResource(hInstance, hSource);
			hr = RES::BytesToStream((PBYTE)pBuffer,cbSize,ppStream);

		} while(false);
		
		if (pBuffer){
 			UnlockResource(hGlobal);
		}
		if (hGlobal) {
			FreeResource(hGlobal); 
		}
 		return hr;
	}
}

namespace ProcHelper 
{
	ATL_NOINLINE inline BOOL GetAutoStart(LPCTSTR pszKey, LPCTSTR pszProc)
	{
 		BOOL fResult = FALSE;
 
  		ATL::CRegKey regKey;
		LPCTSTR StartupKey = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");
		if (ERROR_SUCCESS == regKey.Open(HKEY_CURRENT_USER,StartupKey))	{
			TCHAR szRegister[1024] = {0};
 			DWORD cbRead = 1024;
			if (ERROR_SUCCESS == regKey.QueryStringValue(pszKey,szRegister,&cbRead)  )
			{
				if (!pszProc) {
					fResult = TRUE; 
				}
				else {
					::StrTrim(szRegister,_T("\""));
					if (lstrcmpi(szRegister,pszProc) == 0) {
						fResult = TRUE;
					}
				}
			}
  		}
 		return fResult; 
	}

	ATL_NOINLINE inline BOOL SetAutoStart(LPCTSTR pszKey, LPCTSTR pszProc)
	{
		BOOL fResult = FALSE; 
 
		ATL::CRegKey regKey;
		if (ERROR_SUCCESS == regKey.Open(HKEY_CURRENT_USER,_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"))) {
			if (pszProc) {
				TCHAR sProcess[1024] = {0};
				wsprintf(sProcess, _T("\"%s\""), pszProc);
 				if (ERROR_SUCCESS == regKey.SetStringValue(pszKey,sProcess))
					fResult = TRUE;
			}
			else
 			{
				if (ERROR_SUCCESS == regKey.DeleteValue(pszKey))
					fResult = TRUE;
 			}
		}
  		return fResult ; 
	}

	ATL_NOINLINE inline BOOL IsAutoStart(LPCTSTR pszProc)
	{
		//ATLASSERT(FALSE); 
		return FALSE; 
	}

	ATL_NOINLINE inline BOOL EnableDebugPrivilege()   
	{   
		HANDLE hToken = NULL;   
		LUID sedebugnameValue;   
		TOKEN_PRIVILEGES tkp;   
		
		BOOL fResult = FALSE;
		do {
			if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
				break; 
			if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue))   
				break; 

			tkp.PrivilegeCount = 1;   
			tkp.Privileges[0].Luid = sedebugnameValue;   
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;   
			if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))  
				break;

			fResult = TRUE; 
		} while(false);
		
		if (!fResult && hToken) {
			CloseHandle(hToken);
		}
		return fResult;   
	}
}; 


inline CGlobalMemory::CGlobalMemory()
	: m_h(NULL)
	, m_size(0)
{
}

inline CGlobalMemory::CGlobalMemory(size_t size)
	: m_h(NULL)
	, m_size(0)
{
	Alloc(size); 
}

inline CGlobalMemory::CGlobalMemory(IStream* pStream)
	: m_h(NULL)
	, m_size(0)
{
	FromStream(pStream); 
}


inline HRESULT CGlobalMemory::Alloc(const size_t size, UINT uFlags)
{
	ATLASSUME(m_h == NULL); 
	ATLASSUME(size > 0); 

	m_h = GlobalAlloc(uFlags, size); 
	if (m_h == NULL) {
		return HRESULT_FROM_WIN32(GetLastError()); 
	}
	m_size = size; 
	return S_OK;
}

inline HRESULT CGlobalMemory::Free()
{
	if (m_h) {
		GlobalUnlock(m_h); 
		GlobalFree(m_h); 
	}
	m_h = NULL; 
	m_size = 0; 
	return S_OK;
}

inline HRESULT CGlobalMemory::ReAlloc(const size_t size)
{
	if (!m_h) {
		return Alloc(size); 
	}
	if (!size) {
		Free(); 
		return S_OK; 
	}
	m_h = ::GlobalReAlloc(m_h, size, 0);
	if (!m_h) {
		m_size = 0; 
		return HRESULT_FROM_WIN32(GetLastError()); 
	}
	m_size = size; 
	return S_OK; 
}

inline HRESULT CGlobalMemory::ToStream(IStream** ppStream, BOOL fDeleteOnRelease)
{
	ATLASSUME(m_h != NULL); 
	if (m_h) {
		return ::CreateStreamOnHGlobal(m_h, fDeleteOnRelease, ppStream);
	}
	return E_POINTER;
}

inline HRESULT CGlobalMemory::FromStream(IStream* pStream)
{
	ATLASSUME(m_h == NULL); 
	ATLASSUME(pStream != NULL); 

	HGLOBAL h = 0; 
	HRESULT hr = ::GetHGlobalFromStream(pStream, &h); 
	if (SUCCEEDED(hr)) {
		m_h = h; 
		m_size = ::GlobalSize(m_h); 
	}
	return S_OK;
}

inline void* CGlobalMemory::Lock()
{
	ATLASSUME(m_h != NULL); 
	if (m_h) {
		return GlobalLock(m_h); 
	}
	return NULL;
}

inline void CGlobalMemory::Unlock()
{
	if (m_h) {
		::GlobalUnlock(m_h);
	}
}

inline size_t CGlobalMemory::Write(const unsigned char* buffer, const size_t size)
{
	ATLASSUME(buffer != NULL); 
	ATLASSUME(size != 0);
	ATLASSUME(m_h != NULL); 
	ATLASSERT(size <= m_size); 

	size_t sz = size <= m_size ? size : m_size; 
	void* ptr = Lock(); 

	ATLASSERT(ptr); 

	if (ptr) {
		__try {
			memcpy(ptr, buffer, sz); 
		}
		__except (::GetExceptionCode()) {
			ATLASSERT(FALSE);
			sz = 0; 
		}
		Unlock(); 
	}
	else {
		sz = 0; 
	}
	return sz;
}

inline size_t CGlobalMemory::Write(const unsigned int pos, const unsigned char* buffer, const size_t size)
{
	ATLASSUME(m_h != NULL); 
	ATLASSUME(buffer != NULL && size != 0 && pos < m_size); 

	if (!buffer || !size || pos > m_size) {
		return 0; 
	}
		
	size_t length = m_size - pos; 
	size_t sz = size > length ? length : size; 
	unsigned char* ptr = (unsigned char*)Lock(); 

	ATLASSERT(ptr); 
	if (ptr) {
		__try {
			unsigned char* dest = ptr + pos; 
			memcpy(dest, buffer, sz); 
		}
		__except (::GetExceptionCode()) {
			ATLASSERT(FALSE); 
			sz = 0; 
		}
	}
	else {
		sz = 0; 
	}
	return sz;
}

inline size_t CGlobalMemory::Read(unsigned char* buffer, const size_t size)
{
	ATLASSUME(m_h != NULL);
	ATLASSUME(buffer != NULL && size != 0); 
	ATLASSERT(size <= m_size); 

	size_t sz = size > m_size ? m_size : size; 
	void* ptr = Lock(); 

	if (ptr) {
		__try {
			memcpy(buffer, ptr, sz); 
		}
		__except (::GetExceptionCode()) {
			ATLASSERT(FALSE); 
			sz = 0; 
		}
	}
	else {
		sz = 0; 
	}
	return sz;
}

inline size_t CGlobalMemory::Read(const unsigned int pos, unsigned char* buffer, const size_t size)
{
	ATLASSUME(m_h != NULL);
	ATLASSUME(buffer != NULL && size != 0 && pos < m_size);

	size_t length = m_size - pos;
	size_t sz = size <= length ? size : length; 
	unsigned char* ptr = (unsigned char*)Lock();

	ATLASSERT(ptr);
	if (ptr) {
		__try {
			unsigned char* src = ptr + pos;
			memcpy(buffer, src, sz);
		}
		__except (::GetExceptionCode()) {
			ATLASSERT(FALSE);
			sz = 0;
		}
	}
	else {
		sz = 0;
	}
	return sz;
}

namespace SYNC {
	inline BOOL ExistEvent(LPCSTR lpszEvent, HANDLE* phEvent)
	{
		HANDLE hEvent = ::CreateEventA(NULL, FALSE, FALSE, lpszEvent); 
		if (phEvent) {
			*phEvent = hEvent; 
		}
		return (GetLastError() == ERROR_ALREADY_EXISTS) ? TRUE : FALSE; 
	}
}

#endif

