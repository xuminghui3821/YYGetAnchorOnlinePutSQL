#pragma once
#include <vector>
#include <iostream>
#include <string>
using namespace std;
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "UniCodec.h"
#define MYMAX_PATH MAX_PATH*20

string& replace_all(string&   str,const   string&   old_value, const   string&   new_value)   
{   
	while(true)
	{   
		string::size_type pos(0);
		if((pos=str.find(old_value))!=string::npos)   
			str.replace(pos,old_value.length(),new_value);   
		else break;   
	}   
	return str;   
}  
wstring AnsiToUnicode4(const string& str)
{
	int  unicodeLen = ::MultiByteToWideChar( CP_ACP,0,str.c_str(),-1,NULL,0);  
	wchar_t *  pUnicode;  
	pUnicode = new  wchar_t[unicodeLen];  
	memset(pUnicode,0,(unicodeLen)*sizeof(wchar_t));  
	::MultiByteToWideChar( CP_ACP,0,str.c_str(),-1,(LPWSTR)pUnicode, unicodeLen);  
	wstring  rt;  
	rt = ( wchar_t* )pUnicode;
	delete [] pUnicode; 
	return  rt;  
}
string UnicodeToAnsi6(const wstring wstr)
{
	char* pElementText;
	int iTextLen;
	// wide char to multi char
	iTextLen = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);

	pElementText = new char[iTextLen];
	memset((void*)pElementText, 0, sizeof(char) * (iTextLen));
	::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, pElementText, iTextLen, NULL, NULL);

	string strText;
	strText = pElementText;
	delete[] pElementText;
	return strText;
}
YCodec::YCodec()
{
}

YCodec::~YCodec()
{
}


// char to hex code
// error: return -1 
int YCodec::Char2Hex(char ch)
{
	int n = -1;
	switch(ch)
	{
	case '0':	n = 0;	break;
	case '1':	n = 1;	break;
	case '2':	n = 2;	break;
	case '3':	n = 3;	break;
	case '4':	n = 4;	break;
	case '5':	n = 5;	break;
	case '6':	n = 6;	break;
	case '7':	n = 7;	break;
	case '8':	n = 8;	break;
	case '9':	n = 9;	break;
	case 'A':
	case 'a':	n = 10;	break;
	case 'B':
	case 'b':	n = 11;	break;
	case 'C':
	case 'c':	n = 12;	break;
	case 'D':
	case 'd':	n = 13;	break;
	case 'E':
	case 'e':	n = 14;	break;
	case 'F':
	case 'f':	n = 15;	break;
	default:	break;
	}

	return n;
}

// hex to char
// error: return -1 
char YCodec::Hex2Char(unsigned int n)
{
	char ch;
	if(n>=0 && n<=9)	ch = 48 + n;
	else if(n>=10 && n<=15)	ch = 65 - 10 + n;
	else ch = -1;

	return ch;
}

// num^index
long YCodec::MIndex(int num, int index)
{
	long s = 1;
	int i=0;
	while(i<index)
	{
		s *= num;
		i++;
	}

	return s;
}

// string to hex code
// error: return -1 
long YCodec::String2Hex(char* string, int strlen)
{
	long hex=-1;
	int i=0, n=0;
	char *p = string;
	p += strlen - 1;
	if(string == NULL)	return hex;
	if(strlen <= 0 || strlen > 10)	return hex;

	hex = 0;
	do
	{
		n = Char2Hex(*p--);
		hex += n*MIndex(16, i++);
	}while(i<strlen);

	return hex;
}

// hex to char*
// string==NULL,slen = the size of string(slen as output)
// string!=NULL,input the length of string
// error: return -1
int YCodec::Hex2String(long hex, char* string, int* slen)
{
	char tmp[11] = {0};
	if(hex < 0)	return -1;
	if(string == NULL){// count the length it will be used
		sprintf(tmp, "%x", hex);
		*slen = strlen(tmp);
		return 1;
	}
	memset(string, 0, *slen);
	sprintf(string, "%x", hex);

	return 1;
}

// hex to wchar_t code
// eg: input 0x5e74, return 年
// error: return -1
wchar_t YCodec::Hex2Wchart(long hex)
{
	wchar_t wch = -1;
	if(hex <0)	return wch;
	wch = (wchar_t)hex;

	return wch;
}

