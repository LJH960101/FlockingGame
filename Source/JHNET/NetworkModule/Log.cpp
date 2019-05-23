#include "Log.h"
#include <iostream>
#include <fstream>
#include <time.h>
#include <stdarg.h>

using namespace std;

CLog* CLog::instance = nullptr;

CLog::CLog()
{

}

CLog::~CLog()
{
	Join();
}

void CLog::WriteLog(const ELogType& logType, const ELogLevel& logLevel, const string format)
{
	CLog* logSystem = GetInstance();

	// Calc Time
	time_t t = time(0);
	tm time;
	localtime_s(&time, &t);
	char buf[80];
	sprintf_s(buf, "(%d/%d %d:%d:%d)", (time.tm_mon+1), time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);

	// Create LogText
	string logText = buf + _LogLevelToString(logLevel) + " : " + format + "\n";

	// Push to queue
	FLog newLog;
	newLog.type = logType;
	newLog.level = logLevel;
	newLog.log = logText;

	logSystem->_queueMutex.lock();
	logSystem->_logQueue.push(newLog);
	logSystem->_queueMutex.unlock();

	// Do _WriteLog when not on write
	if (!logSystem->_bWriteMutex.try_lock()) return;
	if (!logSystem->_bOnWrite) {
		logSystem->_bOnWrite = true;
		logSystem->_bWriteMutex.unlock();
		logSystem->_writeLogThread = new thread(&CLog::_WriteLog, logSystem);
	}
	else logSystem->_bWriteMutex.unlock();
}

void CLog::WriteLogS(const string format)
{
	WriteLog(_LogTemp, Warning, format);
}

string CLog::Format(const char * format, ...)
{
	va_list args;
	va_start(args, format);
#ifndef _MSC_VER
	size_t size = std::snprintf(nullptr, 0, format, args) + 1; // Extra space for '\0'
	std::unique_ptr<char[]> buf(new char[size]);
	std::vsnprintf(buf.get(), size, format, args);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
#else
	int size = _vscprintf(format, args);
	std::string result(++size, 0);
	vsnprintf_s((char*)result.data(), size, _TRUNCATE, format, args);
	return result;
#endif
	va_end(args);
}

void CLog::Join()
{
	CLog* logSystem = GetInstance();

	if (logSystem->_writeLogThread == nullptr) return;
	logSystem->_writeLogThread->join();
}

string CLog::_LogTypeToString(const ELogType& logType)
{
	switch (logType)
	{
	case _LogTemp:
		return "LogTemp";
	case LogSystem:
		return "LogSystem";
	case NetworkManager:
		return "NetworkManager";
	case ServerNetworkSystem:
		return "ServerNetworkSystem";
	case RoomManager:
		return "RoomManager";
	case PlayerManager:
		return "PlayerManager";
	default:
		return "UnkownLogType";
	}
}

string CLog::_LogLevelToString(const ELogLevel & logLevel)
{
	switch (logLevel)
	{
	case Warning:
		return "Warning";
	case Error:
		return "Error";
	case Critical:
		return "Critical";
	default:
		return "UnknownLevel";
	}
}

CLog* CLog::GetInstance()
{
	if (instance == nullptr) instance = new CLog();
	return instance;
}

void CLog::_WriteLog(CLog* logSystem)
{
	FLog currentLog;
	while (true) {
		// Pop log
		logSystem->_queueMutex.lock();
		currentLog = logSystem->_logQueue.front();
		logSystem->_logQueue.pop();
		logSystem->_queueMutex.unlock();

		// Create LogFile Name
		string fileName1 = logSystem->_LogTypeToString(currentLog.type);
		string fileName2 = LOG_INTERGRATED;
		fileName1.append(LOG_FILE_TYPE);
		fileName2.append(LOG_FILE_TYPE);

		// Write Log
		logSystem->_fileMutex.lock();
		ofstream output1(fileName1, ios::app);
		ofstream output2(fileName2, ios::app);
		output1 << currentLog.log;
		output2 << _LogTypeToString(currentLog.type) << ": " << currentLog.log;
		output1.close();
		output2.close();
		logSystem->_fileMutex.unlock();

		// break when queue is empty
		logSystem->_queueMutex.lock();
		if (logSystem->_logQueue.empty()) {
			logSystem->_queueMutex.unlock();

			// disable onWrite
			logSystem->_bWriteMutex.lock();
			logSystem->_bOnWrite = false;
			logSystem->_writeLogThread = nullptr;
			logSystem->_bWriteMutex.unlock();

			break;
		}
		logSystem->_queueMutex.unlock();
	}
}
