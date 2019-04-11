#ifndef __CMD_MYSQL__
#define __CMD_MYSQL__
#pragma once
#include "CmdDB.h"
#include "MySQL/MySQLConnect.h"
#include "Json/Json.h"
#include "Log/DbgPrint.h"

class CMySQLSessionConnect : public CDBConnectImpl<CMySQLSessionConnect,CMySQLConnect>
{
public:
	HRESULT OnOpenConnect(LPCSTR szConn, CMySQLConnect** ppConnect)
	{
		Json::Value config;
		Json::Reader reader;
		std::string host,user,pass,db;
		std::string json = szConn;
		if (!reader.parse(json,config)) {
			DbgPrintf("�޷�����MYSQL����JSON ����");
			return E_INVALIDARG;
		}

		if (!config.isMember("host") || !config.isMember("user") || !config.isMember("pass") || !config.isMember("db")) {
			DbgPrintf("����������Ϣ�в�������ص�������Ϣ��");
			return E_INVALIDARG;
		}

		int port = 3306;
		host = config["host"].asString();
		user = config["user"].asString();
		pass = config["pass"].asString();
		db = config["db"].asString();

		if (config.isMember("port")) {
			port = config["port"].asInt();
		}

		CMySQLConnect *pConnect = new CMySQLConnect;
		if (!pConnect) {
			DbgPrintf("�޷�����MySQL���Ӷ���");
			return E_OUTOFMEMORY;
		}

		if (!pConnect->Open(host.c_str(),user.c_str(),pass.c_str(),db.c_str(),port)) {
			*ppConnect = pConnect;
			return S_OK;
		}
		else
		{
			DbgPrintf("�޷��������ݿ⣡(host:%s, user:%s, pass:%s, db:%s, port:%d", host.c_str(),user.c_str(),pass.c_str(),db.c_str(),port);
			delete pConnect;
			*ppConnect = NULL;
			return E_FAIL;
		}
	}
};

//typedef CDBConnectPoolImpl<CMySQLSessionConnect> CMySQLSessionConnectPool;
#endif