#include "stdafx.h"
#include "constants.h"
#include "config.h"
#include "event.h"
#include "minilzo.h"
#include "desc_manager.h"
#include "item_manager.h"
#include "char.h"
#include "char_manager.h"
#include "mob_manager.h"
#include "motion.h"
#include "sectree_manager.h"
#include "shop_manager.h"
#include "regen.h"
#include "text_file_loader.h"
#include "skill.h"
#include "pvp.h"
#include "party.h"
#include "quest_manager.h"
#include "profiler.h"
#include "lzo_manager.h"
#include "messenger_manager.h"
#include "db.h"
#include "log.h"
#include "p2p.h"
#include "guild_manager.h"
#include "dungeon.h"
#include "cmd.h"
#include "refine.h"
#include "banword.h"
#include "priv_manager.h"
#include "war_map.h"
#include "building.h"
#include "target.h"
#include "marriage.h"
#include "wedding.h"
#include "fishing.h"
#include "item_addon.h"
#include "locale_service.h"
#include "arena.h"
#include "ox_event.h"
#include "polymorph.h"
#include "blend_item.h"
#include "ani.h"
#include "mark_manager.h"
#include "spam.h"
#include "threeway_war.h"
#include "skill_power.h"
#include "dragon_soul.h"
#include "anticheat_manager.h"
#include "nearby_scanner.h"
#include "battleground.h"
#include "desc.h"
#include "desc_client.h"
#include "../../libthecore/include/winminidump.h"

#ifdef USE_STACKTRACE
	#ifdef _WIN32
		#include <DbgHelp.h>
	#else
		#include <execinfo.h>
	#endif
#endif

#include <signal.h>

#if defined(__FreeBSD__) && defined(DEBUG_ALLOC)
extern void (*_malloc_message)(const char* p1, const char* p2, const char* p3, const char* p4);
// FreeBSD _malloc_message replacement
void WriteMallocMessage(const char* p1, const char* p2, const char* p3, const char* p4)
{
	FILE* fp = ::fopen(DBGALLOC_LOG_FILENAME, "a");
	if (fp == nullptr) {
		return;
	}
	::fprintf(fp, "%s %s %s %s\n", p1, p2, p3, p4);
	::fclose(fp);
}
#endif


volatile int32_t	num_events_called = 0;
int32_t             max_bytes_written = 0;
int32_t             current_bytes_written = 0;
int32_t             total_bytes_written = 0;
uint8_t		g_bLogLevel = 0;

socket_t	tcp_socket = 0;
socket_t	p2p_socket = 0;

LPFDWATCH	main_fdw = nullptr;

int32_t		io_loop(LPFDWATCH fdw);

int32_t		idle();
void	destroy();


enum EProfile
{
	PROF_EVENT,
	PROF_CHR_UPDATE,
	PROF_IO,
	PROF_HEARTBEAT,
	PROF_MAX_NUM
};

static uint32_t s_dwProfiler[PROF_MAX_NUM];

int32_t g_shutdown_disconnect_pulse;
int32_t g_shutdown_disconnect_force_pulse;
int32_t g_shutdown_core_pulse;
bool g_bShutdown=false;
bool g_bSignalHandled=false;

extern void CancelReloadSpamEvent();

