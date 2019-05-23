#pragma once
#define MAX_STRING_BUF 100
#include <iostream>
#include "JHNET.h"

/*
	Used in network.
	[Type(Sever,Client, Common)]_[Place to use]_[Function]
*/
enum class EMessageType {
	// COMMON -----
	COMMON_ECHO,
	COMMON_PING,

	C_UDP_Reg,
	S_UDP_Response,

	S_Common_RequestId,
	C_Common_AnswerId,

	// DEBUG_ONLY
	C_Debug_RoomStart,
	C_Debug_GameStart,

	S_Room_Info,
	S_Room_ChangeState,

	// Lobby -----
	C_Lobby_InviteFriend_Request,
	S_Lobby_InviteFriend_Request,
	C_Lobby_InviteFriend_Answer,
	S_Lobby_InviteFriend_Failed,
	C_Lobby_Set_PartyKing,
	C_Lobby_FriendKick_Request,

	C_Lobby_MatchRequest,
	S_Lobby_MatchAnswer,
	S_Lobby_GameStart,

	// INGAME
	// Spawn
	C_INGAME_SPAWN,
	S_INGAME_SPAWN,

	// RPC
	C_INGAME_RPC,
	S_INGAME_RPC,

	// RPC
	C_INGAME_SyncVar,
	S_INGAME_SyncVar,

	// Disconnect
	S_DISCONNECT_SLOT,
	S_RECONNECT_SLOT,
	S_RECONNECT,

	S_END_GAME,
	C_END_GAME,

	C_RECONNECT_SERVER,

	MAX
};

struct FSerializableString
{
	int len;
	char buf[MAX_STRING_BUF];
};