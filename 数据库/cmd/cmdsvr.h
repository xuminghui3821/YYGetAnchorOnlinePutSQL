#ifndef __CMD_REMOTE_SERVICE__
#define __CMD_REMOTE_SERVICE__
#pragma once
#include "CmdDoc.h"
#include "CmdMySQL.h"
using namespace CMD;

//////////////////////////////////////////////////////////////////////////////////////////////////////
// �������Ȩ�޿���
// 
#define CMD_INIT				0x00010000		// �����ʼ�����ӣ����г���/MAC��֤����ܷ���
#define CMD_SESSION				CMD_INIT		// ͬCMD_INIT�����봴��SESSION�Ự����ܽ��з���
#define CMD_NODBCONN			0x00020000		// ������֤���ݿ�������Ϣ�� Ĭ�ϱ�����֤���ݿ�����
#define CMD_MACHINE				0x00040000		// ��Ҫ��֤������
#define CMD_PRODUCT				0x00080000		// ��Ҫ��Կͻ��˷��ʲ�Ʒ������֤
#define CMD_RESET				0x00100000		// ��Ҫ���³�ʼ����֤��Ϣ
#define CMD_LOGON				0x10000000		// �û������¼����ܷ���
#define CMD_ADMIN				0x20000000		// �û������¼�Ҿ��й���ԱȨ�޲��ܷ���
#define CMD_SUPER				0x40000000		// �û������¼�Ҿ��г�������ԱȨ�޲��ܷ���

#define CMD_CERT				CMD_MACHINE | CMD_PRODUCT
#define CMD_CERT_USER			CMD_CERT | CMD_LOGON
#define CMD_CERT_ADMIN			CMD_CERT_USERT | CMD_ADMIN
#define CMD_CERT_SUPER			CMD_CERT | CMD_SUPER
//////////////////////////////////////////////////////////////////////////////////////////////////////
// ״̬����
//
#define CMD_STATE_SUCCESS		1				// ����״̬
#define CMD_STATE_DEFAULT		0				// ��ʼ״̬����ȷ����Ҫ����ȷ��
#define CMD_STATE_DISABLE		-1				// ����״̬
#define CMD_STATE_EXPIRE		-2				// ����״̬
#define CMD_STATE_FAIL			-3				// ����״̬
//
#define IsStateSucceeded(state)	(state > 0)
#define IsStateDefault(stat)	(state == 0)
#define IsStateFailed(state)	(state < 0)
//////////////////////////////////////////////////////////////////////////////////////////////////////
// �û�Ȩ�޶���
//
#define CMD_USER_SUPER			2				// ��������ԱȨ��
#define CMD_USER_ADMIN			1				// ����ԱȨ��
#define CMD_USER_NORMAL			0				// ��ͨ�û�
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
		Respone.SetResponeError(Request.GetCommand(),CMD_ERROR_COMMAND,"��֧�ֵ��������");\
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
	CComPtr<ISessionStateService> m_spSessionSvc;	// �Ự����֧��
 	CComPtr<ISession> m_spSession;
	CComPtr<IMemoryCache> m_spBlobCache;			// Blob ����֧��
	CComPtr<IDBConnectPool> m_spConnectPool;
protected:
	CStringA m_sessionId;
