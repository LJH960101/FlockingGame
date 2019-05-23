#include "NetworkModule/Serializer.h"

// RPC
// You need to use function's first line.
// ... is function args.
// RPC(NetBaseCP, AHACKEDCharacter, Move, ENetRpcType::Master, false, speed)
#define RPC(NETBASECP, CLASS_NAME, FUNCTION_NAME, E_NET_RPC_TYPE, IS_RELIABLE, ...)\
PP_MACRO_OVERLOAD_RPC(NETBASECP, CLASS_NAME, FUNCTION_NAME, &CLASS_NAME::FUNCTION_NAME, E_NET_RPC_TYPE, IS_RELIABLE, RPC, __VA_ARGS__)

// BindRPCFunction
// You need to use in begin Play to register function.
// BindRPCFunction(NetBaseCP, AHACKEDCharacter, Move)
#define BindRPCFunction(NETBASECP, CLASS_NAME, FUNCTION_NAME)\
{\
	JHNET_CHECK(this);\
	JHNET_CHECK(NETBASECP);\
	auto rpcDelegate = NETBASECP->CreateBindableDelegateFunction(TEXT(#FUNCTION_NAME));\
	JHNET_CHECK(rpcDelegate);\
  	rpcDelegate->BindUObject(this, &CLASS_NAME::FUNCTION_NAME##_##);\
}

// RPC_FUNCTION
// You need to use this macro up of rpc function.
// (ClassName, FunctionName, args...)
// RPC_FUNCTION(AHACKEDCharacter, Move, int)
// Arg length available is 0~3
#define RPC_FUNCTION(CLASS_NAME, FUNCTION_NAME, ...)\
PP_MACRO_OVERLOAD_RPC_FUNCTION(CLASS_NAME, FUNCTION_NAME, RPC_FUNCTION, __VA_ARGS__)


#define MACRO_TO_CHAR(A)\
#A

template <typename FirstArg>
inline size_t GetSizeOfBuf(const char*) {
	return sizeof(FirstArg);
}

template <>
inline size_t GetSizeOfBuf<FString>(const char* buf) {
	return sizeof(int32_t) + MySerializer::IntDeserialize(buf, nullptr);
}

/*
가변 크기 인자로 인해 쓰이지 않음.
template <typename FirstArg, typename... SizeArgs, std::enable_if_t<(sizeof...(SizeArgs) != 0)>* = nullptr>
inline size_t GetSizeOfBuf(const char* buf)
{
	return GetSizeOfBuf<FirstArg>(buf) + GetSizeOfBuf<SizeArgs...>(buf);
}*/

template <typename T>
inline size_t GetSizeOfOrigin(T origin) {
	return sizeof(origin);
}

template <>
inline size_t GetSizeOfOrigin<FString>(FString origin) {
	int32 NumBytes = 0;
	const TCHAR* CharPos = *origin;

	while (*CharPos && NumBytes < BUFSIZE)
	{
		CharPos++;
		++NumBytes;
	}
	return NumBytes + sizeof(int32_t); // FString은 Byte 앞에 int를 붙이므로 int를 추가로 적어줘야한다.
}

// RPC_FUNCTION
#define RPC_FUNCTION_0(CLASS_NAME, FUNCTION_NAME)\
bool CLASS_NAME##_##FUNCTION_NAME##_INLOOP = false;\
UFUNCTION()\
void FUNCTION_NAME##_##(int len, char* buf){\
	CLASS_NAME##_##FUNCTION_NAME##_INLOOP = true;\
	if(len != 0){\
		JHNET_LOG(Error, "Argment Error!");\
		CLASS_NAME##_##FUNCTION_NAME##_INLOOP = false;\
		return;\
	}\
	auto func = std::bind(&CLASS_NAME::FUNCTION_NAME, this);\
	func();\
	CLASS_NAME##_##FUNCTION_NAME##_INLOOP = false;\
}

#define RPC_FUNCTION_1(CLASS_NAME, FUNCTION_NAME, ARG1)\
bool CLASS_NAME##_##FUNCTION_NAME##_INLOOP = false;\
UFUNCTION()\
void FUNCTION_NAME##_##(int len, char* buf){\
	CLASS_NAME##_##FUNCTION_NAME##_INLOOP = true;\
	if(len != GetSizeOfBuf<ARG1>(buf)){\
		JHNET_LOG(Error, "Argment Error!");\
		CLASS_NAME##_##FUNCTION_NAME##_INLOOP = false;\
		return;\
	}\
	auto func = std::bind(&CLASS_NAME::FUNCTION_NAME, this, std::placeholders::_1);\
	auto arg1 = MySerializer::TDeSerialize<ARG1>(buf);\
	func(arg1);\
	CLASS_NAME##_##FUNCTION_NAME##_INLOOP = false;\
}

#define RPC_FUNCTION_2(CLASS_NAME, FUNCTION_NAME, ARG1, ARG2)\
bool CLASS_NAME##_##FUNCTION_NAME##_INLOOP = false;\
UFUNCTION()\
void FUNCTION_NAME##_##(int len, char* buf){\
	CLASS_NAME##_##FUNCTION_NAME##_INLOOP = true;\
	if(len != GetSizeOfBuf<ARG1>(buf) + \
				GetSizeOfBuf<ARG2>(buf + GetSizeOfBuf<ARG1>(buf))){\
		JHNET_LOG(Error, "Argment Error!");\
		CLASS_NAME##_##FUNCTION_NAME##_INLOOP = false;\
		return;\
	}\
	auto func = std::bind(&CLASS_NAME::FUNCTION_NAME, this, std::placeholders::_1, std::placeholders::_2);\
	auto arg1 = MySerializer::TDeSerialize<ARG1>(buf);\
	auto arg2 = MySerializer::TDeSerialize<ARG2>(buf + GetSizeOfBuf<ARG1>(buf));\
	func(arg1, arg2);\
	CLASS_NAME##_##FUNCTION_NAME##_INLOOP = false;\
}

