///字符串和Unicode编码转换。
#pragma once
//#include <vector>
//#include <iostream>
//using namespace std;

class YCodec
{
	///类型转换通过查表或相应函数，编码转换通过强制转换。类型转换是在两者通用小范围中，编码转换是在所有范围。两种转换结合就实现了。
public:
	YCodec();
	~YCodec();
	// char to hex code 类型转换（‘9’->9）   前四个不是16进制的ascii码转换  而是-16进制数字和16进制数字字串表示间的转换。
	int Char2Hex(char ch);
	// hex to char 类型转换（9->‘9’）
	char Hex2Char(unsigned int n);
	// string to hex code 类型转换（"a29"->a29）
	long String2Hex(char* string, int strlen);
	// hex to char* 类型转换（a29->"a29"）
	int Hex2String(long hex, char* string, int* slen);

	// num^index 计算权重（16进程，第2位（百位）是16*16）
	long MIndex(int num, int index);

	// hex to wchar_t code 编码转换（ascii的33->表示的字符!）  下两个是单个字符的ascii查表
	wchar_t Hex2Wchart(long hex);
	// wchar_t to hex code 编码转换（字符!->表示的ascii的33）
	long Wchart2Hex(wchar_t wch);

	///-------------------------------------------------Unicode码不带\u的纯版本
	// UNICODE十六进制字符串转成中英文 //字节级别
	// hex string to wchar_t*                               下两个是需要先求长度再得结果的 原始版本
	int HexStr2WChars(char* hexstr, int hexstrlen, int eachchar, wchar_t* wchs, int* wchslen);
	// 中英文转成UNICODE十六进制字符串
	// wchar_t* to char*(hex string)
	int Wchart2HexStr(wchar_t* wchs, int wchslen, int eachchar, char* hexstr, int* hexstrlen);

	//------------------ //字串 字节级别
	string Str2Unicode(wchar_t* wchs,int len);
	wstring Unicode2Str(char* str,int len );
	string Str2Unicode(wchar_t* wchs,int len ,char* out ,int& out_len);///中文转Unicode编码
	wstring Unicode2Str(char* str,int len ,WCHAR* out ,int& out_len);///Unicode编码转中文
	///--------------------------------
public:
	///---------------------------------------------------///Unicode码带\u的版本
	///------------字节级别
	//string& replace_all(string&   str,const   string&   old_value, const   string&   new_value) ;
	int Wchart2HexStr_U(wchar_t* wchs, int wchslen, int eachchar, char* hexstr, int* hexstrlen);
	int HexStr2WChars_U(char *hexstr, int hexstrlen, int eachchar, wchar_t *wchs, int *wchsLen); 
	string Str2Unicode_U(wchar_t* wchs,int len ,char* out ,int& out_len);
	wstring Unicode2Str_U(char* str,int strlen ,WCHAR* out ,int& out_len);

	///------------字串级别
	string Str2Unicode_StrU(wchar_t* wchs,int len);///中文转Unicode编码
	wstring Unicode2Str_StrU(char* str,int len);///Unicode编码转中文


	///-----------------------------------------------------中英文分开处理的  Unicode码带\u的版本
	///------------字串级别
	int IncludeChinese(char *str);///判断有无汉字
	vector<string> SplitStr(char* sztext /*,int nlen*/);///把字串按是否汉字分成数段
	string SpliceStr(vector<string> vecC);///编码并合并
	string Chinese2Unicode(char * str);///只把字串中中文转换为Unicode编码

	vector<string> SplitUnicodeStr(char* sztext /*,int nlen*/);///把带URL编码字串按是否URL编码分成数段
	string SpliceUnicodeStr(vector<string> vecC);///解码并合并
	string Unicode2Chinese(char * str);///晕好像Unicode码会自动转换为对应字符串

	///--------字节级别
	int Wchart2HexStr_Gap(wchar_t* wchs, int wchslen, int eachchar, char* hexstr, int* hexstrlen);
	int HexStr2WChars_Gap(char *hexstr, int hexstrlen, int eachchar, wchar_t *wchs, int *wchsLen);
	string Chinese2Unicode(wchar_t* wchs, int len, char* out, int& out_len);
	string Chinese2Unicode(wchar_t* wchs, int len);
	wstring Unicode2Chinese(char* str,int len ,WCHAR* out ,int& out_len);
};
