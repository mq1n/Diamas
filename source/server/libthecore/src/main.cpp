#define __LIBTHECORE__
#include "stdafx.h"
#include "memory.h"
#include "srandomdev.h"
#include "signal.h"

LPHEART		thecore_heart = nullptr;

volatile int	shutdowned = 0;
volatile int	tics = 0;

// newstuff
int	bCheckpointCheck = 1;

static int pid_init(void)
{
#ifdef _WIN32
	return true;
#else
	auto fp = msl::file_ptr("pid", "w");
	if (fp)
	{
		fprintf(fp.get(), "%d", getpid());
		sys_log(0, "\nStart of pid: %d\n", getpid());
	}
	else
	{
		printf("pid_init(): could not open file for writing. (filename: ./pid)");
		sys_err("\nError writing pid file\n");
		return false;
	}
	return true;
#endif
}

static void pid_deinit(void)
{
#ifdef _WIN32
    return;
#else
    remove("./pid");
	sys_log(0, "\nEnd of pid\n");
#endif
}

bool thecore_init()
{
#ifdef _WIN32
    srand(static_cast<unsigned int>(time(nullptr)));
#else
    srandom(time(nullptr) + getpid() + getuid());
    srandomdev();
#endif
#ifdef __FreeBSD__
    signal_setup();
#endif

	if (!log_init() || !pid_init())
		return false;

	return true;
}

bool thecore_set(int fps, HEARTFUNC heartbeat_func)
{
	thecore_heart = heart_new(1000000 / fps, heartbeat_func);
	return true;
}

void thecore_shutdown()
{
    shutdowned = 1;
}

int thecore_idle(void)
{
    thecore_tick();

    if (shutdowned)
		return 0;

	int pulses = heart_idle(thecore_heart);

    return pulses;
}

void thecore_destroy(void)
{
	delete thecore_heart;
	pid_deinit();
	log_destroy();
}

int thecore_pulse(void)
{
	return (thecore_heart->pulse);
}

float thecore_pulse_per_second(void)
{
	return ((float) thecore_heart->passes_per_sec);
}

float thecore_time(void)
{
	return ((float) thecore_heart->pulse / (float) thecore_heart->passes_per_sec);
}

int thecore_is_shutdowned(void)
{
	return shutdowned;
}

void thecore_tick(void)
{
	++tics;
}