// hex string to wchar_t*
// UNICODE十六进制字符串转成中英文
// hexstr每eachchar转换为一个wchar_t
// wchs == NULL, wchsLen as output(the size of wchs will be used)
// error: return -1
int YCodec::HexStr2WChars(char *hexstr, int hexstrlen, int eachchar, wchar_t *wchs, int *wchsLen)
{
	if(hexstr == NULL || hexstrlen <= 0 || eachchar <= 0)	return -1;
	if(wchs == NULL){// count the size wchs it will be used
		*wchsLen = hexstrlen/eachchar + (hexstrlen%eachchar>0 ? 1 : 0);
		return 1;
	}
	memset(wchs, 0, *wchsLen * sizeof(wchar_t));
	char* tmp = new char[eachchar+1];
	char* p = hexstr;
	wchar_t* pwch = wchs;
	for(int i=0; i<hexstrlen; i+=eachchar){
		memset(tmp, 0, eachchar+1);
		// get eachchar char
		for(int j=0; j<eachchar; j++){
			if(i+j > hexstrlen)	break;
			tmp[j] = *p++;
		}
		// char* to hex
		long hex = String2Hex(tmp, strlen(tmp));
		if(hex == -1)	continue;
		// hex to wchar_t
		*pwch++ = Hex2Wchart(hex);
	}

	if(tmp)	delete []tmp;

	return 1;
}
  
int YCodec::HexStr2WChars_U(char *hexstr, int hexstrlen, int eachchar, wchar_t *wchs, int *wchsLen)
{
	if(hexstr == NULL || hexstrlen <= 0 || eachchar <= 0)	return -1;
	if(wchs == NULL){// count the size wchs it will be used
		*wchsLen = hexstrlen/(eachchar +2) + (hexstrlen%(eachchar+2)>0 ? 1 : 0);//
		return 1;
	}
	memset(wchs, 0, *wchsLen * sizeof(wchar_t));
	char* tmp = new char[eachchar+1];
	char* p = hexstr;
	wchar_t* pwch = wchs;
	for(int i=0; i<hexstrlen; i+=eachchar){
		memset(tmp, 0, eachchar+1);
		// get eachchar char
		for(int j=0; j<eachchar; j++){
			if(i+j > hexstrlen)	break;
			if (*p=='\\'){//
				p++;//
				p++;}//
			tmp[j] = *p++;
		}
		// char* to hex
		long hex = String2Hex(tmp, strlen(tmp));
		if(hex == -1)	continue;
		// hex to wchar_t
		*pwch++ = Hex2Wchart(hex);
	}

	if(tmp)	delete []tmp;

	return 1;
}
int YCodec::HexStr2WChars_Gap(char *hexstr, int hexstrlen, int eachchar, wchar_t *wchs, int *wchsLen)
{
	char* p = hexstr;
	wchar_t* pwch = wchs;
	int ict=0;
	if(hexstr == NULL || hexstrlen <= 0 || eachchar <= 0)	return -1;
	if(wchs == NULL){// count the size wchs it will be used
		for(int i=0; i<hexstrlen; i++){
			bool bsign =(p[0] == 92&&p[0+1]==117&&i+5<=hexstrlen\
				&&(p[0+2]<=57&&p[0+2]>=48||p[0+2]>=65&&p[0+2]<=70||p[0+2]>=97&&p[0+2]<=102)\
				&&(p[0+3]<=57&&p[0+3]>=48||p[0+3]>=65&&p[0+3]<=70||p[0+3]>=97&&p[0+3]<=102)\
				&&(p[0+4]<=57&&p[0+4]>=48||p[0+4]>=65&&p[0+4]<=70||p[0+4]>=97&&p[0+4]<=102)\
				&&(p[0+5]<=57&&p[0+5]>=48||p[0+5]>=65&&p[0+5]<=70||p[0+5]>=97&&p[0+5]<=102));
			if(!bsign) //不是Unicode编码
			{
				ict++;
				p++;
			}else{
				p=p+6;
				i=i+5;
			}
		}
		*wchsLen = ict+(hexstrlen-ict)/(eachchar +2) + ((hexstrlen-ict)%(eachchar+2)>0 ? 1 : 0);//
		return 1;
	}
	memset(wchs, 0, *wchsLen * sizeof(wchar_t));
	*wchsLen = 0;
	char* tmp = new char[eachchar+1];
	//char* p = hexstr;
	//wchar_t* pwch = wchs;
	for(int i=0; i<hexstrlen; i++){
		bool bsign =(p[0] == 92&&p[0+1]==117&&i+5<=hexstrlen\
			&&(p[0+2]<=57&&p[0+2]>=48||p[0+2]>=65&&p[0+2]<=70||p[0+2]>=97&&p[0+2]<=102)\
			&&(p[0+3]<=57&&p[0+3]>=48||p[0+3]>=65&&p[0+3]<=70||p[0+3]>=97&&p[0+3]<=102)\
			&&(p[0+4]<=57&&p[0+4]>=48||p[0+4]>=65&&p[0+4]<=70||p[0+4]>=97&&p[0+4]<=102)\
			&&(p[0+5]<=57&&p[0+5]>=48||p[0+5]>=65&&p[0+5]<=70||p[0+5]>=97&&p[0+5]<=102));
		if(!bsign) //不是Unicode编码
		{
			*pwch++ = *p;
			(*wchsLen)++;
			p++;
			continue;
		}

	///for(; i<hexstrlen; i+=eachchar)
	{
		memset(tmp, 0, eachchar+1);
		// get eachchar char
		for(int j=0; j<eachchar; j++){
			if(i+j > hexstrlen)	break;
			if (*p=='\\'){//	
				p=p+2;}//
			tmp[j] = *p++;
			i++;
		}
		i=i+1;// 本来该加2但在for循环中已经多加了1这里就少加1次
		// char* to hex
		long hex = String2Hex(tmp, strlen(tmp));
		if(hex == -1)	continue;
		// hex to wchar_t
		*pwch++ = Hex2Wchart(hex);
		(*wchsLen)++;
	}

	}
	if(tmp)	delete []tmp;

	return 1;
}
// wchar_t to hex code
long YCodec::Wchart2Hex(wchar_t wch)
{
	return (long)wch;
}

