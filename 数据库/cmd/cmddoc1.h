#ifndef __CMD_DOCUMENT__
#define __CMD_DOCUMENT__
#pragma warning( disable : 4018)
#pragma warning( disable : 4267)
#pragma warning( disable : 4819)
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#else
#if _WIN32_IE < 0x0600
#undef _WIN32_IE
#define _WIN32_IE 0x0600
#endif
#endif

#include <atlbase.h>
#include <atlstr.h>
#include <atlcomtime.h>
#include <string>
#include "Json/Json.h"
#include "CmdErr.h"
#include "CmdEnc.h"
#include "Log/Dbgprint.h"
#include "shlwapi.h"
#pragma comment(lib,"shlwapi.lib")

#define CMD_EMPTY_OBJECT		-1
#define CMD_PARAMETER_OBJECT	0
#define CMD_PARAMETERS_OBJECT	1
#define CMD_COMMAND_OBJECT		2
#define CMD_SERVICE_OBJECT		3
#define CMD_PROJECT_OBJECT		4

#define JSON_STYLE				0x0100
#define JSON_FORMATED			JSON_STYLE
#define JSON_SOURCE				0x0200
#define JSON_DOCUMENT			0x0400

#define JSON_DEFAULT			0
#define JSON_REQUEST_SRC		JSON_SOURCE
#define JSON_REQUEST_DOC		JSON_DOCUMENT
#define JSON_RESPONE_SRC		JSON_SOURCE
#define	JSON_RESPONE_DOC		JSON_DOCUMENT

#define PARAM_FLAGS_MUSTBE		0x0001		// 必须包含的参数值
#define PARAM_FLAGS_CODER		0x0002		// 必须进行编码

#define CMD_FLAGS_BINDMAC		0x0001		// 采用本地MAC地址做为私钥加密
#define CMD_FLAGS_SIGNMD5		0x0002		// 命令MD5签名，默认采用CRC32进行签名
#define CMD_FLAGS_BASE64		0x0004		// BASE64编码模式，默认采用HEX进行编码
#define CMD_FLAGS_NOTENC		0x0008		// 发送数据不进行加密
#define CMD_FLAGS_NOTSIGN		0x0010		// 不对实体数据进行签名处理
#define CMD_FLAGS_NOTCODE		0x0020		// 不对实体数据进行编码处理
#define CMD_FLAGS_UTF8			0x0040		// 对实体数据进行UTF8编码
#define CMD_FLAGS_RANDXOR		CMD_FLAGS_BINDMAC	// 对数据信息RANDXOR模式加密

//#define CMD_FLAGS_EX_SIGNCRC32	0x0040
//#define CMD_FLAGS_EX_HEX		0x0080
//#define CMD_FLAGS_EX_UTF8		0x0100
//
//#define CMD_FLAGS_EX_XOR		0x0200
//#define CMD_FLAGS_EX_RANDXOR	CMD_FLAGS_BINDMAC


#define DECLARE_OBJECT_TYPE(x) int _InternalObjectType() const { return x; }

#define TYPE_STR				0
#define TYPE_INT				1
#define TYPE_DATE				2
#define TYPE_JSON				3

namespace JsonHelper
{
	std::string JsonToString(Json::Value &value,bool styled = false);
	bool ReadJsonFile(LPCTSTR lpszFile, Json::Value& Root);
	bool WriteJsonFile(LPCTSTR lpszFile,Json::Value &value);
};

namespace CMD
{
	class CParameter;
	class CParameters;
	class CCommand;
	class CDocument;
	class CRequestDocument;
	class CResponeDocument;
	 
