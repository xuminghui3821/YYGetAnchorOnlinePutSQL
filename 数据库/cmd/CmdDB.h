#ifndef __CMD_DB__
#define __CMD_DB__
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include "Log/DbgPrint.h"

#ifndef CMD_CONN_STRING_LEN
#define CMD_CONN_STRING_LEN		512
#endif

#define CMD_CONNECTION_TIMEOUT						600000
#define CMD_CONNECTION_SWEEPER_TIMEOUT				60000 

typedef CFixedStringT<CStringA, CMD_CONN_STRING_LEN> cmdDataConnectKey;

__interface __declspec(uuid("320252AE-E139-485c-8729-5D6B7CD42826")) 
IDBConnect : public IUnknown
{
	STDMETHOD(Open)(LPCSTR szConn, void **ppDS);
	STDMETHOD(Close)(LPCSTR szConn);
	STDMETHOD(GetAt)(UINT nIndex, void** ppDS);
	STDMETHOD(SetTimeout)(DWORD cbTime);
	STDMETHOD(RemoveTimeoutConnection)();
 	STDMETHOD(Uninitialize)();
};

__interface __declspec(uuid("8500CED1-CC5B-47ee-9670-078D95FA1018")) 
IDBConnectPool : public IUnknown
{   
 	STDMETHOD(Add)(LPCSTR szID, LPCSTR szConn, void** ppDS);
 	STDMETHOD(Remove)(LPCSTR szID);
	STDMETHOD(RemoveAllConnections)();
 	STDMETHOD(Lookup)(LPCSTR szID, void **ppDS);
 	STDMETHOD(LookupEx)(LPCSTR szID, IDBConnect **ppConnect);
 	STDMETHOD(Uninitialize)();
};
typedef CComPtr<IDBConnect> DBCONNECTTYPE;

