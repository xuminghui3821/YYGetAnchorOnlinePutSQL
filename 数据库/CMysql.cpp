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
	sprintf_s(buffer,"%d-%02d-%02d %02d:%02d:%02d  �������%s,����ԭ��%s\r\n",
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
	mysql_init(&myCont);///��ʼ��
	if (mysql_real_connect(&myCont, host, user, pswd, table, port, NULL, 0))///���ӵ�ָ����
	{
		mysql_query(&myCont, "SET NAMES GBK"); //���ñ����ʽ,������cmd���޷���ʾ����///���û�ȡ���ݱ���
		_InitSuccess = true;
	}
	else{
		printf("���ݿ�����ʧ��\r\n");
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
			//printf("\"%s\" ִ�б�������ԭ�� %s\r\n", addstr, mysql_error(&myCont));
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
			//printf("\"%s\" ִ�б�������ԭ�� %s\r\n", delstr, mysql_error(&myCont));
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
			//printf("\"%s\" ִ�б�������ԭ�� %s\r\n", changestr, mysql_error(&myCont));
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
		res = mysql_query(&myCont, findstr);///��ѯ
		if (!res)
		{
			result = mysql_store_result(&myCont);//�����ѯ�������ݵ�result///�����ѯ����
			if (result)
			{
				return result;
				//int i, j;
				//printf("��ȡ����������%d\r\n", mysql_num_rows(result));
				//for (i = 0; fd = mysql_fetch_field(result); i++)///��ȡ����(�ֶ���)
				//{
				//	strcpy(column[i], fd->name);
				//}
				//j = mysql_num_fields(result);///��ȡ����(�ֶ���)��
				//for (i = 0; i < j; i++)
				//{
				//	printf("%s\t", column[i]);
				//}
				//printf("\n");
				//while (sql_row = mysql_fetch_row(result))//��ȡ���������
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

int CMysql::GetSelectNum(const char *findstr)//��ȡ��ѯ��Ŀ
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
		mysql_close(&myCont);//�Ͽ�����
}