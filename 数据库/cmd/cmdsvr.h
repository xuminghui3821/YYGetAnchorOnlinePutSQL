#ifndef __CMD_REMOTE_SERVICE__
#define __CMD_REMOTE_SERVICE__
#pragma once
#include "CmdDoc.h"
#include "CmdMySQL.h"
using namespace CMD;

//////////////////////////////////////////////////////////////////////////////////////////////////////
// 命令访问权限控制
// 
#define CMD_INIT				0x00010000		// 必须初始化连接，进行程序/MAC认证后才能访问
#define CMD_SESSION				CMD_INIT		// 同CMD_INIT，必须创建SESSION会话后才能进行访问
#define CMD_NODBCONN			0x00020000		// 无须验证数据库连接信息， 默认必须验证数据库连接
#define CMD_MACHINE				0x00040000		// 需要验证机器码
#define CMD_PRODUCT				0x00080000		// 需要针对客户端访问产品进行认证
#define CMD_RESET				0x00100000		// 需要重新初始化认证信息
#define CMD_LOGON				0x10000000		// 用户必须登录后才能访问
#define CMD_ADMIN				0x20000000		// 用户必须登录且具有管理员权限才能访问
#define CMD_SUPER				0x40000000		// 用户必须登录且具有超级管理员权限才能访问

#define CMD_CERT				CMD_MACHINE | CMD_PRODUCT
#define CMD_CERT_USER			CMD_CERT | CMD_LOGON
#define CMD_CERT_ADMIN			CMD_CERT_USERT | CMD_ADMIN
#define CMD_CERT_SUPER			CMD_CERT | CMD_SUPER
//////////////////////////////////////////////////////////////////////////////////////////////////////
// 状态控制
//
#define CMD_STATE_SUCCESS		1				// 正常状态
#define CMD_STATE_DEFAULT		0				// 初始状态，正确或需要进行确认
#define CMD_STATE_DISABLE		-1				// 禁用状态
#define CMD_STATE_EXPIRE		-2				// 过期状态
#define CMD_STATE_FAIL			-3				// 错误状态
//
#define IsStateSucceeded(state)	(state > 0)
#define IsStateDefault(stat)	(state == 0)
#define IsStateFailed(state)	(state < 0)
//////////////////////////////////////////////////////////////////////////////////////////////////////
// 用户权限定义
//
#define CMD_USER_SUPER			2				// 超级管理员权限
#define CMD_USER_ADMIN			1				// 管理员权限
#define CMD_USER_NORMAL			0				// 普通用户
//////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
template <typename TBase>
struct ServiceMapEntry
{
	LPCSTR	Cmd;
	DWORD	Flags;
	HTTP_CODE (TBase::*pfnInvoke)(CRequestDocument& Resust, CResponeDocument& Respone, DWORD cbFlags);
};
//////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
#define BEGIN_SERVICE_CLASS_MAP(__class) ServiceMapEntry<__class>* GetClassifiedEntry() { \
 	typedef __class __className; \
	typedef ServiceMapEntry<__className> _MapEntry; \
	static _MapEntry _ClassMaps[] = {

#define SERVICE_CLASS_ENTRY(_class, mothed) \
		{ _class, 0, &__className::mothed },

#define SERVICE_CLASS_ENTRY_EX(_class,flags, mothed) \
		{ _class, flags, &__className::mothed },

#define END_SERVICE_CLASS_MAP() \
		{NULL, 0, NULL}\
	}; \
	return _ClassMaps; \
}

