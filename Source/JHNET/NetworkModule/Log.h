#pragma once
#include <string>
#include <queue>
#include <mutex>
#include <thread>

#define LOG_INTERGRATED "LogIntergrated"
#define LOG_FILE_TYPE ".log"

enum ELogLevel
{
	Warning,
	Error,
	Critical
};
enum ELogType {
	_LogTemp,
	LogSystem,
	NetworkManager,
	ServerNetworkSystem,
	RoomManager,
	PlayerManager
};

struct FLog
{
	ELogType type;
	ELogLevel level;
	std::string log;
};

class CLog
{
	// FILED
private:
	bool _bOnWrite;
	std::thread* _writeLogThread;
	std::mutex _bWriteMutex;
	std::mutex _fileMutex;
	std::mutex _queueMutex;
	std::queue<FLog> _logQueue;
	static std::string _LogTypeToString(const ELogType& logType);
	static std::string _LogLevelToString(const ELogLevel& logLevel);

	// METHOD
public:
	// Join While thread end.
	static void Join();
	// Push Log to queue by many args.
	static void WriteLog(const ELogType& logType, const ELogLevel& logLevel, const std::string format);
	// Push Log to queue by simple string.
	static void WriteLogS(const std::string format);
	static std::string Format(const char* format, ...);

private:
	static CLog* instance;

	// Real Write Log
	CLog();
	~CLog();
	static void _WriteLog(CLog* logSystem);
	static CLog* GetInstance();
};