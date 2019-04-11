#ifndef __CMD_SERVICE__
#define __CMD_SERVICE__
#include "CmdFtp.h"
#include "CmdDoc.h"
#include "inet/SimpleHttpClient.h"
#include <atlutil.h>
#include <update/update_check.h>
#include <cmd/cmdver.h>

#define DEF_FILTER	".zip;.rar;.7z;.jpg;.bmp;.png;.txt;.doc;.docx;.xls;"
__declspec(selectany) HANDLE g_hConnectMutex  = NULL; 
class CConnectLocker {
public:
	CConnectLocker() {
		if (g_hConnectMutex == NULL) {
			g_hConnectMutex = ::CreateMutex(NULL,FALSE, NULL); 
		}
		::WaitForSingleObject(g_hConnectMutex, INFINITE); 
	}
	~CConnectLocker() {
		if (g_hConnectMutex) {
			::ReleaseMutex(g_hConnectMutex); 
		}
	}
}; 

//const int __ivalue = 0;
#ifndef __PROPERTY_INC__
#define __PROPERTY_INC__

#define DECLARE_GET_PROPERTY(type,prop) \
public:\
	__declspec(property(get = Get##prop)) type prop; \
	type Get##prop();

#define DECLARE_SET_PROPERTY(type,prop) \
	public:\
	__declspec(property(put = Set##prop)) type prop;\
	void Set##prop(type rhs);

#define DECLARE_PROPERTY(type,prop) \
	public:\
	__declspec(property(get = Get##prop, put = Set##prop)) type prop; \
	type Get##prop() ;\
	void Set##prop(type rhs);

#define IMPLEMENT_GET_PROPERTY(type, prop) \
	public: \
	type Get##prop() { CmdLocker(); return m_##prop; } \
	type m_##prop;

#define IMPLEMENT_SET_PROPERTY(type, prop) \
	public: \
	void Set##prop(type rhs) { CmdLocker(); if (m_##prop != rhs) { m_IsChanged = true; m_##prop = rhs; } } \
	type m_##prop;

#define IMPLEMENT_PROPERTY(type, prop) \
	public: \
	type Get##prop() { CmdLocker(); return m_##prop; } \
	void Set##prop(type rhs) { CmdLocker(); m_##prop = rhs; } \
	type m_##prop;

#define DECLARE_IMPL_GET_PROPERTY(type,prop) \
	DECLARE_GET_PROPERTY(type,prop) \
	IMPLEMENT_GET_PROPERTY(type,prop)

#define DECLARE_IMPL_SET_PROPERTY(type,prop) \
	DECLARE_SET_PROPERTY(type,prop) \
	IMPLEMENT_SET_PROPERTY(type,prop)

#define DECLARE_IMPL_PROPERTY(type,prop) \
public: \
	__declspec(property(get = Get##prop, put = Set##prop)) type prop;\
	void Set##prop(type nValue) { CmdLocker(); if (m_##prop != rhs) { m_IsChanged = true; m_##prop = rhs; } } \
	type Get##prop() { CmdLocker(); return m_##prop; } \
	type m_##prop;
#endif

#define CmdDefaultLocker ATL::CComAutoCriticalSection

//#define CmdLocker() CMD::CCmdLockerT<CmdLock> Locker(&m_cs);
#define CmdLocker() CmdLocker Locker(&m_cs);

namespace CMD
{
	typedef ATL::CSimpleArray<CStringA> CmdStringArray;

	enum cmdObjType
	{
		cmdObject = 0,
		cmdServer = 1,
		cmdFTP,
		cmdCollection,
		cmdProperty,
		cmdPropertys,
		cmdConnectInfo,
		cmdConnection,
		cmdHeart,
		cmdMachine,
		cmdProduct, 
		cmdFile,
		cmdFileList,
		cmdUser,
		cmdAccountRecode,
		cmdAccountQuery,
		cmdClient
	};

	enum cmdValueType
	{
		cmdInt		= 0,
		cmdInt64	= 1,
		cmdString	= 2,
		cmdDate		= 3,
		cmdBool		= 4
	};

	enum cmdFolderType
	{
		cmdDownload = 0,
		cmdUnDownload = 1,
		cmdDownloded ,
		cmdDownRecy,
		cmdUpload,
		cmdUploadRecy,
		cmdTemplate,
	};

	enum cmdFileType
	{
		cmdPublishFile	= 0,
		cmdDownloadFile = 1,
		cmdUploadFile	= 2,
	};

	__interface ICmdObject;
	__interface ICmdCollection;
	__interface ICmdProperty;
	__interface ICmdPropertys;
	__interface ICmdServer;
	__interface ICmdConnectInfo;
	__interface ICmdConnection;
	__interface ICmdMachine;
	__interface ICmdProduct;
	__interface ICmdFile;
	__interface ICmdFileList;
	__interface ICmdUser;
	__interface ICmdHeart;
	__interface ICmdHeartCallback;
	__interface ICmdClient;
	__interface ICmdFTP;

	class CCmdServer;
	class CCmdConnInfo;
	class CCmdConnection;
	class CCmdMachine;
	class CCmdProduct;
	class CCmdFile;
	class CCmdFileList;
	class CCmdUser;
	class CCmdClient;
	
	typedef struct ProductInfo {
		LPCSTR ServiceURL;
		LPCSTR GroupName;
		LPCSTR GroupIID;
		LPCSTR ProductName;
		LPCSTR ProductIID;
		LPCSTR Version;
	}PRODUCT_INFO,*LPPRODUCT_INFO;
	typedef const PRODUCT_INFO* LPCPRODUCT_INFO;

	__interface ATL_NO_VTABLE __declspec(uuid("F0EA9D11-52B5-40d1-AC39-D5FDDAF10B5B")) 
	ICmdObjectSink {
		void OnObjectChanged(ICmdObject* pObject,bool& bHandled);
	};

 	__interface ATL_NO_VTABLE __declspec(uuid("25889C63-F598-4d49-BBA9-15B03E4C8192")) 
	ICmdObjectNotify {
		void NotifyObjectChanged();
		void AttachObjectSink(ICmdObjectSink* pSink);
	};

	__interface ATL_NO_VTABLE __declspec(uuid("5BAC2883-1DCB-490b-957D-5293F782A51C")) 
	ICmdObject {
 		DECLARE_GET_PROPERTY(LONG,ObjectType);
		DECLARE_GET_PROPERTY(bool,IsChanged);
		DECLARE_PROPERTY(CStringA,CmdLastError);
		HRESULT Clone(void* pObj);
		HRESULT ChangeProperty(LPCSTR pszName,CComVariant &varParam);
	};

	__interface ATL_NO_VTABLE __declspec(uuid("2DD2463D-7E11-4aa3-B7C9-EB90B7174EF0")) 
	ICmdCollection : public ICmdObject {
		DECLARE_GET_PROPERTY(LONG,Count);
		HRESULT Item(LONG nIndex, ICmdObject** ppItem);
		HRESULT Add(ICmdObject* pItem);
		HRESULT Insert(ICmdObject* pItem, LONG nAfter);
		HRESULT Remove(LONG nIndex,bool fRemove = true);
		HRESULT RemoveItem(ICmdObject* pItem,bool fDestory = true);
		HRESULT Clear(bool fDestory = true);
	};

	__interface ATL_NO_VTABLE __declspec(uuid("54A4F01A-9444-4cb7-BE45-627CEE40A660")) 
	ICmdProperty : public ICmdObject  {
		DECLARE_PROPERTY(CStringA, Name)
		DECLARE_PROPERTY(LONG,	   Type)
		DECLARE_PROPERTY(CStringA, Value)
		INT asInt();
		__int64 asInt64();
		COleDateTime asDate();
	};

	__interface ATL_NO_VTABLE __declspec(uuid("89BB16D6-F8B3-45b9-BF48-376B2B063657")) 
	ICmdPropertys : public ICmdObject  {
		DECLARE_GET_PROPERTY(ICmdCollection*,Propertys);
		ICmdProperty* FindProperty(LPCSTR lpszName);
		ICmdProperty* AddProperty(LPCSTR lpszKey,LPCSTR lpszValue,LONG nType = cmdString);
		ICmdProperty* AddProperty(LPCSTR lpszKey,LONG nValue);
		ICmdProperty* AddProperty(LPCSTR lpszKey,__int64 nValue);
		ICmdProperty* AddProperty(LPCSTR lpszKey,COleDateTime tmValue);
	};

	__interface ATL_NO_VTABLE __declspec(uuid("03CD474D-FA86-447d-8877-CE4C9A1FC2C8")) 
	ICmdServer : public ICmdObject  {
		DECLARE_GET_PROPERTY(LONG, ServerID)
		DECLARE_GET_PROPERTY(LONG, State)
		DECLARE_PROPERTY(CStringA, Caption)
		DECLARE_PROPERTY(CStringA, Server)
		DECLARE_PROPERTY(CStringA, Url)
		DECLARE_PROPERTY(WORD, Port)
		DECLARE_PROPERTY(CStringA, UserName)
		DECLARE_PROPERTY(CStringA, Password)
		DECLARE_PROPERTY(WORD, FtpPort)
		DECLARE_PROPERTY(CStringA, FtpUser)
		DECLARE_PROPERTY(CStringA, FtpPass)
		DECLARE_PROPERTY(CString, Description)
		DECLARE_GET_PROPERTY(COleDateTime, CreateTime)
		DECLARE_GET_PROPERTY(COleDateTime, LastTime)
	};

	__interface ATL_NO_VTABLE __declspec(uuid("D233CEAD-9C06-4e53-BDF8-AA5113E579AC")) 
	ICmdFTPFile : public ICmdObject {
		DECLARE_PROPERTY(bool, IsDirectory)
		DECLARE_PROPERTY(CStringA, FileName);
		DECLARE_PROPERTY(CStringA, FilePath);
		DECLARE_PROPERTY(LONGLONG, FileSize);
		DECLARE_GET_PROPERTY(CStringA, FullFileName);
	};

	__interface ATL_NO_VTABLE __declspec(uuid("A4CA4AF9-153F-4bb8-BEE9-D3340CFA8B9A")) 
	ICmdFTP : public ICmdObject  {
		DECLARE_GET_PROPERTY(LONG, ServerID)
		DECLARE_GET_PROPERTY(LONG, Type)
 		DECLARE_PROPERTY(CStringA, Host)
 		DECLARE_PROPERTY(WORD, Port)
		DECLARE_PROPERTY(CStringA, UserName)
		DECLARE_PROPERTY(CStringA, Password)
		DECLARE_PROPERTY(CStringA, PhyPath)

		//HRESULT Connect();
		//HRESULT DisConnect();
		//HRESULT DirFiles(LPCSTR lpszPath, ICmdCollection* pColl, bool fSubDirectory = true);
		//HRESULT CreateDirectory(LPCSTR lpsDirectory);
		//HRESULT DeleteDirectory(LPCSTR lpszDirectory);
		//HRESULT UploadFile(LPCSTR lpszLocal, LPCSTR lpszRemote,DWORD_PTR Contex);
		//HRESULT DwonloadFile(LPCSTR lpszRemote, LPCSTR lpszLocal, DWORD_PTR Contex);
	};

	__interface ATL_NO_VTABLE __declspec(uuid("707FB93B-96A7-43af-BBF4-EB81148C6B62")) 
	ICmdMachine : public ICmdObject {
		DECLARE_GET_PROPERTY(LONG,MachineID)
		DECLARE_GET_PROPERTY(LPCSTR,MacAddress)
		DECLARE_GET_PROPERTY(CStringA,RegCode)
		DECLARE_GET_PROPERTY(LONG,Days)
		DECLARE_GET_PROPERTY(LONG,State)
		DECLARE_GET_PROPERTY(LONG,Style)
	};

	__interface ATL_NO_VTABLE __declspec(uuid("D665577F-AAEE-41bc-BC21-1EBD7123B1B1")) 
	ICmdAccountRecode : public ICmdObject {
		DECLARE_GET_PROPERTY(LONG,AccountID)
		DECLARE_GET_PROPERTY(LONG,Type)
		DECLARE_GET_PROPERTY(LONG,Money)
		DECLARE_GET_PROPERTY(COleDateTime,CreateTime)
		DECLARE_PROPERTY(CStringA,Memo)
 	};

	__interface ATL_NO_VTABLE __declspec(uuid("544F3B73-26D7-4f6a-B25D-4644A43444CF")) 
	ICmdAccountQuery : public ICmdObject {
		DECLARE_GET_PROPERTY(LONG,Balance)
		DECLARE_GET_PROPERTY(LONG,Pay)
		DECLARE_GET_PROPERTY(LONG,Free)
		DECLARE_GET_PROPERTY(ICmdCollection*,Records)
  	};

	__interface ATL_NO_VTABLE _declspec(uuid("9FFC04AB-D82A-47e8-A1A9-23F571213366")) 
	ICmdProduct : public ICmdObject {
		DECLARE_GET_PROPERTY(LONG,ProductID)
		DECLARE_GET_PROPERTY(CStringA,RegCode)
 		DECLARE_GET_PROPERTY(LONG,Days)
		DECLARE_GET_PROPERTY(LONG,State)
		DECLARE_GET_PROPERTY(LONG,Style)
	};

	__interface ATL_NO_VTABLE __declspec(uuid("454AF805-DD26-4d01-8E3F-27A81E794A07")) 
	ICmdFile : public ICmdObject {
		DECLARE_PROPERTY(LONGLONG,RemoteID);
		DECLARE_PROPERTY(CStringA,RemoteFile);
		DECLARE_PROPERTY(CStringA,RemotePath);
		DECLARE_PROPERTY(CStringA,LocalFile);
		DECLARE_PROPERTY(CStringA,LocalPath);
		DECLARE_PROPERTY(CStringA,MD5);
		DECLARE_PROPERTY(LONG,RemoteType);
		DECLARE_PROPERTY(LONG,State);
		DECLARE_PROPERTY(LONG,FileType);
		DECLARE_PROPERTY(LONG,FileState);
		DECLARE_PROPERTY(LONG,FileStyle);
		DECLARE_PROPERTY(LONGLONG,FileSize);
		DECLARE_PROPERTY(LONGLONG,WriteBytes);
		DECLARE_PROPERTY(COleDateTime,CreateTime);
		DECLARE_PROPERTY(COleDateTime,LastTime);
		DECLARE_GET_PROPERTY(bool, IsUpload);
		DECLARE_PROPERTY(CStringA,Memo);
	};

	__interface ATL_NO_VTABLE __declspec(uuid("EDA77A08-A169-40d1-A3E0-A3D78D1D5115")) 
	ICmdFileList : public ICmdObject {
		DECLARE_GET_PROPERTY(LONG,PageNumber);
		DECLARE_GET_PROPERTY(LONG,PageCount);
		DECLARE_GET_PROPERTY(LONG,PageRows);
		DECLARE_GET_PROPERTY(ICmdCollection*,FileList);
	};

	__interface ATL_NO_VTABLE __declspec(uuid("34146B4C-8609-4f33-90E5-C7286B1B620F")) 
	ICmdUser : public ICmdObject {
		DECLARE_GET_PROPERTY(LONG,UserID)
		DECLARE_PROPERTY(LONG,UserType)
		DECLARE_GET_PROPERTY(LONG,State)
		DECLARE_GET_PROPERTY(LONG,Style)
		DECLARE_GET_PROPERTY(LONG,Days)
		DECLARE_GET_PROPERTY(LONG,Rights)
		DECLARE_GET_PROPERTY(LONG,UpThreads)
		DECLARE_GET_PROPERTY(LONG,DownThreads)
		DECLARE_GET_PROPERTY(LONG,ShowAccount)
		DECLARE_PROPERTY(CStringA,UserName)
		DECLARE_PROPERTY(CStringA,Password)
		DECLARE_PROPERTY(CStringA,QQ)
		DECLARE_PROPERTY(CStringA,Email)
		DECLARE_PROPERTY(CStringA,Mobile)
		DECLARE_PROPERTY(CStringA,NickName)
		DECLARE_PROPERTY(CStringA,Question)
		DECLARE_PROPERTY(CStringA,Anwser)
		
		DECLARE_PROPERTY(CStringA,Prov)
		DECLARE_PROPERTY(CStringA,City)
		DECLARE_PROPERTY(CStringA,Zip)
		DECLARE_PROPERTY(CStringA,Conuty)
		DECLARE_PROPERTY(CStringA,Co)
		DECLARE_PROPERTY(CStringA,Address)
		DECLARE_PROPERTY(CStringA,RecvMan)
		DECLARE_PROPERTY(CStringA,Fax)
		DECLARE_PROPERTY(CStringA,Phone)
		DECLARE_PROPERTY(CStringA,CreateTime)

		DECLARE_PROPERTY(LONG,ServerID)
		DECLARE_PROPERTY(LONG,FtpType)
		DECLARE_PROPERTY(CStringA,FtpHost)
		DECLARE_PROPERTY(WORD,FtpPort)
		DECLARE_PROPERTY(CStringA,FtpUser)
		DECLARE_PROPERTY(CStringA,FtpPass)
		DECLARE_PROPERTY(CStringA,FtpPath)
		DECLARE_PROPERTY(CStringA,Template)
		DECLARE_PROPERTY(LONG,Space)
		DECLARE_PROPERTY(LONG,Quality)
		DECLARE_GET_PROPERTY(CmdStringArray&, Filters);

		HRESULT Register();
		HRESULT Refresh();
		HRESULT Save();
		HRESULT PropectPassword(CStringA Query,CStringA answer);
		HRESULT ChangePass(CStringA sOldPass, CStringA sNewPass);
		HRESULT GetAccountRecords(LONG nDays, ICmdAccountQuery* pRecords);
		HRESULT GetAccountRecordsEx(COleDateTime& Start, COleDateTime& End, ICmdAccountQuery* pRecords);
		HRESULT GetUserSpace(Json::Value &value);
		HRESULT GetFileList(cmdFolderType cmdType,ICmdFileList* pFileList, int nPage = 0, int nRows = 50);
		HRESULT UploadFile(HWND hWnd, LPCSTR lpszLocalPath, LPCSTR lpszLocalFile, LONGLONG nFileSize, LPCSTR lpszMD5, LPCSTR lpszMemo, ICmdFile** ppCmdFile, DWORD dwFlags = 0);
		HRESULT WriteFileInfo(BOOL fUpload, LONG nFileID, ULONGLONG nWriteBytes , LONG nFileState, LPCSTR lpszMemo);
		HRESULT DownloadFile(LONG nFileID, ICmdFile** ppCmdFile);
		HRESULT DeleteFile(LPCSTR pszType, LPCSTR pszSubType, LPCSTR pszCmd, ICmdFile* pCmdFile);
		HRESULT FindFiles(LPCSTR pszFind, LPCSTR lpszSubType, ICmdFileList* pDownloads, ICmdFileList* pUploads, int nPage = 0, int nRows = 20);
		HRESULT GetFileInfo(LONG nFileID, ICmdFile** ppCmdFile, bool fUpload = true);
		HRESULT GetClearList(ICmdFileList* pUploads, ICmdFileList* pDownloads);
	};


 	__interface ATL_NO_VTABLE __declspec(uuid("3D49A76C-E830-43d7-9400-7EF2E28E988F")) 
	ICmdHeartCallback {
 		void OnHeartCallback(LONG lCode, LPCSTR lpszMsg, const Json::Value& ResultParameters, bool& fCancel);
	};

	__interface ATL_NO_VTABLE __declspec(uuid("D8F0484E-3C95-4aea-AE4A-0348DDB8CF04"))
		ICmdHeartBuilder {
		void OnHeartBuilder(Json::Value& RequestParameters, bool& fCancel);
	};

	__interface ATL_NO_VTABLE __declspec(uuid("D57947ED-010B-4b4c-9D50-6EC84370C7EE")) 
	ICmdHeart : public ICmdObject {
  		DECLARE_GET_PROPERTY(ICmdClient*,CmdClient);
		HRESULT Initialize(ICmdClient* pClient, DWORD cbTimer = 5000, DWORD_PTR pvContext = NULL);
		HRESULT Shutdown(DWORD);
		HRESULT AddHeartCallback(ICmdHeartCallback* pCallback);
		HRESULT RemoveHeartCallback(ICmdHeartCallback* pCallback);
		HRESULT AddHeartBuilder(ICmdHeartBuilder* pBuilder);
		HRESULT RemoveHeartBuilder(ICmdHeartBuilder* pBuilder);
 	};

	__interface ATL_NO_VTABLE __declspec(uuid("67FF4A08-9895-41bc-BE57-57A8D032183A")) 
	ICmdConnectInfo : public ICmdObject{
		DECLARE_PROPERTY(CStringA,Host);
		DECLARE_PROPERTY(WORD,Port);
		DECLARE_PROPERTY(CStringA,Url);
		DECLARE_PROPERTY(CStringA,User);
		DECLARE_PROPERTY(CStringA,Pass);
		DECLARE_PROPERTY(CStringA,Proxy);
		DECLARE_PROPERTY(WORD,Byport);
		DECLARE_PROPERTY(CStringA,Bypass);
		DECLARE_PROPERTY(CStringA,Byuser);
		HRESULT SetServiceUrl(LPCSTR lpszUrl);
	};

	__interface ATL_NO_VTABLE __declspec(uuid("F76778F0-20CC-4e7d-BFCA-FCDBF50353D1")) 
	ICmdConnection : public ICmdObject {
		DECLARE_PROPERTY(CStringA,Session);
		DECLARE_GET_PROPERTY(ICmdConnectInfo*,ConnectInfo);
		DECLARE_GET_PROPERTY(LONG,State)
		DECLARE_GET_PROPERTY(bool,IsConnected)
		HRESULT Connect(bool fReconn = false);
		HRESULT Execute(LPCSTR lpszUrl, CCommandDocument* pCmd, CResponeDocument& doc,LONG nHashCode = Sign::MAC_Hash() );
		HRESULT Close();
		HRESULT SetUrl(LPCSTR lpszUrl);
 	};

	__interface ATL_NO_VTABLE __declspec(uuid("AEE71743-099B-45ea-B8FF-72B508C89246")) 
	ICmdClient : public ICmdObject {
		DECLARE_GET_PROPERTY(ICmdUser*, CurrentUser);
		DECLARE_GET_PROPERTY(ICmdMachine*, Machine);
		DECLARE_GET_PROPERTY(ICmdProduct*, Product);
		DECLARE_GET_PROPERTY(ICmdConnection*, Connection);
		DECLARE_GET_PROPERTY(ICmdCollection*,Servers);
		DECLARE_GET_PROPERTY(ICmdHeart*, Heart);

		DECLARE_GET_PROPERTY(bool,IsLogined);
		DECLARE_GET_PROPERTY(bool,IsInited);
		DECLARE_GET_PROPERTY(ICmdPropertys*,Propertys);
		DECLARE_GET_PROPERTY(LONG,ServerID);
		
		HRESULT GetServerList(ICmdCollection* ServerList,LPCTSTR lpszCmdUrl = NULL);
 		HRESULT Init(HINSTANCE hInstance,ICmdHeartCallback* pCallback = NULL, LPVOID lpContext = NULL, LONG dwFlags = 1);
		HRESULT Register(ICmdUser* pUser,bool fSimple = false);
		HRESULT Login(LPCSTR lpszUser, LPCSTR lpszPass, LONG nServerID = 0);
		HRESULT Logout();
		HRESULT Term();
		HRESULT QueryUserProtected(CStringA sUser,CStringA& sProtected); // 获取用户密码保护信息
		HRESULT QueryUserPassword(CStringA sUser,CStringA sQuery,CStringA sAnwser, CStringA& sPassword); // 获取用户密码保护信息
		HRESULT TestService(LPCSTR lpszCmdUrl = NULL);
		HRESULT CmdExecute(LPCSTR lpszUrl, CCommandDocument* pCmd, CResponeDocument& doc,LONG nHashCode = Sign::MAC_Hash() );
		HRESULT ExecuteSQL(LPCSTR lpszSQL, Json::Value* pArgs, Json::Value& result, int& resultRows, bool query = false);
		__int64 NewDBID();
		HRESULT RestartService(HINSTANCE g_hInstance);
	};
};

namespace CMD
{

	template <typename TLock>
	class CCmdLockerT {
		TLock* m_p;
	public:
		explicit CCmdLockerT(TLock* p) : m_p(p) { if (m_p) {m_p->Lock(); } }
		~CCmdLockerT() { if (m_p) {m_p->Unlock(); }}
	};

	class CCmdMutex {
		HANDLE m_hMutex;
	public:
		explicit CCmdMutex(HANDLE hMutex = NULL) : m_hMutex(hMutex){
			if (m_hMutex) ::WaitForSingleObject(hMutex,INFINITE);
		}
		~CCmdMutex() { if (m_hMutex) ::ReleaseMutex(m_hMutex); }
	};

	template <typename T>
	class CCmdPtr {
		T* ptr;
	public:
		CCmdPtr(T* p) : ptr(p) {}
		CCmdPtr(): ptr(NULL)  { ptr = new T; }
		~CCmdPtr() { Destory(); }
	public:
		T* Detach() { T* TempPtr = ptr; ptr = NULL; return TempPtr; }
		void Attach(T* p) { Destory(); ptr = p; }
		void Destory() { __SAFE_DELETE_PTR(ptr); }
		T* operator->() { ATLASSERT(ptr); return ptr; }
		operator T*() { return ptr; }
	};

	template <typename T, typename TIID, LONG tObjectType = 0,typename TLock = CmdDefaultLocker>
	class ATL_NO_VTABLE CCmdObjectImpl : public TIID , public ICmdObjectSink, public ICmdObjectNotify
	{
	public:
		typedef typename TLock CmdLock;
		typedef typename CCmdLockerT<CmdLock> CmdLocker;
		CmdLock m_cs;
	public:
		CCmdObjectImpl() { /*Empty();*/ }
 	public:
		IMPLEMENT_PROPERTY(CStringA,CmdLastError);
		IMPLEMENT_GET_PROPERTY(bool,IsChanged);
		LONG GetObjectType() { return tObjectType; }
 		HRESULT Clone(void* ppObj) { return static_cast<T*>(this)->OnClone(ppObj); }
	public:
		void OnObjectChanged(ICmdObject* pObject,bool& bHandled) {} // 属性改变接收器
		void NotifyObjectChanged() {
			for (int i = 0; i < m_Sinks.GetSize(); i++) {
				if (m_Sinks[i]) {
					bool fHandled = false;
					m_Sinks[i]->OnObjectChanged(static_cast<ICmdObject*>(this),fHandled);
					if (fHandled)
						break;
				}
			}
		}
		void AttachObjectSink(ICmdObjectSink* pSink)
		{
			if (pSink && -1 == m_Sinks.Find(pSink)) {
				m_Sinks.Add(pSink);
			}
		}
		HRESULT ChangeProperty(LPCSTR pszName,CComVariant &varParam) 
		{
			return static_cast<T*>(this)->OnChangeProperty(pszName,varParam); 
		}
		HRESULT Empty() { CmdLocker(); m_IsChanged = false; return static_cast<T*>(this)->OnEmpty(); }
	public:
 		HRESULT OnClone(void* ppObj) { return E_NOTIMPL; }
  		HRESULT OnEmpty() { return S_OK; }
		HRESULT OnChangeProperty(LPCSTR pszName,CComVariant &varParam) { return E_NOTIMPL; }
	protected:
		ATL::CSimpleArray<ICmdObjectSink*> m_Sinks;
	};

	class CCmdCollection : public CCmdObjectImpl<CCmdCollection,ICmdCollection,cmdCollection> {
	public:
		LONG GetCount()
		{
			CmdLocker();
			return m_Objects.m_nSize ;
		}
		HRESULT Item(LONG nIndex, ICmdObject** ppItem)
		{
			CmdLocker();
			if (nIndex < 0 || nIndex >= m_Objects.m_nSize)
				return E_INVALIDARG;
			if (!ppItem)
				return E_POINTER;
			*ppItem = m_Objects.m_aT[nIndex];
			return S_OK;
		}
		HRESULT Add(ICmdObject* pItem)
		{
			CmdLocker();
			m_Objects.Add(pItem);
			return S_OK;
		}

		HRESULT Insert(ICmdObject* pItem, LONG nAfter)
		{
			return E_NOTIMPL;
		}

		HRESULT Remove(LONG nIndex,bool fDestory)
		{
			CmdLocker();
			if (nIndex < 0 || nIndex >= m_Objects.m_nSize)
				return E_INVALIDARG;
			if (fDestory) {
 				if (m_Objects.m_aT[nIndex]) {
					delete m_Objects.m_aT[nIndex];
					m_Objects.m_aT[nIndex] = NULL;
				}
			}
			m_Objects.RemoveAt(nIndex);
			return S_OK;
		}

		HRESULT RemoveItem(ICmdObject* pItem,bool fDestory = true)
		{
			int nIndex = -1;
			{
				CmdLocker();
				nIndex = m_Objects.Find(pItem);
			}
			return Remove(nIndex,fDestory);
		}

		HRESULT Clear(bool fDestory = true)
		{
			CmdLocker();
			if (fDestory) {
				for (int i = m_Objects.m_nSize -1; i >= 0; i--) {
					ICmdObject* p = m_Objects.m_aT[i];
					if (p) {
						delete p;
						p = NULL;
					}
				}
			}
			m_Objects.RemoveAll();
			return S_OK;
		}

		template <typename Q> 
		Q* New()
		{
			Q* p = new Q;
			Add(static_cast<ICmdObject*>(p));
			return p;
		}
		
		template <typename Q>
		Q* GetItem(LONG nIndex)
		{
			ICmdObject* p = NULL;
			HRESULT hr = Item(nIndex,&p);
			if (SUCCEEDED(hr))
				return static_cast<Q*>(p);
			return NULL;
		}
		
		ICmdObject* operator[](LONG nIndex)
		{
			if (nIndex < 0 || nIndex >= m_Objects.GetSize())
				return NULL;
			return m_Objects[nIndex];
		}

		template <typename Q>
		HRESULT Clone(ICmdCollection* pColls)
		{
			ATLASSUME(pColls != NULL);
 			if (pColls == static_cast<ICmdCollection*>(this)) {
				return S_OK;
			}
 			pColls->Clear();
			CmdLocker();
			for (int i = 0; i < m_Objects.GetSize(); i++) {
				Q* pObj = new Q;
				m_Objects[i]->Clone((void*)pObj);
				pColls->Add(pObj);
			}
			return S_OK;
		}
	public:
		ATL::CSimpleArray<ICmdObject*> m_Objects;
	};
	
	class CCmdProperty : public CCmdObjectImpl<CCmdProperty,ICmdProperty,cmdProperty>  {
	public:
		IMPLEMENT_PROPERTY(CStringA,Name)
		IMPLEMENT_PROPERTY(LONG, Type)
		IMPLEMENT_PROPERTY(CStringA,Value)
	public:
		INT asInt()
		{
			return StrToIntA((LPCSTR)m_Value);
		}
		__int64 asInt64()
		{
			LONGLONG nResult = 0;
			StrToInt64ExA((LPCSTR)m_Value,STIF_DEFAULT,&nResult);
			return nResult;
		}
		COleDateTime asDate()
		{
			CComVariant var((LPCSTR)m_Value);
			if (SUCCEEDED(var.ChangeType(VT_DATE)))
				return var.date;
			return 0.0f;
		}
	public:
		HRESULT OnEmpty() 
		{ 
			m_Name = m_Value = "";
			m_Type = 0;
			return S_OK; 
		}
		HRESULT OnClone(void* pObject)
		{
			CmdLocker();
			CCmdProperty* p = static_cast<CCmdProperty*>(pObject);
			p->m_Name = m_Name;
			p->m_Type = m_Type;
			p->m_Value = m_Value;
			return S_OK;
		}
	};

	class CCmdPropertys : public CCmdObjectImpl<CCmdPropertys,ICmdPropertys,cmdPropertys>  {
	public:
		ICmdCollection* GetPropertys()
		{
			return &m_Propertys;
		}
		ICmdProperty* FindProperty(LPCSTR lpszName)
		{
			CStringA sKey = lpszName;
			sKey.Trim().MakeLower();
			for (int i = 0; i < m_Propertys.Count; i++) {
				ICmdProperty* p = m_Propertys.GetItem<ICmdProperty>(i);
				if (p && p->Name == sKey) {
					return p;
				}
			}
			return NULL;
		}
		ICmdProperty* AddProperty(LPCSTR lpszKey,LPCSTR lpszValue, LONG nType = cmdString)
		{
			ICmdProperty* PropertyPtr = FindProperty(lpszKey);
			if (PropertyPtr) {
				PropertyPtr->Type = nType;
				PropertyPtr->Value = lpszValue;
			}
			else {
				CStringA sKey = lpszKey;
				sKey.Trim().MakeLower();
				PropertyPtr = m_Propertys.New<CCmdProperty>();
				PropertyPtr->Name = sKey;
				PropertyPtr->Type = nType;
				PropertyPtr->Value = lpszValue;
			}
			return PropertyPtr;
		}

		ICmdProperty* AddProperty(LPCSTR lpszKey,LONG nValue)
		{
			CStringA sValue;
			sValue.Format("%d",nValue);
			return AddProperty(lpszKey,(LPCSTR)sValue,cmdInt);
		}
		
		ICmdProperty* AddProperty(LPCSTR lpszKey,__int64 nValue)
		{
			CHAR szValue[64] = {0};
			_ui64toa(nValue,szValue,10);
			return AddProperty(lpszKey,szValue,cmdInt64);
		}

		ICmdProperty* AddProperty(LPCSTR lpszKey,COleDateTime tmValue)
		{
			CStringA sTime = CT2A(tmValue.Format());
			return AddProperty(lpszKey,(LPCSTR)sTime,cmdDate);
		}

		ICmdProperty* operator[](LONG nIndex)
		{
			return static_cast<ICmdProperty*>(m_Propertys[nIndex]);
		}
		ICmdProperty* operator[](LPCSTR lpszName)
		{
			return FindProperty(lpszName);
		}
	public:
		CCmdCollection m_Propertys;
	};

	class CCmdConnectInfo : public CCmdObjectImpl<CCmdConnectInfo,ICmdConnectInfo,cmdConnectInfo> {
	public:
		CCmdConnectInfo(CStringA sHost = "", WORD nPort = 80, CStringA sUser = "", CStringA sPass = "", CStringA sProxy = "", CStringA sBypass = "")
 		{
			CmdLocker();
			m_Host = sHost;
			m_Port = nPort;
			m_User = sUser;
			m_Pass = sPass;
			m_Proxy = sProxy;
			m_Bypass = sBypass;
			m_Byuser = "";
			m_Byport = 80;
		}
	public:
		IMPLEMENT_PROPERTY(CStringA,Host);
		IMPLEMENT_PROPERTY(WORD,Port);
		IMPLEMENT_PROPERTY(CStringA,Url);
		IMPLEMENT_PROPERTY(CStringA,User);
		IMPLEMENT_PROPERTY(CStringA,Pass);
		IMPLEMENT_PROPERTY(CStringA,Proxy);
		IMPLEMENT_PROPERTY(CStringA,Bypass);
		IMPLEMENT_PROPERTY(CStringA,Byuser);
		IMPLEMENT_PROPERTY(WORD,Byport);
	public:
		HRESULT SetServiceUrl(LPCSTR lpszUrl)
		{
 			TCHAR szHost[MAX_PATH] = {0};
			TCHAR szUrl[MAX_PATH] = {0};
			WORD  nPort = 80;

			INET_GetHost(CA2T(lpszUrl),szHost,szUrl,nPort);
			CmdLocker();
			m_Host = CT2A(szHost);
			m_Url = CT2A(szUrl);
			m_Port = nPort;
			return S_OK;
		}
	public:
		HRESULT OnEmpty() 
		{ 
			m_Host = m_User = m_Pass = m_Proxy = m_Byuser = m_Bypass = m_Url = "";
			m_Port = m_Byport = 80;
			return S_OK; 
		}
		HRESULT OnClone(void* pObject)
		{
			CmdLocker();
			CCmdConnectInfo* p = static_cast<CCmdConnectInfo*>(pObject);
			p->Host = m_Host;
			p->Port = m_Port;
			p->User = m_User;
			p->Pass = m_Pass;
			p->Url = m_Url;
			p->Proxy = m_Proxy;
			p->Bypass = m_Bypass;
			p->Byuser = m_Byuser;
			return S_OK;
		}
 	};

	class CCmdConnection : public CCmdObjectImpl<CCmdConnection,ICmdConnection,cmdConnection> {
	public:
		IMPLEMENT_PROPERTY(CStringA,Session);
		IMPLEMENT_GET_PROPERTY(LONG,State)
		IMPLEMENT_GET_PROPERTY(bool,IsConnected)
	public:
		ICmdConnectInfo* GetConnectInfo() { CmdLocker(); return &m_ConnectInfo; }
	public:
		CCmdConnection() : CCmdObjectImpl()
			, m_pSession(NULL)
			, m_pConnect(NULL)
		{
			m_ConnectInfo.AttachObjectSink(static_cast<ICmdObjectSink*>(this));
		}
		HRESULT Connect(bool fReconn = false)
		{
			if (fReconn) {
				Close();
			}
			if (!IsConnected && m_ConnectInfo.Host == "") {
				SetCmdLastError("尚未设定有效的主机信息");
				return E_FAIL;
			}
			try {
				CmdLocker();
				if (!m_pSession) {
					if (m_ConnectInfo.Proxy == "") {
						m_pSession = new CInternetSession(_T("SFSoft_CmdServiceConnectV1.0"));
					}
					else {
						CStringA sProxy;
						sProxy.Format("%s:%d",m_ConnectInfo.Proxy,m_ConnectInfo.Byport);
						m_pSession = new CInternetSession(_T("SFSoft_CmdServiceConnectV1.0"),INTERNET_OPEN_TYPE_PROXY,(LPCSTR)sProxy);
						if (m_ConnectInfo.Byuser != "") {
							m_pSession->SetOption(INTERNET_OPTION_PROXY_USERNAME,m_ConnectInfo.Byuser);
						}
						if (m_ConnectInfo.Bypass != "") {
							m_pSession->SetOption(INTERNET_OPTION_PROXY_PASSWORD,m_ConnectInfo.Bypass);
						}
					}
					if (m_ConnectInfo.User != "") {
						m_pSession->SetOption(INTERNET_OPTION_USERNAME,m_ConnectInfo.User);
					}
					if (m_ConnectInfo.Pass != "") {
						m_pSession->SetOption(INTERNET_OPTION_PASSWORD,m_ConnectInfo.Pass);
					}
				}
				if (!m_pConnect) {
					m_pConnect = new CHttpConnection(*m_pSession,m_ConnectInfo.Host,m_ConnectInfo.Port);
				}
				m_IsConnected = true;
			}
			catch(CInternetException &err) {
				CStringA sMsg;
				sMsg.Format("创建网络连接错误！错误码:%d, 错误描述:%s", err.GetErrorCode(), CT2A(err.GetInternetErrorMessage()));
				SetCmdLastError(sMsg);
				Close();
				return E_FAIL;
			}
			return S_OK;
		}
		HRESULT Execute(LPCSTR lpszUrl, CCommandDocument* pCmd, CResponeDocument& doc,LONG nHashCode = Sign::MAC_Hash() )
		{
			CConnectLocker Locker; 
			Connect(false);
			if (!IsConnected) {
				doc.SetLastError(CMD_ERROR_NET,CMD_MSG_NOTCONNECT);
				SetCmdLastError(CMD_MSG_NOTCONNECT);
				return E_ABORT;
			}
			if (pCmd->ErrorCode()) {
				doc.SetCommand(pCmd->Command);
				doc.SetLastError(pCmd->ErrorCode(),pCmd->Description);
				return E_FAIL;
			}
			if (m_Session != "") {
				pCmd->SetParameter("sessionid",m_Session);
			}
			CStringA sJson = pCmd->GetDocument(nHashCode);
			if (sJson == "" ) {
				doc.SetLastError(pCmd->ErrorCode(),pCmd->Description);
				return E_FAIL;
			}
			CStringA sPostUrl = lpszUrl;
			CStringA sUrl = lpszUrl;
			sUrl.Trim();
			if (sUrl.Left(1) == "?") {
				sPostUrl = m_ConnectInfo.Url;
				sPostUrl += sUrl;
			}
			try {
				const int BuffSize = 8192;
				CHAR ReadBuffer[BuffSize] = {0};
				CCmdTempBuffer<BuffSize> RecvBuffer;
				CHttpFile file(*m_pConnect,_T("POST"),sPostUrl,_T("1.1"));
				file.SendRequest(NULL,0,(void*)(LPCSTR)sJson,sJson.GetLength());
				for (DWORD dwRead; dwRead = file.Read(ReadBuffer,BuffSize);) {
					RecvBuffer.Append(ReadBuffer,dwRead);
				}
				doc.SetDocument(RecvBuffer);
			}
			catch(CInternetException &err) {
				CStringA sMsg;
				sMsg.Format("创建网络连接错误！错误码:%d, 错误描述:%s", err.GetErrorCode(), CT2A(err.GetInternetErrorMessage()));
				SetCmdLastError(sMsg);
				doc.SetLastError(CMD_ERROR_NET,"连接服务器失败！请稍候重试连接");
				return E_FAIL;
			}
			return S_OK;
		}

		HRESULT Close()
		{
			CmdLocker();
			if (m_pConnect) {
 				delete m_pConnect;
				m_pConnect = NULL;
			}
			if (m_pSession) {
 				delete m_pSession;
				m_pSession = NULL;
			}
			OnEmpty();
			return S_OK;
		}
		HRESULT SetUrl(LPCSTR lpszUrl)
		{
			CCmdPtr<CCmdConnectInfo> conn;
			conn->SetServiceUrl(lpszUrl);
			if (conn->Host != m_ConnectInfo.Host || conn->Port != m_ConnectInfo.Port) {
				conn->Clone(&m_ConnectInfo);
				Connect(true);
			}
 			return S_OK;
		}
		HRESULT OnEmpty()
		{
			m_IsConnected = false;
			m_State = 0;
			m_Session = "";
			return S_OK;
		}
		HRESULT OnClone(void* pObject)
		{
			CmdLocker();
			CCmdConnection* p = static_cast<CCmdConnection*>(pObject);
			m_ConnectInfo.Clone(p->ConnectInfo);
			p->Connect(true);
			p->Session = m_Session;
			return S_OK;
		}
	public:
		CCmdConnectInfo m_ConnectInfo;
	protected:
		CInternetSession *m_pSession;
		CHttpConnection  *m_pConnect;
	};

	class CCmdServer: public CCmdObjectImpl<CCmdServer,ICmdServer,cmdServer>{  
		IMPLEMENT_GET_PROPERTY(LONG, ServerID)
		IMPLEMENT_PROPERTY(CStringA, Caption)
		IMPLEMENT_PROPERTY(CStringA, Server)
		IMPLEMENT_PROPERTY(CStringA, Url)
		IMPLEMENT_PROPERTY(WORD, Port)
		IMPLEMENT_PROPERTY(CStringA, UserName)
		IMPLEMENT_PROPERTY(CStringA, Password)
		IMPLEMENT_PROPERTY(WORD, FtpPort)
		IMPLEMENT_PROPERTY(CStringA, FtpUser)
		IMPLEMENT_PROPERTY(CStringA, FtpPass)
		IMPLEMENT_PROPERTY(CString, Description)
		IMPLEMENT_GET_PROPERTY(LONG, State)
		IMPLEMENT_GET_PROPERTY(COleDateTime, CreateTime)
		IMPLEMENT_GET_PROPERTY(COleDateTime, LastTime)
	};

	class CCmdFTP : public CCmdObjectImpl<CCmdFTP,ICmdFTP,cmdFTP>{
		IMPLEMENT_GET_PROPERTY(LONG, ServerID)
		IMPLEMENT_GET_PROPERTY(LONG, Type)
 		IMPLEMENT_PROPERTY(CStringA, Host)
 		IMPLEMENT_PROPERTY(WORD, Port)
		IMPLEMENT_PROPERTY(CStringA, UserName)
		IMPLEMENT_PROPERTY(CStringA, Password)
		IMPLEMENT_PROPERTY(CStringA, PhyPath)
	};

	class CCmdMachine : public CCmdObjectImpl<CCmdMachine,ICmdMachine,cmdMachine>{
	public:
		IMPLEMENT_GET_PROPERTY(LONG,MachineID)
		IMPLEMENT_GET_PROPERTY(CStringA,RegCode)
		IMPLEMENT_GET_PROPERTY(LONG,Days)
		IMPLEMENT_GET_PROPERTY(LONG,State)
		IMPLEMENT_GET_PROPERTY(LONG,Style)
	public:
		LPCSTR GetMacAddress() {
			static CHAR szMac[64] = {0};
			if (!szMac[0]) {
				StringTool::MacString(szMac,64);
			}
			return szMac;
		}
	public:
		HRESULT OnEmpty() {
			m_RegCode = "";
			m_MachineID = m_Days = m_State = m_Style = 0;
			return S_OK;
		}
		HRESULT OnClone(void* pObject)
		{
			CmdLocker();
			CCmdMachine* p = static_cast<CCmdMachine*>(pObject);
			p->m_MachineID = m_MachineID;
			p->m_RegCode = m_RegCode;
			p->m_Days = m_Days;
			p->m_State = m_State;
			p->m_Style = m_Style;
			return S_OK;
		}
	};

	class CCmdProduct : public CCmdObjectImpl<CCmdProduct,ICmdProduct,cmdProduct> {
	public:
		IMPLEMENT_GET_PROPERTY(LONG,ProductID)
		IMPLEMENT_GET_PROPERTY(CStringA,RegCode)
 		IMPLEMENT_GET_PROPERTY(LONG,Days)
		IMPLEMENT_GET_PROPERTY(LONG,State)
		IMPLEMENT_GET_PROPERTY(LONG,Style)
	public:
		HRESULT OnEmpty() {
			m_RegCode = "";
			m_ProductID = m_Days = m_State = m_Style = 0;
			return S_OK;
		}
	};

	class CCmdAccountRecode : public CCmdObjectImpl<CCmdAccountRecode,ICmdAccountRecode,cmdAccountRecode> {
	public:
		IMPLEMENT_GET_PROPERTY(LONG,AccountID)
		IMPLEMENT_GET_PROPERTY(LONG,Type)
		IMPLEMENT_GET_PROPERTY(LONG,Money)
		IMPLEMENT_GET_PROPERTY(COleDateTime,CreateTime)
		IMPLEMENT_PROPERTY(CStringA,Memo)
	public:
		HRESULT OnEmpty()
		{
			m_AccountID = m_Type = m_Money = 0;
			m_Memo = "";
			m_CreateTime = 0.0f;
			return S_OK;
		}
	};
	class CCmdAccountQuery : public CCmdObjectImpl<CCmdAccountQuery,ICmdAccountQuery,cmdAccountQuery> {
	public:
		IMPLEMENT_GET_PROPERTY(LONG,Balance)
		IMPLEMENT_GET_PROPERTY(LONG,Pay)
		IMPLEMENT_GET_PROPERTY(LONG,Free)
		ICmdCollection* GetRecords() { CmdLocker(); return &m_Records; }

		LRESULT OnEmpty()
		{
			m_Balance = m_Pay = m_Free = 0;
			m_Records.Clear();
			return 0;
		}
		CCmdCollection m_Records;
	};

	class CCmdFile : public CCmdObjectImpl<CCmdFile,ICmdFile,cmdFile> {
	public:
		IMPLEMENT_PROPERTY(LONGLONG,RemoteID);
		IMPLEMENT_PROPERTY(CStringA,RemoteFile);
		IMPLEMENT_PROPERTY(CStringA,RemotePath);
		IMPLEMENT_PROPERTY(CStringA,LocalFile);
		IMPLEMENT_PROPERTY(CStringA,LocalPath);
		IMPLEMENT_PROPERTY(CStringA,MD5);
 		IMPLEMENT_PROPERTY(LONG,RemoteType);
		IMPLEMENT_PROPERTY(LONG,FileType);
		IMPLEMENT_PROPERTY(LONG,FileState);
		IMPLEMENT_PROPERTY(LONG,FileStyle);
		IMPLEMENT_PROPERTY(LONGLONG,FileSize);
		IMPLEMENT_PROPERTY(LONGLONG,WriteBytes);
		IMPLEMENT_PROPERTY(COleDateTime,CreateTime);
		IMPLEMENT_PROPERTY(COleDateTime,LastTime);
		IMPLEMENT_PROPERTY(CStringA,Memo);
		IMPLEMENT_GET_PROPERTY(bool, IsUpload);
		IMPLEMENT_PROPERTY(LONG,State);
	public:
		LRESULT OnEmpty() {
			m_RemoteID = m_FileSize = m_WriteBytes = 0;
			m_RemoteType = m_FileState = m_FileType = m_FileStyle = m_State = 0;
			m_RemoteFile = m_RemotePath = m_LocalFile = m_LocalPath = m_Memo = m_MD5 ="";
			m_IsUpload = false;
			return 0;
		}
		LRESULT OnClone(void* pObj)
		{
			ATLASSUME(pObj != NULL);
			CmdLocker();
			CCmdFile *p = static_cast<CCmdFile*>(pObj);
			p->m_RemoteID = m_RemoteID;
			p->m_RemoteFile = m_RemoteFile;
			p->m_RemotePath = m_RemotePath;
			p->m_LocalFile = m_LocalFile;
			p->m_LocalPath = m_LocalPath;
			p->m_RemoteType = m_RemoteType;
			p->m_FileType = m_FileType;
			p->m_FileState = m_FileState;
			p->m_FileStyle = m_FileStyle;
			p->m_FileSize = m_FileSize;
			p->m_WriteBytes = m_WriteBytes;
			p->m_CreateTime = m_CreateTime;
			p->m_LastTime = m_LastTime;
			p->m_Memo = m_Memo;
			p->m_MD5 = m_MD5;
			p->m_IsUpload = m_IsUpload;
			p->m_State = m_State;
			return 0;
		}
	};

	class CCmdFileList : public CCmdObjectImpl<CCmdFileList,ICmdFileList,cmdFileList> {
	public:
		IMPLEMENT_GET_PROPERTY(LONG,PageNumber);
		IMPLEMENT_GET_PROPERTY(LONG,PageCount);
		IMPLEMENT_GET_PROPERTY(LONG,PageRows);
		ICmdCollection* GetFileList()
		{
			return &m_FileList;
		}
	public:
		LRESULT OnEmpty()
		{
			m_PageNumber = 0; m_PageCount = 0; m_PageRows = 0;
			m_FileList.Clear();
			return 0;
		}
		HRESULT OnClone(void* pObject)
		{
			CmdLocker();
			CCmdFileList* p = static_cast<CCmdFileList*>(pObject);
			p->m_PageNumber = m_PageNumber;
			p->m_PageCount = m_PageCount;
			p->m_PageRows = m_PageRows;
			p->m_FileList.Clear();
			m_FileList.Clone<CCmdFile>(&p->m_FileList);
 			return S_OK;
		}
	public:
		CCmdCollection m_FileList;
	};

	class CCmdUser : public CCmdObjectImpl<CCmdUser, ICmdUser, cmdUser> {
	public:
		ICmdClient* m_pClient ;
	public:
		IMPLEMENT_GET_PROPERTY(LONG,UserID)
		IMPLEMENT_PROPERTY(LONG,UserType)
		IMPLEMENT_GET_PROPERTY(LONG,State)
		IMPLEMENT_GET_PROPERTY(LONG,Style)
		IMPLEMENT_GET_PROPERTY(LONG,Days)
		IMPLEMENT_GET_PROPERTY(LONG,Rights)
		IMPLEMENT_GET_PROPERTY(LONG,UpThreads)
		IMPLEMENT_GET_PROPERTY(LONG,DownThreads)
		IMPLEMENT_GET_PROPERTY(LONG,ShowAccount)

		IMPLEMENT_PROPERTY(CStringA,UserName)
		IMPLEMENT_PROPERTY(CStringA,Password)
		IMPLEMENT_PROPERTY(CStringA,QQ)
		IMPLEMENT_PROPERTY(CStringA,Email)
		IMPLEMENT_PROPERTY(CStringA,Mobile)
		IMPLEMENT_PROPERTY(CStringA,NickName)
		IMPLEMENT_PROPERTY(CStringA,Question)
		IMPLEMENT_PROPERTY(CStringA,Anwser)

		IMPLEMENT_PROPERTY(LONG,ServerID)
		IMPLEMENT_PROPERTY(LONG,FtpType)
		IMPLEMENT_PROPERTY(CStringA,FtpHost)
		IMPLEMENT_PROPERTY(WORD,FtpPort)
		IMPLEMENT_PROPERTY(CStringA,FtpUser)
		IMPLEMENT_PROPERTY(CStringA,FtpPass)
		IMPLEMENT_PROPERTY(CStringA,FtpPath)
		IMPLEMENT_PROPERTY(CStringA,Prov)
		IMPLEMENT_PROPERTY(CStringA,City)
		IMPLEMENT_PROPERTY(CStringA,Zip)
		IMPLEMENT_PROPERTY(CStringA,Conuty)
		IMPLEMENT_PROPERTY(CStringA,Co)
		IMPLEMENT_PROPERTY(CStringA,Address)
		IMPLEMENT_PROPERTY(CStringA,RecvMan)
		IMPLEMENT_PROPERTY(CStringA,Fax)
		IMPLEMENT_PROPERTY(CStringA,Phone)
		IMPLEMENT_PROPERTY(CStringA,CreateTime)
		IMPLEMENT_PROPERTY(CStringA,Template)
		IMPLEMENT_PROPERTY(LONG,Space)
		IMPLEMENT_PROPERTY(LONG,Quality)
		
		CmdStringArray m_Filters;
		CmdStringArray& GetFilters() { return m_Filters; }
	public:
		CCmdUser() : m_pClient(NULL) {}
		LRESULT OnEmpty()
		{
			m_UserID = m_ServerID = m_FtpType = m_UserType =0;
			m_State = m_Style = m_Days = m_Rights = 0;
			m_ShowAccount = 1; 
			m_FtpPort = 21;
			m_Quality = 80;
			m_Space = 100;
			m_UpThreads = m_DownThreads = 0; 
			m_UserName = m_Password = m_QQ = m_Email = m_Mobile = m_NickName = m_Question = "";
			m_FtpHost = m_FtpUser = m_FtpPass = m_FtpPath = m_Template = m_Anwser = "";
			m_Fax = m_Phone = m_RecvMan = m_Address = m_Co = m_Conuty = m_Zip = m_City = m_Prov = "";
			m_Filters.RemoveAll();
			return S_OK;
		}
	public:
		HRESULT Register()
		{
			return S_OK;
		}
		HRESULT Refresh()
		{
			if (!m_pClient) 
				return E_POINTER;

			CResponeDocument Respone;
			CCommandDocument Request("user.refresh");
 			HRESULT hr = m_pClient->Connection->Execute("?User",&Request,Respone);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				CmdLastError = Respone.Description;
				return E_FAIL;
			}
			Json::Value &value = Respone["userinfo"];
			CmdLocker();
			m_ServerID = value["serverid"].asInt();
			m_UserType = value["utype"].asInt();
			m_FtpType = value["type"].asInt();
			m_State = value["state"].asInt();
			m_ShowAccount = value["show"].asInt(); 
			m_Rights = value["rights"].asInt();
			m_FtpPort = value["port"].asInt();
			m_QQ = value["qq"].asString().c_str();
			m_Email = value["email"].asString().c_str();
			m_NickName = value["linkman"].asString().c_str();
			m_Question = value["query"].asString().c_str();
			m_FtpHost = value["host"].asString().c_str();
			m_FtpUser = value["ftpuser"].asString().c_str();
			m_FtpPass = value["ftppass"].asString().c_str();
			m_FtpPath = value["pypath"].asString().c_str();
			m_Fax  = value["fax"].asString().c_str();
			m_Phone = value["phone"].asString().c_str();
			m_Mobile = value["mobile"].asString().c_str();
			m_RecvMan = value["recvman"].asString().c_str();
			m_Address = value["address"].asString().c_str();
			m_Co = value["co"].asString().c_str();
			m_Conuty = value["county"].asString().c_str();
			m_Zip = value["zip"].asString().c_str();
			m_City = value["city"].asString().c_str();
			m_Prov = value["prov"].asString().c_str();
			m_CreateTime = value["ctime"].asString().c_str();
			m_UpThreads = value["uthreads"].asInt();
			m_DownThreads = value["dthreads"].asInt();
			CString sFilter ;
			if (value.isMember("filter")) {
				sFilter = value["filter"].asString().c_str();
			}
			else {
				sFilter = DEF_FILTER;
			}
			sFilter.Trim().MakeLower();
			m_Filters.RemoveAll();

			INT nStart = 0;
			CStringA sTemp = sFilter.Tokenize(";",nStart);
			while(sTemp != "") {
				sTemp.Trim().MakeLower();
				m_Filters.Add(sTemp);
				sTemp = sFilter.Tokenize(";",nStart);
			}
			return S_OK;
		}
		HRESULT Save()
		{
			return S_OK;
		}
		HRESULT PropectPassword(CStringA Query,CStringA answer) 
		{
			if (!m_pClient) {
				CmdLastError = "尚未登录";
				return E_POINTER;
			}
		
			if (Query == "" || answer == "") {
				CmdLastError = "密码保护提示或回复为空";
				return E_FAIL;
			}

			CResponeDocument Respone;
			CCommandDocument Request("user.update");

			Json::Value value;
			value["query"] = (LPCSTR)Query;
			value["answer"] = (LPCSTR)answer;
			Request["userrecvpass"] = value;
			HRESULT hr = m_pClient->Connection->Execute("?User",&Request,Respone);
			CmdLastError = Respone.Description;
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				return E_FAIL;
			}
 			return S_OK;
		}

		HRESULT ChangePass(CStringA sOldPass, CStringA sNewPass)
		{
			if (!m_pClient) {
				CmdLastError = "尚未登录";
				return E_POINTER;
			}
		
			if (sOldPass != Password) {
				CmdLastError = "更新密码失败！旧密码验证错误";
				return E_FAIL;
			}

			CResponeDocument Respone;
			CCommandDocument Request("user.update");

			Json::Value value;
			value["username"] = (LPCSTR)m_UserName;
			value["newpass"] = (LPCSTR)sNewPass;
			Request["changepass"] = value;
			HRESULT hr = m_pClient->Connection->Execute("?User",&Request,Respone);
			CmdLastError = Respone.Description;
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				return E_FAIL;
			}
			Password = sNewPass;
 			return S_OK;
		}
		HRESULT GetAccountRecords(LONG nDays, ICmdAccountQuery* pQuery) 
		{
			if (!m_pClient) 
				return E_POINTER;

			CResponeDocument Respone;
			CCommandDocument Request("account.records");
			Request["days"] = nDays;
			HRESULT hr = m_pClient->Connection->Execute("?Account",&Request,Respone);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				CmdLastError = Respone.Description;
				return E_FAIL;
			}

			CCmdAccountQuery* p = static_cast<CCmdAccountQuery*>(pQuery);
			p->Empty();
			CmdLocker Locker(&p->m_cs);
			CCmdCollection* pRecords = static_cast<CCmdCollection*>(pQuery->Records);
			p->m_Balance = Respone["balance"].asInt();
			p->m_Pay = Respone["pay"].asInt();
			p->m_Free = Respone["free"].asInt();

			if (Respone.HaveParameter("datalist")) {
				Json::Value &list = Respone["datalist"];
				for (int i = 0; i < list.size(); i++) 
				{
					CCmdAccountRecode* p = pRecords->New<CCmdAccountRecode>();
					p->m_AccountID = list[i]["accountid"].asInt();
					p->m_Money = list[i]["money"].asInt();
					p->m_Type = list[i]["type"].asInt();
					
					COleDateTime sTime(CComVariant(list[i]["ctime"].asString().c_str()));
					p->m_CreateTime = sTime;
					p->m_Memo = list[i]["memo"].asString().c_str();
					
				}
			}
			return S_OK;
		}
		HRESULT GetAccountRecordsEx(COleDateTime& Start, COleDateTime& End, ICmdAccountQuery* pQuery) 
		{
			if (!m_pClient) 
				return E_POINTER;

			CResponeDocument Respone;
			CCommandDocument Request("account.records");
			Json::Value param;
			param["start"] = (LPCSTR)Start.Format();
			param["end"] = (LPCSTR)End.Format();

			Request["custom"] = param;
			HRESULT hr = m_pClient->Connection->Execute("?Account",&Request,Respone);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				CmdLastError = Respone.Description;
				return E_FAIL;
			}

			CCmdAccountQuery* p = static_cast<CCmdAccountQuery*>(pQuery);
			p->Empty();
			CmdLocker Locker(&p->m_cs);
			CCmdCollection* pRecords = static_cast<CCmdCollection*>(pQuery->Records);
			p->m_Balance = Respone["balance"].asInt();
			p->m_Pay = Respone["pay"].asInt();
			p->m_Free = Respone["free"].asInt();

			if (Respone.HaveParameter("datalist")) {
				Json::Value &list = Respone["datalist"];
				for (int i = 0; i < list.size(); i++) 
				{
					CCmdAccountRecode* p = pRecords->New<CCmdAccountRecode>();
					p->m_AccountID = list[i]["accountid"].asInt();
					p->m_Money = list[i]["money"].asInt();
					p->m_Type = list[i]["type"].asInt();
					
					COleDateTime sTime(CComVariant(list[i]["ctime"].asString().c_str()));
					p->m_CreateTime = sTime;
					p->m_Memo = list[i]["memo"].asString().c_str();
					
				}
			}
			return S_OK;
		}

		HRESULT GetUserSpace(Json::Value &value)
		{
			if (!m_pClient) 
				return E_POINTER;
			CResponeDocument Respone;
			CCommandDocument Request("user.query");
			Request["type"] = "space";

			HRESULT hr = m_pClient->Connection->Execute("?User",&Request,Respone);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				CmdLastError = Respone.Description;
				return E_FAIL;
			}
			value = Respone.Document ;
			return S_OK;
		}

		HRESULT GetFileList(cmdFolderType cmdType,ICmdFileList* pFileList, int nPage = 0, int nRows = 30)
		{
			ATLASSUME(pFileList != NULL);

			CResponeDocument Respone;
			CCommandDocument Request("file.list");
			Request["page_no"] = nPage;
			Request["page_rows"] = nRows;

			switch (cmdType) 
			{
			case cmdDownload:
				{
					Request["type"] = "download";
					Request["subtype"] = "all";
				}
				break;
			case cmdUnDownload:
				{
					Request["type"] = "download";
					Request["subtype"] = "no";
				}
				break;
			case cmdDownloded :
				{
					Request["type"] = "download";
					Request["subtype"] = "yes";
				}
				break;
			case cmdDownRecy:
				{
					Request["type"] = "download";
					Request["subtype"] = "del";
				}
				break;
			case cmdUpload:
				{
					Request["type"] = "upload";
					Request["subtype"] = "nodel";
				}
				break;
			case cmdUploadRecy:
				{
					Request["type"] = "upload";
					Request["subtype"] = "del";
				}
				break;
			case cmdTemplate:
				break;
			};

			HRESULT hr = m_pClient->Connection->Execute("?File",&Request,Respone);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				CmdLastError = Respone.Description;
				return E_FAIL;
			}
			CmdLocker();
			
			CCmdFileList* p = static_cast<CCmdFileList*>(pFileList);
			p->Empty();
			p->m_PageCount = Respone["page_count"].asInt();
			p->m_PageRows = Respone["page_rows"].asInt();
			p->m_PageNumber = Respone["page_no"].asInt();

			INT ResultRows = Respone["ret_rows"].asInt();
			if (ResultRows > 0) {
				Json::Value result = Respone["filelist"];
				for (Json::UInt i = 0; i < result.size(); i++) {
					Json::Value item = result[i];
					CCmdFile* pFile = p->m_FileList.New<CCmdFile>();
					pFile->m_RemoteID = item["id"].asInt64();
					pFile->m_RemoteFile = item["filename"].asString().c_str();
					pFile->m_FileSize = item["filesize"].asInt64();
					pFile->m_FileType = item["type"].asInt();
 					pFile->m_State = item["state"].asInt();
					pFile->m_FileStyle = item["style"].asInt();
					pFile->m_CreateTime = COleDateTime(CComVariant(item["ctime"].asString().c_str()));
					pFile->m_LastTime = COleDateTime(CComVariant(item["ltime"].asString().c_str()));
					if (item.isMember("filestate")) {
						pFile->m_FileState = item["filestate"].asInt();
					}
					if (item.isMember("localpath")) {
						pFile->m_LocalPath = item["localpath"].asString().c_str();
						pFile->m_IsUpload = true;
					}
					else {
						pFile->m_IsUpload = false;
					}

					if (item.isMember("localfile")) {
						pFile->m_LocalFile = item["localfile"].asString().c_str();
					}
				}
			}
			return 0;
		}

		HRESULT UploadFile(HWND hWnd, LPCSTR lpszLocalPath, LPCSTR lpszLocalFile, LONGLONG nFileSize, LPCSTR lpszMD5, LPCSTR lpszMemo, ICmdFile** ppCmdFile, DWORD dwFlags = 0)
		{
			CResponeDocument Respone;
			CCommandDocument Request("file.upload");

			Request["must"] = false;
			Request["localpath"] = lpszLocalPath;
			Request["localfile"] = lpszLocalFile;
			Request["md5"] = lpszMD5;
			Request["memo"] = lpszMemo;
			Request["filesize"] = nFileSize;

			HRESULT hr = m_pClient->Connection->Execute("?File",&Request,Respone);
			if (Respone.ErrorCode() == 200) {
				if (IDYES == MessageBox(hWnd,Respone.Description,"重新上传",MB_YESNO | MB_ICONQUESTION)) {
					Request["must"] = true;
					Respone.Destroy();
					hr = m_pClient->Connection->Execute("?File",&Request,Respone);
				}
			}
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				CmdLastError = Respone.Description;
				return E_FAIL;
			}

			Json::Value result = Respone["cmdfile"];
			CCmdFile* p = new CCmdFile;
			p->Empty();
			p->m_RemoteID = result["fileid"].asInt64();
			p->m_FileSize = result["filesize"].asInt64();
			p->m_FileState = result["filestate"].asInt();
			p->m_FileStyle = result["style"].asInt();
			p->m_MD5 = result["md5"].asString().c_str();
			p->m_LocalPath = result["localpath"].asString().c_str();
			p->m_LocalFile = result["localfile"].asString().c_str();
			p->m_RemoteFile = result["filename"].asString().c_str();
			p->m_RemotePath = result["filepath"].asString().c_str();
			p->m_RemoteType = result["type"].asInt();
			p->State = result["state"].asInt();
			p->m_CreateTime = CComVariant(CComBSTR(result["ctime"].asString().c_str()));
			p->m_LastTime = CComVariant(CComBSTR(result["ltime"].asString().c_str()));
			p->m_Memo = result["memo"].asString().c_str();
			*ppCmdFile = static_cast<ICmdFile*>(p);
			return S_OK;
			
		}

		HRESULT WriteFileInfo(BOOL fUpload, LONG nFileID, ULONGLONG nWriteBytes , LONG nFileState, LPCSTR lpszParam)
		{
			CResponeDocument Respone;
			CCommandDocument Request("file.write");

			Request["fileid"] = nFileID;
			if (fUpload) {
				Request["type"] = "upload";
				if (nWriteBytes > 0) {
					Request["writebytes"] = (Json::Int64)nWriteBytes;
				}
				if (lpszParam) {
					Request["memo"] = lpszParam;
				}
				if (nFileState >= -1 && nFileState < 3) {
					Request["filestate"] = nFileState;
				}
			}
			else {
				Request["type"] = "download";
				if (nWriteBytes > 0) {
					Request["writebytes"] = (Json::Int64)nWriteBytes;
				}
				if (nFileState >= -1 && nFileState < 3) {
					Request["state"] = nFileState;
				}
				if (lpszParam) {
					Request["localfile"] = lpszParam;
				}
			}
			HRESULT hr = m_pClient->Connection->Execute("?File",&Request,Respone);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				CmdLastError = Respone.Description;
				return E_FAIL;
			}
 			return hr;
		}

		HRESULT DownloadFile(LONG nFileID, ICmdFile** ppCmdFile)
		{
			CResponeDocument Respone;
			CCommandDocument Request("file.download");

			Request["fileid"] = nFileID;
			HRESULT hr = m_pClient->Connection->Execute("?File",&Request,Respone);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				CmdLastError = Respone.Description;
				return E_FAIL;
			}

			Json::Value result = Respone["result"];

			CCmdPtr<CCmdFile> FilePtr;
			FilePtr->Empty();
			FilePtr->m_RemoteID = result["downid"].asInt64();
			FilePtr->m_RemoteFile = result["filename"].asString().c_str();
			FilePtr->m_LocalFile = result["localfile"].asString().c_str();
			FilePtr->m_FileSize = result["filesize"].asInt64();
			FilePtr->m_FileState = result["state"].asInt();
			FilePtr->m_FileType = result["filetype"].asInt();
			FilePtr->m_MD5 = result["md5"].asString().c_str();
			FilePtr->State = result["state"].asInt();
			FilePtr->m_CreateTime = CComVariant(CComBSTR(result["ctime"].asString().c_str()));
			FilePtr->m_LastTime = CComVariant(CComBSTR(result["ltime"].asString().c_str()));
			*ppCmdFile = FilePtr.Detach();
			return S_OK;
		}

		HRESULT DeleteFile(LPCSTR pszType, LPCSTR pszSubType, LPCSTR pszCmd, ICmdFile* pCmdFile)
		{
			CResponeDocument Respone;
			CCommandDocument Request("file.delete");

			Request["fileid"] = pCmdFile->RemoteID;
			if (pszType) {
				Request["type"] = pszType;
			}
			if (pszSubType) {
				Request["subtype"] = pszSubType;
			}
			if (pszCmd) {
				Request["cmd"] = pszCmd;
			}
			HRESULT hr = m_pClient->Connection->Execute("?File",&Request,Respone);
			CmdLastError = Respone.Description;
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				return E_FAIL;
			}
			return S_OK;
		}

		HRESULT ValueToFile(Json::Value &item, CCmdFile* pFile)
		{
			if (!pFile) return E_POINTER;
			pFile->m_RemoteID = item["id"].asInt64();
			pFile->m_RemoteFile = item["filename"].asString().c_str();
			pFile->m_FileSize = item["filesize"].asInt64();
			pFile->m_FileType = item["type"].asInt();
			pFile->m_State = item["state"].asInt();
			pFile->m_FileStyle = item["style"].asInt();
			pFile->m_CreateTime = COleDateTime(CComVariant(item["ctime"].asString().c_str()));
			pFile->m_LastTime = COleDateTime(CComVariant(item["ltime"].asString().c_str()));
			if (item.isMember("filestate")) {
				pFile->m_FileState = item["filestate"].asInt();
			}
			if (item.isMember("localpath")) {
				pFile->m_LocalPath = item["localpath"].asString().c_str();
				pFile->m_IsUpload = true;
			}
			else {
				pFile->m_IsUpload = false;
			}
			if (item.isMember("localfile")) {
				pFile->m_LocalFile = item["localfile"].asString().c_str();
			}
			if (item.isMember("memo")) {
				pFile->m_Memo = item["memo"].asString().c_str();
			}
			return S_OK;
		}

		HRESULT ValueToFileList(Json::Value& value, ICmdFileList* pFileList)
		{
			CCmdFileList* p = static_cast<CCmdFileList*>(pFileList);
			p->Empty();
			p->m_PageCount = value["page_count"].asInt();
			p->m_PageRows = value["page_rows"].asInt();
			p->m_PageNumber = value["page_no"].asInt();

			INT ResultRows = value["ret_rows"].asInt();
			if (ResultRows > 0) {
				Json::Value result = value["filelist"];
				for (Json::UInt i = 0; i < result.size(); i++) {
					Json::Value item = result[i];
					CCmdFile* pFile = p->m_FileList.New<CCmdFile>();
					ValueToFile(item, pFile);
				}
			}
			return S_OK;
		}

		HRESULT FindFiles(LPCSTR pszFind, LPCSTR lpszSubType, ICmdFileList* pDownloads, ICmdFileList* pUploads, int nPage = 0, int nRows = 20) 
		{
			CResponeDocument Respone;
			CCommandDocument Request("file.find");
			
			Request["search"] = pszFind;
			Request["type"] = lpszSubType;
			Request["page_no"] = nPage;
			Request["page_rows"] = nRows;

			HRESULT hr = m_pClient->Connection->Execute("?File",&Request,Respone);
			CmdLastError = Respone.Description;
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				return E_FAIL;
			}
			
			if (Respone.HaveParameter("download")) {
				ValueToFileList(Respone["download"], pDownloads);
			}

			if (Respone.HaveParameter("upload")) {
				ValueToFileList(Respone["upload"], pUploads);
			}

			return S_OK;
 		}

		HRESULT GetFileInfo(LONG nFileID, ICmdFile** ppCmdFile, bool fUpload = true)
		{
			CResponeDocument Respone;
			CCommandDocument Request("file.info");
			
			Request["fileid"] = nFileID;
			Request["type"] = ( fUpload ? "upload" : "download");

			HRESULT hr = m_pClient->Connection->Execute("?File",&Request,Respone);
			CmdLastError = Respone.Description;
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				return E_FAIL;
			}

			CCmdFile* pFile = new CCmdFile;
			ValueToFile(Respone["fileinfo"], pFile);
			*ppCmdFile = pFile;
			return S_OK;
		}

		HRESULT GetFileInfo(LONG nFileID, ICmdFile* pCmdFile, bool fUpload)
		{
			if (!pCmdFile) {
				return E_POINTER;
			}
			CResponeDocument Respone;
			CCommandDocument Request("file.info");
			
			Request["fileid"] = nFileID;
			Request["type"] = ( fUpload ? "upload" : "download");

			HRESULT hr = m_pClient->Connection->Execute("?File",&Request,Respone);
			CmdLastError = Respone.Description;
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				return E_FAIL;
			}
			ValueToFile(Respone["fileinfo"], static_cast<CCmdFile*>(pCmdFile));
			return S_OK;
		}

		HRESULT GetClearList(ICmdFileList* pUploads, ICmdFileList* pDownloads)
		{
			CResponeDocument Respone;
			CCommandDocument Request("file.clearlist");
			
			HRESULT hr = m_pClient->Connection->Execute("?File",&Request,Respone);
			CmdLastError = Respone.Description;
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				return E_FAIL;
			}

			CCmdFileList* pUplist = static_cast<CCmdFileList*>(pUploads);
			CCmdFileList* pDownlist = static_cast<CCmdFileList*>(pDownloads);
			pUplist->Empty();
			pDownlist->Empty();

			pUploads->FileList->Clear();
			pDownloads->FileList->Clear();

			if (Respone.HaveParameter("upload")) {
				Json::Value list = Respone["upload"]["filelist"];
				for (int i = 0; i < list.size(); i++) {
					CCmdFile* pFile = new CCmdFile;
					ValueToFile(list[i], pFile);
					pUplist->FileList->Add(pFile);
				}
 			}
			if (Respone.HaveParameter("download")) {
				Json::Value list = Respone["download"]["filelist"];
				for (int i = 0; i < list.size(); i++) {
					CCmdFile* pFile = new CCmdFile;
					ValueToFile(list[i], pFile);
					pDownlist->FileList->Add(pFile);
				}
			}
			return S_OK;
		}

	};

	template <typename T>
	class CCmdHeartImpl : public CCmdObjectImpl<T,ICmdHeart,cmdHeart>
		, public IWorkerThreadClient {
	public:
		CCmdHeartImpl() { Empty(); }
		~CCmdHeartImpl() { Shutdown(); }
	public:
		// -- ICmdHeart interface
 		IMPLEMENT_GET_PROPERTY(ICmdClient*,CmdClient);

		HRESULT Initialize(ICmdClient* pClient, DWORD cbTimer = 5000, DWORD_PTR pvContext = NULL)
 		{
			Shutdown(); {
				CmdLocker();
				m_CmdClient = pClient;
 			}
			HANDLE hTimer = NULL;
 			Worker.Initialize();
			return Worker.AddTimer(cbTimer,this,(DWORD_PTR)pvContext,&hTimer);
 		}
		HRESULT Shutdown(DWORD dwWait = 1000)
		{
 			HRESULT hr = static_cast<T*>(this)->OnShutdown(dwWait);
			if (SUCCEEDED(hr)) {
				hr = Worker.Shutdown(dwWait);
			}
			return hr;
		}
		HRESULT AddHeartCallback(ICmdHeartCallback* pCallback)
		{
			ATLASSERT(pCallback);
			CmdLocker();
			int nIndex = m_HeartCallbacks.Find(pCallback);
			if (nIndex == -1) {
				m_HeartCallbacks.Add(pCallback);
			}
			return S_OK;
		}
		HRESULT RemoveHeartCallback(ICmdHeartCallback* pCallback)
		{
			ATLASSERT(FALSE);
			CmdLocker();
			int nIndex = m_HeartCallbacks.Find(pCallback);
			if (nIndex != -1) {
				m_HeartCallbacks.RemoveAt(nIndex);
			}
			return S_OK;
		}
		HRESULT AddHeartBuilder(ICmdHeartBuilder* pBuilder)
		{
			ATLASSERT(pBuilder); 
			CmdLocker();
			int nIndex = m_HeartBuilders.Find(pBuilder); 
			if (nIndex == -1 && pBuilder) {
				m_HeartBuilders.Add(pBuilder);
			}
			return S_OK; 
		}

		HRESULT RemoveHeartBuilder(ICmdHeartBuilder* pBuilder)
		{
			ATLASSERT(pBuilder);
			CmdLocker();
			int nIndex = m_HeartBuilders.Find(pBuilder); 
			if (nIndex != -1) {
				m_HeartBuilders.RemoveAt(nIndex);
			}
			return S_OK; 
		}

		LRESULT OnEmpty()
		{
 			m_CmdClient = NULL;
			m_ConnectPtr = NULL;
			return 0;
		}
	public:
		// -- IWorkerThreadClient Interface
		HRESULT Execute(DWORD_PTR dwParam, HANDLE hObject)
		{
 			CResponeDocument Respone;
			CCommandDocument Request("service.heart");

			for (int i = 0; i < m_HeartBuilders.GetSize(); i++) {
				if (!m_HeartBuilders[i])
					continue; 
				bool fCancel = false; 
				m_HeartBuilders[i]->OnHeartBuilder(Request.Document, fCancel); 
				if (fCancel) {
					break; 
				}
			}

			HRESULT hr = static_cast<T*>(this)->OnRequestHeartCommand(Request,Respone);
			if (SUCCEEDED(hr)) {
				for (int i = 0; i < m_HeartCallbacks.GetSize(); i++) {
					CMD::ICmdHeartCallback *pCallback = m_HeartCallbacks[i];
 					if (!pCallback)
						continue;
					
					bool fCancel = false;
 					pCallback->OnHeartCallback(Respone.ErrorCode(),Respone.Description,Respone.Document,fCancel);
					if (fCancel)
						break; 
				}
			}
 			return hr;
		}
		HRESULT CloseHandle(HANDLE hHandle)
		{
			::CloseHandle(hHandle);
			return S_OK;
		}
	public:
		ICmdConnection* GetConnectPtr()
		{
			CmdLocker();
			if (m_ConnectPtr) {
				return m_ConnectPtr;
			}
			if (!m_CmdClient) {
				return NULL;
			}
			m_ConnectPtr = new CCmdConnection;
			if (FAILED(m_CmdClient->Connection->Clone((void*)m_ConnectPtr))) {
  				delete m_ConnectPtr;
				m_ConnectPtr = NULL;
 			}				
  			return m_ConnectPtr;
		}
		HRESULT OnShutdown(DWORD dwWait) 
		{
			CmdLocker();
			if (m_ConnectPtr) {
				delete m_ConnectPtr;
				m_ConnectPtr = NULL;
			}
			m_CmdClient = NULL;
			m_HeartCallbacks.RemoveAll();
			return S_OK;
		}
		HRESULT OnRequestHeartCommand(CCommandDocument &Request,CResponeDocument &Respone)
		{
			T* pT = static_cast<T*>(this);
			ICmdConnection* ConnectPtr = pT->GetConnectPtr();
			if (!ConnectPtr) {
				CmdLastError = "尚未链接远端服务";
				return E_POINTER;
			}
			HRESULT hr = ConnectPtr->Execute("?Service",&Request,Respone);
			CmdLastError = Respone.Description;
 			return hr;
		}
	protected:
		ATL::CWorkerThread<Win32ThreadTraits> Worker;
		CMD::ICmdConnection* m_ConnectPtr; 
		ATL::CSimpleArray<CMD::ICmdHeartCallback*>  m_HeartCallbacks;
		ATL::CSimpleArray<CMD::ICmdHeartBuilder*>   m_HeartBuilders;
 	};

	class CCmdHeart : public CCmdHeartImpl<CCmdHeart> {};

	template <typename T, const ProductInfo &tProduct, class ThreadTraits=DefaultThreadTraits>
	class CCmdClientImpl : public CCmdObjectImpl<T, ICmdClient, cmdClient> {
	public:
		CCmdPtr<CCmdHeart> m_Heart;
	public:
		IMPLEMENT_GET_PROPERTY(bool,IsLogined);
		IMPLEMENT_GET_PROPERTY(bool,IsInited);
		IMPLEMENT_GET_PROPERTY(LONG,ServerID);

		ICmdUser* GetCurrentUser() { CmdLocker(); return &m_User; }
		ICmdMachine* GetMachine() { CmdLocker(); return &m_Machine; }
		ICmdProduct* GetProduct() { CmdLocker(); return &m_Product; }
		ICmdConnection* GetConnection() { CmdLocker(); return &m_HttpConnect; }
		ICmdCollection* GetServers() { CmdLocker(); return &m_Servers; }
		ICmdPropertys* GetPropertys() { CmdLocker(); return &m_Propertys; }
		ICmdHeart* GetHeart() { CmdLocker(); return m_Heart; }
	public:
		CCmdClientImpl()
		{
			Empty();
		}
	public:
		const ProductInfo & GetProductInfo() { return tProduct; }
	public:
		LPCSTR GetServiceURL()
		{
			return GetProductInfo().ServiceURL;
		}
	public:
		HRESULT TestService(LPCSTR lpszCmdUrl = NULL)
		{
			HRESULT hr;
			LPCSTR HostUrl = lpszCmdUrl;
			if (!HostUrl) {
				HostUrl = static_cast<T*>(this)->GetServiceURL();
			}
			do {
 				CCmdPtr<CCmdConnection> Conn;
				if (FAILED(hr = Conn->SetUrl(HostUrl)))
					break;
				if (FAILED(hr = Conn->Connect(true)))
					break;
				CResponeDocument Respone;
				CCommandDocument Request("version",(DWORD)0);
				if (FAILED(hr = Conn->Execute("?Version",&Request,Respone,0)))
					break;

				if (!Respone.HaveParameter("version")) {
					hr = E_FAIL;
				}
				else hr = S_OK;
			} while(false);
			return hr;
		}
		HRESULT RestartService(HINSTANCE hInstance)
		{
			CStringA sUser = m_User.UserName;
			CStringA sPass = m_User.Password; 

			HRESULT hr = Init(hInstance, NULL, NULL, 0); 
			if (SUCCEEDED(hr) && (m_IsLogined || sUser != "")) {
				hr = Login(sUser, sPass, m_ServerID); 
			}
			return hr; 
		}

		__int64 NewDBID()
		{
			CResponeDocument Respone;
			CCommandDocument Request("service.query",CMD_FLAGS_SIGNMD5);
			
			Request["type"] = "newid";
  			HRESULT hr = m_HttpConnect.Execute("?Service",&Request,Respone,0);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				SetCmdLastError(Respone.Description);
				return -1;
			}
			return Respone["newid"].asInt64();
		}

		HRESULT GetServerList(ICmdCollection* ServerList,LPCSTR lpszCmdUrl = NULL)
		{
			HRESULT hr;
			if (!ServerList) {
				return E_POINTER;
			}
			LPCSTR HostUrl = lpszCmdUrl;
			if (!HostUrl) {
				HostUrl = static_cast<T*>(this)->GetServiceURL();
			}
			do {
 				CCmdPtr<CCmdConnection> Conn;
				if (FAILED(hr = Conn->SetUrl(HostUrl)))
					break;
				if (FAILED(hr = Conn->Connect(true)))
					break;

				CResponeDocument Respone;
				CCommandDocument Request("service.query",CMD_FLAGS_SIGNMD5);
				Request["type"] = "serverlist";
				if (FAILED(hr = Conn->Execute("?Service",&Request,Respone,0)))
					break;

				ServerList->Clear();
				CCmdCollection* List = static_cast<CCmdCollection*>(ServerList);
				Json::Value& value = Respone["servers"];
				for (size_t i = 0; i < value.size(); i++)
				{
					Json::Value &item = value[(int)i];
					CCmdServer* p = List->New<CCmdServer>();
					CmdLocker locker(&p->m_cs);
					p->m_ServerID = item["serverid"].asInt();
					p->m_Caption = item["caption"].asString().c_str();
					p->m_Server = item["host"].asString().c_str();
					p->m_Url = item["url"].asString().c_str();
					p->m_Port = item["port"].asInt();
					p->m_State = item["state"].asInt();
					p->m_UserName = item["webuser"].asString().c_str();
					p->m_Password = item["webpass"].asString().c_str();
					p->m_FtpPort = item["ftpport"].asInt();
					p->m_FtpUser = item["ftpuser"].asString().c_str();
					p->m_FtpPass = item["ftppass"].asString().c_str();
				}
			} while(false);
			return hr;
		}

		HRESULT Init(HINSTANCE hInstance,ICmdHeartCallback* pCallback = NULL, LPVOID lpContext = NULL, LONG dwFlags = 1)
		{
			CResponeDocument Respone;
			CCommandDocument Request("service.init",CMD_FLAGS_SIGNMD5 | CMD_FLAGS_XOR );
			
			Request["machine"] = m_Machine.MacAddress ;
			Request["groupname"] = tProduct.GroupName ;
			Request["group"] = tProduct.GroupIID ;
			Request["productname"] = tProduct.ProductName ;
			Request["product"] = tProduct.ProductIID  ;
			Request["setcookie"] = "true";
			//Request["cmdver"] = "3.2.0.0";

			//CLocalVersion version; 
			CHAR szVer[64] = { 0 }; 
			CFileVersion version((HMODULE)NULL); 
			Request["version"] = (LPCSTR)version.ToString(szVer);

			CSystemVersion ver; 
			Request["os"] = ver.GetVersionIndex(); 
			Request["x86"] = ver.GetX86Index(); 

			if (m_HttpConnect.ConnectInfo->Host == "") {
 				m_HttpConnect.SetUrl(tProduct.ServiceURL); 
			}

 			HRESULT hr = m_HttpConnect.Execute("?Service",&Request,Respone,0);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				SetCmdLastError(Respone.Description);
				ATLTRACE("LastError: %s\n", CmdLastError);
				return E_FAIL;
			}
			m_HttpConnect.Session = Respone["sessionid"].asString().c_str();

			if (dwFlags) {
				m_Heart->Initialize(this,3000);
			}

			{
				CmdLocker Locker(&m_Machine.m_cs);
				m_Machine.m_MachineID = Respone["pmachineid"].asInt();
				m_Machine.m_Days = Respone["mdays"].asInt();
				m_Machine.m_State = Respone["mstate"].asInt();
				m_Product.m_ProductID = Respone["productid"].asInt();
				m_Product.m_State = Respone["pstate"].asInt();
				m_Product.m_Days = Respone["pdays"].asInt();
			}
			if (m_Machine.m_MachineID <= 0 || m_Machine.State <= 0 || m_Machine.Days <= 0) {
				CmdLastError = "主机尚未开通或已过期！";
				return E_ABORT;
			}
			if (m_Product.ProductID <= 0 || m_Product.State <= 0 || m_Product.Days <= 0) {
				CmdLastError = "程序尚未开通或已过期!";
				return E_ABORT;
			}

			if (!Respone.HaveParameter("version")) {
				CmdLastError = "IISAPI和客户端版本不匹配，客户端无法访问"; 
				return E_ABORT; 
			}

			if (Respone.HaveParameter("servers"))
			{
				m_Servers.Clear();
				Json::Value& value = Respone["servers"];
				for (size_t i = 0; i < value.size(); i++)
				{
					Json::Value &item = value[(int)i];
					CCmdServer* p = m_Servers.New<CCmdServer>();
					CmdLocker locker(&p->m_cs);
					p->m_ServerID = item["serverid"].asInt();
					p->m_Caption = item["caption"].asString().c_str();
					p->m_Server = item["host"].asString().c_str();
					p->m_Url = item["url"].asString().c_str();
					p->m_Port = item["port"].asInt();
					p->m_State = item["state"].asInt();
					p->m_UserName = item["webuser"].asString().c_str();
					p->m_Password = item["webpass"].asString().c_str();
					p->m_FtpPort = item["ftpport"].asInt();
					p->m_FtpUser = item["ftpuser"].asString().c_str();
					p->m_FtpPass = item["ftppass"].asString().c_str();
				}
			}
			m_IsInited = true;
 			return S_OK;
		}
		HRESULT Register(ICmdUser* pUser, bool fSimple = false)
		{
			if(!pUser) {
				return E_POINTER;
			}
			if (!IsInited)
				return E_ACCESSDENIED;

 			CResponeDocument Respone;
			CCommandDocument Request("user.register");

			if (fSimple) {
				Request["simple"] = true;
			}

			Request["name"] = (LPCSTR)pUser->UserName ;
			Request["pass"] = (LPCSTR)pUser->Password ;
			Request["mac"] = (LPCSTR)m_Machine.MacAddress;
			Request["mobile"] = (LPCSTR)pUser->Mobile ;
			Request["email"] = (LPCSTR)pUser->Email ;
			Request["co"] = (LPCSTR)pUser->Co ;
			Request["qq"] = (LPCSTR)pUser->QQ ;
			Request["query"] = (LPCSTR)pUser->Question ;
			Request["anwser"] = (LPCSTR)pUser->Anwser ; 
			
			HRESULT hr = m_HttpConnect.Execute("?User",&Request,Respone,0);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				SetCmdLastError(Respone.Description);
				return E_FAIL;
			}
			else {
				CmdLocker Locker(&m_User.m_cs);
				CCmdUser* pCmdUser = static_cast<CCmdUser*>(pUser);
				pCmdUser->m_UserID = Respone["userid"].asInt();
				pCmdUser->m_State = Respone["state"].asInt();
				pCmdUser->m_Days = Respone["days"].asInt();
			}
			return hr;
		}
		HRESULT QueryUserProtected(CStringA sUser,CStringA& sProtected) // 获取用户密码保护信息
		{
 			if (!IsInited)
				return E_ACCESSDENIED;

 			CResponeDocument Respone;
			CCommandDocument Request("user.recvprot");

			Request["user"] = (LPCSTR)sUser ;
 			
			HRESULT hr = m_HttpConnect.Execute("?User",&Request,Respone,0);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				SetCmdLastError(Respone.Description);
				return E_FAIL;
			}
			else {
 				sProtected = Respone["query"].asString().c_str();
			}
			return hr;
		}

		HRESULT QueryUserPassword(CStringA sUser,CStringA sQuery,CStringA sAnwser, CStringA& sPassword) // 获取用户密码保护信息
		{
 			if (!IsInited)
				return E_ACCESSDENIED;

 			CResponeDocument Respone;
			CCommandDocument Request("user.recvpass");

			Request["user"] = (LPCSTR)sUser ;
			Request["query"] = (LPCSTR)sQuery ;
			Request["anwser"] = (LPCSTR)sAnwser ;
 			
			HRESULT hr = m_HttpConnect.Execute("?User",&Request,Respone,0);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				SetCmdLastError(Respone.Description);
				return E_FAIL;
			}
			else {
 				sPassword = Respone["userpass"].asString().c_str();
			}
			return hr;
		}

		HRESULT Login(LPCSTR lpszUser, LPCSTR lpszPass, LONG nServerID = 0)
		{
 			CResponeDocument Respone;
			CCommandDocument Request("user.login");
			Request["user"] = lpszUser;
			Request["pass"] = lpszPass;

			CLocalVersion version; 
			if (version.Version.GetLength() > 0) {
				Request["version"] = (LPCSTR)version.Version; 
			}

			if (nServerID != 0) {
				Request["serverid"] = nServerID; 
				m_ServerID = nServerID; 
			}
			HRESULT hr = m_HttpConnect.Execute("?User",&Request,Respone);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				SetCmdLastError(Respone.Description);
				return E_FAIL;
			}
			else {
				m_User.UserName =lpszUser;
				
				CmdLocker();
				m_User.m_Password = lpszPass;
				m_User.m_UserID = Respone["userid"].asInt();
				m_User.m_UserName = Respone["username"].asString().c_str();
				m_User.m_State = Respone["state"].asInt();
				m_User.m_Style = Respone["style"].asInt();
				m_User.m_Rights = Respone["rights"].asInt();
				m_User.m_Days = Respone["days"].asInt();
				if (Respone.HaveParameter("space")) 
				{
					Json::Value &ftp = Respone["space"];
					m_User.m_ServerID = ftp["serverid"].asInt();
					m_User.m_FtpType = ftp["type"].asInt();
					m_User.m_FtpHost = ftp["host"].asString().c_str();
					m_User.m_FtpPort = ftp["port"].asInt();
					m_User.m_FtpUser = ftp["username"].asString().c_str();
					m_User.m_FtpPass = ftp["password"].asString().c_str();
					m_User.m_FtpPath = ftp["pypath"].asString().c_str();
					m_User.m_Template = ftp["template"].asString().c_str();
					m_User.m_Quality = ftp["quality"].asInt();
					m_User.m_Space = ftp["space"].asInt();
					m_User.m_UpThreads = ftp["uthreads"].asInt();
					m_User.m_DownThreads = ftp["dthreads"].asInt();

					CString sFilter ;
					if (ftp.isMember("filter")) {
						sFilter = ftp["filter"].asString().c_str();
					}
					else {
						sFilter = DEF_FILTER;
					}
					sFilter.Trim().MakeLower();
					m_User.m_Filters.RemoveAll();

					INT nStart = 0;
					CStringA sTemp = sFilter.Tokenize(";",nStart);
					while(sTemp != "") {
						sTemp.Trim().MakeLower();
						m_User.m_Filters.Add(sTemp);
						sTemp = sFilter.Tokenize(";",nStart);
					}
				}
				m_User.m_pClient = this;
 			}
			if (m_User.UserID <= 0 || m_User.Days <= 0 || m_User.State <= 0) {
				CmdLastError = "无效的用户或用户已过期";
				m_IsLogined = false;
				hr = E_FAIL;
			}
			else {
				m_User.Refresh();
				CmdLocker();
				m_IsLogined = true;
			}
			return hr;
		}
		HRESULT Logout()
		{
			if (!IsLogined)
				return S_OK;

 			CResponeDocument Respone;
			CCommandDocument Request("user.logout");
			HRESULT hr = m_HttpConnect.Execute("?User",&Request,Respone,0);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				CmdLastError = Respone.Description;
				return E_FAIL;
			}
			else {
				m_User.Empty();
			}
			return S_OK;
		}
		HRESULT Term()
		{
			if (!IsInited) {
				return S_OK;
			}
			if (IsLogined) {
				Logout();
			}
 			CResponeDocument Respone;
			CCommandDocument Request("service.term");
			
			HRESULT hr = m_HttpConnect.Execute("?Service",&Request,Respone,0);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				SetCmdLastError(Respone.Description);
				return E_FAIL;
			}
			Empty();
			m_HttpConnect.Close();
			m_Heart->Shutdown();
			return S_OK;
		}
	
		HRESULT CmdExecute(LPCSTR lpszUrl, CCommandDocument* pCmd, CResponeDocument& doc,LONG nHashCode = Sign::MAC_Hash() )
		{
			return m_HttpConnect.Execute(lpszUrl,pCmd,doc,nHashCode);
		}

		HRESULT ExecuteSQL(LPCSTR lpszSQL, Json::Value* pArgs, Json::Value& result, int& resultRows, bool query = false)
		{
			if (!IsInited || !IsLogined ) {
				SetCmdLastError("程序尚未初始化或尚未登录，无效权限拒绝服务");
				return E_ACCESSDENIED;
			}
 			
			CResponeDocument Respone;
			CCommandDocument Request("service.execute");
			Request["sql"] = lpszSQL;
			Request["query"] = query;
			if (pArgs) {
				Request["args"] = *pArgs;
			}
			HRESULT hr = m_HttpConnect.Execute("?Service",&Request,Respone,0);
			if (FAILED(hr) || Respone.ErrorCode() != 0) {
				SetCmdLastError(Respone.Description);
				return E_FAIL;
			}
			
			if (Respone.HaveParameter("rows")) 
			{
				resultRows = Respone["rows"].asInt();
			}

			if (Respone.HaveParameter("data")) 
			{
				result = Respone["data"];
			}
			return S_OK;
		}

	public:
		HRESULT OnEmpty()
		{
			m_User.Empty();
			m_Machine.Empty();
			m_Product.Empty();
			m_IsInited = false;
			m_IsLogined = false;
			m_ServerID = 0; 
			return S_OK;
		}
	public:
		CCmdUser		m_User;
		CCmdMachine		m_Machine;
		CCmdProduct		m_Product;
		CCmdConnection	m_HttpConnect;
		CCmdCollection  m_Servers;
		CCmdFTP			m_FTP;
		CCmdPropertys   m_Propertys;
	};
};

