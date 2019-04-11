#pragma once 
#define CMD_VER		0x0200

#include <atlbase.h>
#include <atlstr.h>
#include <atlcomtime.h>
#include <log/DbgPrint.h>
#include <inet/SimpleHttpClient.h>
#include "CmdDoc.h"
#include "Cmd.h"

using namespace CMD;

class CCommands : public CSimpleArray<CCommand*>
{
public:
	CCommands() {}
	~CCommands()
	{
		Destroy();
	}
public:
	int FindCommand(CString sCmd)
	{
		sCmd.Trim().MakeLower();
		for (int i = 0; i < m_nSize; i++) {
			CString Cmd = m_aT[i]->Command ;
			Cmd.Trim().MakeLower();
			if (Cmd == sCmd)
				return i;
		}
		return -1;
	}

	
	CCommand* GetCommand(CString sCmd)
	{
		int nIndex = FindCommand(sCmd);
		if (nIndex == -1) {
			return NULL; 
		}
		return m_aT[nIndex];
	}

	/*CCommand* operator[](CString sCmd)
	{
		return GetCommand(sCmd);
	}*/

	void Destroy()
	{
		for (int i = 0; i < m_nSize; i++) {
			if (m_aT[i]) {
				delete m_aT[i];
				m_aT[i] = NULL;
			}
		}
		RemoveAll();
	}
private:
	CCommands(const CCommands& ) {}
};

struct RequestDocuments {
	CString Send;
	CString Recv;
	CString Code;
public:
	CStringA GetSendSource()
	{
		CMD::CRequestDocument Doc(CT2A(Send),Sign::MAC_Hash());
		CStringA sDoc = Doc.GetDocument(Sign::MAC_Hash(),JSON_SOURCE | JSON_STYLE);
		return sDoc;
	}
	CStringA GetRecvSource()
	{
		CMD::CResponeDocument Doc;
		Doc.SetDocument(CT2A(Recv));
		CStringA sDoc = Doc.GetDocument(Sign::MAC_Hash(),JSON_SOURCE | JSON_STYLE);
		return sDoc;
 	}
};

class CCmdService : public CStreamHelperImpl<CCmdService>
{
public:
	DECLARE_OBJECT_TYPE(CMD_SERVICE_OBJECT)
public:
	CCmdService() {};
	~CCmdService()
	{
		Destroy();
	}
public:
	RequestDocuments* Execute(CCommand* pCmd,LONG nHash = Sign::MAC_Hash() )
	{
		CResponeDocument doc;

  		HttpRequest.Execute(pCmd,doc,nHash);

		RequestDocuments* pDocs = Documents.Lookup(pCmd);
 		if (!pDocs) {
			pDocs = new RequestDocuments;
			Documents.Add(pCmd,pDocs);
 		}
		
		if (pDocs) {
 			pDocs->Send = HttpRequest.SendDocument ;
			pDocs->Recv = HttpRequest.RecvDocument ;
		}
 		return pDocs;
	}
public:
	void SetUrl(CString sUrl)
	{
		Url = sUrl;
		HttpRequest.SetUrl(sUrl);
	}
	CString GetUrl()
	{
		return Url;
	}
	CCommand* NewCommand(CString sCommand = _T(""))
	{
		CCommand* pCmd = new CCommand;
		pCmd->Command = sCommand;
		Commands.Add(pCmd);
		return pCmd;
	}
	void RemoveCommand(CCommand* pCmd)
	{
		int nIndex = Commands.Find(pCmd);
		if (nIndex != -1) {
			delete pCmd;
			Commands.RemoveAt(nIndex);
		}
	}
public:
	void OnGetJson(Json::Value &_Value, int nFlags)
	{
		CStringA sTemp ;
		Json::Value Root;
		Json::Value commands;
		Root["title"] = (LPCSTR)CT2A(Title);
		Root["url"] = (LPCSTR)CT2A(Url);
		Root["description"] = (LPCSTR)HEX::HexEncode(Description,sTemp);

		for (int i = 0; i < Commands.GetSize(); i++) {
			Json::Value _Command;
			Commands[i]->GetJson(_Command,0);
			commands.append(_Command);
		}
		Root["commands"] = commands;
		_Value["service"] = Root;
	}
	void OnSetJson(Json::Value &value)
	{
		Destroy();

		Title = CA2T(value["service"]["title"].asString().c_str());
 		Url = CA2T(value["service"]["url"].asString().c_str());
		SetUrl(Url);
 		CStringA sTemp = value["service"]["description"].asString().c_str();
		Description = HEX::HexDecode(sTemp,Description);

		Json::Value commands = value["service"]["commands"];
		for (Json::UInt i = 0; i < commands.size(); i++) {
			CCommand* pCmd = new CCommand;
			pCmd->SetJson(commands[i]);
			Commands.Add(pCmd);
		}
	}
	void Destroy()
	{
		Title = "";
		Url = "";
		Description = "";
		Commands.Destroy();
	}
public:
	CString Title;
	CCommands Commands;
public:
	CHttpRequest HttpRequest;
	CString Url;

	ATL::CSimpleMap<CCommand*, RequestDocuments*> Documents;
};

