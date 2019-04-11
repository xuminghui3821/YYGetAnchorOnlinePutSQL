#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <shlwapi.h>
#include <sstream>
#include <iosfwd>
#include <fstream>
#include <time.h>
#include<regex>
#pragma comment(lib,"shlwapi.lib")
#include "CMysql.h"
#include "Helper.h"
#include "json/json.h"
#include "cmd/cmdenc.h"
#include "UniCodec.h"
using namespace std;


void WriteException(const char *data){//异常
	SYSTEMTIME st = { 0 };
	GetLocalTime(&st);
	char buffer[1024] = { 0 };
	sprintf_s(buffer, "%d-%02d-%02d %02d:%02d:%02d  【异常】：%s",
		st.wYear,
		st.wMonth,
		st.wDay,
		st.wHour,
		st.wMinute,
		st.wSecond,
		data);
	printf(buffer);
	ofstream OutFile("./errinfo.txt", ios::app);
	OutFile << buffer;
	OutFile.close();
}


std::string UtfToGbk(string &utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	utf8 = str;
	delete str;
	return utf8;
}

void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst)
{
	std::string::size_type pos = 0;
	std::string::size_type srclen = strsrc.size();
	std::string::size_type dstlen = strdst.size();

	while ((pos = strBig.find(strsrc, pos)) != std::string::npos)
	{
		strBig.replace(pos, srclen, strdst);
		pos += dstlen;
	}
}

void CharacterRangeFiltering(string &str){//过滤超过55215的字符
	wstring wstr = Helper::AnsiToUnicode(str);
	wstring wtemp = L"";
	for (int i = 0; i < wstr.length(); i++)
	if (wstr.at(i) < 55215)
		wtemp += wstr.at(i);

	str = Helper::UnicodeToAnsi(wtemp);
}

long long HotAnchor[50] = { //这50个主播在线状态写入为3（大主播，即使不在线也有人送礼物，所以设置为3这样也可以采集到送礼物数据）
	999315, 6152663, 9140074, 10869329, 12496500, 16670139,
	18227896, 19766694, 21716566, 21718060, 22420806, 23974338,
	25231256, 29058263, 29112250, 30870813, 32007046, 32791189,
	33448555, 35595711, 36280888, 36821911, 36933349, 39542117,
	40574648, 48148222, 48202337, 50958424, 51214070, 53350304,
	55132557, 55896398, 58702480, 67673627, 69983689, 73208027,
	73544684, 77959998, 85520900, 85784083, 85931665, 86300079,
	88394999, 89703802, 89754948, 92912734, 96756925, 99558315,
	1349346555, 1351992921
};

bool FindHotAnchor(long long id){
	for (int i = 0; i < 50; i++)
	if (HotAnchor[i] == id)
		return true;
	return false;
}

void UpdateOnline(CMysql &m_sql, string biz, string desc, string moduleid, string name, string sid, string ssid, string tag, string tagstyle, string lasttime){//更新主播数据
	static UINT32 processid = 0;
	//先过率数据库不支持的字符
	CharacterRangeFiltering(name);
	CharacterRangeFiltering(tag);
	CharacterRangeFiltering(desc);
	//再清洗数据
	string_replace(name, "'", "");
	string_replace(name, "\\", "");
	string_replace(tag, "'", "");
	string_replace(tag, "\\", "");
	string_replace(desc, "'", "");
	string_replace(desc, "\\", "");


	char sqlbuffer[1024] = { 0 };
	sprintf_s(sqlbuffer, "SELECT * FROM yycaiji WHERE sid = %s", sid.c_str());
	int num = m_sql.GetSelectNum(sqlbuffer);
	if (num != -1){
		if (num == 0){//没有这个主播则添加到数据库中
			ZeroMemory(sqlbuffer, 1024);
			processid %= 3;
			sprintf_s(sqlbuffer, "insert into yycaiji (biz,_desc,moduleid,name,sid,ssid,tag,tagstyle,processid,inline,lasttime) values('%s','%s','%s','%s','%s','%s','%s','%s','%d','1','%s')",
				biz.c_str(), desc.c_str(), moduleid.c_str(), name.c_str(), sid.c_str(), sid.c_str(), tag.c_str(), tagstyle.c_str(), processid, lasttime.c_str());
			//printf(sqlbuffer);
			m_sql.ChangeSQL(sqlbuffer);
			processid++;
		}
		else{//已经存在了则修改状态
			ZeroMemory(sqlbuffer, 1024);
			if (FindHotAnchor(atoll(sid.c_str())))
				sprintf_s(sqlbuffer, "update yycaiji set lasttime='%s',inline = '1' where sid= '%s'", lasttime.c_str(), sid.c_str());
			else
				sprintf_s(sqlbuffer, "update yycaiji set lasttime='%s',inline = '1' where sid= '%s'", lasttime.c_str(), sid.c_str());

			//printf(sqlbuffer);
			m_sql.AddSQL(sqlbuffer);
		}
	}
}


