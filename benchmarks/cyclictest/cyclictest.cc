#include "../timing.h"
#include "const.h"
#include "riscvreg.h"
#include "tick_macros.h"
#include "utils.hh"
#include <compartment.h>
#include <cstdint>
#include <futex.h>
#include <stdio.h>
#include <thread.h>
#include <timeout.h>

#ifdef STRESSORS_ENABLED
void __cheri_compartment("stressors") wait_stressors_ready();
void __cheri_compartment("stressors") stop_stressors();
#endif

int __cheri_compartment("cyclictest") entry_waiter()
{
	auto readCycles = []() -> uint64_t {
#ifndef SIMULATION
		return rdcycle();
#else
		return rdcycle64();
#endif
	};
	printf("\n------------------------------\n");
	printf("|     STARTING CYCLICTEST     |\n");
	printf("------------------------------\n\n");

	/*Print parameters from system config*/
	printf("SYSTEM INFO\n\n");
	printf("\tCPU_TIMER_HZ:\t\t\t%d Hz\n", static_cast<int>(CPU_TIMER_HZ));
	printf("\tTICK_RATE_HZ:\t\t\t%d Hz\n", static_cast<int>(TICK_RATE_HZ));
	printf("\tTIMERCYCLES_PER_TICK:\t\t%d cycles\t(%d us)\n",
	       static_cast<int>(TIMERCYCLES_PER_TICK),
	       static_cast<int>(us_from_cycles(TIMERCYCLES_PER_TICK)));
	printf("\n------------------------------\n\n");

	/*Print parameters from cyclictest config*/
	printf("TEST PARAMETERS\n\n");
#if INTERVAL_MODE == INTERVAL_US
	const uint32_t IntervalCycles = cycles_from_us(INTERVAL);
	printf("\tInterval:\t\t\t%d cycles\t(%d us)\n",
	       static_cast<int>(IntervalCycles),
	       static_cast<int>(INTERVAL));
#elif INTERVAL_MODE == INTERVAL_MS
	const uint32_t IntervalCycles = cycles_from_ms(INTERVAL);
	printf("\tInterval:\t\t\t%d ms\t(%d cycles\t(%d ms)\n",
	       static_cast<int>(IntervalCycles),
	       static_cast<int>(INTERVAL));
#elif INTERVAL_MODE == INTERVAL_CYCLES
	const uint32_t IntervalCycles = INTERVAL;
	printf("\tInterval:\t\t\t%d cycles\t(%d us)\n",
	       static_cast<int>(IntervalCycles),
	       static_cast<int>(us_from_cycles(INTERVAL)));
#else
	printf("ERROR: INTERVAL_MODE is not correctly defined");
	return -1;
#endif
		
	printf("\tIterations:\t\t\t%d\n", static_cast<int>(MAX_ITERATION));
#if SLEEP_TICKS_MODE == 0
	printf("\tTicks compute mode:\t\tNEAREST\n");
#elif SLEEP_TICKS_MODE == 1
	printf("\tTicks compute mode:\t\tUPPER\n");
#elif SLEEP_TICKS_MODE == 2
	printf("\tTicks compute mode:\t\tLOWER\n");
#else
	printf("ERROR: SLEEP_TICKS_MODE is not correctly defined");
	return -1;	
#endif

#ifdef STRESSORS_ENABLED
	printf("\tStressors:\t\t\tENABLED\n");
#else
	printf("\tStressors:\t\t\tDISABLED\n");
#endif
	printf("\n------------------------------\n\n");

	/*Start the loop*/

#ifdef VERBOSE_ENABLED
	printf("ITERATION\tSTART\t\tEXPECTED\tREMAINING\t\tACTUAL\t\tJITTER\n\n");
#else
#	ifdef OUTPUT_ENABLED
	printf("start\n");
#	endif
#endif

#ifdef STRESSORS_ENABLED
	wait_stressors_ready();
#endif

	uint32_t actualCycles, absJitter;
	int32_t  jitter;
	uint32_t  minAbsJitter           = UINT32_MAX;
	uint32_t  maxAbsJitter           = 0;
	uint64_t  sumAbsJitter           = 0;
	uint32_t frameSkippedForCatchup = 0;

	uint32_t nowCycles = readCycles();

	uint32_t nextWakeCycle = nowCycles + IntervalCycles;

	uint32_t remainingCycles =
		  (nowCycles < nextWakeCycle) ? (nextWakeCycle - nowCycles) : 0;

#if SLEEP_TICKS_MODE == NEAREST_TICK
	uint32_t remainingTicks =
		  (remainingCycles + TIMERCYCLES_PER_TICK / 2) / TIMERCYCLES_PER_TICK;
#elif SLEEP_TICKS_MODE == UPPER_TICK
	uint32_t remainingTicks =
		  (remainingCycles + TIMERCYCLES_PER_TICK - 1) / TIMERCYCLES_PER_TICK;
#elif SLEEP_TICKS_MODE == LOWER_TICK
	uint32_t remainingTicks = remainingCycles / TIMERCYCLES_PER_TICK;
#endif

	for (uint32_t i = 0; i < MAX_ITERATION; i++)
	{
		

		Timeout sleepTime{0, remainingTicks};
		thread_sleep(&sleepTime, ThreadSleepNoEarlyWake);

		actualCycles = readCycles();

		jitter    = static_cast<int32_t>(static_cast<int64_t>(actualCycles) -
		                                 static_cast<int64_t>(nextWakeCycle));
		absJitter = (jitter < 0) ? -jitter : jitter;

		if (absJitter < minAbsJitter)
		{
			minAbsJitter = absJitter;
		}
		if (absJitter > maxAbsJitter)
		{
			maxAbsJitter = absJitter;
		}
		sumAbsJitter += absJitter;

#ifdef VERBOSE_ENABLED
		printf("%d\t\t%d\t\t%d\t\t%d\t(%d ticks)\t%d\t\t%d\t(%d us)\n",
		       static_cast<int>(i),
		       static_cast<int>(nowCycles),
		       static_cast<int>(nextWakeCycle),
		       static_cast<int>(remainingCycles),
		       static_cast<int>(remainingTicks),
		       static_cast<int>(actualCycles),
		       static_cast<int>(jitter),
		       static_cast<int>(us_from_cycles(absJitter)));
#else

#	ifdef OUTPUT_ENABLED
		printf("%d:%d\n", static_cast<int>(i), static_cast<int>(jitter));
#	endif

#endif
		nextWakeCycle += IntervalCycles;

		//Skip next frame if it is already in the past
		while (nextWakeCycle <= readCycles())
		{
			nextWakeCycle += IntervalCycles;
			frameSkippedForCatchup++;
		}

		/*Compute the remaining ticks to sleep*/
		nowCycles = readCycles();

		remainingCycles =
		  (nowCycles < nextWakeCycle) ? (nextWakeCycle - nowCycles) : 0;

#if SLEEP_TICKS_MODE == NEAREST_TICK
		remainingTicks =
		  (remainingCycles + TIMERCYCLES_PER_TICK / 2) / TIMERCYCLES_PER_TICK;
#elif SLEEP_TICKS_MODE == UPPER_TICK
		remainingTicks =
		  (remainingCycles + TIMERCYCLES_PER_TICK - 1) / TIMERCYCLES_PER_TICK;
#elif SLEEP_TICKS_MODE == LOWER_TICK
		remainingTicks = remainingCycles / TIMERCYCLES_PER_TICK;
#endif
	}


	int32_t meanAbsJitter = static_cast<int32_t>(sumAbsJitter / MAX_ITERATION);
#ifdef OUTPUT_ENABLED
	printf("end\n");
	printf("\n------------------------------\n\n");
#endif

	if (frameSkippedForCatchup > 0)
	{
		printf("WARNING: %d frame(s) had to be skiped due to nextWakeCycle "
		       "being in the past\n\n",
		       static_cast<int>(frameSkippedForCatchup));
	}
	printf("RESULTS\n\n");
	printf("\tmin:\t\t\t\t%d cycles\t(%d us)\n",
	       static_cast<int>(minAbsJitter),
	       static_cast<int>(us_from_cycles(minAbsJitter)));
	printf("\tmax:\t\t\t\t%d cycles\t(%d us)\n",
	       static_cast<int>(maxAbsJitter),
	       static_cast<int>(us_from_cycles(maxAbsJitter)));
	printf("\tmean abs:\t\t\t%d cycles\t(%d us)\n",
	       static_cast<int>(meanAbsJitter),
	       static_cast<int>(us_from_cycles(meanAbsJitter)));
	// printf("\tCyclictest duration:\t\t%d cycles\n",
	// static_cast<int>(thread_elapsed_cycles_current()));

#ifdef STRESSORS_ENABLED
	stop_stressors();
#endif

	return 0;
}