template <typename T, typename TConnect, typename TCritSec=CComFakeCriticalSection> 
class CDBConnectImpl 
	: public IDBConnect
	, public CComObjectRootEx<CComGlobalsThreadModel>
{
public:
	typedef typename TConnect _DBConnect;
	class ConnectInfo {
	public:
		ConnectInfo() 
			: LastTime(0) 
			, Connect(NULL) 
		{}
		~ConnectInfo()
		{
			if (Connect) {
				delete Connect;
				Connect = NULL;
			}
		}
	public:
		DWORD LastTime;
		_DBConnect* Connect;
	};
public:
	typedef CDBConnectImpl<T,TConnect, TCritSec> thisClass;
	typedef ATL::CSimpleMap<cmdDataConnectKey, ConnectInfo*> cmdDBConnectCacheMap;
	DWORD m_dwTimeout;
public:
	BEGIN_COM_MAP(thisClass)
		COM_INTERFACE_ENTRY(IDBConnect)
	END_COM_MAP()
public:
	CDBConnectImpl()
		: m_dwTimeout(CMD_CONNECTION_TIMEOUT)
	{
		m_cs.Init();
	}
	virtual ~CDBConnectImpl()
	{
		Uninitialize();
	}
public:
	STDMETHOD(Open)(LPCSTR szConn, void **ppDS)
	{
		HRESULT hr = E_FAIL;
		if (!szConn) 
 			return E_INVALIDARG;
 		
		if (!ppDS) 
 			return E_POINTER;
 		
		hr = m_cs.Lock();
		if (FAILED(hr))	
 			return hr;
 
		T* pT = static_cast<T*>(this);

		_DBConnect* pConnect = NULL;
		ConnectInfo* pConnectInfo = m_ConnectionMap.Lookup(szConn);
		if (!pConnectInfo)
		{
 			hr = pT->OnOpenConnect(szConn, &pConnect);
			if (hr == S_OK)
			{
				_ATLTRY
				{
					pConnectInfo = new ConnectInfo;
					pConnectInfo->Connect = pConnect;
					pConnectInfo->LastTime = GetTickCount();
					if (m_ConnectionMap.Add(szConn, pConnectInfo))
					{
						if (ppDS) // we allow NULL here
							*ppDS = (void*)pConnect; // copy connection to output.
						hr = S_OK;
					}
					else
					{
						delete pConnectInfo;
						hr = E_FAIL; // map add failed
					}
				}
				_ATLCATCHALL()
				{
					hr = E_FAIL;
				}			
			}
		}
		else // lookup succeeded, entry is already in cache
		{
			// Instead of opening a new connection, just copy
			// the one we already have in the cache.
			if (ppDS) {
				*ppDS = (void*)pConnectInfo->Connect ;
				pConnectInfo->LastTime = GetTickCount();
			}
			hr = S_OK;
		}
		m_cs.Unlock();
		return hr;
	}
	STDMETHOD(GetAt)(UINT nIndex, void** ppDS) 
	{
		if ( (int)nIndex < 0)
			return E_INVALIDARG;
		if ( !ppDS) 
			return E_POINTER;

		HRESULT hr = E_FAIL;

		hr = m_cs.Lock();
		if (FAILED(hr))	
		{
			return hr;
		}

		do {
			if (nIndex >= m_ConnectionMap.GetSize())
			{
				hr = E_INVALIDARG;
				break;
			}
			ConnectInfo* p = m_ConnectionMap.GetValueAt(nIndex);
			if (!p) 
			{
				hr = E_POINTER;
			}
			else
			{
				*ppDS = (void*)p->Connect;
				p->LastTime = GetTickCount();
				hr = S_OK;
			}
		} while(false);

		m_cs.Unlock();
		return hr;
	}

	STDMETHOD(Close)(LPCSTR szConn)
	{
		HRESULT hr = E_FAIL;
		if (!szConn) 
		{
			return E_INVALIDARG;
		}

		hr = m_cs.Lock();
		if (FAILED(hr))	
		{
			return hr;
		}
		
		T* pT = static_cast<T*>(this);
		ConnectInfo* pConnectInfo =	m_ConnectionMap.Lookup(szConn);

		if (pConnectInfo)
		{
			hr = pT->OnCloseConnect(pConnectInfo->Connect);
			if (S_OK == hr) 
			{
				delete pConnectInfo;
				hr = m_ConnectionMap.Remove(szConn) ? S_OK : E_FAIL;
			}
		}
		m_cs.Unlock();
		return hr;
	}
 	STDMETHOD(Uninitialize)()
	{
		DbgPrintf("IDBConnect Uninitialize.....");
		HRESULT hr = E_FAIL;
		hr = m_cs.Lock();
		if (FAILED(hr))	
		{
			return hr;
		}

		T* pT = static_cast<T*>(this);
		int nCount = m_ConnectionMap.GetSize();
		for (int i = nCount - 1; i >= 0; i--) 
		{
			ConnectInfo* p = m_ConnectionMap.GetValueAt(i);
			if (S_OK == pT->OnCloseConnect(p->Connect) ) {
				delete p;
				m_ConnectionMap.RemoveAt(i);		
			}
		}
		m_cs.Unlock();
		return S_OK;
	}

	STDMETHOD(SetTimeout)(DWORD cbTime)
	{
		HRESULT hr = E_FAIL;
		hr = m_cs.Lock();
		if (FAILED(hr))	{
			return hr;
		}

		m_dwTimeout = cbTime;
		m_cs.Unlock();
		return S_OK;
	}

	STDMETHOD(RemoveTimeoutConnection)()
	{
		HRESULT hr = E_FAIL;
		hr = m_cs.Lock();
		if (FAILED(hr))	
		{
			return hr;
		}
		T* pT = static_cast<T*>(this);

 		for (int i = m_ConnectionMap.GetSize() -1; i >= 0; i--) 
		{
			ConnectInfo* p = m_ConnectionMap.GetValueAt(i);
			if ((GetTickCount() - p->LastTime) >= m_dwTimeout) 
			{
				DbgPrintf("连接长时间无活动！将结束会话的数据库连接！");
				if (S_OK == pT->OnCloseConnect(p->Connect) ) 
				{
					delete p;
					m_ConnectionMap.RemoveAt(i);		
				}			
			}
		}
		m_cs.Unlock();
		return S_OK;
	}

public:
	HRESULT OnOpenConnect(LPCSTR szConn, _DBConnect **pConnect)
	{
		return E_NOTIMPL;
	}
	HRESULT OnCloseConnect(_DBConnect* &pConnect)
	{
		if (pConnect) {
			delete pConnect;
			pConnect = NULL;
		}
		return S_OK;
	}
protected:
	cmdDBConnectCacheMap m_ConnectionMap;
	TCritSec m_cs;
};