namespace CMD
{
	class CHttpRequest;
	class CHttpRequest : public CSimpleHttpClient {
	public:
		CHttpRequest(LPCSTR lpszUrl = NULL) {
			SetUrl(lpszUrl);
		}
		CHttpRequest(LPCSTR lpszHost, LPCSTR lpszUrl, WORD nPort = 80) {
			HostInfo.Host = lpszHost;
			HostInfo.Url = lpszUrl;
			HostInfo.Port = nPort;
		}
	public:
		bool Execute(CCommand* pCmd, CResponeDocument& doc, LONG nHashCode = 0, LONG nStyle = JSON_SOURCE | JSON_STYLE) {
			if(!pCmd) {
				doc.SetLastError(CMD_ERROR_POINTER,"无效命令对象指针");
				return false;
			}
 			CCommandDocument cmd(*pCmd);
			if (!pCmd->Flags.IsBindMac()) {
				nHashCode = 0;
			}
			return Execute(&cmd,doc,nHashCode,nStyle);
		}
		bool Execute(CCommandDocument* pCmd, CResponeDocument& doc,LONG nHashCode = 0, LONG nStyle = JSON_SOURCE | JSON_STYLE ) {
			if (!pCmd) {
				doc.SetLastError(CMD_ERROR_POINTER,"无效命令对象指针");
				return false;
			}
			if (pCmd->ErrorCode()) {
				doc.SetCommand(pCmd->Command);
				doc.SetLastError(pCmd->ErrorCode(),pCmd->Description);
				return false;
			}
			CStringA sJson = pCmd->GetDocument(nHashCode,nStyle);
			if (sJson == "" ) {
				doc.SetLastError(doc.ErrorCode(),doc.Description);
				return false;
			}
			return Execute((LPCSTR)sJson, doc);
		}
 		bool Execute(LPCTSTR lpszUrl,CCommandDocument* pCmd, CResponeDocument& doc,LONG nHashCode = 0, LONG nStyle = JSON_SOURCE | JSON_STYLE ) {
			if (!pCmd) {
				doc.SetLastError(CMD_ERROR_POINTER,"无效命令对象指针");
				return false;
			}
			if (pCmd->ErrorCode()) {
				doc.SetCommand(pCmd->Command);
				doc.SetLastError(pCmd->ErrorCode(),pCmd->Description);
				return false;
			}
			SendDocument = pCmd->GetDocument(nHashCode,nStyle);
			OutputDebugStringA("Send: ");
			OutputDebugStringA(SendDocument);
			OutputDebugStringA("\r\n");
			if (SendDocument== "" ) {
				doc.SetLastError(doc.ErrorCode(),doc.Description);
				return false;
			}
  			if (Post(lpszUrl,(LPCSTR)SendDocument,RecvDocument)) {
				doc.SetDocument(RecvDocument);
				return true;
 			}
			else {
				doc.SetLastError(CMD_ERROR_NET,"网络连接异常");
			}
			return false;
		}
		bool Execute(LPCSTR lpszJson, CResponeDocument& doc) {
			EmptyDocument();
			SendDocument = lpszJson;
 			if (Post(HostInfo.Url,lpszJson,RecvDocument)) {
				doc.SetDocument(RecvDocument);
				return true;
 			}
			else {
				doc.SetLastError(CMD_ERROR_NET,"网络连接异常");
			}
			return false;
		}	
		bool Execute(LPCSTR lpszCmd, LPCSTR lpszJson, CResponeDocument& doc, LONG nHashCode = 0, LONG nStyle = JSON_SOURCE | JSON_STYLE ) {
			CCommandDocument cmd(lpszCmd,lpszJson);
			return Execute(&cmd,doc,nHashCode,nStyle);
		}
		bool Execute(LPCSTR lpszCmd, const Json::Value& value, CResponeDocument& doc, LONG nHashCode = 0, LONG nStyle = JSON_SOURCE | JSON_STYLE ) {
			CCommandDocument cmd(lpszCmd,value);
			return Execute(&cmd,doc,nHashCode,nStyle);
		}
 		void EmptyDocument() {
			SendDocument = "";
			RecvDocument = "";
		}
 	public:
		CStringA SendDocument;
		CStringA RecvDocument;
	};