#define RPC_FUNCTION_3(CLASS_NAME, FUNCTION_NAME, ARG1, ARG2, ARG3)\
bool CLASS_NAME##_##FUNCTION_NAME##_INLOOP = false;\
UFUNCTION()\
void FUNCTION_NAME##_##(int len, char* buf){\
	CLASS_NAME##_##FUNCTION_NAME##_INLOOP = true;\
	if(len != GetSizeOfBuf<ARG1>(buf) + \
				GetSizeOfBuf<ARG2>(buf + GetSizeOfBuf<ARG1>(buf)) + \
				GetSizeOfBuf<ARG3>(buf + GetSizeOfBuf<ARG1>(buf) + GetSizeOfBuf<ARG2>(buf + GetSizeOfBuf<ARG1>(buf)))){\
		JHNET_LOG(Error, "Argment Error!");\
		CLASS_NAME##_##FUNCTION_NAME##_INLOOP = false;\
		return;\
	}\
	auto func = std::bind(&CLASS_NAME::FUNCTION_NAME, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);\
	auto arg1 = MySerializer::TDeSerialize<ARG1>(buf);\
	auto arg2 = MySerializer::TDeSerialize<ARG2>(buf + GetSizeOfBuf<ARG1>(buf));\
	auto arg3 = MySerializer::TDeSerialize<ARG3>(buf + GetSizeOfBuf<ARG1>(buf) + GetSizeOfBuf<ARG2>(buf + GetSizeOfBuf<ARG1>(buf)));\
	func(arg1, arg2, arg3);\
	CLASS_NAME##_##FUNCTION_NAME##_INLOOP = false;\
}

