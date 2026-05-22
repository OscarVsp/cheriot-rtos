#include <cstdint>
#include <interrupt.h>
#include <futex.h>
#include <platform-rdcycle.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread.h>
#include <timeout.h>


static volatile uint32_t stressor_event = 0;
static volatile uint32_t stressor_ready = 0;  // bit 0 = service ready
static volatile bool stressor_stop = false;


static void stressor_workload()
{
    uint32_t *buf = static_cast<uint32_t *>(malloc(2048));
    if (!buf)
    {
        return;
    }

    uint32_t seed = stressor_event;
    for (int i = 0; i < 512; ++i)
    {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        buf[i] = seed;
    }

    for (int pass = 0; pass < 10; ++pass)
    {
        for (int i = 0; i < 512; ++i)
        {
            uint32_t val = buf[i];
            val = (val * 73) ^ (val >> 7);
            val = (val * 97) ^ (val >> 5);
            val = val / 7;
            val = val * (val % 13 + 1);
            buf[i] = val;
        }
    }

    free(buf);

}

int __cheri_compartment("stressors") entry_stressor()
{

    uint32_t progress = 0;
    
    __atomic_fetch_or(&stressor_ready, 0x1, __ATOMIC_RELEASE);
    futex_wake(&stressor_ready, 1);

    while (!__atomic_load_n(&stressor_stop, __ATOMIC_ACQUIRE))
    {
        stressor_workload();
        progress++;
        yield();
    }

    //printf("\tStressor duration:\t\t%d cycles\n", static_cast<int>(thread_elapsed_cycles_current()));

    return 0;
}

void __cheri_compartment("stressors") wait_stressors_ready()
{
    while ((__atomic_load_n(&stressor_ready, __ATOMIC_ACQUIRE) & 0x1) != 0x1)
    {
        futex_wait(&stressor_ready, __atomic_load_n(&stressor_ready, __ATOMIC_ACQUIRE));
    }
}

void __cheri_compartment("stressors") stop_stressors()
{
    __atomic_store_n(&stressor_stop, true, __ATOMIC_RELEASE);
}
