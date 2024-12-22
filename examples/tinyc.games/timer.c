#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

// You must X-define TIMER_NAMES before including this file, e.g:
//
// #define TIMER_NAMES X(create_hmap), X(update_world), X(update_player),
//

#ifndef TIMER_NAMES
#define TIMER_NAMES
#endif

#define TIMER(name) {                                                          \
        unsigned long long now = SDL_GetPerformanceCounter();                  \
        if (!timer_then) timer_then = now;                                     \
        timer_times[ timer_curr_id ] += now - timer_then;                      \
        timer_curr_id = timer_ ## name;                                        \
        timer_then = now;                                                      \
}

// FIXME: push id onto stack if re-entering
#define TIMECALL(f, args) {                                                    \
        unsigned long long now = SDL_GetPerformanceCounter();                  \
        if (!timer_then) timer_then = now;                                     \
        timer_times[ timer_curr_id ] += now - timer_then;                      \
        timer_then = now;                                                      \
        (f)args;                                                               \
        now = SDL_GetPerformanceCounter();                                     \
        timer_times[ timer_ ## f ] += now - timer_then;                        \
        timer_curr_id = timer_;                                                \
        timer_then = now;                                                      \
}

enum timernames {
        #define X(x) timer_ ## x
        TIMER_NAMES
        #undef X
        timer_
};

char timernamesprint[][80] = {
        #define X(x) #x
        TIMER_NAMES
        #undef X
        "uncounted"
};

int timer_curr_id = timer_;
unsigned long long timer_then = 0;
unsigned long long timer_times[timer_ + 1] = { 0 };

void timer_print(char *buf, size_t n, bool show_all)
{
        char *p = buf;
        int i = 0;
        unsigned long long sum = 0;
        unsigned long long freq = SDL_GetPerformanceFrequency();

        for (i = 0; i <= timer_; i++)
                sum += timer_times[i];

        for (i = 0; i <= timer_; i++)
        {
                float secs = (float)timer_times[i] / (float)freq;
                float pct = 100.f * (float)timer_times[i] / sum;
                if ((show_all && secs > 0.f) || pct >= 0.1f || secs >= 0.01f)
                        p += snprintf(p, n - (p-buf),
                                "%6.1f  %2.0f%%  %s\n", secs, pct, timernamesprint[i]);
        }
}
