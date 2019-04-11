#ifndef __json_simple_tool_helper_h__
#define __json_simple_tool_helper_h__
#include <string>
#include <cassert>
#include "json.h"
#include <vector>
namespace json_tool {
	inline void split(std::string& s, std::vector< std::string >* ret, const char* delim = ";,")
	{
		size_t last = 0;
		size_t index = s.find_first_of(delim, last);
		while (index != std::string::npos)
		{
			ret->push_back(s.substr(last, index - last));
			last = index + 1;
			index = s.find_first_of(delim, last);
		}
		if (index - last>0)
		{
			ret->push_back(s.substr(last, index - last));
		}
	}

	inline bool is_exist_key_or(const Json::Value& value, const char* keys)
	{
		std::string skey = keys;
		std::vector<std::string> lst; 
		split(skey,  &lst);

 		for (auto it : lst) {
			if (value.isMember(it)) {
				return true; 
			}
		}
		return false; 
	}

	inline bool is_exist_key_and(const Json::Value& value, const char* keys)
	{
		if (value.isNull())
			return false;

		std::string skey = keys;
		std::vector<std::string> lst;
		split(skey, &lst);

		for (auto it : lst) {
			if (!value.isMember(it)) {
				return false;
			}
		}
		return true;
	}

	inline bool is_not_exist_key(const Json::Value& value, const char* keys) {
		return !is_exist_key_and(value, keys);
	}

	inline std::string result_value_string(const Json::Value& value, const char* key, const char* defaultValue = "")
	{
		if (value.isMember(key)) {
			assert(value[key].isString());
			return value[key].asString(); 
		}
		return defaultValue; 
	}
	
	inline int result_value_int(const Json::Value& value, const char* key, const int defaultValue = 0)
	{
		if (value.isMember(key)) {
			if (value[key].isNumeric())
				return value[key].asInt();
			else if (value[key].isString())
				return atoi(value[key].asString().c_str());
		}
		return defaultValue;
	}

	inline unsigned int result_value_uint(const Json::Value& value, const char* key, const unsigned int defaultValue = 0)
	{
		if (value.isMember(key)) {
			return value[key].asUInt();
		}
		return defaultValue;
	}

	inline __int64 result_value_int64(const Json::Value& value, const char* key, const __int64 defaultValue = 0)
	{
		if (value.isMember(key)) {
			return value[key].asInt64();
		}
		return defaultValue; 
	}
	inline float result_value_float(const Json::Value& value, const char* key, const float defaultValue = 0.0f)
	{
		if (value.isMember(key)) {
			return value[key].asFloat();
		}
		return defaultValue;
	}
	inline double result_value_int64(const Json::Value& value, const char* key, const double defaultValue = 0.0f)
	{
		if (value.isMember(key)) {
			return value[key].asDouble();
		}
		return defaultValue;
	}

	inline bool result_value_bool(const Json::Value& value, const char* key, const bool defaultValue = true)
	{
		if (value.isMember(key)) {
			return value["key"].asBool(); 
		}
		return defaultValue; 
	}
}
#endif