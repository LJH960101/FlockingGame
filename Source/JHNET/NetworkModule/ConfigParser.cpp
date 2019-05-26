#include "ConfigParser.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

CConfigParser::CConfigParser(std::string path)
{
	std::string paths[3];
	paths[0] = std::string(TCHAR_TO_UTF8(*(FPaths::ProjectDir()))) + path;
	paths[1] = std::string(TCHAR_TO_UTF8(*(FPaths::RootDir()))) + path;
	paths[2] = std::string(TCHAR_TO_UTF8(*(FString("C:\\")))) + path;

	for (int i = 0; i < 3; ++i) {
		// read File
		std::ifstream openFile(paths[i]);
		if (openFile.is_open()) {
			std::string line;
			while (getline(openFile, line)) {
				std::string delimiter = " = ";
				if (std::string::npos == line.find(delimiter)) delimiter = "=";
				std::string token1 = line.substr(0, line.find(delimiter)); // token is "scott"
				std::string token2 = line.substr(line.find(delimiter) + delimiter.length(), line.length()); // token is "scott"
				m_table[token1] = token2;
			}
			openFile.close();
			break;
		}
	}
}

void CConfigParser::ChagnePath(std::string path)
{
	m_table.clear();

	// read File
	std::ifstream openFile(path);
	if (openFile.is_open()) {
		std::string line;
		while (getline(openFile, line)) {
			std::string delimiter = " = ";
			if (std::string::npos == line.find(delimiter)) delimiter = "=";
			std::string token1 = line.substr(0, line.find(delimiter)); // token is "scott"
			std::string token2 = line.substr(line.find(delimiter) + delimiter.length(), line.length()); // token is "scott"
			m_table[token1] = token2;
		}
		openFile.close();
	}
}

bool CConfigParser::Contain(std::string name)
{
	if (m_table.find(name) == m_table.end()) return false;
	return true;
}

bool CConfigParser::GetBool(std::string name)
{
	if (Contain(name)) {
		if (m_table[name][0] == 't' || m_table[name][0] == 'T') return true;
		else return false;
	}
	else {
		throw std::invalid_argument("Not exist Name.");
	}
}

std::string CConfigParser::GetString(std::string name)
{
	if (Contain(name)) {
		if (m_table[name].find("\"") == std::string::npos) return m_table[name];
		else return m_table[name].substr(1, m_table[name].length() - 2);
	}
	else {
		throw std::invalid_argument("Not exist Name.");
	}
}

float CConfigParser::GetFloat(std::string name)
{
	if (Contain(name)) {
		return std::stof(m_table[name]);
	}
	else {
		throw std::invalid_argument("Not exist Name.");
	}
}

int CConfigParser::GetInt(std::string name)
{
	if (Contain(name)) {
		return std::stoi(m_table[name]);
	}
	else {
		throw std::invalid_argument("Not exist Name.");
	}
}