	template <typename T>
	class CDWORDFlagsImpl {
	public:
		CDWORDFlagsImpl(DWORD nFlags = 0) : _Flags(nFlags) {}
		CDWORDFlagsImpl(const CDWORDFlagsImpl<T>& obj) { _Flags = obj._Flags; }
	public:
		DWORD _Get() const				{ return _Flags; }
		DWORD _Set(DWORD cbFlags)		{_Flags = cbFlags; return _Flags; }
		DWORD _Add(DWORD cbFlags)		{ _Flags |= cbFlags; return _Flags; }
		DWORD _Remove(DWORD cbFlags)	{ _Flags &= cbFlags; return _Flags; }
		DWORD _Modify(DWORD cbAdd, DWORD cbRemove = 0);
		DWORD _Modify(bool fAdd, DWORD cbFlags);
	public:
		operator DWORD()				{ return _Flags; }
 	public:
		bool _Have(DWORD cbFlags)const	{ return (_Flags & cbFlags) == cbFlags;	}
		bool _Not(DWORD cbFlags) const 	{ return !_Have(cbFlags); }
		bool IsEmpty()const				{ return _Flags == 0; }
		void Empty()					{ _Flags = 0; }
	protected:
		DWORD _Flags;
	};

	class CParameterFlags : public CDWORDFlagsImpl<CParameterFlags> {
	public:
		CParameterFlags(DWORD cbFlags = 0) : CDWORDFlagsImpl(cbFlags) {}
	public:
		bool IsCoder() const { return _Have(PARAM_FLAGS_CODER); }
		bool IsMust() const { return _Have(PARAM_FLAGS_MUSTBE); }
		void SetCoder(bool rhs) { _Modify(rhs,PARAM_FLAGS_CODER); }
		void SetMust(bool rhs) { _Modify(rhs,PARAM_FLAGS_MUSTBE); }
	};

	class CCmdFlags : public CDWORDFlagsImpl<CCmdFlags>{
	public:
		CCmdFlags(DWORD cbFlags = 0) : CDWORDFlagsImpl(cbFlags) {}
	public:
		bool IsBindMac() const 		{ return _Have(CMD_FLAGS_BINDMAC); }
		bool IsSign() const			{ return !_Have(CMD_FLAGS_NOTSIGN); }
		bool IsSignMD5() const 		{ return _Have(CMD_FLAGS_SIGNMD5); }
		bool IsSignCRC32() const	{ return IsSign() && !IsSignMD5(); }

		bool IsCoder() const		{ return !_Have(CMD_FLAGS_NOTCODE); }
		bool IsBase64Coder() const	{ return _Have(CMD_FLAGS_BASE64); }
		bool IsUTF8Coder() const	{ return _Have(CMD_FLAGS_UTF8); }
		bool IsHEXCoder() const		{ return IsCoder() && !IsBase64Coder() && !IsUTF8Coder();}

		bool IsEncrypt() const 		{ return _Not(CMD_FLAGS_NOTENC); }	
		bool IsEncryptRandXOR() const { return _Have(CMD_FLAGS_RANDXOR); }
		bool IsEncryptXOR() const	{ return IsEncrypt() && !IsEncryptRandXOR(); }

		void SetBindMac(bool rhs) { _Modify(rhs, CMD_FLAGS_BINDMAC); }
		void SetEncryptRandXOR(bool rhs) { _Modify(rhs, CMD_FLAGS_RANDXOR); }
		void SetSignMD5(bool rhs) { _Modify(rhs, CMD_FLAGS_SIGNMD5); }

		void SetBase64Coder(bool rhs){ _Modify(rhs, CMD_FLAGS_BASE64); }
		void SetUTF8Coder(bool rhs)  { _Modify(rhs, CMD_FLAGS_UTF8); }

		void SetEncrypt(bool rhs) { _Modify(!rhs, CMD_FLAGS_NOTENC); }
		void SetCoder(bool rhs) { _Modify(!rhs, CMD_FLAGS_NOTCODE); } 
		void SetSign(bool rhs) { _Modify(!rhs, CMD_FLAGS_NOTSIGN); } 
		
		void SetSignCRC32(bool rhs) {
			if (rhs) _Modify((DWORD)0, CMD_FLAGS_NOTSIGN | CMD_FLAGS_SIGNMD5); 
			else _Modify((DWORD)CMD_FLAGS_NOTSIGN , (DWORD)0); 
		}
		void SetEncryptXOR(bool rhs) {
			if (rhs) _Modify((DWORD)0, CMD_FLAGS_NOTENC | CMD_FLAGS_RANDXOR); 
			else _Modify((DWORD)CMD_FLAGS_NOTENC , (DWORD) 0); 
		}
		void SetHEXCoder(bool rhs) {
			if (rhs) _Modify((DWORD)0, CMD_FLAGS_NOTCODE | CMD_FLAGS_BASE64 | CMD_FLAGS_UTF8); 
			else  _Modify((DWORD)CMD_FLAGS_NOTCODE, (DWORD)0); 
		}
	};