#define DECLARE_EMPTY_SERVICE_CLASS_MAP(__class) \
	BEGIN_SERVICE_CLASS_MAP(__class) \
	END_SERVICE_CLASS_MAP()

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
#define BEGIN_SERVICE_COMMAND_MAP(__classname,__func) \
DWORD __func(CRequestDocument& Request, CResponeDocument& Respone, DWORD cbFlags) \
{ \
 	typedef __classname __className; \
	typedef ServiceMapEntry<__className> _MapEntry; \
	static _MapEntry _ServiceMaps[] = {

#define SERVICE_COMMAND_HANDLER(command, mothed) \
		{ command, 0, &__className::mothed },

#define SERVICE_COMMAND_HANDLER_EX(command, flags, mothed) \
		{ command,flags,&__className::mothed },

#define END_SERVICE_COMMAND_MAP() \
		{ NULL, 0, NULL } \
	};\
 	_MapEntry *pMap = NULL; \
	for (_MapEntry* p = _ServiceMaps; p->Cmd ; p++ ) \
	{  \
		CStringA sCmd = p->Cmd; sCmd.Trim().MakeLower(); \
		if (Request.Command == p->Cmd) \
		{ \
			pMap = p; \
			break; \
		}\
	} \
	if (pMap) \
	{ \
		if (!OnValidateRequestCommand(Request,Respone,pMap->Flags)) \
		{ \
			return Report(Respone,JSON_SOURCE | JSON_STYLE); \
		} \
		(this->* pMap->pfnInvoke)(Request,Respone,pMap->Flags); \
	} \
	else \
	{ \
		Respone.SetResponeError(Request.GetCommand(),CMD_ERROR_COMMAND,"不支持的命令服务");\
		return Report(Respone, JSON_SOURCE | JSON_STYLE); \
	} \
	m_HttpResponse << Respone.GetDocument(GetMachineCode(JSON_DOCUMENT),JSON_DOCUMENT); \
	return HTTP_SUCCESS; \
} 
//////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CServiceFlags : public CDWORDFlagsImpl<CServiceFlags> {
public:
	CServiceFlags(DWORD cbFlags = 0) : CDWORDFlagsImpl(cbFlags) {}
public:
	bool IsInited() const { return _Have(CMD_INIT); }
	bool IsMachine() const { return _Have(CMD_MACHINE); }
	bool IsProduct() const { return _Have(CMD_PRODUCT); }
	bool IsLogon() const { return _Have(CMD_LOGON) || _Have(CMD_ADMIN) || _Have(CMD_SUPER); }
	bool IsAdmin() const { return _Have(CMD_ADMIN); }
	bool IsSuper() const { return _Have(CMD_SUPER); }
	bool IsDBConnect() const { return !_Have(CMD_NODBCONN); }
};

//extern 	CMySQLConnect MySQL;

template <typename T>
class CCmdHandlerImpl : public CRequestHandlerT< T >
{
protected:
	CComPtr<ISessionStateService> m_spSessionSvc;	// 会话服务支持
 	CComPtr<ISession> m_spSession;
	CComPtr<IMemoryCache> m_spBlobCache;			// Blob 缓存支持
	CComPtr<IDBConnectPool> m_spConnectPool;
protected:
	CStringA m_sessionId;
public:
	HTTP_CODE ValidateAndExchange()
	{
		CResponeDocument Respone;
		m_HttpResponse.SetContentType("text/html"); 		// 设置内容类型

		// 从 ISAPI 扩展中获取 ISessionStateService
		if (FAILED(m_spServiceProvider->QueryService(__uuidof(ISessionStateService), &m_spSessionSvc))) {
			return Report(Respone,"ServiceError", CMD_ERROR_SERVICE, "无法获取会话服务");
 		}
		// 从 ISAPI 扩展中获取 IMemoryCache 服务
		if (FAILED(m_spServiceProvider->QueryService(__uuidof(IMemoryCache), &m_spBlobCache))) {
			return Report(Respone,"ServiceError", CMD_ERROR_SERVICE, "无法获取缓存服务");
		}
		if (FAILED(m_spServiceProvider->QueryService(__uuidof(IDBConnectPool), &m_spConnectPool))) {
			return Report(Respone,"ServiceError", CMD_ERROR_SERVICE, "无法获取数据库连接池服务");
		}
	
		T* pT = static_cast<T*>(this);

		// 进行服务初始化设置
		if (!pT->OnInitServiceHandler(Respone)) {
			return Report(Respone, JSON_SOURCE | JSON_STYLE);
		}

		// 获取参数信息
		CStringA sParam = m_HttpRequest.GetQueryString();
		sParam.Trim().MakeLower();

		OutputDebugStringA((LPCSTR)sParam); 
		if (sParam == "" || sParam == "version") {
			return pT->OnVersion(Respone);
		}

		// 查找定义的服务分类，并验证服务分类是否正确
		typedef ServiceMapEntry<T> _MapEntry;
		_MapEntry* pMap = NULL;
		_MapEntry* pEntry = pT->GetClassifiedEntry();
		for (_MapEntry* p = pEntry ; p->Cmd && p->pfnInvoke ; p++) {
			CStringA sCmd = p->Cmd ;
			sCmd.Trim().MakeLower();
			if (sCmd == sParam) {
				pMap = p;
				break;
			}
		}
		if (!pMap) {
			return Report(Respone,"ServiceError", CMD_ERROR_SERVICE,"无效命令分类",JSON_SOURCE | JSON_STYLE);
		}
		
		// 服务分类查找完成，获取客户端投递的JSON数据
		CStringA sJson;
		if (!GetRequestJson(sJson,Respone)) {
			return Report(Respone, JSON_SOURCE | JSON_STYLE);
		}

		// 创建会话设置信息
		if (S_OK != RetrieveOrCreateSession() ) {
			return Report(Respone, "ServiceError", CMD_ERROR_SESSION, "无法创建或获取会话信息", JSON_SOURCE | JSON_STYLE);
		}

		// 解析客户端投递数据，如果解析成功则跳转到分类的服务功能接口处理函数
		CRequestDocument Request(sJson, pT->GetMachineCode(JSON_DOCUMENT));
		Respone.SetRequestDocument(Request);
		if (!Request.IsSucceeded()) {
			if (Respone.Command == "") {
				Respone.SetCommand("ClientError");
			}
			return Report(Respone,JSON_SOURCE | JSON_STYLE);
		}
		return (HTTP_CODE)(pT->* pMap->pfnInvoke)(Request,Respone,pMap->Flags);
	}

public:
	HTTP_CODE Report(CResponeDocument &Respone, DWORD cbFlags = 0,bool fWrite = true)
	{
 		CDocFlags Flags(cbFlags);
		if (Flags.IsSource()) {
			Respone.Flags.SetEncrypt(false);
		}
		if (fWrite) {
			m_HttpResponse << Respone.GetDocument(static_cast<T*>(this)->GetMachineCode(cbFlags),cbFlags);
		}
		return HTTP_SUCCESS;
 	}
	