void ContinueOnFatalError()
{
#ifdef USE_STACKTRACE
	#ifdef _WIN32
		void *frames[64] = { 0 };
		auto wCapturedFrames = RtlCaptureStackBackTrace(2, 64, frames, nullptr);

		std::ostringstream oss;
		oss << std::endl;

		for (auto i = 0; i < wCapturedFrames; i++)
		{
			uint8_t buffer[sizeof(IMAGEHLP_SYMBOL64) + 128];
			auto symbol64 = reinterpret_cast<IMAGEHLP_SYMBOL64*>(buffer);
			memset(symbol64, 0, sizeof(IMAGEHLP_SYMBOL64) + 128);
			symbol64->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
			symbol64->MaxNameLength = 128;

			DWORD64 displacement = 0;
			auto result = SymGetSymFromAddr64(GetCurrentProcess(), (DWORD64)frames[i], &displacement, symbol64);
			if (result)
			{
				IMAGEHLP_LINE64 line64;
				memset(&line64, 0, sizeof(IMAGEHLP_LINE64));
				line64.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

				auto dwDisplacement = 0UL;
				result = SymGetLineFromAddr64(GetCurrentProcess(), (DWORD64)frames[i], &dwDisplacement, &line64);
				if (result)
				{
					oss << "  Stack> " << symbol64->Name << " File: " << line64.FileName << " Line: " << (int32_t)line64.LineNumber << std::endl;
				}
				else
				{
					oss << "  Stack> " << symbol64->Name << std::endl;
				}
			}
		}

		sys_err("FatalError on %s", oss.str().c_str());

	#else
	void* array[200];
	std::size_t size;
	char** symbols;

	size = backtrace(array, 200);
	symbols = backtrace_symbols(array, size);

	std::ostringstream oss;
	oss << std::endl;
	for (std::size_t i = 0; i < size; ++i) {
		oss << "  Stack> " << symbols[i] << std::endl;
	}

	free(symbols);

	sys_err("FatalError on %s", oss.str().c_str());
	#endif
#else
	sys_err("FatalError");
#endif
}

void __cdecl InterruptHandler(int)
{
	ContinueOnFatalError();
	g_bSignalHandled = true;
}

void TerminateHandler()
{
	ContinueOnFatalError();
	g_bSignalHandled = true;
}

void ShutdownOnFatalError()
{
	if (!g_bShutdown)
	{
		sys_err("ShutdownOnFatalError!!!!!!!!!!");
		{
			char buf[256];

			strlcpy(buf, LC_TEXT("서버에 치명적인 오류가 발생하여 자동으로 재부팅됩니다."), sizeof(buf));
			SendNotice(buf);
			strlcpy(buf, LC_TEXT("10초후 자동으로 접속이 종료되며,"), sizeof(buf));
			SendNotice(buf);
			strlcpy(buf, LC_TEXT("5분 후에 정상적으로 접속하실수 있습니다."), sizeof(buf));
			SendNotice(buf);
		}

		g_bShutdown = true;
		g_bNoMoreClient = true;

		g_shutdown_disconnect_pulse = thecore_pulse() + PASSES_PER_SEC(10);
		g_shutdown_disconnect_force_pulse = thecore_pulse() + PASSES_PER_SEC(20);
		g_shutdown_core_pulse = thecore_pulse() + PASSES_PER_SEC(30);
	}
}

namespace
{
	struct SendDisconnectFunc
	{
		void operator () (LPDESC d)
		{
			if (d->GetCharacter())
			{
				if (d->GetCharacter()->GetGMLevel() == GM_PLAYER)
					d->GetCharacter()->ChatPacket(CHAT_TYPE_COMMAND, "quit Shutdown(SendDisconnectFunc)");
			}
		}
	};

	struct DisconnectFunc
	{
		void operator () (LPDESC d)
		{
			if (d->GetType() == DESC_TYPE_CONNECTOR)
				return;

			if (d->IsPhase(PHASE_P2P))
				return;

			d->SetCloseReason("DC_FUNC");
			d->SetPhase(PHASE_CLOSE);
		}
	};
}

extern std::vector<TPlayerTable> g_vec_save;
uint32_t save_idx = 0;