// wchar_t* to char*(hex string)
// 中英文转成UNICODE十六进制字符串
// eachchar用于控制每个wchar_t转换成多少个char字符 unicode码是4位的所以这里恒写4
// hexstr == NULL,hexstrlen as output(the size of hexstr will be used)
// error: return -1
int YCodec::Wchart2HexStr(wchar_t *wchs, int wchslen, int eachchar, char* hexstr, int* hexstrlen)
{
	if(wchs == NULL || wchslen <= 0 || eachchar <= 0)	return -1;
	if(hexstr == NULL){// count the size of hexstr will be used
		*hexstrlen = wchslen*eachchar;
		return 1;
	}
	memset(hexstr, 0, *hexstrlen);
	char* p = hexstr;
	wchar_t* pwch = wchs;
	char* tmp = new char[eachchar+1];///tmp对应的是hexstr，因为tmp是堆空间所以外边hexstr对应实参一定也要是堆空间变量！否则这里最后析构不了！或者中间以字符串对象（非常亮）做转换才行！
	for(int i=0; i<wchslen; i++){
		// wchar_t to hex
		long hex = Wchart2Hex(*pwch++);
		// hex to char*
		int charlen = 0;
		if(Hex2String(hex, NULL, &charlen) != -1){
			char* str = new char[charlen+1];
			memset(str, 0, charlen+1);
			int n = Hex2String(hex, str, &charlen);
			if(n != -1){
				int k=0;
				memset(tmp, 0, eachchar+1);
				for(k=0; k<eachchar-charlen; k++)	tmp[k] = '0';
				tmp = strcat(tmp, str);
				p = strcat(p, tmp);
			}
			if(str)	delete []str;
		}
		if(i > *hexstrlen)	break;
	}
	if(tmp)	delete []tmp;

	return 1;
}
int YCodec::Wchart2HexStr_U(wchar_t *wchs, int wchslen, int eachchar, char* hexstr, int* hexstrlen)
{
	if(wchs == NULL || wchslen <= 0 || eachchar <= 0)	return -1;
	if(hexstr == NULL){// count the size of hexstr will be used
		*hexstrlen = wchslen*(eachchar+2);//
		return 1;
	}
	memset(hexstr, 0, *hexstrlen);
	char* p = hexstr;
	wchar_t* pwch = wchs;
	char* tmp = new char[eachchar+1  +2];///tmp对应的是hexstr，因为tmp是堆空间所以外边hexstr对应实参一定也要是堆空间变量！否则这里最后析构不了！或者中间以字符串对象（非常亮）做转换才行！
	for(int i=0; i<wchslen; i++){
		// wchar_t to hex
		long hex = Wchart2Hex(*pwch++);
		// hex to char*
		int charlen = 0;
		if(Hex2String(hex, NULL, &charlen) != -1){
			char* str = new char[charlen+1];
			memset(str, 0, charlen+1);
			int n = Hex2String(hex, str, &charlen);
			if(n != -1){
				int k=0;
				memset(tmp, 0, eachchar+1);
				tmp = strcat(tmp, "\\u");//
				//for(k=0; k<eachchar-charlen; k++)	tmp[k] = '0';
				for(k=2; k<eachchar-charlen+2; k++)	tmp[k] = '0';
				tmp = strcat(tmp, str);
				p = strcat(p, tmp);
			}
			if(str)	delete []str;
		}
		if(i > *hexstrlen)	break;
	}
	if(tmp)	delete []tmp;

	return 1;
}
int YCodec::Wchart2HexStr_Gap(wchar_t *wchs, int wchslen, int eachchar, char* hexstr, int* hexstrlen)
{
	char* p = hexstr;///在下边中间指针通过自加来依次设置目标指针的数据
	wchar_t* pwch = wchs;
	int ict=0;
	if(wchs == NULL || wchslen <= 0 || eachchar <= 0)	return -1;
	if(hexstr == NULL){// count the size of hexstr will be used
		for(int i=0; i<wchslen; i++){
			if( *pwch >= 0 && *pwch <= 127 )
			{ 
				ict++;	
			}
			*pwch++;
		}
		*hexstrlen =ict + (wchslen-ict)*(eachchar+2);//
		return 1;
	}
	memset(hexstr, 0, *hexstrlen);
	*hexstrlen = 0;
	//char* p = hexstr;///在下边中间指针通过自加来依次设置目标指针的数据
	//wchar_t* pwch = wchs;
	char* tmp = new char[eachchar+1  +2];///tmp对应的是hexstr，因为tmp是堆空间所以外边hexstr对应实参一定也要是堆空间变量！否则这里最后析构不了！或者中间以字符串对象（非常亮）做转换才行！
	for(int i=0; i<wchslen; i++){
		if( *pwch >= 0 && *pwch <= 127 )///非中文？ 判断全半角而非中英文比如中文符号是全角 中文至少是由两个全角字节组成的但是由两个全角字节组成的不一定是汉字比如两个连续的全角符号
		{//p = strcat(p, tmp);
			p[0] = *pwch++; 
			(*hexstrlen)++;
			p++;
			continue;}///

		// wchar_t to hex
		long hex = Wchart2Hex(*pwch++);
		// hex to char*
		int charlen = 0;
		if(Hex2String(hex, NULL, &charlen) != -1){
			char* str = new char[charlen+1];
			memset(str, 0, charlen+1);
			int n = Hex2String(hex, str, &charlen);
			if(n != -1){
				int k=0;
				memset(tmp, 0, eachchar+1);
				tmp = strcat(tmp, "\\u");//
				//for(k=0; k<eachchar-charlen; k++)	tmp[k] = '0';
				for(k=2; k<eachchar-charlen+2; k++)	tmp[k] = '0';
				tmp = strcat(tmp, str);
				p = strcat(p, tmp);
				p = p+6;///
				(*hexstrlen)=(*hexstrlen)+6;
			}
			if(str)	delete []str;
		}
		if(i > *hexstrlen)	break;
	}
	if(tmp)	delete []tmp;

	return 1;
}

