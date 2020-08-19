#include "StdAfx.h"
#include "DiscordGameSDKIntegration.h"
#include "PythonNetworkStream.h"
#include <csignal>
#include <signal.h>

// #define ENABLE_DISCORD_NETWORK_CONNECTIONS

static constexpr auto DISCORD_APP_ID   = 744842429693165629;

namespace
{
	volatile bool g_interrupted{false};
}
void __cdecl OnSignal(int signal)
{
	g_interrupted = true;
	CDiscordGameSDKIntegration::Instance().DisconnectLobby();
}


CDiscordGameSDKIntegration::CDiscordGameSDKIntegration() : 
	m_bInitialized(false), m_lobbyId(0)
{
}

void CDiscordGameSDKIntegration::ResetLobbyID()
{
	m_lobbyId = 0;
}
void CDiscordGameSDKIntegration::DisconnectLobby()
{
	// Disconnect than lobby
	if (m_lobbyId)
	{
#ifdef ENABLE_DISCORD_NETWORK_CONNECTIONS
		m_state.core->LobbyManager().DisconnectNetwork(m_lobbyId);
#endif
		m_state.core->LobbyManager().DisconnectVoice(m_lobbyId, [&](discord::Result result) {
			Tracenf("[Discord] DisconnectVoice ret: %d", result);
		});

		std::int32_t member_count = 0;
		const auto ret = m_state.core->LobbyManager().MemberCount(m_lobbyId, &member_count);
		Tracenf("[Discord] Lobby member count: %d", member_count);

		if (ret != discord::Result::Ok || member_count == 1)
		{
			m_state.core->LobbyManager().DeleteLobby(m_lobbyId, [&](discord::Result result) {
				Tracenf("[Discord] DeleteLobby ret: %d", result);
			});
		} else {
			m_state.core->LobbyManager().DisconnectLobby(m_lobbyId, [&](discord::Result result) {
				Tracenf("[Discord] DisconnectLobby ret: %d", result);
			});
		}
	}

	ResetLobbyID();
}
void CDiscordGameSDKIntegration::ConnectToLobby(const DiscordLobbyData& data)
{
	Tracenf("[Discord] ConnectToLobby | Target lobby: %lld secret: %s", data.lobbyId, data.secret);

	// Connect to already exist lobby
	m_state.core->LobbyManager().ConnectLobby(data.lobbyId, data.secret, [&](discord::Result result, const discord::Lobby& lobby) {
		if (result == discord::Result::Ok)
		{
			Tracenf("[Discord] Succesfully connected to lobby: %lld", data.lobbyId);

#ifdef ENABLE_DISCORD_NETWORK_CONNECTIONS
			result = m_state.core->LobbyManager().ConnectNetwork(lobby.GetId());
			if (result != discord::Result::Ok)
			{
				TraceError("[Discord] Could not connected to network channel. Error: %d", result);
				return;
			}
			Tracenf("[Discord] Connected to network channel");

			result = m_state.core->LobbyManager().OpenNetworkChannel(lobby.GetId(), 0, true);
			if (result != discord::Result::Ok)
			{
				TraceError("[Discord] Could not open network channel. Error: %d", result);
				return;
			}
			Tracenf("[Discord] Opened network channel");
#endif

			m_state.core->LobbyManager().ConnectVoice(lobby.GetId(), [&](discord::Result result) {
				if (result != discord::Result::Ok)
				{
					TraceError("[Discord] Could not connected to voice channel. Error: %d", result);
					return;
				}
				Tracenf("[Discord] Connected to voice channel");
			});

			m_lobbyId = data.lobbyId;
		}
		else
		{
			TraceError("[Discord] Could not connected to lobby. Error: %d", result);
		}

		SPacketCGDiscordLobbyJoinRet ret_packet;
		ret_packet.result = static_cast<uint8_t>(result);
		if (!ret_packet.result)
		{
			ret_packet.lobby = data.lobbyId;
			strcpy(ret_packet.secret, data.secret);
		}
		CPythonNetworkStream::Instance().Send(sizeof(ret_packet), &ret_packet);
	});
}
void CDiscordGameSDKIntegration::CreateLobby(std::uint32_t capacity, discord::LobbyType type, _Out_ DiscordLobbyData& data)
{
	// Create the transaction
	discord::LobbyTransaction transaction{};
	auto result = m_state.core->LobbyManager().GetLobbyCreateTransaction(&transaction);
	if (result != discord::Result::Ok)
	{
		TraceError("[Discord] Failed creating lobby transaction. Error: %d", result);
		return;
	}

	// Set lobby information
	transaction.SetCapacity(capacity);
	transaction.SetType(type);

	// Create it!
	m_state.core->LobbyManager().CreateLobby(transaction, [&](discord::Result result, const discord::Lobby& lobby) {
		if (result == discord::Result::Ok)
		{
			Tracenf("[Discord] %s lobby %lld created with secret %s", (result == discord::Result::Ok) ? "Succeeded" : "Failed", lobby.GetId(), lobby.GetSecret());

#ifdef ENABLE_DISCORD_NETWORK_CONNECTIONS
			result = m_state.core->LobbyManager().ConnectNetwork(lobby.GetId());
			if (result != discord::Result::Ok)
			{
				TraceError("[Discord] Could not connected to network channel. Error: %d", result);
				return;
			}
			Tracenf("Connected to network channel");

			result = m_state.core->LobbyManager().OpenNetworkChannel(lobby.GetId(), 0, true);
			if (result != discord::Result::Ok)
			{
				TraceError("[Discord] Could not open network channel. Error: %d", result);
				return;
			}
			Tracenf("[Discord] Opened network channel");
#endif

			m_state.core->LobbyManager().ConnectVoice(lobby.GetId(), [&](discord::Result result) {
				if (result != discord::Result::Ok)
				{
					TraceError("Could not connected to voice channel. Error: %d", result);
					return;
				}
				Tracenf("[Discord] Connected to voice channel");
			});

			m_lobbyId = lobby.GetId();
			data.lobbyId = lobby.GetId();
			data.secret = lobby.GetSecret();
		}
		else
		{
			TraceError("[Discord] Could not created the lobby. Error: %d", result);
		}

		SPacketCGDiscordLobbyCreateRet ret_packet;
		ret_packet.result = static_cast<uint8_t>(result);
		if (!ret_packet.result)
		{
			ret_packet.lobby = data.lobbyId;
			strcpy(ret_packet.secret, data.secret);
		}
		CPythonNetworkStream::Instance().Send(sizeof(ret_packet), &ret_packet);
	});
}

