#pragma once
#include "../../common/defines.h"
#include <discord_rpc.h>

enum EDiscordAPIStates
{
	DISCORD_STATE_NULL,
	DISCORD_STATE_CLEAR,
	DISCORD_STATE_INITIALIZE,
	DISCORD_STATE_SELECT,
	DISCORD_STATE_LOAD,
	DISCORD_STATE_GAME
};

class CDiscordRPCIntegration : public CSingleton <CDiscordRPCIntegration>
{
	public:
		CDiscordRPCIntegration();
		virtual ~CDiscordRPCIntegration();

		void InitializeDiscordAPI(HWND hWnd);
		void ReleaseDiscordAPI();

		void UpdateDiscordState(uint8_t stateType, uint8_t channelID = 0, const std::string& mapKey = "", const std::string& name = "");

		bool IsInitialized() const { return m_bInitialized; };

	protected:
		void __InitializeDiscordAPIImpl();

	private:
		bool m_bInitialized;
		HWND m_hWnd;
		bool m_bSendDiscordPresence;
		UINT m_nDiscordUpdateTimerID;
		uint64_t m_nAppStartTimestamp;	
};