string YCodec::Str2Unicode(wchar_t* wchs,int len ,char* out ,int& out_len)
{
	string tmp="";
	int hexlen = 0;
	if(Wchart2HexStr(wchs, len, 4, NULL, &hexlen) != -1){
		char* hexstr = new char[hexlen+1];
		memset(hexstr, 0, hexlen+1);
		int n = Wchart2HexStr(wchs, len, 4, hexstr, &hexlen);
		if(n != -1){
			// char* to CString
			char* p = hexstr;
			for(int i=0; i<hexlen; i++)	tmp+=(*p++);
			//tmp=hexstr;//也行啊

			out_len = hexlen;//
			memcpy(out,hexstr,out_len);//
			//tmp[out_len] ='\0';//
		}
		if(hexstr)	delete []hexstr;
	}

	//if(wchs)	delete []wchs;
	return tmp;
};
string YCodec::Str2Unicode(wchar_t* wchs,int len)
{
	char* out ;
	int out_len;
	string tmp="";
	int hexlen = 0;
	if(Wchart2HexStr(wchs, len, 4, NULL, &hexlen) != -1){
		char* hexstr = new char[hexlen+1];
		memset(hexstr, 0, hexlen+1);
		int n = Wchart2HexStr(wchs, len, 4, hexstr, &hexlen);
		if(n != -1){
			// char* to CString
			char* p = hexstr;
			for(int i=0; i<hexlen; i++)	tmp+=(*p++);
			//tmp=hexstr;//也行啊

			out_len = hexlen;//
			memcpy(out,hexstr,out_len);//
			//tmp[out_len] ='\0';//
		}
		if(hexstr)	delete []hexstr;
	}

	//if(wchs)	delete []wchs;
	return tmp;
};
string YCodec::Str2Unicode_StrU(wchar_t* wchs,int len)
{
	string tmp="";
	int hexlen = 0;
	if(Wchart2HexStr(wchs, len, 4, NULL, &hexlen) != -1){
		char* hexstr = new char[hexlen+1 +2*hexlen/4];
		memset(hexstr, 0, hexlen+1 +2*hexlen/4);
		int n = Wchart2HexStr_U(wchs, len, 4, hexstr, &hexlen);
		if(n != -1){
			// char* to CString
			char* p = hexstr;
			for(int i=0; i<hexlen/*+2*hexlen/4*/; i++)	tmp+=(*p++);
			//tmp=hexstr;//也行啊
		}
		if(hexstr)	delete []hexstr;
	}

	//if(wchs)	delete []wchs;
	return tmp;
};
string YCodec::Str2Unicode_U(wchar_t* wchs,int len  ,char* out ,int& out_len)
{
	string tmp="";
	int hexlen = 0;
	if(Wchart2HexStr_U(wchs, len, 4, NULL, &hexlen) != -1){
		char* hexstr = new char[hexlen+1 ];
		memset(hexstr, 0, hexlen+1 );
		int n = Wchart2HexStr_U(wchs, len, 4, hexstr, &hexlen);
		if(n != -1){
			// char* to CString
			char* p = hexstr;
			for(int i=0; i<hexlen/*+2*hexlen/4*/; i++)	tmp+=(*p++);
			//tmp=hexstr;//也行啊
			out_len = hexlen;//
			memcpy(out,hexstr,out_len);//
			//tmp[out_len] = '\0';//
		}
		if(hexstr)	delete []hexstr;
	}
	
	//if(wchs)	delete []wchs;
	return tmp;
};
string YCodec::Chinese2Unicode(wchar_t* wchs,int len ,char* out ,int& out_len){
	string tmp = "";
	string retstr = "";
	int hexlen = 0;
	if(Wchart2HexStr_Gap(wchs, len, 4, NULL, &hexlen) != -1){
		char* hexstr = new char[hexlen+1 ];
		memset(hexstr, 0, hexlen+1 );
		int n = Wchart2HexStr_Gap(wchs, len, 4, hexstr, &hexlen);
		if(n != -1){
			// char* to CString
			char* p = hexstr;
			for(int i=0; i<hexlen/*+2*hexlen/4*/; i++)	tmp+=(*p++);
			//tmp=hexstr;//也行啊
			out_len = hexlen;//
		memcpy(out,hexstr,out_len);//
			//tmp[out_len] = '\0';//这里不会越界错误 为什么 因为for循环中分配过多的string空间了 所以没越界 而for之所以过多分配是当时以为每个Unicode码多加了\u两个字符但实际多加的字符已经在获得的长度中计算了所以不需要多分配
		}
		if(hexstr)	delete []hexstr;
	}

	//if(wchs)	delete []wchs;
	return tmp;
};