void CDiscordGameSDKIntegration::OnUpdate()
{
	if (m_bInitialized && m_state.core)
	{
		m_state.core->RunCallbacks();
	}
}
void CDiscordGameSDKIntegration::InitializeDiscordAPI()
{
	// Core
	discord::Core* core{};
	auto result = discord::Core::Create(DISCORD_APP_ID, DiscordCreateFlags_NoRequireDiscord, &core);
	m_state.core.reset(core);
	if (!m_state.core)
	{
		TraceError("[Discord] Failed to instantiate discord core! (err '%d')", result);
		PostQuitMessage(0);
	}
	Tracenf("[Discord] Discord core succesfully created!");

	// Common hook
	m_state.core->SetLogHook(discord::LogLevel::Debug, [&](discord::LogLevel level, const char* message) {
		TraceError("[Discord-Core] [%u]: %s", level, message);
	});

	// Lobby hooks
	m_state.core->LobbyManager().OnLobbyUpdate.Connect([&](std::int64_t lobbyId) {
		Tracenf("[Discord] OnLobbyUpdate: %lld", lobbyId);
	});
	m_state.core->LobbyManager().OnLobbyDelete.Connect([&](std::int64_t lobbyId, std::uint32_t reason) {
		Tracenf("[Discord] OnLobbyDelete: %lld %u", lobbyId, reason);
		ResetLobbyID();
	});
	m_state.core->LobbyManager().OnMemberConnect.Connect([&](std::int64_t lobbyId, std::int64_t userId) {
		Tracenf("[Discord] OnMemberConnect: %lld %u", lobbyId, userId);
	});
	m_state.core->LobbyManager().OnMemberUpdate.Connect([&](std::int64_t lobbyId, std::int64_t userId) {
		Tracenf("[Discord] OnMemberUpdate: %lld %u", lobbyId, userId);
	});
	m_state.core->LobbyManager().OnMemberDisconnect.Connect([&](std::int64_t lobbyId, std::int64_t userId) {
		Tracenf("[Discord] OnMemberDisconnect: %lld %u", lobbyId, userId);
		if (userId == m_state.currentUser.GetId())
			ResetLobbyID();
	});
	m_state.core->LobbyManager().OnLobbyMessage.Connect([&](std::int64_t lobbyId, std::int64_t userId, std::uint8_t* payload, std::uint32_t payloadLength) {
		std::vector<uint8_t> buffer{};
		buffer.resize(payloadLength);

		memcpy(buffer.data(), payload, payloadLength);
		Tracenf("[Discord] Lobby message, %lld from %lld of length %u ptr %p", lobbyId, userId, payloadLength, payload);
	});
	m_state.core->LobbyManager().OnSpeaking.Connect([&](std::int64_t lobbyId, std::int64_t userId, bool speaking) {
		Tracenf("[Discord] OnSpeaking: %lld %lld %d", lobbyId, userId, speaking ? 1 : 0);
	});
#ifdef ENABLE_DISCORD_NETWORK_CONNECTIONS
	m_state.core->LobbyManager().OnNetworkMessage.Connect([&](std::int64_t lobbyId, std::int64_t userId, std::uint8_t channelId, std::uint8_t* data, std::uint32_t dataSize) {
		Tracenf("[Discord] OnNetworkMessage: %lld %lld %u %p %u", lobbyId, userId, channelId, data, dataSize);
	});
#endif

	std::signal(SIGINT, OnSignal);

	m_bInitialized = true;
}
void CDiscordGameSDKIntegration::ReleaseDiscordAPI()
{
	if (!m_bInitialized)
		return;

	m_bInitialized = false;
}
