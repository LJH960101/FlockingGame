#include "NetworkTool.h"
#include "Serializer.h"
#include <memory>

using namespace std;
using namespace NetworkTool;
using namespace MySerializer;

int NetworkTool::Send(SOCKET sock, const char * buf, int len, int flags)
{
	shared_ptr<char[]> newBuf(new char[len + sizeof(int)]);
	int intLen = IntSerialize(newBuf.get(), len);
	memcpy(newBuf.get() + intLen, buf, len);
	return SendWithoutLen(sock, newBuf.get(), len + intLen, flags);
}

int NetworkTool::SendWithoutLen(SOCKET sock, const char* buf, int len, int flags)
{
	return send(sock, buf, len, flags);
}

int NetworkTool::SendTo(SOCKET sock, const char* buf, const int& len, sockaddr* addr, const int& addrLen)
{
	shared_ptr<char[]> newBuf(new char[len + sizeof(int)]);
	int intLen = IntSerialize(newBuf.get(), len);
	memcpy(newBuf.get() + intLen, buf, len);
	return SendToWithoutLen(sock, newBuf.get(), len + intLen, addr, addrLen, 0);
}

int NetworkTool::SendToWithoutLen(SOCKET sock, const char* buf, const int& len, sockaddr* addr, const int& addrLen, int flags)
{
	return sendto(sock, buf, len, flags, addr, addrLen);
}

bool NetworkTool::IsBigEndian()
{
	static bool onInit = false;
	static bool isBigEndian = false;

	if (!onInit) {
		onInit = true;
		unsigned int x = 0x76543210;
		char *c = (char*)&x;
		if (*c == 0x10)
		{
			isBigEndian = false;
		}
		else
		{
			isBigEndian = true;
		}
	}
	return isBigEndian;
}

FString NetworkTool::UINT64ToFString(const UINT64 val)
{
	return FString::Printf(TEXT("%llu"), val);
}

UINT64 NetworkTool::FStringToUINT64(const FString val)
{
	return FCString::Strtoui64(*val, NULL, 10);
}