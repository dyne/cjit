#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "threads.h"
#include <stdatomic.h>

#define NR_THREADS 16
#define NR_STEPS 10000

#define BUG_ON(COND) \
    do { \
        if (!!(COND)) \
            abort(); \
    } while (0)

typedef struct {
    atomic_flag flag;
    atomic_uchar uc;
    atomic_ushort us;
    atomic_uint ui;
    atomic_size_t ul;
} counter_type;

static
void *adder_simple(void *arg)
{
    size_t step;
    counter_type *counter = arg;

    for (step = 0; step < NR_STEPS; ++step) {
        atomic_fetch_add_explicit(&counter->uc, 1, memory_order_relaxed);
        atomic_fetch_add_explicit(&counter->us, 1, memory_order_relaxed);
        atomic_fetch_add_explicit(&counter->ui, 1, memory_order_relaxed);
        atomic_fetch_add_explicit(&counter->ul, 1, memory_order_relaxed);
    }

    return NULL;
}

static
void *adder_cmpxchg(void *arg)
{
    size_t step;
    counter_type *counter = arg;

    for (step = 0; step < NR_STEPS; ++step) {
        unsigned char xchgc;
        unsigned short xchgs;
        unsigned int xchgi;
        size_t xchgl;
        unsigned char cmpc = atomic_load_explicit(&counter->uc, memory_order_relaxed);
        unsigned short cmps = atomic_load_explicit(&counter->us, memory_order_relaxed);
        unsigned int cmpi = atomic_load_explicit(&counter->ui, memory_order_relaxed);
        size_t cmpl = atomic_load_explicit(&counter->ul, memory_order_relaxed);

        do {
            xchgc = (cmpc + 1);
        } while (!atomic_compare_exchange_strong_explicit(&counter->uc,
            &cmpc, xchgc, memory_order_relaxed, memory_order_relaxed));
        do {
            xchgs = (cmps + 1);
        } while (!atomic_compare_exchange_strong_explicit(&counter->us,
            &cmps, xchgs, memory_order_relaxed, memory_order_relaxed));
        do {
            xchgi = (cmpi + 1);
        } while (!atomic_compare_exchange_strong_explicit(&counter->ui,
            &cmpi, xchgi, memory_order_relaxed, memory_order_relaxed));
        do {
            xchgl = (cmpl + 1);
        } while (!atomic_compare_exchange_strong_explicit(&counter->ul,
            &cmpl, xchgl, memory_order_relaxed, memory_order_relaxed));
    }

    return NULL;
}

static
void *adder_test_and_set(void *arg)
{
    size_t step;
    counter_type *counter = arg;

    for (step = 0; step < NR_STEPS; ++step) {
        while (atomic_flag_test_and_set(&counter->flag));
        ++counter->uc;
        ++counter->us;
        ++counter->ui;
        ++counter->ul;
        atomic_flag_clear(&counter->flag);
    }

    return NULL;
}

static
void atomic_counter_test(void *(*adder)(void *arg))
{
    size_t index;
    counter_type counter;
    pthread_t thread[NR_THREADS];

    atomic_flag_clear(&counter.flag);
    atomic_init(&counter.uc, 0);
    atomic_init(&counter.us, 0);
    atomic_init(&counter.ui, 0);
    atomic_init(&counter.ul, 0);

    for (index = 0; index < NR_THREADS; ++index)
        BUG_ON(pthread_create(&thread[index], NULL, adder, (void *)&counter));

    for (index = 0; index < NR_THREADS; ++index)
        BUG_ON(pthread_join(thread[index], NULL));

    if (atomic_load(&counter.uc) == ((NR_THREADS * NR_STEPS) & 0xffu)
        && atomic_load(&counter.us) == ((NR_THREADS * NR_STEPS) & 0xffffu)
        && atomic_load(&counter.ui) == (NR_THREADS * NR_STEPS)
        && atomic_load(&counter.ul) == (NR_THREADS * NR_STEPS)
        )
        printf("SUCCESS\n");
    else
        printf("FAILURE\n");
}

int main(void)
{
    atomic_counter_test(adder_simple);
    atomic_counter_test(adder_cmpxchg);
    atomic_counter_test(adder_test_and_set);

    return 0;
}