	HTTP_CODE Report(CResponeDocument &Respone, int nCode, LPCSTR lpszError, DWORD cbFlags = 0,bool fWrite = true)
	{
		Respone.SetLastError(nCode,lpszError);
		return Report(Respone,cbFlags,fWrite);
	}

	HTTP_CODE Report(CResponeDocument &Respone, LPCSTR lpszCmd, int nCode, LPCSTR lpszError, DWORD cbFlags = 0,bool fWrite = true)
	{
		Respone.SetResponeError(lpszCmd,nCode,lpszError);
		return Report(Respone,cbFlags,fWrite);
	}
 	
	BOOL GetRequestJson(CStringA &sJson, CResponeDocument& Respone)	
	{
		CComPtr<IHttpServerContext> spContext;
		if (FAILED(m_HttpRequest.GetServerContext(&spContext))) {  
			Respone.SetResponeError("ServiceError",CMD_ERROR_SERVICE,"无法获取IHttpServerContext对象");  
			return FALSE; 
		} 
 		DWORD dwBytesTotal = m_HttpRequest.GetTotalBytes(); 
		if ((dwBytesTotal+1 < dwBytesTotal) || (dwBytesTotal < 0)) {  
			Respone.SetResponeError("ServerError",CMD_ERROR_SERVICE,"缓存错误或无效的数据源");  
			return FALSE; 
		}  
 		DWORD cbRead = 0; 
		LPSTR pBuffer = sJson.GetBufferSetLength(dwBytesTotal+1); 
		BOOL fResult = ReadClientData(spContext,pBuffer,&dwBytesTotal,0); 
		if (fResult) {
			pBuffer[dwBytesTotal] = 0x00; 
		}  
		sJson.ReleaseBuffer();  
		return fResult;  
	}
public:
	HRESULT RetrieveOrCreateSession()
	{
		return RetrieveOrCreateSession(m_sessionId,&m_spSession);
	}

