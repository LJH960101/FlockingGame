#define INCL_EXTRA_HTON_FUNCTIONS
#include "Serializer.h"
#include <stdio.h>
#include "NetworkTool.h"
#include "NetworkModule/GameInfo.h"
using namespace NetworkTool;

int MySerializer::SerializeEnum(const EMessageType & type, char * outBuf)
{
	int toIntType = (int)type;
	char buf[sizeof(int)];
	int intBufSize = IntSerialize(buf, toIntType);
	memcpy(outBuf, buf, intBufSize);
	return intBufSize;
}

int MySerializer::SerializeWithEnum(const EMessageType& type, const char* inBuf, const int& bufLen, char * outBuf)
{
	int intBufSize = SerializeEnum(type, outBuf);
	if (bufLen != 0) memcpy(outBuf + intBufSize, inBuf, bufLen);
	return intBufSize + bufLen;
}

int MySerializer::BoolSerialize(char * buf, const bool& data)
{
	buf[0] = data;
	return 1;
}

bool MySerializer::BoolDeserialize(const char * buf, int* cursor)
{
	bool result;
	if (cursor == nullptr) result = static_cast<bool>(*buf);
	else {
		result = static_cast<bool>(*(buf + *cursor));
		*cursor += 1;
	}
	return result;
}

int MySerializer::CharSerialize(char * buf, const char& data)
{
	buf[0] = data;
	return 1;
}


char MySerializer::CharDeserialize(const char * buf, int* cursor)
{
	char result;
	if (cursor == nullptr) result = *buf;
	else {
		result = *(buf + *cursor);
		*cursor += 1;
	}
	return result;
}

EMessageType MySerializer::GetEnum(char * buf, int* cursor)
{
	int type = IntDeserialize(buf, cursor);
	return EMessageType(type);
}

int MySerializer::IntSerialize(char * buf, const int32_t& val)
{
	Serialize((char*)&val, buf, sizeof(int32_t));
	return sizeof(int32_t);
}

int32_t MySerializer::IntDeserialize(const char * buf, int* cursor)
{
	int32_t res = 0;
	if (cursor == nullptr) {
		DeSerialize(buf, (char*)&res, sizeof(int32_t));
	}
	else {
		DeSerialize(buf + *cursor, (char*)&res, sizeof(int32_t));
		*cursor += sizeof(int32_t);
	}
	return res;
}

int MySerializer::UInt64Serialize(char * buf, const unsigned __int64 & val)
{
	Serialize((char*)&val, buf, sizeof(unsigned __int64));
	return sizeof(unsigned __int64);
}

unsigned __int64 MySerializer::UInt64Deserialize(const char * buf, int* cursor)
{
	unsigned __int64 res = 0;
	if (cursor == nullptr) {
		DeSerialize(buf, (char*)&res, sizeof(unsigned __int64));
	}
	else {
		DeSerialize(buf + *cursor, (char*)&res, sizeof(unsigned __int64));
		*cursor += sizeof(unsigned __int64);
	}
	return res;
}

int MySerializer::FloatSerialize(char * buf, const float& val)
{
	Serialize((char*)&val, buf, sizeof(float));
	return sizeof(float);
}

float MySerializer::FloatDeserialize(const char * buf, int* cursor)
{
	float res = 0;
	if (cursor == nullptr) {
		DeSerialize(buf, (char*)&res, sizeof(float));
	}
	else {
		DeSerialize(buf + *cursor, (char*)&res, sizeof(float));
		*cursor += sizeof(float);
	}
	return res;
}

int MySerializer::Vector3Serialize(char * buf, const FVector & val)
{
	FloatSerialize(buf, val.X);
	FloatSerialize(buf + sizeof(float), val.Y);
	FloatSerialize(buf + (sizeof(float) * 2), val.Z);
	return sizeof(float) * 3;
}

FVector MySerializer::Vector3Deserialize(const char * buf, int* cursor)
{
	float x;
	float y;
	float z;
	if (cursor == nullptr) {
		x = FloatDeserialize(buf);
		y = FloatDeserialize(buf + sizeof(float));
		z = FloatDeserialize(buf + sizeof(float) * 2);
	}
	else {
		x = FloatDeserialize(buf, cursor);
		y = FloatDeserialize(buf, cursor);
		z = FloatDeserialize(buf, cursor);
	}
	FVector result(x, y, z);
	return result;
}

int MySerializer::StringSerialize(char * buf, const char * source, const int & len)
{
	if (len >= MAX_STRING_BUF) {
		JHNET_LOG_ERROR("Wrong String...");
		return 0;
	}
	int retval = IntSerialize(buf, len);
	memcpy(buf + retval, source, len);
	return len + retval;
}

FSerializableString MySerializer::StringDeserialize(const char * buf, int* cursor)
{
	int len = IntDeserialize(buf, cursor);
	if (len >= MAX_STRING_BUF || len < 0) {
		JHNET_LOG_ERROR("Wrong String...");
		*cursor = BUFSIZE + 10000;
		return FSerializableString();
	}
	FSerializableString outData;
	outData.len = len;
	if (cursor == nullptr) {
		memcpy(outData.buf, buf, len);
	}
	else {
		memcpy(outData.buf, buf + *cursor, len);
		*cursor += len;
	}
	return outData;
}

int MySerializer::StringSerialize(char * buf, std::string source)
{
	int len = source.length();
	if (len >= MAX_STRING_BUF) return 0;
	int retval = IntSerialize(buf, len);
	memcpy(buf + retval, source.c_str(), len);
	return len + retval;
}

void MySerializer::Serialize(char* source, char* dest, int size)
{
	if (IsBigEndian()) {
		memcpy(dest, source, size);
	}
	else {
		for (int i = 0; i < size; ++i) {
			dest[i] = *(source + (size - 1 - i));
		}
	}
}

FString MySerializer::FStringDeserialize(const char* buf, int* cursor)
{
	if (cursor == nullptr) {
		int len = 0;
		int stringLen = IntDeserialize(buf, &len);
		FString newString = BytesToString((uint8 *)((void*)(buf + len)), stringLen);
		return newString;
	}
	else {
		int stringLen = IntDeserialize(buf, cursor);
		FString newString = BytesToString((uint8 *)((void*)(buf + *cursor)), stringLen);
		*cursor += stringLen;
		return newString;
	}
}

int MySerializer::FStringSerialize(char* buf, const FString& source, int maxLen)
{
	int len = StringToBytes(source, (uint8 *)((void*)(buf + sizeof(int32_t))), maxLen);
	return IntSerialize(buf, len) + len;
}

void MySerializer::DeSerialize(const char * source, char * dest, int size)
{
	if (IsBigEndian()) {
		memcpy(dest, source, size);
	}
	else {
		for (int i = 0; i < size; ++i) {
			dest[i] = *(source + (size - 1 - i));
		}
	}
}