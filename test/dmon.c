#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#if defined(_WIN32)
#include <windows.h>
#endif
#define DMON_IMPL
#include <dmon.h>

static volatile sig_atomic_t keep_running = 1;
static volatile sig_atomic_t event_count;
static int expected_events;

static void stop_monitor(int signum)
{
    (void)signum;
    keep_running = 0;
}

static void watch_callback(dmon_watch_id watch_id, dmon_action action, const char* rootdir,
                           const char* filepath, const char* oldfilepath, void* user)
{
    (void)(user);
    (void)(watch_id);
	(void)(oldfilepath);
	event_count++;

    switch (action) {
    case DMON_ACTION_CREATE:
        fprintf(stderr,"CREATE: [%s]%s\n", rootdir, filepath);
        break;
    case DMON_ACTION_DELETE:
        fprintf(stderr,"DELETE: [%s]%s\n", rootdir, filepath);
        break;
    case DMON_ACTION_MODIFY:
        fprintf(stderr,"MODIFY: [%s]%s\n", rootdir, filepath);
        break;
    case DMON_ACTION_MOVE:
        fprintf(stderr,"MOVE: [%s]%s -> [%s]%s\n", rootdir, oldfilepath, rootdir, filepath);
        break;
    }
}

int main(int argc, char* argv[])
{
	const char *directory = NULL;
	if (argc > 1 && strncmp(argv[1], "--count=", 8) == 0) {
		expected_events = atoi(argv[1] + 8);
		if (argc > 2) directory = argv[2];
	} else if (argc > 1) {
		directory = argv[1];
	}
    if (directory) {
#if !defined(_WIN32)
        signal(SIGHUP, stop_monitor);
#endif
        signal(SIGINT, stop_monitor);
        signal(SIGTERM, stop_monitor);
        dmon_init();
		puts("READY");
		fflush(stdout);
        dmon_watch(directory, watch_callback, DMON_WATCHFLAGS_RECURSIVE, NULL);
        while (keep_running && (!expected_events || event_count < expected_events)) {
#if defined(_WIN32)
			Sleep(10);
#else
			struct timespec delay = { 0, 10000000 };
			nanosleep(&delay, NULL);
#endif
        }
        dmon_deinit();
    } else {
        puts("usage: test dirname");
    }
    return 0;
}
