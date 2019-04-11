#include <stdio.h>
#include<fstream>
#include<iostream>
#include "CMysql.h"
#include <time.h>
using namespace std;



void WriteErr(const char *sql,const char *errinfo){
	SYSTEMTIME st = { 0 };
	GetLocalTime(&st);
	char buffer[1024] = { 0 };
	sprintf_s(buffer,"%d-%02d-%02d %02d:%02d:%02d  错误语句%s,错误原因%s\r\n",
		st.wYear,
		st.wMonth,
		st.wDay,
		st.wHour,
		st.wMinute,
		st.wSecond, 
		sql, errinfo
		);
	printf(buffer);
	ofstream OutFile("./errinfo.txt", ios::app);
	OutFile << buffer;
	OutFile.close();
}




CMysql::CMysql(char *user, char *pswd, char *host, char *table, unsigned int port) :
_InitSuccess(0)
{
	_user = user;
	_pswd = pswd;
	_host = host;
	_table = table;
	_port = port;
	mysql_init(&myCont);///初始化
	if (mysql_real_connect(&myCont, host, user, pswd, table, port, NULL, 0))///链接到指定表
	{
		mysql_query(&myCont, "SET NAMES GBK"); //设置编码格式,否则在cmd下无法显示中文///设置获取数据编码
		_InitSuccess = true;
	}
	else{
		printf("数据库链接失败\r\n");
	}
}

bool CMysql::AddSQL(const char *addstr){
	if (_InitSuccess){
		int res;
		res = mysql_query(&myCont, addstr);
		if (!res)
			return true;
		else{
			WriteErr(addstr, mysql_error(&myCont));
			//printf("\"%s\" 执行报错，错误原因 %s\r\n", addstr, mysql_error(&myCont));
			return false;
		}
		return !res;
	}
	return false;
}

bool CMysql::DeleteSQL(const char *delstr){
	if (_InitSuccess){
		int res;
		res = mysql_query(&myCont, delstr);
		if (!res)
			return true;
		else{
			WriteErr(delstr, mysql_error(&myCont));
			//printf("\"%s\" 执行报错，错误原因 %s\r\n", delstr, mysql_error(&myCont));
			return false;
		}
		return !res;
	}
	return false;
}

bool CMysql::ChangeSQL(const char *changestr){
	if (_InitSuccess){
		int res;
		res = mysql_query(&myCont, changestr);
		if (!res)
			return true;
		else{
			WriteErr(changestr, mysql_error(&myCont));
			//printf("\"%s\" 执行报错，错误原因 %s\r\n", changestr, mysql_error(&myCont));
			return false;
		}
		return !res;
	}
	return false;
}

MYSQL_RES * CMysql::FindSql(const char *findstr){
	if (_InitSuccess){
		int res;
		MYSQL_RES *result;
		//MYSQL_FIELD *fd;
		MYSQL_ROW sql_row;
		char column[32][32];
		res = mysql_query(&myCont, findstr);///查询
		if (!res)
		{
			result = mysql_store_result(&myCont);//保存查询到的数据到result///保存查询数据
			if (result)
			{
				return result;
				//int i, j;
				//printf("获取到的数据数%d\r\n", mysql_num_rows(result));
				//for (i = 0; fd = mysql_fetch_field(result); i++)///获取列名(字段名)
				//{
				//	strcpy(column[i], fd->name);
				//}
				//j = mysql_num_fields(result);///获取列名(字段名)数
				//for (i = 0; i < j; i++)
				//{
				//	printf("%s\t", column[i]);
				//}
				//printf("\n");
				//while (sql_row = mysql_fetch_row(result))//获取具体的数据
				//{
				//	for (i = 0; i < j; i++)
				//	{
				//		printf("%s\n", sql_row[i]);
				//	}
				//	printf("\n");
				//}
			}
			else
				return NULL;
		}
		else
		{
			WriteErr(findstr, mysql_error(&myCont));
			return NULL;
		}
	}
	return NULL;
}

int CMysql::GetSelectNum(const char *findstr)//获取查询数目
{
	MYSQL_RES *result = FindSql(findstr);
	if (result == NULL)
		return -1;
	else
	{
		int num = mysql_num_rows(result);
		mysql_free_result(result);
		return num;
	}
}

CMysql::~CMysql(){
	if (_InitSuccess)
		mysql_close(&myCont);//断开连接
}