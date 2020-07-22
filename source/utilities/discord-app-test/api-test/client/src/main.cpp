#define _CRT_SECURE_NO_WARNINGS

#include <array>
#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>

#include "fmt/format.h"
#include "discord.h"

#pragma comment( lib, "discord_game_sdk.dll.lib" )
#pragma comment( lib, "discord_sdk.lib" )

static inline std::string __ReadFromFile(const std::string& stFileName)
{
	std::ifstream in(stFileName.c_str(), std::ios_base::binary);
	in.exceptions(std::ios_base::badbit | std::ios_base::failbit | std::ios_base::eofbit);
	return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}
static inline std::vector <std::string> __StringSplit(const std::string& str, const std::string& tok = " ")
{
	std::vector <std::string> vec;

	std::size_t prev = 0;
	auto cur = str.find(tok);
	while (cur != std::string::npos)
	{
		vec.emplace_back(str.substr(prev, cur - prev));
		prev = cur + tok.size();
		cur = str.find(tok, prev);
	}

	vec.emplace_back(str.substr(prev, cur - prev));
	return vec;
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

	// Core
    auto result = discord::Core::Create(734739162141294694, DiscordCreateFlags_NoRequireDiscord, &core);
    state.core.reset(core);
    if (!state.core)
    {
        std::cout << "Failed to instantiate discord core! (err " << static_cast<int32_t>(result) << ")" << std::endl;
        std::exit(-1);
    }

	// Hooks
    state.core->SetLogHook(
		discord::LogLevel::Debug, [](discord::LogLevel level, const char* message) {
			std::cerr << "Log(" << static_cast<uint32_t>(level) << "): " << message << std::endl;
    });

    state.core->LobbyManager().OnLobbyMessage.Connect([&](std::int64_t lobbyId, std::int64_t userId,
                                                          std::uint8_t* payload, std::uint32_t payloadLength) {
        std::vector<uint8_t> buffer{};
        buffer.resize(payloadLength);

        memcpy(buffer.data(), payload, payloadLength);
        std::cout << "Lobby message, " << lobbyId << " from " << userId << " of length "
                  << payloadLength << " bytes." << std::endl;

        char fourtyNinetySix[4096];
        state.core->LobbyManager().GetLobbyMetadataValue(lobbyId, "foo", fourtyNinetySix);

        std::cout << "Metadata for key foo is " << fourtyNinetySix << std::endl;;
    });
	/*
	state.core->NetworkManager().OnMessage.Connect([&](discord::NetworkPeerId peerId, discord::NetworkChannelId channelId,
													   std::uint8_t* payload, std::uint32_t payloadLength) {
        std::vector<uint8_t> buffer{};
        buffer.resize(payloadLength);

        memcpy(buffer.data(), payload, payloadLength);
        std::cout << "Network message, Peer: " << peerId << " from " << channelId << " of length "
                  << payloadLength << " bytes." << std::endl;

		std::cout << fmt::format("Message from {0}: {1}", peerId, buffer.data())  << std::endl;
	});
	*/
#if 0
	state.core->NetworkManager().OnRouteUpdate.Connect([&state](char const* route) {
		std::cout << "Route " << route << std::endl;

		/*
		auto txn = state.core->LobbyManager().GetMemberUpdateTransaction();
		txn.SetMetadata("route", route);
		lobbyManager.UpdateMember(lobbyId, me.Id, txn, (result =>
		{
			// Who needs error handling anyway
			Console.WriteLine(result);
		}))
		*/
	});
#endif

	// Lobby DB
	auto lobbyData = __ReadFromFile("lobby_info.dat");
	if (lobbyData.empty())
	{
        std::cout << "Failed to reading lobby information db file" << std::endl;
        std::exit(-1);
	}
	auto parsedLobbyContent = __StringSplit(lobbyData, "\r\n");
	if (parsedLobbyContent.size() != 3) // id + secret + last line
	{
        std::cout << "Failed to parse lobby information db file" << std::endl;
        std::exit(-1);		
	}

	auto targetLobbyId = std::stoll(parsedLobbyContent.at(0));
	auto targetLobbySecret = parsedLobbyContent.at(1);
	std::cout << fmt::format("Target lobby: {} secret: {}", targetLobbyId, targetLobbySecret) << std::endl;

	// Funcs
	state.core->LobbyManager().ConnectLobby(targetLobbyId, targetLobbySecret.c_str(), 
										  [&state](discord::Result result, discord::Lobby const& lobby) {
		std::cout << "test" << std::endl;

		if (result == discord::Result::Ok)
		{
			std::cout << "succesfully connected to lobby" << std::endl;
			state.core->LobbyManager().ConnectNetwork(lobby.GetId());
			state.core->LobbyManager().OpenNetworkChannel(lobby.GetId(), 0, true);
		}
		else
		{
            std::cout << "Failed connecting lobby. (err " << static_cast<int32_t>(result) << ")\n";
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