int main()
{
	//wstring aa = L"五棵草—婷姐海鲜馃【HP】仙女阿姨馃?♀?小仙女馃?♀?（求大哥）";
	//string ss = Helper::UnicodeToAnsi(aa);
	//CharacterRangeFiltering(ss);
	//
	//	std::ifstream t1("c:/err.txt"); //读文件ifstream,写文件ofstream，可读可写fstream
	//	std::stringstream buffer;
	//	buffer << t1.rdbuf();
	//	std::string data = buffer.str();
	//std:wstring wdata = Helper::AnsiToUnicode(data);
	//	YCodec sUni;
	//	string str2 = sUni.Chinese2Unicode((WCHAR *)wdata.c_str(), wdata.length());
	//
	//	
	//
	//
	//	Json::Reader reader;
	//	Json::Value root;
	//	if (reader.parse(data, root)){
	//		int i = 5;
	//	}
	//	else{
	//		int j = 6;
	//	}
	//
	//
	//

	string UrlList[] = {//各种频道
		"biz=dance&subBiz=idx&page=%d&moduleId=313&pageSize=100",
		"biz=sing&subBiz=idx&page=%d&moduleId=308&pageSize=100",
		"biz=talk&subBiz=idx&page=%d&moduleId=328&pageSize=100",
		"biz=red&subBiz=idx&page=%d&moduleId=1872&pageSize=100",
		// "biz=game&subBiz=idx&page=%d&moduleId=1180&pageSize=9999",
		"biz=mc&subBiz=idx&page=%d&moduleId=322&pageSize=100"
	};
	//CMysql m_sql("root", "3821304091", "127.0.0.1", "yy", 3306);
	CMysql m_sql("root", "zhenghao18183002", "127.0.0.1", "caiji", 43306);

	time_t t;
	string biz;
	string desc;
	string moduleid;
	string name;
	string sid;
	string ssid;
	string tag;
	string tagstyle;
	string lasttime;
	while (true)
	{
		m_sql.ChangeSQL("update yycaiji set inline = 0 where inline = 2");
		m_sql.ChangeSQL("update yycaiji set inline = 2 where inline = 1");
		time(&t);
		lasttime = to_string(t);
		int OnlineAnchorNum = 0;
		for (int i = 0; i < 5; i++){
			for (int j = 1; true; j++){
				char buffer[MAX_PATH] = { 0 };
				sprintf_s(buffer, UrlList[i].c_str(), j);
				string data = Helper::Post("www.yy.com", "/more/page.action?", buffer);
				data = UtfToGbk(data);
				wstring wdata = Helper::AnsiToUnicode(data);
				YCodec sUni;
				data = sUni.Chinese2Unicode((WCHAR *)wdata.c_str(), wdata.length());
				if (data.length() > 100){
					Json::Reader reader;
					Json::Value root;
					if (reader.parse(data, root))  // reader将Json字符串解析到root，root将包含Json里所有子元素  
					{
						if (root.isMember("data"))//如果有这个节点
						{
							Json::Value &data = root["data"];
							if (data.isMember("data")){
								Json::Value &data_data = data["data"];
								OnlineAnchorNum += data_data.size();
								for (int i = 0; i < data_data.size(); i++){//遍历所有主播数据
									biz = "";
									desc = "";
									moduleid = "";
									name = "";
									sid = "";
									ssid = "";
									tag = "";
									tagstyle = "";

									if (data_data[i]["biz"].isString())
										biz = data_data[i]["biz"].asCString();
									if (data_data[i]["desc"].isString())
										desc = data_data[i]["desc"].asCString();
									if (data_data[i]["moduleId"].isInt())
										moduleid = to_string(data_data[i]["moduleId"].asInt());
									if (data_data[i]["name"].isString())
										name = data_data[i]["name"].asCString();
									if (data_data[i]["sid"].isInt())
										sid = to_string(data_data[i]["sid"].asInt());
									if (data_data[i]["ssid"].isInt())
										ssid = to_string(data_data[i]["ssid"].asInt());
									if (data_data[i]["tag"].isString())
										tag = data_data[i]["tag"].asCString();
									if (data_data[i]["tagStyle"].isInt())
										tagstyle = to_string(data_data[i]["tagstyle"].asInt());

									UpdateOnline(m_sql, biz, desc, moduleid,
										name, sid, ssid, tag, tagstyle, lasttime);
								}
							}
							else
								WriteException("找不到第二个data");
						}
						else
							WriteException("找不到第一个data");
					}
					else
						WriteException("JSON读取错误");
				}
				else
					break;
			}

		}
		SYSTEMTIME st = { 0 };
		GetLocalTime(&st);
		printf("%d-%02d-%02d %02d:%02d:%02d  采集结束，当前在线主播%d人\n",
			st.wYear,
			st.wMonth,
			st.wDay,
			st.wHour,
			st.wMinute,
			st.wSecond,
			OnlineAnchorNum);
		Sleep(1000 * 60 * 5);
	}
	system("pause");
	return 0;
}

