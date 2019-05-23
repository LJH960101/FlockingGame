#pragma once
#include "NetworkModule/GameInfo.h"
#include "NetworkData.h"
#include <stdint.h>
#include "JHNET.h"

namespace MySerializer {
	int SerializeEnum(const EMessageType& type, char* outBuf);
	int SerializeWithEnum(const EMessageType& type, const char* inBuf, const int& bufLen, char* outBuf);
	EMessageType GetEnum(char* buf, int* cursor = nullptr);
	int BoolSerialize(char* buf, const bool& data);
	bool BoolDeserialize(const char* buf, int* cursor = nullptr);
	int CharSerialize(char* buf, const char& data);
	char CharDeserialize(const char* buf, int* cursor = nullptr);
	int IntSerialize(char* buf, const int32_t& val);
	int32_t IntDeserialize(const char* buf, int* cursor = nullptr);
	int UInt64Serialize(char* buf, const unsigned __int64& val);
	unsigned __int64 UInt64Deserialize(const char* buf, int* cursor = nullptr);
	int FloatSerialize(char* buf, const float& val);
	float FloatDeserialize(const char* buf, int* cursor = nullptr);
	int Vector3Serialize(char* buf, const FVector& val);
	FVector Vector3Deserialize(const char* buf, int* cursor = nullptr);
	int StringSerialize(char* buf, const char* source, const int& len);
	FSerializableString StringDeserialize(const char* buf, int* cursor = nullptr);
	int StringSerialize(char* buf, std::string source);
	FString FStringDeserialize(const char* buf, int* cursor = nullptr);
	int FStringSerialize(char* buf, const FString& source, int maxLen = 1000);
	void Serialize(char* source, char* dest, int size);
	void DeSerialize(const char* source, char* dest, int size);

	// DeSerializer
	template<typename T>
	inline T TDeSerialize(char* buf);
	template<>
	inline int TDeSerialize<int>(char* buf) {
		return IntDeserialize(buf, nullptr);
	}
	template<>
	inline EMessageType TDeSerialize<EMessageType>(char* buf) {
		return GetEnum(buf, nullptr);
	}
	template<>
	inline bool TDeSerialize<bool>(char* buf) {
		return BoolDeserialize(buf, nullptr);
	}
	template<>
	inline char TDeSerialize<char>(char* buf) {
		return CharDeserialize(buf, nullptr);
	}
	template<>
	inline unsigned __int64 TDeSerialize<unsigned __int64>(char* buf) {
		return UInt64Deserialize(buf, nullptr);
	}
	template<>
	inline float TDeSerialize<float>(char* buf) {
		return FloatDeserialize(buf, nullptr);
	}
	template<>
	inline FVector TDeSerialize<FVector>(char* buf) {
		return Vector3Deserialize(buf, nullptr);
	}
	template<>
	inline FSerializableString TDeSerialize<FSerializableString>(char* buf) {
		return StringDeserialize(buf, nullptr);
	}
	template<>
	inline FString TDeSerialize<FString>(char* buf) {
		return FStringDeserialize(buf, nullptr);
	}

	// Serializer
	template<typename T>
	inline int TSerialize(char* buf, const T& data);
	template<>
	inline int TSerialize<int>(char* buf, const int& data) {
		return IntSerialize(buf, data);
	}
	template<>
	inline int TSerialize<EMessageType>(char* buf, const EMessageType& data) {
		return SerializeEnum(data, buf);
	}
	template<>
	inline int TSerialize<bool>(char* buf, const bool& data) {
		return BoolSerialize(buf, data);
	}
	template<>
	inline int TSerialize<char>(char* buf, const char& data) {
		return CharSerialize(buf, data);
	}
	template<>
	inline int TSerialize<unsigned __int64>(char* buf, const unsigned __int64& data) {
		return UInt64Serialize(buf, data);
	}
	template<>
	inline int TSerialize<float>(char* buf, const float& data) {
		return FloatSerialize(buf, data);
	}
	template<>
	inline int TSerialize<FVector>(char* buf, const FVector& data) {
		return Vector3Serialize(buf, data);
	}
	template<>
	inline int TSerialize<std::string>(char* buf, const std::string& data) {
		return StringSerialize(buf, data);
	}
	template<>
	inline int TSerialize<FString>(char* buf, const FString& data) {
		return FStringSerialize(buf, data, 10000);
	}
}