class CCmdSerivces : public CSimpleArray<CCmdService*>
{
public:
	int FindService(CString sUrl)
	{
		sUrl.Trim();
		for (int i = 0; i < GetSize(); i++) {
			CString Url = m_aT[i]->Url ;
			Url.Trim().MakeLower();
			if (sUrl == Url)
				return i;
		}
		return -1;
	}
	CCmdService* GetService(CString sUrl)
	{
		int nIndex = FindService(sUrl);
		if (nIndex == -1)
		{
			ATLASSERT(FALSE);
			return NULL;  //must be able to convert
		}
		return m_aT[nIndex];
	}

	void RemoveService(CCmdService* ObjectPtr)
	{
		int nIndex = Find(ObjectPtr);
		if (nIndex != -1) {
			delete ObjectPtr;
			RemoveAt(nIndex);
		}
	}

	void Destroy()
	{
		for (int i = 0; i < m_nSize; i++) {
			__SAFE_DELETE_PTR(m_aT[i]);
		}
		RemoveAll();
	}
	/*CCmdService* operator[](LPCTSTR sUrl)
	{
		return GetService(sUrl);
	}*/
};

class CCmdProject : public CStreamHelperImpl<CCmdProject>
{
public:
	DECLARE_OBJECT_TYPE(CMD_PROJECT_OBJECT)
public:
	CCmdProject() : ModifyTime(0.0f), CreateTime(0.0f) {}
	CCmdService* NewService(CString sTitle = _T(""), CString sUrl = _T(""))
	{
		CCmdService *pService = new CCmdService;
		pService->Title = sTitle;
		pService->Url = sUrl;
		Services.Add(pService);
		return pService;
	}
public:
	bool Save(CString sFile = _T(""))
	{
		CString sSave = sFile;
		if (sSave == _T(""))
			sSave = ProjectFile;
		sSave.Trim();
		if (sSave == _T("")) {
			return false;
		}

		CStringA sJson;
		GetJson(sJson,0);
		if (CreateTime.m_dt == 0) {
			CreateTime = COleDateTime::GetCurrentTime();
		}
		if (ModifyTime.m_dt == 0 || sJson != JsonDocument) {
			ModifyTime = COleDateTime::GetCurrentTime();
		}
		GetJson(JsonDocument,JSON_STYLE);
		if (SUCCEEDED(::INET_WriteBufferToFile(sSave,(PBYTE)(LPCSTR)JsonDocument,JsonDocument.GetLength()))) {
			ProjectFile = sSave;
			return true;
		}
		return false;
	}
	bool Load(CString sFile) 
	{
		sFile.Trim().MakeLower();
		if (sFile == ProjectFile) {
			return true;
		}
		Json::Reader _Reader;
		Json::Value  _Root;

 		LPSTR lpszJson = NULL;
		DWORD cbSize = 0;
		bool fSucceeded = false;
		if (FAILED(::INET_ReadFileToBuffer((LPCTSTR)sFile,(PBYTE*)&lpszJson,&cbSize))) {
			goto _Exit;
		}
		

		if (!_Reader.parse(lpszJson,_Root)) {
			goto _Exit;
		}

		SetJson(_Root);

		fSucceeded = true;
		JsonDocument = lpszJson;
		ProjectFile = sFile;
_Exit:
		__SAFE_FREE_PTR(lpszJson);
		return fSucceeded;
 	}
	bool IsChanged()
	{
		CStringA sJson;
		GetJson(sJson,0);
		return sJson != JsonDocument;
	}
	bool IsEmpty()
	{
		Title.Trim();
		Description.Trim();
		return !(Title.GetLength() || Description.GetLength() || Services.GetSize());
	}
public:
	void OnGetJson(Json::Value &_Value,int nFlags = 0)
	{
		Json::Value Root,services;
		CStringA sTemp ;

		Root["title"] = (LPCSTR)CT2A(Title);
		//Root["projectfile"] = CT2A(ProjectFile);
		Root["create"] = (LPCSTR)CT2A(CreateTime.Format());
		Root["modify"] = (LPCSTR)CT2A(ModifyTime.Format());
		Root["description"] = (LPCSTR)HEX::HexEncode(Description,sTemp);

		for (int i = 0; i < Services.GetSize(); i++)
		{
			Json::Value _Service;
			Services[i]->GetJson(_Service,0);
			services.append(_Service);
		}
		Root["services"] = services;
 		_Value["project"] = Root;
	}

	void OnSetJson(Json::Value &value)
	{
		Destroy();
		Title = value["project"]["title"].asString().c_str();

		CComVariant var(value["project"]["create"].asString().c_str());
		var.ChangeType(VT_DATE);
		CreateTime = var.date;

		CComVariant varModify(value["project"]["modify"].asString().c_str());
		varModify.ChangeType(VT_DATE);
		ModifyTime = varModify.date ;

		CStringA sTemp = value["project"]["description"].asString().c_str();
		HEX::HexDecode(sTemp,Description);
		Json::Value services = value["project"]["services"];
		for (int i = 0; i < services.size(); i++) {
			CCmdService *p = new CCmdService;
			p->SetJson(services[i]);
			Services.Add(p);
		}
	}
	void Destroy()
	{
		Title = "";
		ProjectFile = "";
		CreateTime = 0.0f;
		ModifyTime = 0.0f;
		Description = "";
		Services.Destroy();
	}
public:
	CString			Title;
	CString			ProjectFile;
	COleDateTime	CreateTime;
	COleDateTime	ModifyTime;
	CCmdSerivces	Services;
public:
	CStringA		JsonDocument;
};