	class CProductInfo {
	public:
		CProductInfo() { Empty(); }
	public:
		void Empty() { ZeroMemory(this,sizeof(*this)); }
	public:
		DECLARE_PROPERTY(LONG,ProductID)
		DECLARE_PROPERTY(LONG,ProductState)
		DECLARE_PROPERTY(LONG,ProductStyle)
		DECLARE_PROPERTY(LONG,ProductGroupID)
		DECLARE_PROPERTY(LONG,ProductGroupState)
		DECLARE_PROPERTY(LONG,ProductMachineID)
		DECLARE_PROPERTY(LONG,ProductMachineState)
		DECLARE_PROPERTY(LONG,ProductDays)
		DECLARE_PROPERTY(CStringA,ProductMachineRegcode)
	};

	class CMachineInfo {
	public:
		CMachineInfo() { Empty(); }
	public:
		void Empty() { ZeroMemory(this,sizeof(*this));}
		__declspec(property(get = GeMacAddress)) LPCSTR MacAddress;
		LPCSTR GetMacAddress()
		{
			static CHAR szMac[64] = {0};
			if (!szMac[0]) {
				StringTool::MacString(szMac,64);
			}
			return szMac;
		}
	protected:
		DECLARE_PROPERTY(LONG,MachineID);
		DECLARE_PROPERTY(LONG,MachineState);
	};