void heartbeat(LPHEART ht, int32_t pulse) 
{
	auto t = get_dword_time();
	num_events_called += event_process(pulse);
	s_dwProfiler[PROF_EVENT] += (get_dword_time() - t);

	t = get_dword_time();

	// 1초마다
	if (!(pulse % ht->passes_per_sec))
	{
		DESC_MANAGER::Instance().ProcessExpiredLoginKey();

		{

			int count = 0;
			if (save_idx < g_vec_save.size())
			{
				count = MIN(100, g_vec_save.size() - save_idx);

				for (int32_t i = 0; i < count; ++i, ++save_idx)
					db_clientdesc->DBPacket(HEADER_GD_PLAYER_SAVE, 0, &g_vec_save[save_idx], sizeof(TPlayerTable));

				sys_log(0, "SAVE_FLUSH %d", count);
			}
		}
	}

	//
	// 25 PPS(Pulse per second) 라고 가정할 때
	//

	// 약 1.16초마다
	if (!(pulse % (passes_per_sec + 4)))
		CHARACTER_MANAGER::Instance().ProcessDelayedSave();

	// 약 5.08초마다
	if (!(pulse % (passes_per_sec * 5 + 2)))
	{
		ITEM_MANAGER::Instance().Update();
		DESC_MANAGER::Instance().UpdateLocalUserCount();
	}

	s_dwProfiler[PROF_HEARTBEAT] += (get_dword_time() - t);

	DBManager::Instance().Process();
	AccountDB::Instance().Process();
	CPVPManager::Instance().Process();

	if (g_bShutdown)
	{
		if (thecore_pulse() > g_shutdown_disconnect_pulse)
		{
			const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::Instance().GetClientSet();
			std::for_each(c_set_desc.begin(), c_set_desc.end(), ::SendDisconnectFunc());
			g_shutdown_disconnect_pulse = INT_MAX;
		}
		else if (thecore_pulse() > g_shutdown_disconnect_force_pulse)
		{
			const DESC_MANAGER::DESC_SET & c_set_desc = DESC_MANAGER::Instance().GetClientSet();
			std::for_each(c_set_desc.begin(), c_set_desc.end(), ::DisconnectFunc());
		}
		else if (thecore_pulse() > g_shutdown_disconnect_force_pulse + PASSES_PER_SEC(5))
		{
			thecore_shutdown();
		}
	}
}

static void CleanUpForEarlyExit()
{
	CancelReloadSpamEvent();
}


int32_t start()
{
#if defined(__FreeBSD__) && defined(DEBUG_ALLOC)
	_malloc_message = WriteMallocMessage;
#endif

	config_init("");

	bool bVerbose = false;
#ifdef _WIN32
	bVerbose = true;
#endif
	if (!bVerbose)
		freopen("stdout", "a", stdout);

	thecore_init();
	bool is_thecore_initialized = thecore_set(25, heartbeat);
	if (!is_thecore_initialized)
	{
		fprintf(stderr, "Could not initialize thecore, check owner of pid, syslog\n");
		exit(0);
	}
	sys_log(0, "the core init completed");

	if (false == CThreeWayWar::Instance().LoadSetting("forkedmapindex.txt"))
	{
		if (false == g_bAuthServer)
		{
			fprintf(stderr, "Could not Load ThreeWayWar Setting file");
			exit(0);
		}
	}

	signal_timer_disable();
	
	main_fdw = fdwatch_new(4096);

	if ((tcp_socket = socket_tcp_bind(g_szPublicIP, mother_port)) == INVALID_SOCKET)
	{
		sys_err("socket_tcp_bind: tcp_socket");
		perror("socket_tcp_bind: tcp_socket");
		return 0;
	}

	// if internal ip exists, p2p socket uses internal ip, if not use public ip
	//if ((p2p_socket = socket_tcp_bind(*g_szInternalIP ? g_szInternalIP : g_szPublicIP, p2p_port)) == INVALID_SOCKET)
	if ((p2p_socket = socket_tcp_bind(g_szPublicIP, p2p_port)) == INVALID_SOCKET)
	{
		sys_err("socket_tcp_bind: p2p_socket");
		perror("socket_tcp_bind: p2p_socket");
		return 0;
	}

	fdwatch_add_fd(main_fdw, tcp_socket, nullptr, FDW_READ, false);
	fdwatch_add_fd(main_fdw, p2p_socket, nullptr, FDW_READ, false);

	db_clientdesc = DESC_MANAGER::Instance().CreateConnectionDesc(main_fdw, db_addr, db_port, PHASE_DBCLIENT, true);
	if (!g_bAuthServer) 
	{
		db_clientdesc->UpdateChannelStatus(0, true);
	}

	if (g_bAuthServer)
	{
		if (g_stAuthMasterIP.length() != 0)
		{
			fprintf(stderr, "SlaveAuth\n");
			g_pkAuthMasterDesc = DESC_MANAGER::Instance().CreateConnectionDesc(main_fdw, g_stAuthMasterIP.c_str(), g_wAuthMasterPort, PHASE_P2P, true); 
			P2P_MANAGER::Instance().RegisterConnector(g_pkAuthMasterDesc);
			g_pkAuthMasterDesc->SetP2P(g_stAuthMasterIP.c_str(), g_wAuthMasterPort, g_bChannel);

		}
		else
		{
			fprintf(stderr, "MasterAuth\n");
		}
	}

	signal_timer_enable(30);
	return 1;
}