string YCodec::Chinese2Unicode(wchar_t* wchs, int len){
	string tmp = "";
	string retstr = "";
	int hexlen = 0;
	if (Wchart2HexStr_Gap(wchs, len, 4, NULL, &hexlen) != -1){
		char* hexstr = new char[hexlen + 1];
		memset(hexstr, 0, hexlen + 1);
		int n = Wchart2HexStr_Gap(wchs, len, 4, hexstr, &hexlen);
		if (n != -1){
			// char* to CString
			char* p = hexstr;
			for (int i = 0; i < hexlen/*+2*hexlen/4*/; i++)	tmp += (*p++);
			//tmp=hexstr;//也行啊
			//tmp[out_len] = '\0';//这里不会越界错误 为什么 因为for循环中分配过多的string空间了 所以没越界 而for之所以过多分配是当时以为每个Unicode码多加了\u两个字符但实际多加的字符已经在获得的长度中计算了所以不需要多分配
		}
		if (hexstr)	delete[]hexstr;
	}

	//if(wchs)	delete []wchs;
	return tmp;
};


wstring YCodec::Unicode2Str(char* str,int strlen  ,WCHAR* out ,int& out_len)
{
	wstring tmp=L"";
	int wchslen = 0;
	if(HexStr2WChars(str, strlen, 4, NULL, &wchslen) != -1){
		wchar_t* wchs = new wchar_t[wchslen+1];
		memset(wchs, 0, sizeof(wchar_t)*(wchslen+1));
		int n = HexStr2WChars(str, strlen, 4, wchs, &wchslen);
		if(n != -1){
			// wchar_t to CString
			wchar_t* pwch = wchs;
			for(int i=0; i<wchslen; i++)	tmp+=(*pwch++);

			out_len = wchslen;//
			memcpy(out,wchs,out_len*2);//
			//tmp[out_len] = L'\0';//
		}
		if(wchs)	delete []wchs;
	}
	return tmp;
};
wstring YCodec::Unicode2Str(char* str,int strlen  )
{
	WCHAR* out ;
	int out_len;
	wstring tmp=L"";
	int wchslen = 0;
	if(HexStr2WChars(str, strlen, 4, NULL, &wchslen) != -1){
		wchar_t* wchs = new wchar_t[wchslen+1];
		memset(wchs, 0, sizeof(wchar_t)*(wchslen+1));
		int n = HexStr2WChars(str, strlen, 4, wchs, &wchslen);
		if(n != -1){
			// wchar_t to CString
			wchar_t* pwch = wchs;
			for(int i=0; i<wchslen; i++)	tmp+=(*pwch++);

			out_len = wchslen;//
			memcpy(out,wchs,out_len*2);//
			//tmp[out_len] = L'\0';//
		}
		if(wchs)	delete []wchs;
	}
	return tmp;
};
wstring YCodec::Unicode2Str_StrU(char* str,int strlen)
{
	wstring tmp=L"";
	int wchslen = 0;
	string strtmp ,strstr;
	strstr = str;
	strtmp = replace_all(strstr,"\\u","");
	strlen=strtmp.length();
	strcpy(str,(char*)strtmp.c_str());

	if(HexStr2WChars(str, strlen, 4, NULL, &wchslen) != -1){
		wchar_t* wchs = new wchar_t[wchslen+1];
		memset(wchs, 0, sizeof(wchar_t)*(wchslen+1));
		int n = HexStr2WChars(str, strlen, 4, wchs, &wchslen);
		if(n != -1){
			// wchar_t to CString
			wchar_t* pwch = wchs;
			for(int i=0; i<wchslen; i++)	tmp+=(*pwch++);
		}
		if(wchs)	delete []wchs;
	}
	return tmp;
};
wstring YCodec::Unicode2Str_U(char* str,int strlen ,WCHAR* out ,int& out_len)
{
	wstring tmp=L"";
	int wchslen = 0;
	//string strtmp ,strstr;
	//strstr = str;
	//strtmp = replace_all(strstr,"\\u","");
	//strlen=strtmp.length();
	//strcpy(str,(char*)strtmp.c_str());

	if(HexStr2WChars_U(str, strlen, 4, NULL, &wchslen) != -1){
		wchar_t* wchs = new wchar_t[wchslen+1];
		memset(wchs, 0, sizeof(wchar_t)*(wchslen+1));
		int n = HexStr2WChars_U(str, strlen, 4, wchs, &wchslen);
		if(n != -1){
			// wchar_t to CString
			wchar_t* pwch = wchs;
			for(int i=0; i<wchslen; i++)	tmp+=(*pwch++);

			out_len = wchslen;//
			memcpy(out,wchs,out_len*2);//
			//tmp[out_len] = L'\0';//
		}
		if(wchs)	delete []wchs;
	}
	return tmp;
};
wstring YCodec::Unicode2Chinese(char* str,int len ,WCHAR* out ,int& out_len){
	wstring tmp=L"";
	int wchslen = 0;
	//string strtmp ,strstr;
	//strstr = str;
	//strtmp = replace_all(strstr,"\\u","");
	//strlen=strtmp.length();
	//strcpy(str,(char*)strtmp.c_str());

	if(HexStr2WChars_Gap(str, len, 4, NULL, &wchslen) != -1){
		wchar_t* wchs = new wchar_t[wchslen+1];
		memset(wchs, 0, sizeof(wchar_t)*(wchslen+1));
		int n = HexStr2WChars_Gap(str, len, 4, wchs, &wchslen);
		if(n != -1){
			// wchar_t to CString
			wchar_t* pwch = wchs;
			for(int i=0; i<wchslen; i++)	tmp+=(*pwch++);

			out_len = wchslen;//
			memcpy(out,wchs,out_len*2);//
			//tmp[out_len] = L'\0';//不需要 而且这里越界
		}
		if(wchs)	delete []wchs;
	}
	return tmp;
};

