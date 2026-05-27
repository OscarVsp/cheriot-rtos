#include "../timing.h"
#include "tick_macros.h"
#include <compartment.h>
#include <cstdint>
#include <debug.hh>
#include <stdio.h>
#include <futex.h>
#include <thread.h>
#include <timeout.h>

using Debug = ConditionalDebug<true, "uart-test">;

static constexpr uint32_t MAX_ITERATION = 10000;

int __cheri_compartment("uart-test") entry_waiter()
{
	printf("Starting uart-test\n");

	printf("CPU_TIMER_HZ: %d Hz\n", static_cast<int>(CPU_TIMER_HZ));
	printf("TICK_RATE_HZ: %d Hz (%d ms)\n",
	           static_cast<int>(TICK_RATE_HZ),
	           static_cast<int>(MS_PER_TICK));
	printf("TIMERCYCLES_PER_TICK: %d cycles\n",
	           static_cast<int>(TIMERCYCLES_PER_TICK));

	uint32_t beforeCycles, afterCycle, diffCycle, meanDiff;
	uint64_t sumDiff = 0;

	printf("\n");
	printf("-------------------------------\n");
	printf("\n");

	printf("TESTING UART DURATION\n");
	printf("\n");

	const char *text = "testtesttesttesttesttesttesttest";

	for (uint32_t i = 0; i < MAX_ITERATION; i++)
	{
// In sail, the rdcycle function uses the mintret register instead of mcycle, so
// we uses rdcycle64 instead
#ifndef SIMULATION
		beforeCycles = rdcycle();
#else
		beforeCycles = rdcycle64();
#endif
		printf("%s", text);

// Same as before
#ifndef SIMULATION
		afterCycle = rdcycle();
#else
		afterCycle = rdcycle64();
#endif

		diffCycle = static_cast<int32_t>(static_cast<int64_t>(afterCycle) -
		                                 static_cast<int64_t>(beforeCycles));

		
		printf("[%d] latency: %d %d %d\n",
		           static_cast<int>(i),
		           static_cast<int>(beforeCycles),
		           static_cast<int>(afterCycle),
		           static_cast<int>(diffCycle));
		

		sumDiff += diffCycle;
	}

	meanDiff = static_cast<int32_t>(sumDiff / MAX_ITERATION);

	printf("%d bytes latency average: %d cycles\n",
	           static_cast<int>(strlen(text)),
	           static_cast<int>(meanDiff));

	printf("\n");
	printf("-------------------------------\n");
	return 0;
}