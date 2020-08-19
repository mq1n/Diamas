#pragma once
#include "../../common/defines.h"
#include <discord.h>

struct DiscordLobbyData
{
	discord::LobbyId lobbyId{};
	discord::LobbySecret secret{};
};

class CDiscordGameSDKIntegration : public CSingleton <CDiscordGameSDKIntegration>
{
	struct DiscordState
	{
		discord::User currentUser{};
		std::unique_ptr <discord::Core> core{};
	};

	public:
		CDiscordGameSDKIntegration();
		virtual ~CDiscordGameSDKIntegration() = default;

		void InitializeDiscordAPI();
		void ReleaseDiscordAPI();
		void OnUpdate();

		auto IsInitialized() const { return m_bInitialized; };

		void ResetLobbyID();
		void ConnectToLobby(_In_ const DiscordLobbyData& data);
		void CreateLobby(_In_ std::uint32_t capacity, _In_ discord::LobbyType type, _Out_ DiscordLobbyData& data);
		void DisconnectLobby();

	private:
		bool m_bInitialized;

		DiscordState m_state{};
		discord::LobbyId m_lobbyId;
};
