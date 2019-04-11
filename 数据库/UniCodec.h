///�ַ�����Unicode����ת����
#pragma once
//#include <vector>
//#include <iostream>
//using namespace std;

class YCodec
{
	///����ת��ͨ��������Ӧ����������ת��ͨ��ǿ��ת��������ת����������ͨ��С��Χ�У�����ת���������з�Χ������ת����Ͼ�ʵ���ˡ�
public:
	YCodec();
	~YCodec();
	// char to hex code ����ת������9��->9��   ǰ�ĸ�����16���Ƶ�ascii��ת��  ����-16�������ֺ�16���������ִ���ʾ���ת����
	int Char2Hex(char ch);
	// hex to char ����ת����9->��9����
	char Hex2Char(unsigned int n);
	// string to hex code ����ת����"a29"->a29��
	long String2Hex(char* string, int strlen);
	// hex to char* ����ת����a29->"a29"��
	int Hex2String(long hex, char* string, int* slen);

	// num^index ����Ȩ�أ�16���̣���2λ����λ����16*16��
	long MIndex(int num, int index);

	// hex to wchar_t code ����ת����ascii��33->��ʾ���ַ�!��  �������ǵ����ַ���ascii���
	wchar_t Hex2Wchart(long hex);
	// wchar_t to hex code ����ת�����ַ�!->��ʾ��ascii��33��
	long Wchart2Hex(wchar_t wch);

	///-------------------------------------------------Unicode�벻��\u�Ĵ��汾
	// UNICODEʮ�������ַ���ת����Ӣ�� //�ֽڼ���
	// hex string to wchar_t*                               ����������Ҫ���󳤶��ٵý���� ԭʼ�汾
	int HexStr2WChars(char* hexstr, int hexstrlen, int eachchar, wchar_t* wchs, int* wchslen);
	// ��Ӣ��ת��UNICODEʮ�������ַ���
	// wchar_t* to char*(hex string)
	int Wchart2HexStr(wchar_t* wchs, int wchslen, int eachchar, char* hexstr, int* hexstrlen);

	//------------------ //�ִ� �ֽڼ���
	string Str2Unicode(wchar_t* wchs,int len);
	wstring Unicode2Str(char* str,int len );
	string Str2Unicode(wchar_t* wchs,int len ,char* out ,int& out_len);///����תUnicode����
	wstring Unicode2Str(char* str,int len ,WCHAR* out ,int& out_len);///Unicode����ת����
	///--------------------------------
public:
	///---------------------------------------------------///Unicode���\u�İ汾
	///------------�ֽڼ���
	//string& replace_all(string&   str,const   string&   old_value, const   string&   new_value) ;
	int Wchart2HexStr_U(wchar_t* wchs, int wchslen, int eachchar, char* hexstr, int* hexstrlen);
	int HexStr2WChars_U(char *hexstr, int hexstrlen, int eachchar, wchar_t *wchs, int *wchsLen); 
	string Str2Unicode_U(wchar_t* wchs,int len ,char* out ,int& out_len);
	wstring Unicode2Str_U(char* str,int strlen ,WCHAR* out ,int& out_len);

	///------------�ִ�����
	string Str2Unicode_StrU(wchar_t* wchs,int len);///����תUnicode����
	wstring Unicode2Str_StrU(char* str,int len);///Unicode����ת����


	///-----------------------------------------------------��Ӣ�ķֿ������  Unicode���\u�İ汾
	///------------�ִ�����
	int IncludeChinese(char *str);///�ж����޺���
	vector<string> SplitStr(char* sztext /*,int nlen*/);///���ִ����Ƿ��ֳַ�����
	string SpliceStr(vector<string> vecC);///���벢�ϲ�
	string Chinese2Unicode(char * str);///ֻ���ִ�������ת��ΪUnicode����

	vector<string> SplitUnicodeStr(char* sztext /*,int nlen*/);///�Ѵ�URL�����ִ����Ƿ�URL����ֳ�����
	string SpliceUnicodeStr(vector<string> vecC);///���벢�ϲ�
	string Unicode2Chinese(char * str);///�κ���Unicode����Զ�ת��Ϊ��Ӧ�ַ���

	///--------�ֽڼ���
	int Wchart2HexStr_Gap(wchar_t* wchs, int wchslen, int eachchar, char* hexstr, int* hexstrlen);
	int HexStr2WChars_Gap(char *hexstr, int hexstrlen, int eachchar, wchar_t *wchs, int *wchsLen);
	string Chinese2Unicode(wchar_t* wchs, int len, char* out, int& out_len);
	string Chinese2Unicode(wchar_t* wchs, int len);
	wstring Unicode2Chinese(char* str,int len ,WCHAR* out ,int& out_len);
};
