#pragma once
#include <Windows.h>
#include <mysql.h> 
#pragma comment(lib,"libmysql.lib")


class CMysql{
public:
	CMysql(char *user, char *pswd, char *host, char *table, unsigned int port);
	~CMysql();
	bool AddSQL(const char *addstr);
	bool DeleteSQL(const char *delstr);
	bool ChangeSQL(const char *changestr);
	MYSQL_RES* FindSql(const char *findstr);
	int GetSelectNum(const char *findstr);//获取查询数目
private:
	MYSQL myCont;
private:
	char *_user;
	char *_pswd;
	char *_host;
	char *_table;
	unsigned int _port;
	bool _InitSuccess;
};