	class CUserInfo {
	public:
		CUserInfo() { Empty(); }
	public:
		void Empty() { ZeroMemory(this,sizeof(*this));}
	protected:
		DECLARE_PROPERTY(LONG,UserID);
		DECLARE_PROPERTY(LONG,UserState);
		DECLARE_PROPERTY(LONG,UserStyle);
		DECLARE_PROPERTY(LONG,UserDays);
 		DECLARE_PROPERTY(LONG,UserMachineID);
		DECLARE_PROPERTY(LONG,UserMachineState);
		DECLARE_PROPERTY(CStringA,ProductMachineRegcode)
	};

	template <typename T, const PRODUCT_INFO& tProduct, typename TLock = ATL::CComAutoCriticalSection>
	class CCmdServiceClientImpl {
	public:
		TLock	m_cs;
 	public:
		bool Init(DWORD fHeart = 10000)
		{
			Term();
			return ErrorCode == 0;
		}
		bool Term()
		{
			
 		}
		CHttpRequest* NewHttpRequest(bool fCookie = true)
		{
		}

		const CProductInfo& GetProductInfo()  { return Product; }
		const CMachineInfo& GetMachineInfo()  { return Machine; }
		const CUserInfo& GetCurrentUserInfo() { return UserInfo; }
	public:
		CHttpRequest CmdRequest;
		CStringA	 SessionId;
		CStringA	 LastError;
		LONG		 ErrorCode;
		
	protected:
		CProductInfo Product;
		CMachineInfo Machine;
		CUserInfo	 UserInfo;
	};
}
#endif