///--------------------------------------------------------------------
int YCodec::IncludeChinese(char *str)//返回0：无中文，返回1：有中文
{
	char c;
	while(1)
	{
		c=*str++;
		if (c==0) break;  //如果到字符串尾则说明该字符串没有中文字符
		if (c&0x80)        //如果字符高位为1且下一字符高位也是1则有中文字符
			if (*str & 0x80) return 1;
	}
	return 0;
}
vector<string> YCodec::SplitStr(char* sztext /*,int nlen*/)
{
	//char sztext[] = " 是ciw."; 
	//char c = 0; 
	string c;
	char szChinese[MAX_PATH] = {0}; 
	char szEnglish[MAX_PATH] = {0};
	vector<string> vecC;
	//vector<stirng> vecE;
	vecC.clear();
	bool bVaryOld = 0;
	bool bVaryNew = 0;
	int i = 0, nlen = strlen(sztext); 

	for(; i < nlen; i++) 
	{ 
		if( sztext[i] >= 0 && sztext[i] <= 127 ) //不是全角字符？
		{
			if (i==0)
			{
				bVaryOld = bVaryNew =0;
			} 
			else
			{
				bVaryOld = bVaryNew;
				bVaryNew = 0;
			}
			if (bVaryNew!=bVaryOld)//
			{
				vecC.push_back(szChinese);
				ZeroMemory(szChinese,MAX_PATH);
			}
			//c = sztext[i], printf("%c\n", c);
			c = sztext[i];
			strcat(szEnglish,c.c_str());
			if (i==nlen-1)//
			{
				vecC.push_back(szEnglish);
				ZeroMemory(szEnglish,MAX_PATH);
			}
		}
		else //是全角字符
		{
			if (i==0)
			{
				bVaryOld = bVaryNew =1;
			} 
			else
			{
				bVaryOld = bVaryNew;
				bVaryNew = 1;
			}
			if (bVaryNew!=bVaryOld)//
			{
				vecC.push_back(szEnglish);
				ZeroMemory(szEnglish,MAX_PATH);
			}
			//szChinese[0] = sztext[i], szChinese[1] = sztext[i + 1], printf("%s\n", szChinese), i++; //中文是2个字节,所以i++
			c = sztext[i];
			strcat(szChinese,c.c_str());
			c = sztext[i+1];
			strcat(szChinese,c.c_str());
			if (i==nlen-2)//
			{
				vecC.push_back(szChinese);
				ZeroMemory(szChinese,MAX_PATH);
			}
			i++;
		}	
	} 
	return vecC;
}
string YCodec::SpliceStr(vector<string> vecC)
{
	YCodec urlcd;
	string strTmp="";
	int nSize = vecC.size();
	if (nSize<1)
	{
		return strTmp;
	}
	for(int i= 0;i<nSize;i++)
	{
		if (i%2==1)
		{
			wstring strW = AnsiToUnicode4(vecC.at(i));
			int ilen = strW.length();
			//strTmp += urlcd.Str2Unicode((wchar_t*)AnsiToUnicode4(vecC.at(i)).c_str(),vecC.at(i).length());
			strTmp += urlcd.Str2Unicode_StrU((wchar_t*)strW.c_str(),ilen);
		} 
		else
		{
			strTmp += vecC.at(i);
		}
	}
	return strTmp;
};
string YCodec::Chinese2Unicode(char * str)
{
	string strTmp="http";///代表英文开头的 随意取的一段字串
	strTmp+=str;
	vector<string> vecC;
	vecC = SplitStr((char*)strTmp.c_str());
	string b = SpliceStr(vecC);
	b=b.substr(4,b.length()-4);
	return b;
};
vector<string> YCodec::SplitUnicodeStr(char* sztext /*,int nlen*/)
{
	//char sztext[] = " 是ciw."; 
	//char c = 0; 
	string c;
	char szChinese[MYMAX_PATH] = {0}; 
	char szEnglish[MYMAX_PATH] = {0};
	vector<string> vecC;
	//vector<stirng> vecE;
	vecC.clear();
	bool bVaryOld = 0;
	bool bVaryNew = 0;
	int i = 0, nlen = strlen(sztext); 

	for(; i < nlen; i++) 
	{ 
		bool bsign =(sztext[i] == 92&&sztext[i+1]==117&&i+5<=nlen\
			&&(sztext[i+2]<=57&&sztext[i+2]>=48||sztext[i+2]>=65&&sztext[i+2]<=70||sztext[i+2]>=97&&sztext[i+2]<=102)\
			&&(sztext[i+3]<=57&&sztext[i+3]>=48||sztext[i+3]>=65&&sztext[i+3]<=70||sztext[i+3]>=97&&sztext[i+3]<=102)\
			&&(sztext[i+4]<=57&&sztext[i+4]>=48||sztext[i+4]>=65&&sztext[i+4]<=70||sztext[i+4]>=97&&sztext[i+4]<=102)\
			&&(sztext[i+5]<=57&&sztext[i+5]>=48||sztext[i+5]>=65&&sztext[i+5]<=70||sztext[i+5]>=97&&sztext[i+5]<=102));
		if(!bsign) //不是url编码
		{
			
			if (i==0)
			{
				bVaryOld = bVaryNew =0;
			} 
			else
			{
				bVaryOld = bVaryNew;
				bVaryNew = 0;
			}
			if (bVaryNew!=bVaryOld)//
			{
				vecC.push_back(szChinese);
				ZeroMemory(szChinese,MYMAX_PATH);
			}
			//c = sztext[i], printf("%c\n", c);
			c = sztext[i];
			strcat(szEnglish,c.c_str());
			if (i==nlen-1)//
			{
				vecC.push_back(szEnglish);
				ZeroMemory(szEnglish,MYMAX_PATH);
			}
		}
		else //是url编码
		{
			if (i==0)
			{
				bVaryOld = bVaryNew =1;
			} 
			else
			{
				bVaryOld = bVaryNew;
				bVaryNew = 1;
			}
			if (bVaryNew!=bVaryOld)//
			{
				vecC.push_back(szEnglish);
				ZeroMemory(szEnglish,MYMAX_PATH);
			}
			//szChinese[0] = sztext[i], szChinese[1] = sztext[i + 1], printf("%s\n", szChinese), i++; //中文是2个字节,所以i++
			c = sztext[i];
			strcat(szChinese,c.c_str());
			c = sztext[i+1];
			strcat(szChinese,c.c_str());
			c = sztext[i+2];
			strcat(szChinese,c.c_str());
			c = sztext[i+3];
			strcat(szChinese,c.c_str());
			c = sztext[i+4];
			strcat(szChinese,c.c_str());
			c = sztext[i+5];
			strcat(szChinese,c.c_str());
			if (i==nlen-6)//
			{
				vecC.push_back(szChinese);
				ZeroMemory(szChinese,MYMAX_PATH);
			}
			i+=5;
		}	
	} 
	return vecC;
}
string YCodec::SpliceUnicodeStr(vector<string> vecC)
{
	YCodec urlcd;
	string strTmp="";
	int nSize = vecC.size();
	if (nSize<1)
	{
		return strTmp;
	}
	for(int i= 0;i<nSize;i++)
	{
		if (i%2==1)
		{
			string strtt = vecC.at(i);
			int nlen = vecC.at(i).length();
			strTmp += UnicodeToAnsi6( urlcd.Unicode2Str_StrU((char*)strtt.c_str(),nlen));
			int len = strTmp.length();
			//strTmp = strTmp.substr(0,len-1);
		} 
		else
		{
			strTmp += vecC.at(i);
		}
	}
	return strTmp;
};
string YCodec::Unicode2Chinese(char * str)
{
	string strtmp = "http";
	strtmp += str;

	vector<string> vecC;
	vecC = SplitUnicodeStr((char*)strtmp.c_str());
	string b = SpliceUnicodeStr(vecC);
	b=b.substr(4,b.length()-4);
	return b;
}