	HRESULT RetrieveSession(CStringA& sessionId, ISession** ppSession)
	{
		sessionId.Trim();
		if (sessionId.GetLength() == 0)
		{
			m_HttpRequest.GetSessionCookie().GetValue( sessionId );
			if (sessionId.GetLength() == 0)
				return E_FAIL;
		}
		HRESULT hr ;
		CComPtr<ISession> pSession;
		if (SUCCEEDED(hr = m_spSessionSvc->GetSession(sessionId, &pSession))) {
			hr = pSession.CopyTo(ppSession);
		}
		return hr;
	}

	HRESULT CreateSession(CStringA& sessionId, ISession** ppSession)
	{
		HRESULT hr = E_FAIL;
		sessionId.Trim();
		if (sessionId.GetLength() == 0)		{
			m_HttpRequest.GetSessionCookie().GetValue( sessionId );
		}
		
		CComPtr<ISession> pSession;
		sessionId.Trim();
		if (sessionId.GetLength() == 0)		{
			const size_t nCharacters = 64;
			CHAR szID[ nCharacters + 1] = {0};
			DWORD dwCharacters = nCharacters;
			hr = m_spSessionSvc->CreateNewSession(szID, &dwCharacters, &pSession);
			if (SUCCEEDED(hr) && pSession) {
				sessionId = szID;
			}
		}
		else {
			hr = m_spSessionSvc->CreateNewSessionByName(sessionId.GetBuffer(),&pSession);
			sessionId.ReleaseBuffer();
		}
		if (SUCCEEDED(hr) && pSession) {
			hr = pSession.CopyTo(ppSession);
		}
		return hr;
	}

	HRESULT RetrieveOrCreateSession(CStringA& sessionId, ISession** ppSession)
	{
		CComPtr<ISession> pSession;
		HRESULT hr = RetrieveSession(sessionId,&pSession);
		if (FAILED(hr) || !pSession) {
			hr = CreateSession(sessionId,&pSession);
		}
		if (SUCCEEDED(hr) && pSession) {
			hr = pSession.CopyTo(ppSession);
		}
		return hr;
	}

	/*HRESULT SetResponseCookie(const CStringA& sessionId)
	{
		const CCookie& cookie = m_HttpRequest.GetSessionCookie();
	}*/
public:
	CStringA __stdcall GetRemoteIP()
	{
		CStringA strIP = "";
		CComPtr<IHttpServerContext> spContext;
 		HRESULT hr =  m_HttpResponse.GetServerContext(&spContext);
		if (SUCCEEDED(hr))
		{
			CHAR szIP[MAX_PATH];
			DWORD cbSize(MAX_PATH);
			if (SUCCEEDED(spContext->GetServerVariable("REMOTE_ADDR",szIP,&cbSize)))
			{
				strIP = szIP;
			}
		}
 		return strIP;
	}


	LONG GetSessionInt(LPCSTR lpszName)
	{
		if (!m_spSession) {
			return -1; // 未初始化或未创建SESSION
		}
		CComVariant varCode((LONG)0);
		if (FAILED(m_spSession->GetVariable(lpszName,&varCode))) {
			return -1;	// 未初始化或未设定有效会话参数
		}
		return (LONG)varCode.lVal ;
	}
	CStringA GetSessionString(LPCSTR lpszName)
	{
		if (!m_spSession) {
			return "";
		}
		CComVariant varTemp;
		if (FAILED(m_spSession->GetVariable(lpszName,&varTemp))) {
			return "";
		}
		CComVariant varStr;
		if (FAILED(varTemp.ChangeType(VT_LPSTR,&varStr))) {
			return "";
		}
		return varStr.pcVal ;
	}
	HRESULT GetSessionDate(LPCSTR lpszName,COleDateTime& oTime)
	{
		if (!m_spSession) {
			return E_POINTER;
		}
		HRESULT hr;
		CComVariant varTemp;
		if (FAILED(hr =m_spSession->GetVariable(lpszName,&varTemp))) {
			return hr;
		}
		CComVariant varDate;
		if (FAILED(hr = varTemp.ChangeType(VT_DATE,&varDate))) {
			return hr;
		}
		oTime = varDate ;
		return S_OK;
	}
	HRESULT SetSession(LPCSTR lpszName, LONG nValue)
	{ 
		if (!m_spSession || !lpszName)  {
			return E_POINTER;
		}
		CComVariant var(nValue);
		return m_spSession->SetVariable(lpszName,var);
	}
	HRESULT SetSession(LPCSTR lpszName, LPCSTR lpszValue)
	{
		if (!m_spSession || !lpszName || !lpszValue) {
			return E_POINTER;
		}
		CComVariant var(lpszValue);
		return m_spSession->SetVariable(lpszName,var);
	}
	HRESULT SetSession(LPCSTR lpszName, COleDateTime tValue)
	{
		if (!m_spSession || !lpszName ) {
			return E_POINTER;
		}
		CComVariant var(tValue);
		return m_spSession->SetVariable(lpszName,var);
	}