public:
	HTTP_CODE ValidateAndExchange()
	{
		CResponeDocument Respone;
		m_HttpResponse.SetContentType("text/html"); 		// ������������

		// �� ISAPI ��չ�л�ȡ ISessionStateService
		if (FAILED(m_spServiceProvider->QueryService(__uuidof(ISessionStateService), &m_spSessionSvc))) {
			return Report(Respone,"ServiceError", CMD_ERROR_SERVICE, "�޷���ȡ�Ự����");
 		}
		// �� ISAPI ��չ�л�ȡ IMemoryCache ����
		if (FAILED(m_spServiceProvider->QueryService(__uuidof(IMemoryCache), &m_spBlobCache))) {
			return Report(Respone,"ServiceError", CMD_ERROR_SERVICE, "�޷���ȡ�������");
		}
		if (FAILED(m_spServiceProvider->QueryService(__uuidof(IDBConnectPool), &m_spConnectPool))) {
			return Report(Respone,"ServiceError", CMD_ERROR_SERVICE, "�޷���ȡ���ݿ����ӳط���");
		}
	
		T* pT = static_cast<T*>(this);

		// ���з����ʼ������
		if (!pT->OnInitServiceHandler(Respone)) {
			return Report(Respone, JSON_SOURCE | JSON_STYLE);
		}

		// ��ȡ������Ϣ
		CStringA sParam = m_HttpRequest.GetQueryString();
		sParam.Trim().MakeLower();

		OutputDebugStringA((LPCSTR)sParam); 
		if (sParam == "" || sParam == "version") {
			return pT->OnVersion(Respone);
		}

		// ���Ҷ���ķ�����࣬����֤��������Ƿ���ȷ
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
			return Report(Respone,"ServiceError", CMD_ERROR_SERVICE,"��Ч�������",JSON_SOURCE | JSON_STYLE);
		}
		
		// ������������ɣ���ȡ�ͻ���Ͷ�ݵ�JSON����
		CStringA sJson;
		if (!GetRequestJson(sJson,Respone)) {
			return Report(Respone, JSON_SOURCE | JSON_STYLE);
		}

		// �����Ự������Ϣ
		if (S_OK != RetrieveOrCreateSession() ) {
			return Report(Respone, "ServiceError", CMD_ERROR_SESSION, "�޷��������ȡ�Ự��Ϣ", JSON_SOURCE | JSON_STYLE);
		}

		// �����ͻ���Ͷ�����ݣ���������ɹ�����ת������ķ����ܽӿڴ�����
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
			Respone.SetResponeError("ServiceError",CMD_ERROR_SERVICE,"�޷���ȡIHttpServerContext����");  
			return FALSE; 
		} 
 		DWORD dwBytesTotal = m_HttpRequest.GetTotalBytes(); 
		if ((dwBytesTotal+1 < dwBytesTotal) || (dwBytesTotal < 0)) {  
			Respone.SetResponeError("ServerError",CMD_ERROR_SERVICE,"����������Ч������Դ");  
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
			return -1; // δ��ʼ����δ����SESSION
		}
		CComVariant varCode((LONG)0);
		if (FAILED(m_spSession->GetVariable(lpszName,&varCode))) {
			return -1;	// δ��ʼ����δ�趨��Ч�Ự����
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
			return 0;	// ��Դ�뷽ʽ��������
		}
		LONG nCode = GetSessionInt("MachineCode");
		if (nCode == -1) {
			nCode = 0;	// δ�趨MAC��ϣֵ���ȡ�趨��������
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
			Respone.SetResponeError("ServiceError",CMD_ERROR_DATABASE,"�޷���������Ч�����ݿ����ӳط���");
			return NULL;
		}
		CStringA sessionId = m_sessionId;
		if (sessionId.GetLength() <= 0) {
			m_HttpRequest.GetSessionCookie().GetValue( sessionId );
		}
		if (sessionId.GetLength() == 0) {
			Respone.SetResponeError("ServiceError",CMD_ERROR_DATABASE,"��Ч�ĻỰ��Ϣ���޷������ݿ�����Ӷ���");
			return NULL;
 		}

		T* pT = static_cast<T*>(this);
		LPCSTR lpszConnect = pT->OnGenericConnectString(Request);
		if (!lstrlenA(lpszConnect)) {
			Respone.SetResponeError("ServiceError",CMD_ERROR_DATABASE,"��Ч�����ݿ������ִ����ִ�Ϊ��");
			return NULL;
 		}

		HRESULT hr = m_spConnectPool->Add(sessionId,lpszConnect,(void**)&pConn);
		if (hr != S_OK) {
			Respone.SetResponeError("ServiceError",CMD_ERROR_DATABASE,"�޷������ӳ��м����򴴽����ݿ�����");
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
			DbgPrintf("�޷�����MYSQL����JSON ����");
			return false;
		}

		if (!config.isMember("host") || !config.isMember("user") || !config.isMember("pass") || !config.isMember("db")) {
			DbgPrintf("����������Ϣ�в�������ص�������Ϣ��");
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
			Respone.SetResponeError("ServerError",CMD_ERROR_SERVICE,"�����ļ������ڻ��ȡ����"); 
			return false;
		}
		if (MySQL.Open(_Config["host"].asString().c_str(),
				_Config["user"].asString().c_str(),
				_Config["pass"].asString().c_str(),
				_Config["db"].asString().c_str(),
				_Config["port"].asInt()
			))
		{
			Respone.SetResponeError("ServerError",CMD_ERROR_DATABASE,"�޷����ӵ���̨���ݿ⣡"); 
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
				Respone.SetLastError(CMD_ERROR_SESSION,"��δ��ʼ��");
				return false;
			}
		}
 		
		if (S_OK == m_spSession->IsExpired()) {
			Respone.SetLastError(CMD_ERROR_SESSION,"�Ự����");
			return false;
		};

		if (ServiceFlags.IsDBConnect()) {
			CMySQLConnect* p = pT->GetDBConnectPtr(Request,Respone);
			if (!p) {
				return false;
			}
		}

		if ( ServiceFlags.IsProduct() ) {
			// ��Ҫ���пͻ��˷��ʲ�Ʒ��������
			LONG nProductID = GetSessionInt("ProductID");
			if (nProductID <= 0) {
				Respone.SetLastError(CMD_ERROR_PRODUCT,"��Ч�ͻ���");
				return false;
			}
			LONG nProductState = GetSessionInt("ProductState");
			if (nProductState == 0) {
				Respone.SetLastError(CMD_ERROR_PRODUCT,"�ͻ��˳�����δͨ����֤");
				return false;
			}
			else if (nProductState < 0) {
				Respone.SetLastError(CMD_ERROR_PRODUCT,"�ͻ��˳���״̬�쳣�򱻹���Ա����");
				return false;
			}
		}
 		
		if (ServiceFlags.IsMachine()) {
			LONG nMachineID = GetSessionInt("ProductMachineID");
			if (nMachineID <= 0) {
				Respone.SetLastError(CMD_ERROR_PRODUCT,"�ͻ��˷���������δע��");
				return false;
			}
			LONG nMachineState = GetSessionInt("ProductMachineState");
			if (nMachineState == 0) {
				Respone.SetLastError(CMD_ERROR_PRODUCT,"�ͻ��˷���������δͨ����֤");
				return false;
			}
			else if (nMachineState < 0) {
				Respone.SetLastError(CMD_ERROR_PRODUCT,"�ͻ��˷�������״̬�쳣�򱻹���Ա����");
				return false;
			}
 		}
 		if (!ServiceFlags.IsLogon())
			return true;

		LONG nUserID = GetSessionInt("UserID");
		if (nUserID <= 0) {
			Respone.SetLastError(CMD_ERROR_LOGON,"��δ���е�¼");
			return 0;
		}
		
		LONG nUserState = GetSessionInt("UserState");
		if (nUserState == 0) {
			Respone.SetLastError(CMD_ERROR_ACCESS,"�û���δ��ͨ");
			return false;
		}
		else if (nUserState < 0) {
			Respone.SetLastError(CMD_ERROR_ACCESS,"�û�״̬�쳣");
			return false;
		}
 
		if (ServiceFlags.IsAdmin() || ServiceFlags.IsSuper()) {
			LONG nRight = GetSessionInt("UserRight");
			if (nRight <= 0) {
				Respone.SetLastError(CMD_ERROR_ACCESS,"��Ч�û�Ȩ��");
				return false;
			}
			if (ServiceFlags.IsSuper() && nRight < 2) {
				Respone.SetLastError(CMD_ERROR_ACCESS,"��Ҫ��������ԱȨ��");
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
		Respone.SetLastError(0,"����WEB����");
		Respone["frame_version"]	= "2.0.0.0";
		Respone["frame_build"]		= "2014.07.16.2";
		Respone["frame_product"]	= "IISAPI WEB�����ܿ�";
		Respone["copyright"]		= "SF_Soft(r) �ķ����幤����";
		if (fReport) {
			return Report(Respone, JSON_SOURCE | JSON_STYLE);
		}
		return HTTP_SUCCESS;
	}
}; // �� CCmdHandler

#endif // end of define __CMD_SERVICE__