	class CDocFlags : public CDWORDFlagsImpl<CDocFlags> {
	public:
		CDocFlags(DWORD cbFlags = 0) : CDWORDFlagsImpl(cbFlags) {}
	public:
		bool IsStyled() const	{ return _Have(JSON_STYLE); }
		bool IsDocument() const { return _Have(JSON_DOCUMENT); }
		bool IsSource() const   { return _Have(JSON_SOURCE); }
		void SetStyled(bool rhs)  { _Modify(rhs,JSON_STYLE); }
		void SetSource(bool rhs)  { _Modify(rhs,JSON_SOURCE); }
		void SetDocument(bool rhs){ _Modify(rhs,JSON_DOCUMENT); }
	};

	template <size_t PSIZE = 8192>
	class CCmdTempBuffer {
	public:
		CCmdTempBuffer() ;
		~CCmdTempBuffer();
	public:
		char* Detach();
		void Free();
		char* Append(void* buffer, size_t nSize);
	public:
		size_t Alloced() const { return m_alloced; }
		size_t Postion() const { return m_postion; }
		operator const char*() { return (char*)m_ptr; }
		operator char*() { return (char*)m_ptr; }
	protected:
		char* m_ptr;
		size_t m_alloced,m_postion;
	};

	__interface ICmdStream {
		void GetJson(CStringA& jsonString, int nFlags = 0);
		void SetJson(Json::Value &value);
		int ObjectType();
	};

	template <typename T>
	class CStreamHelperImpl : public ICmdStream
	{
	public:
		void GetJson(CStringA &JsonString, int nFlags = 0);
		void GetJson(Json::Value &Value, int nFlags = 0);
		void SetJson(Json::Value &value);
	public:
		int ObjectType();
		void OnGetJson(Json::Value &Value, int nFlags);
		void OnSetJson(Json::Value &value);
	public:
 		CStringA Description;
	};

	class CParameter : public CStreamHelperImpl<CParameter>
	{
	public:
		DECLARE_OBJECT_TYPE(CMD_PARAMETER_OBJECT)
	public:
		CParameter() : Type(0),Tag(NULL) {}
	public:
		CStringA Name;
		CStringA Value;
		int		 Type;
		CParameterFlags	 Flags;
		DWORD_PTR Tag;
	public:
		CStringA GetValue()			{ return Value;	}
		int GetIntValue()			{ return StrToIntA((LPCSTR)Value); }
		COleDateTime GetDateValue()	{ return COleDateTime(CComVariant(Value)); }
	public:
		void OnGetJson(Json::Value &_Value,int _Flags);
		void OnSetJson(Json::Value &value);
	};

	class CParameters : public CSimpleArray<CParameter*>
		, public CStreamHelperImpl<CParameters>
	{
	public:
		DECLARE_OBJECT_TYPE(CMD_PARAMETERS_OBJECT)
	public:
		~CParameters();
	public:
		int FindIndex(CString sKey);
		void Destroy();
	public:
		CParameter* FindParameter(CString sKey);
 		CParameter* AddParameter(CString sName, CString sValue, INT nType = 0, DWORD cbFlags = 0);
		CParameter* AddParameter(CString sName, int nValue);
		CParameter* AddParameter(CString sName, COleDateTime oTime);
		CParameter* AddParameter(CString sName,Json::Value &value);
		CParameter* AddParameter(Json::Value &value);
		CParameter* NewParameter();
	};
	 
	class CCommand : public CStreamHelperImpl<CCommand>	{
	public:
		DECLARE_OBJECT_TYPE(CMD_COMMAND_OBJECT)
	public:
		CCommand() : Flags(0) {}
	public:
		void OnGetJson(Json::Value &_Value, int nFlags);
		void OnSetJson(Json::Value &value);
		void Destory();
	public:
		CString     Command;
		CCmdFlags	Flags;
		CParameters Parameters;
		CParameters ResultParameters;
	};

