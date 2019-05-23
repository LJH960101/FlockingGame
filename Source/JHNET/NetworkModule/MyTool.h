#pragma once
#include "JHNET.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "AllowWindowsPlatformTypes.h"
#include <WinSock2.h>
#include "HideWindowsPlatformTypes.h"

namespace MyTool {
	// Send with len(int)
	int Send(SOCKET sock, const char* buf, int len, int flags = 0);
	int SendTo(SOCKET sock, const char* buf, const int& len, sockaddr* addr, const int& addrLen);
	int SendWithoutLen(SOCKET sock, const char* buf, int len, int flags = 0);
	int SendToWithoutLen(SOCKET sock, const char* buf, const int& len, sockaddr* addr, const int& addrLen, int flags = 0);
	FString UINT64ToFString(const UINT64 val);
	UINT64 FStringToUINT64(const FString val);
	bool IsBigEndian();
}