int32_t main(int32_t argc, char **argv)
{
	if (setup_minidump_generator() == false)
		return 1;

#ifdef USE_STACKTRACE
	#ifdef _WIN32
		SymInitialize(GetCurrentProcess(), nullptr, TRUE);
	#endif
#endif

#ifdef DEBUG_ALLOC
	DebugAllocator::StaticSetUp();
#endif

	ilInit(); // DevIL Initialize

	SECTREE_MANAGER	sectree_manager;
	CHARACTER_MANAGER	char_manager;
	ITEM_MANAGER	item_manager;
	CShopManager	shop_manager;
	CMobManager		mob_manager;
	CMotionManager	motion_manager;
	CPartyManager	party_manager;
	CSkillManager	skill_manager;
	CPVPManager		pvp_manager;
	LZOManager		lzo_manager;
	DBManager		db_manager;
	AccountDB 		account_db;

	LogManager		log_manager;
	MessengerManager	messenger_manager;
	P2P_MANAGER		p2p_manager;
	CGuildManager	guild_manager;
	CGuildMarkManager mark_manager;
	CDungeonManager	dungeon_manager;
	CRefineManager	refine_manager;
	CBanwordManager	banword_manager;
	CPrivManager	priv_manager;
	CWarMapManager	war_map_manager;
	building::CManager	building_manager;
	CTargetManager	target_manager;
	marriage::CManager	marriage_manager;
	marriage::WeddingManager wedding_manager;
	CItemAddonManager	item_addon_manager;
	CArenaManager arena_manager;
	COXEventManager OXEvent_manager;
	CNearbyScanner	nearby_scanner;
	DESC_MANAGER	desc_manager;
	CTableBySkill SkillPowerByLevel;
	CPolymorphUtils polymorph_utils;
	CProfiler		profiler;
	SpamManager		spam_mgr;
	CThreeWayWar	threeway_war;
	DSManager dsManager;
	CBattlegroundManager bgManager;

	if (!start()) 
	{
		CleanUpForEarlyExit();
		return 0;
	}

	quest::CQuestManager quest_manager;

	if (!quest_manager.Initialize())
	{
		CleanUpForEarlyExit();
		return 0;
	}

	CAnticheatManager ACManager;
	MessengerManager::Instance().Initialize();
	CGuildManager::Instance().Initialize();
	fishing::Initialize();
	OXEvent_manager.Initialize();
	bgManager.Initialize();

	Cube_init();
	Blend_Item_init();
	ani_init();

	if (!g_bAuthServer)
	{
		if (!ACManager.Initialize())
		{
			fprintf(stderr, "Failed To Initialize AC");
			CleanUpForEarlyExit();
			return 0;
		}
	}
	/*
	_set_abort_behavior(0, _CALL_REPORTFAULT | _WRITE_ABORT_MSG);

	signal(SIGTERM, &InterruptHandler);
	signal(SIGABRT, &InterruptHandler);
	signal(SIGSEGV, &InterruptHandler);

	std::set_terminate(&TerminateHandler);
	*/

	while (idle());

	sys_log(0, "<shutdown> Starting...");
	g_bShutdown = true;
	g_bNoMoreClient = true;

	if (g_bAuthServer)
	{
		int32_t iLimit = DBManager::Instance().CountQuery() / 50;
		int32_t i = 0;

		do
		{
			uint32_t dwCount = DBManager::Instance().CountQuery();
			sys_log(0, "Queries %u", dwCount);

			if (dwCount == 0)
				break;

			usleep(500000);

			if (++i >= iLimit)
				if (dwCount == DBManager::Instance().CountQuery())
					break;
		} while (1);
	}

	sys_log(0, "<shutdown> Destroying CArenaManager...");
	arena_manager.Destroy();
	sys_log(0, "<shutdown> Destroying COXEventManager...");
	OXEvent_manager.Destroy();

	sys_log(0, "<shutdown> Disabling signal timer...");
	signal_timer_disable();

	sys_log(0, "<shutdown> Shutting down CHARACTER_MANAGER...");
	char_manager.GracefulShutdown();
	sys_log(0, "<shutdown> Shutting down ITEM_MANAGER...");
	item_manager.GracefulShutdown();

	sys_log(0, "<shutdown> Flushing db_clientdesc...");
	db_clientdesc->FlushOutput();
	sys_log(0, "<shutdown> Flushing p2p_manager...");
	p2p_manager.FlushOutput();

	sys_log(0, "<shutdown> Destroying CShopManager...");
	shop_manager.Destroy();
	sys_log(0, "<shutdown> Destroying CHARACTER_MANAGER...");
	char_manager.Destroy();
	sys_log(0, "<shutdown> Destroying ITEM_MANAGER...");
	item_manager.Destroy();
	sys_log(0, "<shutdown> Destroying DESC_MANAGER...");
	desc_manager.Destroy();
	sys_log(0, "<shutdown> Destroying quest::CQuestManager...");
	quest_manager.Destroy();
	sys_log(0, "<shutdown> Destroying building::CManager...");
	building_manager.Destroy();
	sys_log(0, "<shutdown> Destroying BATTLEGROUND_MANAGER...");
	bgManager.Destroy();

	if (!g_bAuthServer)
	{
		sys_log(0, "<shutdown> Releasing Anticheat manager...");
		ACManager.Release();
	}
	
	destroy();

#ifdef DEBUG_ALLOC
	DebugAllocator::StaticTearDown();
#endif

	return 1;
}