	template <typename T>
	class CDocumentImpl {
	public:
		CDocumentImpl(DWORD nFlags = 0) : _Succeeded(true), _ErrorCode(0), Flags(nFlags) {}
	public:
		Json::Value& operator[](LPCSTR lpszKey)		{ return Document[lpszKey];}
		Json::Value& GetJson()						{ return Document;}

		CStringA GetDocument(LONG nHashCode = Sign::MAC_Hash(),int nFlags = JSON_DOCUMENT | JSON_STYLE)	;
		Json::Value& SetDocument(const BSTR bstrDoc, long nHashCode = Sign::MAC_Hash() );
		Json::Value& SetDocument(LPCSTR lpszDocument, long nHashCode = Sign::MAC_Hash() );
	public:
		bool OnReadDocument(const Json::Value &Cmd)	{ return true;}
 		bool OnWriteDocument(Json::Value &Root)		{ return true;}
	public:
		void Destroy();
		bool IsSucceeded() const					{ return _Succeeded; }
		int ErrorCode()								{ return _ErrorCode; }
 		void SetLastError(int nCode, LPCSTR lpszError = "");
		void SetCommand(LPCSTR lpszCmd);
		CStringA GetCommand()						{ return Command;	}
		void SetSession(LPCSTR lpszSession)			{ SessionId = lpszSession; }
		CStringA GetSession()						{ return SessionId; }
	public:
		bool HaveParameter(LPCSTR lpszParameter) ;
		bool HaveParameters(LPCSTR lpszParameters[]);
		void SetParameter(LPCSTR lpszKey, LPCSTR lpszValue);
		void SetParameter(LPCSTR lpszKey, INT nValue);
	public:
		Json::Value Document;
	public:
		CCmdFlags	Flags;
		CStringA	Description;
		CStringA	Command;
		CStringA	SessionId;
	protected:
		bool		_Succeeded;
		int			_ErrorCode;
	};

	/*
		类名称： CCommandDocument
		类功能:  客户端发送请求命令数据包
	*/
	class CCommandDocument : public CDocumentImpl<CCommandDocument> {
	public:
		CCommandDocument(LPCSTR lpszCmd = NULL, DWORD nFlags = CMD_FLAGS_BINDMAC);
		CCommandDocument(LPCSTR lpszCmd, const Json::Value& _Parameters, DWORD nFlags = CMD_FLAGS_BINDMAC);
		CCommandDocument(LPCSTR lpszCmd, LPCSTR lpszJson, DWORD nFlags = CMD_FLAGS_BINDMAC);
		CCommandDocument(CCommand& Cmd);
	public:
		bool OnWriteDocument(Json::Value &Root);
	};
 	/*
		类名称: CRequestDocument
		类功能: WEB端调用接收并解析客户端命令请求
	*/
	class CRequestDocument : public CDocumentImpl<CRequestDocument> {
	public:
		CRequestDocument(LPCSTR lpszDocument = NULL, LONG nHashCode = 0) ;
		CRequestDocument(BSTR bstrDocument, LONG nHashCode = 0) ;
	};

	class CResponeDocument : public CDocumentImpl<CResponeDocument> {
	public:
		CResponeDocument();

		CResponeDocument(CRequestDocument& Doc) ;
		CResponeDocument(const CCommandDocument& Doc);
		CResponeDocument(LPCSTR lpszCmd, int ErrorCode, LPCSTR lpszError = "") ;
		CResponeDocument(LPCSTR lpszDocument);
	public:
		bool Succeeded() { return _ErrorCode == 0; }
		void SetRequestDocument(CRequestDocument &Doc);
		void SetResponeError(LPCSTR lpszCmd, int nErrorCode = 0, LPCSTR lpszError = "") ;
	public:
		bool OnReadDocument(const Json::Value &Cmd)	;
		bool OnWriteDocument(Json::Value &Root)	;
	};
}; // end of namespace CMD;

#include "CmdDoc1.inl"
#endif