template <class MonitorClass, typename TDBConnect> 
class CDBConnectPoolImpl 
	: public IDBConnectPool
	, public CComObjectRootEx<CComGlobalsThreadModel>
	, public IWorkerThreadClient
{
protected:
	MonitorClass m_Monitor;
	HANDLE m_hTimer;
	CComPtr<IServiceProvider> m_spServiceProvider;
public:
	typedef typename TDBConnect::_DBConnect DBConnect;
	typedef CDBConnectPoolImpl<MonitorClass,TDBConnect> thisClass;

	typedef CComObject<TDBConnect> comObject;
	BEGIN_COM_MAP(thisClass)
		COM_INTERFACE_ENTRY(IDBConnectPool)
	END_COM_MAP()
public:
	CDBConnectPoolImpl() : m_hTimer(NULL) {}
	~CDBConnectPoolImpl() throw() { ATLASSUME(m_hTimer == NULL); }
public:
	STDMETHOD(Add)(LPCSTR szID, LPCSTR szConn, void** ppDS)
	{
		HRESULT hr = E_FAIL;
		DBConnectMapType::CPair *pPair = NULL;

		if (ppDS)
			*ppDS = NULL;
		else
			return E_POINTER;

		if (!szID)
			return E_INVALIDARG;

		CSLockType lock(m_CritSec, false);
		hr = lock.Lock();
		if (FAILED(hr))
			return hr;

		hr = E_FAIL;
		CComPtr<IDBConnect> spConnect = NULL;
		_ATLTRY
		{
			pPair = m_Connections.Lookup(szID); 
			if (pPair) // the session exists and is in our local map of sessions
			{
				hr = pPair->m_value.QueryInterface(&spConnect);
				if (S_OK == hr) {
					hr = spConnect->Open(szConn,ppDS);
				}
			}
			else
			{
 				CComObject<TDBConnect> *spNewConnect = NULL;
				CComObject<TDBConnect>::CreateInstance(&spNewConnect);
				if (spNewConnect == NULL) {
					hr = E_OUTOFMEMORY;
				}
				else
				{
 					hr = spNewConnect->QueryInterface(&spConnect);
					if (SUCCEEDED(hr))
					{
						hr = spConnect->Open(szConn,ppDS);
						if (hr == S_OK) {
							hr = m_Connections.SetAt(szID, spConnect) != NULL ? S_OK : E_FAIL;
							if (hr != S_OK) {
								spConnect->Close(szConn);
								*ppDS = NULL;
							}
						}
					}
				}
			}
		}
		_ATLCATCHALL()
		{
			return E_UNEXPECTED;
		}
 		return hr;	
	}
 	STDMETHOD(Remove)(LPCSTR szID)
	{
		if (!szID)
			return E_INVALIDARG;

		HRESULT hr = E_FAIL;
		CSLockType lock(m_CritSec, false);
		hr = lock.Lock();
		if (FAILED(hr))
			return hr;
		_ATLTRY
		{
			hr = m_Connections.RemoveKey(szID) ? S_OK : E_UNEXPECTED;
		}
		_ATLCATCHALL()
		{
			hr = E_UNEXPECTED;
		}
		return hr;
	}
 	
	STDMETHOD(RemoveAllConnections)()
	{
		HRESULT hr = S_OK;
		CSLockType lock(m_CritSec, false);
		if (FAILED(hr = lock.Lock()))
			return hr;
		m_Connections.RemoveAll();
		return S_OK;
	}

	STDMETHOD(Lookup)(LPCSTR szID, void **ppDS)
	{
		HRESULT hr = E_FAIL;
		DBConnectMapType::CPair *pPair = NULL;

		if (!szID)
			return E_INVALIDARG;
		
		if (ppDS)
			*ppDS = NULL;
		else
			return E_POINTER;


		CSLockType lock(m_CritSec, false);
		hr = lock.Lock();
		if (FAILED(hr))
			return hr;

		hr = E_FAIL;
		CComPtr<IDBConnect> spConnect = NULL;
		_ATLTRY
		{
			pPair = m_Connections.Lookup(szID); 
			if (pPair) // the session exists and is in our local map of sessions
			{
				hr = pPair->m_value.QueryInterface(&spConnect);
				if (S_OK == hr) {
					hr = spConnect->GetAt(0,ppDS);
				}
			}
			else
			{
				// cout found
				hr = E_FAIL;
			}
		}
		_ATLCATCHALL()
		{
			return E_UNEXPECTED;
		}
 		return hr;	
	}
 	STDMETHOD(LookupEx)(LPCSTR szID, IDBConnect **ppConnect)
	{
		HRESULT hr = E_FAIL;
		DBConnectMapType::CPair *pPair = NULL;

		if (!szID)
			return E_INVALIDARG;
		
		if (ppConnect)
			*ppConnect = NULL;
		else
			return E_POINTER;


		CSLockType lock(m_CritSec, false);
		hr = lock.Lock();
		if (FAILED(hr))
			return hr;

		hr = E_FAIL;
		CComPtr<IDBConnect> spConnect = NULL;
		_ATLTRY
		{
			pPair = m_Connections.Lookup(szID); 
			if (pPair) // the session exists and is in our local map of sessions
			{
				hr = pPair->m_value.QueryInterface(&spConnect);
				if (S_OK == hr ) {
					hr = spConnect.CopyTo(ppConnect);
				}
			}
			else
			{
				// cout found
				hr = E_FAIL;
			}
		}
		_ATLCATCHALL()
		{
			return E_UNEXPECTED;
		}
		return hr;
	}
 	STDMETHOD(Uninitialize)()
	{
		return RemoveAllConnections();
	}

	HRESULT Initialize(DWORD ,IServiceProvider* ) throw()
	{
 		return m_CritSec.Init();
	}

	template <class ThreadTraits>
	HRESULT Initialize(
		CWorkerThread<ThreadTraits> *pWorker,
		IServiceProvider *pServiceProvider = NULL,
 		unsigned __int64 dwTimeout = CMD_CONNECTION_SWEEPER_TIMEOUT) throw()
	{
		HRESULT hr = m_CritSec.Init();
		if (FAILED(hr))
			return hr;

		if (!pWorker)
			return E_INVALIDARG;

		if (pServiceProvider)
			m_spServiceProvider = pServiceProvider;

		hr = m_Monitor.Initialize(pWorker);
		if (hr == S_OK)
		{
			DbgPrintf("IDBConnectPool Moniter Thread starting......");
			//sweep every 60000ms
			hr = m_Monitor.AddTimer((DWORD)dwTimeout, this, 0, &m_hTimer);
		}
		return hr;
	}

	void Shutdown() throw()
	{
		DbgPrintf("IDBConnectPool Shutdown......");
		if (m_hTimer)
		{
			if(FAILED(m_Monitor.RemoveHandle(m_hTimer)))
			{
				/* can't report from here */
				ATLASSERT(FALSE);
			}
			m_hTimer = NULL;
		}
		RemoveAllConnections();
	}

	// Implementation
	HRESULT Execute(DWORD_PTR /*dwParam*/, HANDLE /*hObject*/) throw()
	{
		DbgPrintf("连接池监控现场启动.......");
		HRESULT hr = S_OK;
		POSITION posRemove = NULL;
		const DBConnectMapType::CPair *pPair = NULL;
		POSITION pos = NULL;
		CComPtr<ISessionStateService> spSessionSvc;	// 会话服务支持
		
		CSLockType lock(m_CritSec, false);
		if (FAILED(hr = lock.Lock()))
			return hr;

		if (!m_spServiceProvider) {
			DbgPrintf("未初始化或无效的IServiceProvider服务支持接口");
			return E_POINTER;
		}

		if (FAILED(hr = m_spServiceProvider->QueryService(__uuidof(ISessionStateService), &spSessionSvc))) {
			DbgPrintf("无法获取ISessionStateService服务接口");
			return hr;
		}

		pos = m_Connections.GetStartPosition();
		while (pos)
		{
			posRemove = pos;
			pPair = m_Connections.GetNext(pos);
			if (pPair)
			{
				DbgPrintf("验证[%s]会话连接信息！",(LPCSTR)pPair->m_key);
				CComPtr<ISession> spSession;
				hr = spSessionSvc->GetSession(pPair->m_key,&spSession);
				bool fRemove = (FAILED(hr) || spSession == NULL);
				if (!fRemove) {
					fRemove =  (S_OK == spSession->IsExpired());
				}

				if (fRemove) {
					DbgPrintf("将移除[%s]会话连接信息！",(LPCSTR)pPair->m_key);
					m_Connections.RemoveAtPos(posRemove);
				}
				else{
					DbgPrintf("[%s]会话连接处于存活期！",(LPCSTR)pPair->m_key);
					pPair->m_value->RemoveTimeoutConnection();
				}

			}
		}
		return S_OK;
	}

	HRESULT CloseHandle(HANDLE hHandle) throw()
	{
		::CloseHandle(hHandle);
		m_hTimer = NULL;
		return S_OK;
	}
public:
	typedef CAtlMap<CStringA,
				DBCONNECTTYPE,
				CStringElementTraits<CStringA>,
				CElementTraitsBase<DBCONNECTTYPE> > DBConnectMapType;

	DBConnectMapType m_Connections; // map for holding sessions in memory
	CComCriticalSection m_CritSec; // for synchronizing access to map
	typedef CComCritSecLock<CComCriticalSection> CSLockType;
};

#endif 