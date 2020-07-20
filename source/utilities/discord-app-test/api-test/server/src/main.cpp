#define _CRT_SECURE_NO_WARNINGS

#include <array>
#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <vector>

#include "fmt/format.h"
#include "discord.h"

#pragma comment( lib, "discord_game_sdk.dll.lib" )
#pragma comment( lib, "discord_sdk.lib" )

#ifdef _DEBUG
	#pragma comment( lib, "fmt_debug.lib" )
#else
	#pragma comment( lib, "fmt.lib" )
#endif

#define LOBBY_DATA_FILENAME "lobby_info.dat"

// https://discordapp.com/developers/docs/game-sdk/networking#example-connecting-to-another-player-in-a-lobby

static void __WriteToFile(const std::string& stFileName, const std::string& stData)
{
	std::ofstream f(stFileName.c_str(), std::ofstream::out | std::ofstream::app);
	f << stData.c_str() << std::endl;
	f.close();
}

struct DiscordState
{
    discord::User currentUser;

    std::unique_ptr<discord::Core> core;
};

namespace
{
volatile bool interrupted{false};
}

int32_t main(int32_t, char**)
{
	// Variables
    DiscordState state{};
    discord::Core* core{};
    discord::LobbyTransaction transaction{};

	// Remove old lobby informations
	std::filesystem::remove(LOBBY_DATA_FILENAME);

	// Core
    auto result = discord::Core::Create(734739162141294694, DiscordCreateFlags_NoRequireDiscord, &core);
    state.core.reset(core);
    if (!state.core)
    {
        std::cout << "Failed to instantiate discord core! (err " << static_cast<int32_t>(result) << ")\n";
        std::exit(-1);
    }

	// Hooks
    state.core->SetLogHook(
		discord::LogLevel::Debug, [](discord::LogLevel level, const char* message) {
			std::cerr << "Log(" << static_cast<uint32_t>(level) << "): " << message << "\n";
    });

    state.core->LobbyManager().OnLobbyUpdate.Connect([](std::int64_t lobbyId) {
		std::cout << "Lobby update " << lobbyId << "\n";
	});
    state.core->LobbyManager().OnLobbyDelete.Connect([](std::int64_t lobbyId, std::uint32_t reason) {
		std::cout << "Lobby delete " << lobbyId << " (reason: " << reason << ")\n";
    });
    state.core->LobbyManager().OnMemberConnect.Connect([&state](std::int64_t lobbyId, std::int64_t userId) {
        state.core->UserManager().GetUser(userId,
                                          [](discord::Result result, discord::User const& user) {
                                              if (result == discord::Result::Ok) {
                                                  std::cout << "Get " << user.GetUsername() << "\n";
                                              }
                                              else {
                                                  std::cout << "Failed to get David!\n";
                                              }
                                          });

        std::cout << "Lobby member connect " << lobbyId << " userId " << userId << "\n";

		state.core->LobbyManager().SendNetworkMessage(lobbyId, userId, 0, (uint8_t*)"Hello!", 6);
		state.core->NetworkManager().Flush();
    });
    state.core->LobbyManager().OnMemberUpdate.Connect([&state](std::int64_t lobbyId, std::int64_t userId) {
        state.core->UserManager().GetUser(userId,
                                          [](discord::Result result, discord::User const& user) {
                                              if (result == discord::Result::Ok) {
                                                  std::cout << "Get " << user.GetUsername() << "\n";
                                              }
                                              else {
                                                  std::cout << "Failed to get David!\n";
                                              }
                                          });

        std::cout << "Lobby member update " << lobbyId << " userId " << userId << "\n";
    });
    state.core->LobbyManager().OnMemberDisconnect.Connect([](std::int64_t lobbyId, std::int64_t userId) {
        std::cout << "Lobby member disconnect " << lobbyId << " userId " << userId << "\n";
    });
    state.core->LobbyManager().OnLobbyMessage.Connect([&](std::int64_t lobbyId, std::int64_t userId,
                                                          std::uint8_t* payload, std::uint32_t payloadLength) {
        std::vector<uint8_t> buffer{};
        buffer.resize(payloadLength);

        memcpy(buffer.data(), payload, payloadLength);
        std::cout << "Lobby message " << lobbyId << " from " << userId << " of length "
                  << payloadLength << " bytes.\n";

        char fourtyNinetySix[4096];
        state.core->LobbyManager().GetLobbyMetadataValue(lobbyId, "foo", fourtyNinetySix);

        std::cout << "Metadata for key foo is " << fourtyNinetySix << "\n";
    });
    state.core->LobbyManager().OnSpeaking.Connect([&](std::int64_t, std::int64_t userId, bool speaking) {
        std::cout << "User " << userId << " is " << (speaking ? "" : "NOT ") << "speaking.\n";
    });
#if 0
	state.core->NetworkManager().OnMessage.Connect([&](discord::NetworkPeerId peerId, discord::NetworkChannelId channelId,
													   std::uint8_t* payload, std::uint32_t payloadLength) {
		std::cout << "peer " << peerId << std::endl;
		/*
        std::vector<uint8_t> buffer{};
        buffer.resize(payloadLength);

        memcpy(buffer.data(), payload, payloadLength);
        std::cout << "Network message, Peer: " << peerId << " from " << channelId << " of length "
                  << payloadLength << " bytes." << std::endl;

		std::cout << fmt::format("Message from {0}: {1}", peerId, buffer.data())  << std::endl;
		*/
	});
#endif
	
	// Create the transaction
	result = state.core->LobbyManager().GetLobbyCreateTransaction(&transaction);
	if (result != discord::Result::Ok)
	{
		std::cout << "Failed creating lobby transaction. (err " << static_cast<int32_t>(result) << ")\n";
        std::exit(-1);
	}

	// Set lobby information
	transaction.SetCapacity(1024);
	transaction.SetType(discord::LobbyType::Private);
	transaction.SetMetadata("a", "123");
    transaction.SetMetadata("foo", "bar");

	// Create it!
	state.core->LobbyManager().CreateLobby(transaction, [&state](discord::Result result, discord::Lobby const& lobby) {
		if (result == discord::Result::Ok)
		{
			std::cout << fmt::format("{} lobby {} created with secret {}",
				(result == discord::Result::Ok) ? "Succeeded" : "Failed", lobby.GetId(), lobby.GetSecret()) << std::endl;

			__WriteToFile(LOBBY_DATA_FILENAME, fmt::format("{}\n{}", lobby.GetId(), lobby.GetSecret()));

			std::cout << "owner: " << int64_t(lobby.GetOwnerId()) << std::endl;
			std::cout << "locked: " << int64_t(lobby.GetLocked()) << std::endl;

			auto ret = state.core->LobbyManager().ConnectNetwork(lobby.GetId());
			std::cout << "connect " << int32_t(ret) << std::endl;
			ret = state.core->LobbyManager().OpenNetworkChannel(lobby.GetId(), 0, true);
			std::cout << "open " << int32_t(ret) << std::endl;

			/*
			std::array<uint8_t, 234> data{};
			state.core->LobbyManager().SendLobbyMessage(
				lobby.GetId(),
				reinterpret_cast<uint8_t*>(data.data()), data.size(),
				[](discord::Result result) {
					std::cout << "Sent message. Result: " << static_cast<int32_t>(result) << "\n";
				}
			);
			*/
		}
		else
		{
            std::cout << "Failed creating lobby. (err " << static_cast<int32_t>(result) << ")\n";
        }
	});

	// Main loop
    std::signal(SIGINT, [](int32_t) { interrupted = true; });
    do {
        state.core->RunCallbacks();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    } while (!interrupted);
    return 0;
}