void destroy()
{
	sys_log(0, "<shutdown> Canceling ReloadSpamEvent...");
	CancelReloadSpamEvent();

	sys_log(0, "<shutdown> regen_free()...");
	regen_free();

	sys_log(0, "<shutdown> Closing sockets...");
	socket_close(tcp_socket);
	socket_close(p2p_socket);

	sys_log(0, "<shutdown> fdwatch_delete()...");
	fdwatch_delete(main_fdw);

	sys_log(0, "<shutdown> event_destroy()...");
	event_destroy();

	sys_log(0, "<shutdown> CTextFileLoader::DestroySystem()...");
	CTextFileLoader::DestroySystem();

	sys_log(0, "<shutdown> thecore_destroy()...");
	thecore_destroy();
}

int32_t idle()
{
	static struct timeval	pta = { 0, 0 };
	static int32_t			process_time_count = 0;
	struct timeval		now;

	if (pta.tv_sec == 0)
		gettimeofday(&pta, (struct timezone *) 0);

	int32_t passed_pulses;

	if (!(passed_pulses = thecore_idle()))
		return 0;

	assert(passed_pulses > 0);

	uint32_t t;

	while (passed_pulses--) 
	{
		heartbeat(thecore_heart, ++thecore_heart->pulse);

		// To reduce the possibility of abort() in checkpointing
		thecore_tick();
	}

	t = get_dword_time();
	CHARACTER_MANAGER::Instance().Update(thecore_heart->pulse);
	db_clientdesc->Update(t);
	s_dwProfiler[PROF_CHR_UPDATE] += (get_dword_time() - t);

	t = get_dword_time();
	if (!io_loop(main_fdw)) 
		return 0;
	s_dwProfiler[PROF_IO] += (get_dword_time() - t);

	log_rotate();

	gettimeofday(&now, (struct timezone *) nullptr);
	++process_time_count;

	if (now.tv_sec - pta.tv_sec > 0)
	{
		num_events_called = 0;
		current_bytes_written = 0;

		process_time_count = 0;
		gettimeofday(&pta, (struct timezone *) nullptr);

		memset(&s_dwProfiler[0], 0, sizeof(s_dwProfiler));
	}

#ifdef _WIN32
	if (_kbhit()) 
	{
		int32_t c = _getch();
		switch (c) 
		{
			case VK_ESCAPE:
				return 0; // shutdown
				break;
			default:
				break;
		}
	}
#endif

	return 1;
}