	HRESULT RemoveSession(LPCSTR lpszName)
	{
		if (!m_spSession || !lpszName) {
			return E_POINTER;
		}
		return m_spSession->RemoveVariable(lpszName);
	}

	HRESULT TermSession()
	{
		CStringA sessionId;
		m_HttpRequest.GetSessionCookie().GetValue( sessionId );
#if 1
		return TermSession(sessionId);
#else
		if (SUCCEEDED(TermSession(sessionId))) {
			m_HttpResponse.DeleteCookie(SESSION_COOKIE_NAME);
		}
		return S_OK;
#endif
	}

	HRESULT TermSession(LPCSTR lpszID)
	{
		CStringA sessionId(lpszID);
		sessionId.Trim();

		CComPtr<ISession> spSession;
		HRESULT hr = m_spSessionSvc->GetSession(sessionId,&spSession);
		if (SUCCEEDED(hr)) {
			spSession->RemoveAllVariables();
			m_spConnectPool->Remove(sessionId);
			m_spSessionSvc->CloseSession(sessionId);
		}
		return S_OK;
	}

	int GetDBID(CMySQLConnect* pConnect)
	{
		if (!pConnect)
			return -1;
		static LPCSTR sUpdateSQL = "update `sid` set `pid` = `pid`+1 where `sname` ='cid'";
		static LPCSTR sSelectSQL = "select `pid` from `sid` where `sname` = 'cid'";
		if (-1 != pConnect->ExecSQL(sUpdateSQL)) {
			CMySQLQuery query = pConnect->QuerySQL(sSelectSQL);
			if (query.Rows()) {
				return query.GetIntField(0,-1);
			}
		}
		return -1;
	}

	CStringA GetCurrentTimeString()
	{
		SYSTEMTIME stm = {0};
		GetLocalTime(&stm);
		CStringA sTime; 
		sTime.Format("%04d-%02d-%02d %02d:%02d:%02d", 
				stm.wYear, stm.wMonth, stm.wDay, stm.wHour, stm.wMinute , stm.wSecond);
		return sTime;
	}
public:
	LONG GetMachineCode(DWORD cbFlags)
	{
		CDocFlags Flags(cbFlags);
		if (Flags.IsSource()) {
			return 0;	// 以源码方式返回数据
		}
		LONG nCode = GetSessionInt("MachineCode");
		if (nCode == -1) {
			nCode = 0;	// 未设定MAC哈希值或读取设定发生错误
		}
		return nCode;
	}
	void SetMachineCode(LONG nMachineCode)
	{
		if (m_spSession) {
			CComVariant varCode((LONG)nMachineCode);
			m_spSession->SetVariable("MachineCode",varCode);
		}
	}

	CMySQLConnect* GetDBConnectPtr(CRequestDocument& Request, CResponeDocument& Respone)
	{
		CMySQLConnect* pConn = NULL;
		if (!m_spConnectPool) {
			Respone.SetResponeError("ServiceError",CMD_ERROR_DATABASE,"无法启动或无效的数据库连接池服务");
			return NULL;
		}
		CStringA sessionId = m_sessionId;
		if (sessionId.GetLength() <= 0) {
			m_HttpRequest.GetSessionCookie().GetValue( sessionId );
		}
		if (sessionId.GetLength() == 0) {
			Respone.SetResponeError("ServiceError",CMD_ERROR_DATABASE,"无效的会话信息，无法检数据库测连接对象");
			return NULL;
 		}

		T* pT = static_cast<T*>(this);
		LPCSTR lpszConnect = pT->OnGenericConnectString(Request);
		if (!lstrlenA(lpszConnect)) {
			Respone.SetResponeError("ServiceError",CMD_ERROR_DATABASE,"无效的数据库连接字串或字串为空");
			return NULL;
 		}

		HRESULT hr = m_spConnectPool->Add(sessionId,lpszConnect,(void**)&pConn);
		if (hr != S_OK) {
			Respone.SetResponeError("ServiceError",CMD_ERROR_DATABASE,"无法从连接池中检索或创建数据库连接");
			return NULL;
 		}
		return pConn;
	}
	
