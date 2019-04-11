namespace JsonHelper
{
	inline std::string JsonToString(Json::Value &value,bool styled )
	{
		if (styled) {
			Json::StyledWriter _Writer;
			return _Writer.write(value);
		}
		else {
			Json::FastWriter _Writer;
			return _Writer.write(value);
		}
	}
	inline bool ReadJsonFile(LPCTSTR lpszFile, Json::Value& Root)
	{
		bool fResult = false;
		PSTR pBuffer = NULL;
		DWORD cbSize = 0;
		HRESULT hr = FileHelper::ReadToBuffer(lpszFile,(BYTE**)&pBuffer,&cbSize);
		if (SUCCEEDED(hr)) {
			Json::Reader _Reader;
			fResult = _Reader.parse(pBuffer,Root);
		}
		__SAFE_FREE_PTR(pBuffer);
		return fResult;
	}
	inline bool WriteJsonFile(LPCTSTR lpszFile,Json::Value &value)
	{
		Json::StyledWriter _Writer;
		std::string json = _Writer.write(value);
		return SUCCEEDED(FileHelper::WriteToFile(lpszFile,(LPVOID)json.c_str(),json.length()));
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace CMD
{
	template <typename T>
	inline DWORD CDWORDFlagsImpl<T>::_Modify(DWORD cbAdd, DWORD cbRemove ) 
	{
		_Flags |= cbAdd; _Flags &= ~cbRemove; return _Flags;
	}

	template <typename T>
	inline DWORD CDWORDFlagsImpl<T>::_Modify(bool fAdd, DWORD cbFlags) 
	{
		fAdd ? _Modify((DWORD)cbFlags,(DWORD)0) : _Modify((DWORD)0,(DWORD)cbFlags);
		return _Flags;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template <size_t PSIZE>
	inline 	CCmdTempBuffer<PSIZE>::CCmdTempBuffer() 
		: m_ptr(NULL)
		, m_alloced(0)
		, m_postion(0) 
	{}

	template <size_t PSIZE>
	inline 	CCmdTempBuffer<PSIZE>::~CCmdTempBuffer() 
	{ 
		Free(); 
	}

	template <size_t PSIZE>
	inline char* CCmdTempBuffer<PSIZE>::Detach() 
	{ 
		char* tmp = m_ptr; 
		m_ptr = 0;
		return tmp; 
	}
	
	template <size_t PSIZE>
	inline void CCmdTempBuffer<PSIZE>::Free() 
	{ 
		if (m_ptr) { 
			free(m_ptr); 
			m_ptr = 0;
		} 
		m_alloced = 0; 
		m_postion = 0; 
	}

	template <size_t PSIZE>
	inline char* CCmdTempBuffer<PSIZE>::Append(void* buffer, size_t nSize)
	{
		if ((m_postion + nSize) > m_alloced) {
			m_alloced += nSize;
			m_alloced += (PSIZE - (nSize % PSIZE))+1;
		}
		if (!m_ptr) {
			m_ptr = (char*)calloc(m_alloced,1);
		}
		else {
			m_ptr = (char*)realloc(m_ptr,m_alloced);
		}
		if (m_ptr) {
			memcpy(&m_ptr[m_postion],buffer,nSize);
			m_postion += nSize;
			m_ptr[m_postion] = 0x00;
		}
		return m_ptr;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename T> 
	inline void CStreamHelperImpl<T>::GetJson(CStringA &JsonString, int nFlags ) 
	{
		Json::Value  _Value;
		GetJson(_Value,nFlags);

		if ( (nFlags & JSON_FORMATED) == JSON_FORMATED) {
			Json::StyledWriter _Writer;
			JsonString = _Writer.write(_Value).c_str();
		}
		else {
			Json::FastWriter _Writer;
			JsonString = _Writer.write(_Value).c_str();
		}
	}

	template <typename T> 
	inline void CStreamHelperImpl<T>::GetJson(Json::Value &Value, int nFlags) 
	{
		static_cast<T*>(this)->OnGetJson(Value,nFlags);
	}

	template <typename T> 
	inline void CStreamHelperImpl<T>::SetJson(Json::Value &value)
	{
		static_cast<T*>(this)->OnSetJson(value);
	}

	template <typename T> 
	inline int CStreamHelperImpl<T>::ObjectType()
	{
		return static_cast<T*>(this)->_InternalObjectType();
	}

	template <typename T> 
	inline void CStreamHelperImpl<T>::OnGetJson(Json::Value &Value, int nFlags) 
	{
		ATLASSERT(FALSE);
	}

	template <typename T> 
	inline void CStreamHelperImpl<T>::OnSetJson(Json::Value &value) 
	{
		ATLASSERT(FALSE);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline void CParameter::OnGetJson(Json::Value &_Value,int _Flags)	{
		int nFlags = (int)LOBYTE(LOWORD(_Flags));
		if (nFlags == 0){
			_Value["name"] = (LPCSTR)Name;
			_Value["type"] = Type;
			_Value["flags"] = (int)Flags._Get();
			if (Type == TYPE_JSON) {
				Json::Value value;
				Json::Reader reader;
				reader.parse((LPCSTR)Value,value);
				_Value["value"] = value;
			}
			else {
				_Value["value"] = (LPCSTR)Value;
			}
			_Value["description"] = (LPCSTR)HEX::HexEncode(Description);
		}
	}

	inline void CParameter::OnSetJson(Json::Value &value)
	{
		Name = value["name"].asString().c_str();
		Type = value["type"].asInt();
		Flags._Set((DWORD)value["flags"].asInt());
		if (value["value"].isObject()) {
			Json::StyledWriter writer;
			Value = writer.write(value).c_str();
		}
		else {
			Value = value["value"].asString().c_str();
		}
		CStringA sTemp = value["description"].asString().c_str();
		HEX::HexDecode(sTemp,Description);
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline CParameters::~CParameters()
	{
		Destroy();
	}

	inline int CParameters::FindIndex(CString sKey) {
		sKey.Trim().MakeLower();
		for (int i = 0; i < GetSize(); i++) {
			if (m_aT[i]->Name == CT2A(sKey))
				return i;
		}
		return -1;
	}

	inline CParameter* CParameters::FindParameter(CString sKey) {
		int nIndex = FindIndex(sKey);
		if (nIndex != -1) {
			return m_aT[nIndex];
		}
		return NULL;
	}

	inline CParameter* CParameters::AddParameter(CString sName, CString sValue, INT nType , DWORD cbFlags)
	{
		sName.Trim().MakeLower();
		CParameter* ptr = FindParameter(sName);
		if (!ptr) {
			ptr = new CParameter;
			Add(ptr);
		}
		ptr->Name = CT2A(sName);
		ptr->Value = CT2A(sValue);
		ptr->Type = nType;
		ptr->Flags._Set(cbFlags);
		return ptr;
	}

	inline CParameter* CParameters::AddParameter(CString sName, int nValue)	
	{
		CString sTemp;
		sTemp.Format(_T("%d"),nValue);
		return AddParameter(sName,sTemp,1);
	}

	inline CParameter* CParameters::AddParameter(CString sName, COleDateTime oTime)	
	{
		return AddParameter(sName,oTime.Format(),2);
	}

	inline CParameter* CParameters::AddParameter(CString sName,Json::Value &value)
	{
		Json::FastWriter writer;
		CString sJson = CA2T(writer.write(value).c_str());
		return AddParameter(sName,sJson,TYPE_JSON);
	}

	inline CParameter* CParameters::AddParameter(Json::Value &value)
	{
		CParameter* p = NewParameter();
		p->Name = value["name"].asString().c_str();
		p->Type = value["type"].asInt();
		p->Flags._Set((DWORD)value["flags"].asInt());
		CStringA sTemp = value["description"].asString().c_str();
		if (sTemp.GetLength()) {
			HEX::HexDecode(sTemp,p->Description);
		}
		if (value["value"].isObject()) {
			Json::FastWriter writer;
			p->Value = CA2T(writer.write(value["value"]).c_str());
		}
		else {
			p->Value = value["value"].asString().c_str();
		}
		return p;
	}

	inline CParameter* CParameters::NewParameter()
	{
		CParameter* p = new CParameter;
		Add(p);
		return p;
	}

	inline void CParameters::Destroy()
	{
		for (int i =0; i < m_nSize; i++) {
			__SAFE_DELETE_PTR(m_aT[i]);
		}
		RemoveAll();
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	inline void CCommand::OnGetJson(Json::Value &_Value, int nFlags)	
	{
		int _Flags = (int)LOBYTE(LOWORD(nFlags));
		if (_Flags == 0 /*JSON_DEFAULT*/) {
			Json::Value Root;
			Json::Value Params;
			Json::Value Results;
			Root["function"] = (LPCSTR)CT2A(Command);
			Root["flags"] = (int)Flags._Get();
			Root["description"] = (LPCSTR)HEX::HexEncode(Description);
			
			for (int i = 0; i < Parameters.GetSize(); i++)	{
				Json::Value Param;
				Parameters[i]->GetJson(Param,0);
				Params.append(Param);
			}
			Root["parameters"] = Params;

			for (int i = 0; i < ResultParameters.GetSize(); i++)	{
				Json::Value Param;
				ResultParameters[i]->GetJson(Param,0);
				Results.append(Param);
			}
			Root["result"] = Results;

			_Value["command"] = Root;
		}
		else if (_Flags == 1 /*JSON_SEND_SRC*/)	{
			_Value.clear();
			for (int i = 0; i < Parameters.GetSize(); i++) {
				CParameter *p = Parameters[i];
				if(p->Type == 1)/*INT*/ {
					_Value[(LPCSTR)p->Name] = StrToIntA(p->Value); 
				}
				else if (p->Type == TYPE_JSON) {
					Json::Value value;
					Json::Reader reader;
					reader.parse((LPCSTR)p->Value,value);
					_Value[(LPCSTR)p->Name] = value;
				}
				else {// String or DateTime
					_Value[(LPCSTR)p->Name] = (LPCSTR)p->Value; 
				}
			}
		}
	}
	
	inline void CCommand::OnSetJson(Json::Value &value)
	{
		Destory();
		Command = CA2T(value["command"]["function"].asString().c_str());
		Flags._Set((DWORD)value["command"]["flags"].asInt());
		
		CStringA sTemp = value["command"]["description"].asString().c_str();
		HEX::HexDecode(sTemp,Description);

		Json::Value params = value["command"]["parameters"];
		for (Json::UInt i = 0; i < params.size(); i++) {
			Parameters.AddParameter(params[i]);
		}
		if (value["command"].isMember("result")) {
			Json::Value results = value["command"]["result"];
			for (Json::UInt i = 0; i < results.size(); i++) {
				ResultParameters.AddParameter(results[i]);
			}
		}
	}

	inline void CCommand::Destory()
	{
		Command = _T("");
		Flags = 0;
		Parameters.RemoveAll();
		ResultParameters.RemoveAll();
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename T>
	inline CStringA CDocumentImpl<T>::GetDocument(LONG nHashCode,int nFlags)
	{
		CDocFlags DocFlags((DWORD)nFlags);
		CCmdFlags _TempFlags(Flags._Get());
		Json::Value Doc;
		Json::Value Root;

		if (!static_cast<T*>(this)->OnWriteDocument(Root)) {
			return "";
		}
		CStringA sTime = CT2A(COleDateTime::GetCurrentTime().Format());
		if (DocFlags.IsDocument() ) {
			_TempFlags.SetBindMac(nHashCode != 0);
			_TempFlags.SetEncrypt(true);
			if (Document.size()) {
				Json::FastWriter _Writer;
				std::string json = _Writer.write(Document);
				CStringA sDoc;
				int nCoder = Encrypt::HEX ;
				if (_TempFlags.IsBase64Coder()) {
					nCoder = Encrypt::BASE64 ;
				}
				if (_TempFlags.IsBindMac()) {
					// 硬件MAC为私钥，时间为公钥的随机散列+表置换加密
					Encrypt::TXOR::EncryptString(json.c_str(), sDoc, nHashCode, Encrypt::TXOR::GetPublicKey(sTime),nCoder);
				}
				else {
					// 采用随机散列XOR加密
					Encrypt::RandXOR::EncryptString(json.c_str(),sDoc,Encrypt::RandXOR::GetHashKey(sTime),nCoder);
				}
				CStringA sCode;
				if (_TempFlags.IsSignMD5()) {
					// MD5签名
					sCode = Sign::MD5_Hash(sDoc);
				}
				else {
					// CRC32 签名
					sCode.Format("%d",Sign::CRC_Hash(sDoc));
				}
				Root["code"] = (LPCSTR)sCode;
				Root["zdata"] = (LPCSTR)sDoc;				
			}
		}
		else {
			if (Flags.IsEncrypt()) {
				_TempFlags.SetEncrypt(false);
			}
			if (Document.size()) {
				Root["zdata"] = Document;
			}
			Root["code"] = "0";
		}
		Root["flags"] = (int)_TempFlags._Get();
		Root["time"] = (LPCSTR)sTime;
		Doc[(LPCSTR)Command] = Root;
 		
		if ( DocFlags.IsStyled() ) {
			Json::StyledWriter _Writer;
			return _Writer.write(Doc).c_str();
		}
		else {
			Json::FastWriter _Writer;
			return _Writer.write(Doc).c_str();
		}
	}

	template <typename T>
	inline Json::Value& CDocumentImpl<T>::SetDocument(const BSTR bstrDoc, long nHashCode )
	{
		return SetDocument(CW2A(bstrDoc),nHashCode);
	}

	template <typename T>
	inline Json::Value& CDocumentImpl<T>::SetDocument(LPCSTR lpszDocument, long nHashCode)
	{
		Document.clear();
		_Succeeded = false;
		if (!lpszDocument) {
			return Document;
		}

		Json::Reader _Reader;
		Json::Value  _Root;
		std::string  json = lpszDocument;

		Description  = "";
		SessionId = "";
		if (_Reader.parse(json,_Root))	{
			Command = _Root.getMemberNames()[0].c_str();
			Command.Trim().MakeLower();
			if (Command == "") {
				SetLastError(CMD_ERROR_COMMAND, "未声明有效的命令关键字");
				goto _Exit;
			}
			Json::Value Cmd = _Root[(LPCSTR)Command];
			if (!Cmd.isMember("flags")) {
				SetLastError(CMD_ERROR_FORMAT, "未发现[flags]参数");
				goto _Exit;
			}
			Flags._Set(Cmd["flags"].asInt());
			if (Cmd.isMember("session")) {
				SessionId = Cmd["session"].asString().c_str();
			}
			if (!static_cast<T*>(this)->OnReadDocument(Cmd)) {
				goto _Exit;
			}
			// 如果发现实体传输数据，则解析数据
			if (Cmd.isMember("zdata")) {
				if (Flags.IsEncrypt()) {
					if (!Cmd.isMember("time") || !Cmd.isMember("code")) {
						SetLastError(CMD_ERROR_FORMAT,"不存在[time]或[code]参数");
						goto _Exit;
					}
					bool fSucceeded = false;
					std::string parameters = Cmd["zdata"].asString();

					if (Flags.IsSignMD5()) {
						fSucceeded = ( Sign::MD5_Hash(parameters) == Cmd["code"].asString().c_str() );
					}
					else {
						fSucceeded = ( Sign::CRC_Hash(parameters) == (long)StrToIntA(Cmd["code"].asString().c_str()) );
					}
					if (!fSucceeded) {
						SetLastError(CMD_ERROR_SIGN, "数据签名错误");
						goto _Exit;
					}
					std::string sParameters;
					INT nCoder = Encrypt::HEX ;
					if (Flags.IsBase64Coder()) {
						nCoder = Encrypt::BASE64 ;
					}

					if (Flags.IsBindMac() && nHashCode == 0) {
						DbgPrintf("未设定有效的硬件绑定信息");
						Flags.SetBindMac(false);
					}

					if (Flags.IsBindMac()) {
						if (nHashCode == 0) {
							SetLastError(CMD_ERROR_MACHINE,"未设定有效的硬件绑定信息");
							goto _Exit;
						}
						unsigned int nPublicKey = Encrypt::TXOR::GetPublicKey(Cmd["time"].asString().c_str());
						Encrypt::TXOR::DecryptString(parameters.c_str(),sParameters,(unsigned int)nHashCode,nPublicKey,nCoder);
					}
					else {
						LONG nPublicKey = (LONG)Encrypt::RandXOR::GetHashKey(Cmd["time"].asString().c_str());
						Encrypt::RandXOR::DecryptString(parameters.c_str(),sParameters,(LONG)nPublicKey,nCoder);
					}
					if (!(fSucceeded = _Reader.parse(sParameters,Document))) {
						SetLastError(CMD_ERROR_JSON,"解析失败！无效返回JSON数据");
						Document.clear();
						goto _Exit;
					}
				}
				else {
					Document = Cmd["zdata"];
				}
			}
			_Succeeded = true;
		}
		else {
			SetLastError(CMD_ERROR_JSON,"解析失败！无效数据包或非JSON数据");
		}
	_Exit:
		return Document;
	}

	template <typename T>
	inline void CDocumentImpl<T>::Destroy() 
	{
		Command = "";
		_ErrorCode = 0;
		_Succeeded = false;
		Description = "";
		SessionId = "";
		Flags._Set(0);
		Document.clear();
	}
	
	template <typename T>
	inline void CDocumentImpl<T>::SetLastError(int nCode, LPCSTR lpszError)	
	{
		_ErrorCode = nCode;
		if (_ErrorCode != 0) {
			Flags.SetEncrypt(false);
		}
		Description = lpszError;
	}
	
	template <typename T>
	inline void CDocumentImpl<T>::SetCommand(LPCSTR lpszCmd)	
	{
		Command = lpszCmd;
		Command.Trim().MakeLower();
	}
	
	template <typename T>
	inline bool CDocumentImpl<T>::HaveParameter(LPCSTR lpszParameter) 
	{
		CStringA sKey = lpszParameter;
		sKey.Trim().MakeLower();
		return Document.isMember((LPCSTR)sKey);
	}
	
	template <typename T>
	inline bool CDocumentImpl<T>::HaveParameters(LPCSTR lpszParameters[]) 
	{
		for (int i = 0; lpszParameters[i] ; i++) {
			CStringA sKey = lpszParameters[i];
			DbgPrintf(sKey);
			sKey.Trim().MakeLower();
			if (!Document.isMember((LPCSTR)sKey))
				return false;
		}
		return true;
	}

	template <typename T>
	inline void CDocumentImpl<T>::SetParameter(LPCSTR lpszKey, LPCSTR lpszValue)	
	{
		CStringA sKey(lpszKey);
		sKey.Trim().MakeLower();
		Document[(LPCSTR)sKey] = lpszValue;
	}

	template <typename T>
	inline void CDocumentImpl<T>::SetParameter(LPCSTR lpszKey, INT nValue)
	{
		CStringA sKey(lpszKey);
		sKey.Trim().MakeLower();
		Document[(LPCSTR)sKey] = nValue;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/*
		类名称： CCommandDocument
		类功能:  客户端发送请求命令数据包
	*/
	inline CCommandDocument::CCommandDocument(LPCSTR lpszCmd , DWORD nFlags) 
	{
		SetCommand(lpszCmd);
		Flags._Set(nFlags);
	}
	
	inline CCommandDocument::CCommandDocument(LPCSTR lpszCmd, const Json::Value& _Parameters, DWORD nFlags ) 
	{
		SetCommand(lpszCmd);
		Document = _Parameters;
		Flags._Set(nFlags);
	}
	
	inline CCommandDocument::CCommandDocument(LPCSTR lpszCmd, LPCSTR lpszJson, DWORD nFlags)	
	{
		Json::Reader _Reader;
		if (_Reader.parse(lpszJson,Document)) {
			SetCommand(lpszCmd);
			Flags._Set(nFlags);
		}
	}
	
	inline CCommandDocument::CCommandDocument(CCommand& Cmd)	
	{
		SetCommand(CT2A(Cmd.Command));
		Flags._Set(Cmd.Flags._Get());
		Cmd.GetJson(Document,1);
	}
	
	inline bool CCommandDocument::OnWriteDocument(Json::Value &Root) 
	{
		Command.Trim().MakeLower();
		if (Command == "") {
			SetLastError(CMD_ERROR_COMMAND,"命令字为空");
			return false;
		}
		return true;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  	/*
		类名称: CRequestDocument
		类功能: WEB端调用接收并解析客户端命令请求
	*/
	inline CRequestDocument::CRequestDocument(LPCSTR lpszDocument, LONG nHashCode ) 
	{
		SetDocument(lpszDocument, nHashCode);
	}
	
	inline CRequestDocument::CRequestDocument(BSTR bstrDocument, LONG nHashCode) 
	{
		SetDocument(bstrDocument,nHashCode);
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	inline CResponeDocument::CResponeDocument() 
		: CDocumentImpl() 
	{}

	inline CResponeDocument::CResponeDocument(CRequestDocument& Doc) 
	{
		SetRequestDocument(Doc);
	}
	
	inline CResponeDocument::CResponeDocument(const CCommandDocument& Doc) 
	{
		Command = Doc.Command;
		Flags = Doc.Flags._Get();
	}
	
	inline CResponeDocument::CResponeDocument(LPCSTR lpszCmd, int ErrorCode, LPCSTR lpszError) 
	{
		SetResponeError(lpszCmd,ErrorCode,lpszError);
	}

	inline CResponeDocument::CResponeDocument(LPCSTR lpszDocument) 
	{
		SetDocument(lpszDocument,Sign::MAC_Hash());
	}

	inline void CResponeDocument::SetRequestDocument(CRequestDocument &Doc) 
	{
		Command = Doc.GetCommand();
		Flags = Doc.Flags._Get();
		if (!Doc.IsSucceeded()) {
			_ErrorCode = Doc.ErrorCode();
			Description = Doc.Description;
		}
	}
	
	inline void CResponeDocument::SetResponeError(LPCSTR lpszCmd, int nErrorCode, LPCSTR lpszError) 
	{
		SetCommand(lpszCmd);
		SetLastError(nErrorCode,lpszError);
		Flags._Set(0);
	}

	inline bool CResponeDocument::OnReadDocument(const Json::Value &Cmd)	
	{
		if (!Cmd.isMember("result")) {
			SetLastError(CMD_ERROR_FORMAT,"未知返回状态描述");
			return false;
		}
		_ErrorCode = Cmd["result"].asInt();
		if (Cmd.isMember("description")) {
			Description = Cmd["description"].asString().c_str();
		}
		return true;
	}

	inline bool CResponeDocument::OnWriteDocument(Json::Value &Root)	
	{
		Command.Trim().MakeLower();
		Root["result"] = _ErrorCode;
		Root["description"] = (LPCSTR)Description;
		return true;
	}
 };