#pragma once
#include <vector>
using namespace  std;

#define FOR_EACH(it,vec) \
	for(it = vec.begin();it != vec.end();++it)
class Helper
{
public:
	Helper(void);
	~Helper(void);
	//转换UTF8字符串为宽字节
	static  wstring		    UTF8ToUnicode(const string& str);
	static  string			UnicodeToUTF8(const wstring& wStr);
	static  string		 	UnicodeToUTF8(const wchar_t* szWstr);
	static  void			UnicodeToUTF8(const wchar_t *wchar, char *chr, int length);
	static  void			UnicodeToAnsi(const wchar_t *wchar, char *chr, int length);
	static  string			UnicodeToAnsi(const wstring wstr);
	static  string			UTF8ToAnsi(const string& str );
	static  wstring		    AnsiToUnicode(const string& str);
	static  void			splitstring(string& str,char c,vector<string>& vstr);
	static  void			splitstring(wstring& wstr,wchar_t wc,vector<wstring>& vwstr);
	static  string			Post(string url,string file,string prm);
	static  string          Post_PORT8090(string url, string file, string prm);
	static  string			Get(string url,string file);
	static  bool            Bxp();
	static  int             GetSystemBits();//查看当前系统是32还是64
	static  void            PRINTF(const char *format, ...);//输出
};