	bool ConnectDB(CMySQLConnect& Conn)
	{
		Json::Value config;
		Json::Reader reader;
		std::string host,user,pass,db;
		std::string json = GenericConnectString();
		if (!reader.parse(json,config)) {
			DbgPrintf("无法解析MYSQL连接JSON 配置");
			return false;
		}

		if (!config.isMember("host") || !config.isMember("user") || !config.isMember("pass") || !config.isMember("db")) {
			DbgPrintf("连接配置信息中不包含相关的连接信息！");
			return false;
		}

		int port = 3306;
		host = config["host"].asString();
		user = config["user"].asString();
		pass = config["pass"].asString();
		db = config["db"].asString();

		if (config.isMember("port")) {
			port = config["port"].asInt();
		}
 		return !Conn.Open(host.c_str(),user.c_str(),pass.c_str(),db.c_str(),port);
	}
public:
	LPCSTR GenericConnectString()
	{
		static CStringA sConnect = "";
		if (sConnect == "") {
			CString sConfig = PathHelper::CurrentModulePath();
			sConfig += _T("db.config");
			DWORD cbSize = 0;
			LPSTR pBuffer = NULL;
			FileHelper::ReadToBuffer(sConfig,(PBYTE*)&pBuffer,&cbSize);
			sConnect = pBuffer;
			__SAFE_FREE_PTR(pBuffer);
		}
		return (LPCSTR)sConnect;
	}
	LPCSTR OnGenericConnectString(CRequestDocument& Request)
	{
		return GenericConnectString();
	}
	
	void QueryToValue(CMySQLQuery &Query, Json::Value &_Item)
	{
		for (unsigned int col = 0; col < Query._field_count ; col++) {
			CStringA sField = Query.GetFieldName(col);
			sField.Trim().MakeLower();
			if (Query.IsIntField(col)) {
				_Item[(LPCSTR)sField] = (__int64)Query.GetHugeIntField(col);
			}
			else if (Query.IsDobuleField(col)) {
				_Item[(LPCSTR)sField] = Query.GetFloatField(col);
			}
			else {
				_Item[(LPCSTR)sField] = Query.GetStringField(col);
			}
		}
	}

	void QueryToValues(CMySQLQuery &Query, Json::Value &_Value)
	{
		for (unsigned int row = 0 ; row < Query.Rows(); row++) {
			Json::Value _Item;
			QueryToValue(Query,_Item);
			_Value.append(_Item);
			Query.NextRow();
		}
	}
public:

