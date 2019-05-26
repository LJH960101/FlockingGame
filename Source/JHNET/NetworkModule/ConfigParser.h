#pragma once
/*
*		Config ������ �Ľ��ϴ� Ŭ����
*/

#include "JHNET.h"
#include <string>
#include <map>

class CConfigParser {
public:
	CConfigParser(std::string path); 
	void ChagnePath(std::string path);
	bool IsSuccess() { return m_table.size() != 0; }
	bool Contain(std::string name);
	bool GetBool(std::string name);
	std::string GetString(std::string name);
	float GetFloat(std::string name);
	int GetInt(std::string name);

private:
	std::map<std::string, std::string> m_table;
};