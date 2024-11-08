#pragma once
#include "typedef.h"
#include <stdint.h>
#include <string>

#define ACMD(name)  void (name)(LPCHARACTER ch, const char *argument, int32_t cmd, int32_t subcmd)
#define CMD_NAME(name) cmd_info[cmd].command

struct command_info
{
	const char * command;
	void (*command_pointer) (LPCHARACTER ch, const char *argument, int32_t cmd, int32_t subcmd);
	int32_t subcmd;
	int32_t minimum_position;
	int32_t gm_level;
};

extern struct command_info cmd_info[];

extern void interpret_command(LPCHARACTER ch, const char * argument, size_t len);
extern void interpreter_set_privilege(const char * cmd, int32_t lvl);

enum SCMD_ACTION
{
	SCMD_SLAP,
	SCMD_KISS,
	SCMD_FRENCH_KISS,
	SCMD_HUG,
	SCMD_LONG_HUG,
	SCMD_SHOLDER,
	SCMD_FOLD_ARM
};

enum SCMD_CMD
{
	SCMD_LOGOUT,
	SCMD_QUIT,
	SCMD_PHASE_SELECT,
	SCMD_SHUTDOWN,
};

enum SCMD_RESTART
{
	SCMD_RESTART_TOWN,
	SCMD_RESTART_HERE,
	SCMD_RESTART_BASE
};

enum SCMD_XMAS
{
	SCMD_XMAS_BOOM,
	SCMD_XMAS_SNOW,
	SCMD_XMAS_SANTA,
};

extern void Shutdown(int32_t iSec);
extern void SendLog(const char * c_pszBuf);		
extern void SendNotice(const char * c_pszBuf, bool bBigFont=false);
extern void BroadcastNotice(const char * c_pszBuf, bool bBigFont=false);
extern void SendNoticeMap(const char* c_pszBuf, int32_t nMapIndex, bool bBigFont); // 지정 맵에만 공지

// LUA_ADD_BGM_INFO
void CHARACTER_SetBGMVolumeEnable();
void CHARACTER_AddBGMInfo(uint32_t mapIndex, const char* name, float vol);
// END_OF_LUA_ADD_BGM_INFO

// LUA_ADD_GOTO_INFO
extern void CHARACTER_AddGotoInfo(const std::string& c_st_name, uint8_t empire, int32_t mapIndex, uint32_t x, uint32_t y);
// END_OF_LUA_ADD_GOTO_INFO