int32_t io_loop(LPFDWATCH fdw)
{
	LPDESC	d;
	int32_t		num_events, event_idx;

	DESC_MANAGER::Instance().DestroyClosed(); // PHASE_CLOSE인 접속들을 끊어준다.
	DESC_MANAGER::Instance().TryConnect();

	if ((num_events = fdwatch(fdw, nullptr)) < 0)
		return 0;

	for (event_idx = 0; event_idx < num_events; ++event_idx)
	{
		d = (LPDESC) fdwatch_get_client_data(fdw, event_idx);

		if (!d)
		{
			if (FDW_READ == fdwatch_check_event(fdw, tcp_socket, event_idx))
			{
				DESC_MANAGER::Instance().AcceptDesc(fdw, tcp_socket);
				fdwatch_clear_event(fdw, tcp_socket, event_idx);
			}
			else if (FDW_READ == fdwatch_check_event(fdw, p2p_socket, event_idx))
			{
				DESC_MANAGER::Instance().AcceptP2PDesc(fdw, p2p_socket);
				fdwatch_clear_event(fdw, p2p_socket, event_idx);
			}
			continue; 
		}

		int32_t iRet = fdwatch_check_event(fdw, d->GetSocket(), event_idx);

		switch (iRet)
		{
			case FDW_READ:
				if (db_clientdesc == d)
				{
					int32_t size = d->ProcessInput();

					if (size)
						sys_log(1, "DB_BYTES_READ: %d", size);

					if (size < 0)
					{
						d->SetCloseReason("WRONG_READ");
						d->SetPhase(PHASE_CLOSE);
					}
				}
				else if (d->ProcessInput() < 0)
				{
					d->SetCloseReason("INVALID_INPUT");
					d->SetPhase(PHASE_CLOSE);
				}
				break;

			case FDW_WRITE:
				if (db_clientdesc == d)
				{
					int32_t buf_size = buffer_size(d->GetOutputBuffer());
					int32_t sock_buf_size = fdwatch_get_buffer_size(fdw, d->GetSocket());

					int32_t ret = d->ProcessOutput();

					if (ret < 0)
					{
						d->SetCloseReason("WRONG_WRITE");
						d->SetPhase(PHASE_CLOSE);
					}

					if (buf_size)
						sys_log(1, "DB_BYTES_WRITE: size %d sock_buf %d ret %d", buf_size, sock_buf_size, ret);
				}
				else if (d->ProcessOutput() < 0)
				{
					d->SetCloseReason("INVALID_OUT");
					d->SetPhase(PHASE_CLOSE);
				}
				break;

			case FDW_EOF:
				{
					d->SetCloseReason("FDW_END");
					d->SetPhase(PHASE_CLOSE);
				}
				break;

			default:
				sys_err("fdwatch_check_event returned unknown %d", iRet);
				d->SetPhase(PHASE_CLOSE);
				break;
		}
	}

	return 1;
}