	bool OnInitServiceHandler(CResponeDocument& Respone)
	{
		/*CMySQLConnect* pConn = GetDBConnectPtr(Respone);
		if (!pConn) {
			return false;
		}*/

		//return true;
		/*if (MySQL.GetHandle())
			return true;

		Json::Value _Config;
		CString sConfig = PathHelper::CurrentModulePath();
		sConfig += _T("db.config");
		if (!JsonHelper::ReadJsonFile(sConfig,_Config)) {
			Respone.SetResponeError("ServerError",CMD_ERROR_SERVICE,"配置文件不存在或读取错误！"); 
			return false;
		}
		if (MySQL.Open(_Config["host"].asString().c_str(),
				_Config["user"].asString().c_str(),
				_Config["pass"].asString().c_str(),
				_Config["db"].asString().c_str(),
				_Config["port"].asInt()
			))
		{
			Respone.SetResponeError("ServerError",CMD_ERROR_DATABASE,"无法连接到后台数据库！"); 
			return false;
		}*/
		return true;
	}
 	virtual bool OnValidateRequestCommand(CRequestDocument& Request, CResponeDocument& Respone, DWORD cbFlags)
	{
  		CServiceFlags ServiceFlags(cbFlags);
		if (cbFlags == 0) 
			return true;

		T* pT = static_cast<T*>(this);
		if (!m_spSession) {
 			if (Request.HaveParameter("sessionid")) {
				m_sessionId = Request["sessionid"].asString().c_str();
			}
			m_sessionId.Trim();
			HRESULT hr = pT->RetrieveSession(m_sessionId,&m_spSession);
			bool fInited = (m_spSession != NULL);
			if (fInited) {
				LONG lCount = 0;
				hr = m_spSession->GetCount(&lCount);
				if (FAILED(hr) || lCount <= 0) {
					fInited = false;
				}
			}
			if (!fInited) {
				Respone.SetLastError(CMD_ERROR_SESSION,"尚未初始化");
				return false;
			}
		}
 		
		if (S_OK == m_spSession->IsExpired()) {
			Respone.SetLastError(CMD_ERROR_SESSION,"会话超期");
			return false;
		};

		if (ServiceFlags.IsDBConnect()) {
			CMySQLConnect* p = pT->GetDBConnectPtr(Request,Respone);
			if (!p) {
				return false;
			}
		}

		if ( ServiceFlags.IsProduct() ) {
			// 需要进行客户端访问产品进行认真
			LONG nProductID = GetSessionInt("ProductID");
			if (nProductID <= 0) {
				Respone.SetLastError(CMD_ERROR_PRODUCT,"无效客户端");
				return false;
			}
			LONG nProductState = GetSessionInt("ProductState");
			if (nProductState == 0) {
				Respone.SetLastError(CMD_ERROR_PRODUCT,"客户端程序尚未通过认证");
				return false;
			}
			else if (nProductState < 0) {
				Respone.SetLastError(CMD_ERROR_PRODUCT,"客户端程序状态异常或被管理员禁用");
				return false;
			}
		}
 		
		if (ServiceFlags.IsMachine()) {
			LONG nMachineID = GetSessionInt("ProductMachineID");
			if (nMachineID <= 0) {
				Respone.SetLastError(CMD_ERROR_PRODUCT,"客户端访问主机尚未注册");
				return false;
			}
			LONG nMachineState = GetSessionInt("ProductMachineState");
			if (nMachineState == 0) {
				Respone.SetLastError(CMD_ERROR_PRODUCT,"客户端访问主机尚未通过认证");
				return false;
			}
			else if (nMachineState < 0) {
				Respone.SetLastError(CMD_ERROR_PRODUCT,"客户端访问主机状态异常或被管理员禁用");
				return false;
			}
 		}
 		if (!ServiceFlags.IsLogon())
			return true;

		LONG nUserID = GetSessionInt("UserID");
		if (nUserID <= 0) {
			Respone.SetLastError(CMD_ERROR_LOGON,"尚未进行登录");
			return 0;
		}
		
		LONG nUserState = GetSessionInt("UserState");
		if (nUserState == 0) {
			Respone.SetLastError(CMD_ERROR_ACCESS,"用户尚未开通");
			return false;
		}
		else if (nUserState < 0) {
			Respone.SetLastError(CMD_ERROR_ACCESS,"用户状态异常");
			return false;
		}
 
		if (ServiceFlags.IsAdmin() || ServiceFlags.IsSuper()) {
			LONG nRight = GetSessionInt("UserRight");
			if (nRight <= 0) {
				Respone.SetLastError(CMD_ERROR_ACCESS,"无效用户权限");
				return false;
			}
			if (ServiceFlags.IsSuper() && nRight < 2) {
				Respone.SetLastError(CMD_ERROR_ACCESS,"需要超级管理员权限");
				return false;
			}
		}

		Respone.Flags._Set(Request.Flags._Get()); 
		return true;
	}
public:
	HTTP_CODE OnVersion(CResponeDocument& Respone, bool fReport = true)
	{
		Respone.SetCommand("Version");
		Respone.SetLastError(0,"关于WEB服务");
		Respone["frame_version"]	= "2.0.0.0";
		Respone["frame_build"]		= "2014.07.16.2";
		Respone["frame_product"]	= "IISAPI WEB服务框架库";
		Respone["copyright"]		= "SF_Soft(r) 四方软体工作室";
		if (fReport) {
			return Report(Respone, JSON_SOURCE | JSON_STYLE);
		}
		return HTTP_SUCCESS;
	}
}; // 类 CCmdHandler

#endif // end of define __CMD_SERVICE__