// RPC
#define RPC_0(NETBASECP, CLASS_NAME, FUNCTION_NAME, CLASS_AND_FUNCTION_NAME, E_NET_RPC_TYPE, IS_RELIABLE)\
{\
	if(NETBASECP && !CLASS_NAME##_##FUNCTION_NAME##_INLOOP){\
		NETBASECP->ExecuteNetFunc(\
			TEXT(#FUNCTION_NAME),\
			E_NET_RPC_TYPE, 0, nullptr, IS_RELIABLE, false);\
		if(E_NET_RPC_TYPE == ENetRPCType::MASTER && NETBASECP->IsSlave()) return;\
	}\
}

#define RPC_1(NETBASECP, CLASS_NAME, FUNCTION_NAME, CLASS_AND_FUNCTION_NAME, E_NET_RPC_TYPE, IS_RELIABLE, ARG1)\
{\
	if(NETBASECP && !CLASS_NAME##_##FUNCTION_NAME##_INLOOP){\
		std::shared_ptr<char[]> newBuf(new char[GetSizeOfOrigin(ARG1)]);\
		int len = MySerializer::TSerialize(newBuf.get(), ARG1);\
		NETBASECP->ExecuteNetFunc(\
			TEXT(#FUNCTION_NAME),\
			E_NET_RPC_TYPE, len, newBuf.get(), IS_RELIABLE, false);\
		if(E_NET_RPC_TYPE == ENetRPCType::MASTER && NETBASECP->IsSlave()) return;\
	}\
}

#define RPC_2(NETBASECP, CLASS_NAME, FUNCTION_NAME, CLASS_AND_FUNCTION_NAME, E_NET_RPC_TYPE, IS_RELIABLE, ARG1, ARG2)\
{\
	if(NETBASECP && !CLASS_NAME##_##FUNCTION_NAME##_INLOOP){\
		std::shared_ptr<char[]> newBuf(new char[GetSizeOfOrigin(ARG1) + GetSizeOfOrigin(ARG2)]);\
		int len = MySerializer::TSerialize(newBuf.get(), ARG1);\
		len += MySerializer::TSerialize(newBuf.get() + GetSizeOfOrigin(ARG1), ARG2);\
		NETBASECP->ExecuteNetFunc(\
			TEXT(#FUNCTION_NAME),\
			E_NET_RPC_TYPE, len, newBuf.get(), IS_RELIABLE, false);\
		if(E_NET_RPC_TYPE == ENetRPCType::MASTER && NETBASECP->IsSlave()) return;\
	}\
}

#define RPC_3(NETBASECP, CLASS_NAME, FUNCTION_NAME, CLASS_AND_FUNCTION_NAME, E_NET_RPC_TYPE, IS_RELIABLE, ARG1, ARG2, ARG3)\
{\
	if(NETBASECP && !CLASS_NAME##_##FUNCTION_NAME##_INLOOP){\
		std::shared_ptr<char[]> newBuf(new char[GetSizeOfOrigin(ARG1) + GetSizeOfOrigin(ARG2) + GetSizeOfOrigin(ARG3)]);\
		int len = MySerializer::TSerialize(newBuf.get(), ARG1);\
		len += MySerializer::TSerialize(newBuf.get() + GetSizeOfOrigin(ARG1), ARG2);\
		len += MySerializer::TSerialize(newBuf.get() + GetSizeOfOrigin(ARG1) + GetSizeOfOrigin(ARG2), ARG3);\
		NETBASECP->ExecuteNetFunc(\
			TEXT(#FUNCTION_NAME),\
			E_NET_RPC_TYPE, len, newBuf.get(), IS_RELIABLE, false);\
		if(E_NET_RPC_TYPE == ENetRPCType::MASTER && NETBASECP->IsSlave()) return;\
	}\
}

// General utility macro
#define PP_CAT( A, B ) A ## B
#define PP_EXPAND(...) __VA_ARGS__

// Macro overloading feature support
#define PP_VA_ARG_SIZE(...) PP_EXPAND(PP_APPLY_ARG_N((PP_ZERO_ARGS_DETECT(__VA_ARGS__), PP_RSEQ_N)))

#define PP_ZERO_ARGS_DETECT(...) PP_EXPAND(PP_ZERO_ARGS_DETECT_PREFIX_ ## __VA_ARGS__ ## _ZERO_ARGS_DETECT_SUFFIX)
#define PP_ZERO_ARGS_DETECT_PREFIX__ZERO_ARGS_DETECT_SUFFIX ,,,,,,,,,,,0

#define PP_APPLY_ARG_N(ARGS) PP_EXPAND(PP_ARG_N ARGS)
#define PP_ARG_N(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N,...) N
#define PP_RSEQ_N 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define PP_OVERLOAD_SELECT(NAME, NUM) PP_CAT( NAME ## _, NUM)

#define PP_MACRO_OVERLOAD_RPC_FUNCTION(CLASS_NAME, FUNCTION_NAME, NAME, ...)\
PP_OVERLOAD_SELECT(NAME, PP_VA_ARG_SIZE(__VA_ARGS__))(CLASS_NAME, FUNCTION_NAME, __VA_ARGS__)

#define PP_MACRO_OVERLOAD_RPC(NETBASECP, CLASS_NAME, FUNCTION_NAME, CLASS_AND_FUNCTION_NAME, E_NET_RPC_TYPE, IS_RELIABLE, NAME, ...)\
PP_OVERLOAD_SELECT(NAME, PP_VA_ARG_SIZE(__VA_ARGS__))(NETBASECP, CLASS_NAME, FUNCTION_NAME, CLASS_AND_FUNCTION_NAME, E_NET_RPC_TYPE, IS_RELIABLE, __VA_ARGS__)