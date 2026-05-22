#include "../timing.h"
#include "const.h"
#include "riscvreg.h"
#include "utils.hh"
#include <compartment.h>
#include <cstdint>
#include <futex.h>
#include <stdio.h>
#include <thread.h>
#include <timeout.h>

uint32_t __cheri_compartment("callee") callee_entry();


int __cheri_compartment("caller") caller_entry()
{
	auto readCycles = []() -> uint64_t {
#ifndef SIMULATION
    return rdcycle();
#else
    return rdcycle64();
#endif
};

	printf("\n-------------------------------------------\n");
	printf("|     STARTING COMPARTMENT SWITCH TEST     |\n");
	printf("-------------------------------------------\n\n");


	uint32_t before, after;
	uint32_t jitter;
	uint32_t minJitter           	= INT32_MAX;
	uint32_t maxJitter           	= 0;
	uint64_t sumJitter           	= 0;

	
	/*Start the loop*/
	printf("start\n");

	for (uint32_t i = 0; i < MAX_ITERATION; i++)
	{
		/*Compute the remaining ticks to sleep*/

		before = readCycles();

		after = callee_entry();

		jitter    = static_cast<int32_t>(static_cast<int64_t>(after) -
		                                 static_cast<int64_t>(before));

		if (jitter < minJitter)
		{
			minJitter = jitter;
		}
		if (jitter > maxJitter)
		{
			maxJitter = jitter;
		}
		sumJitter += jitter;

		printf("%d:%d\n", static_cast<int>(i), static_cast<int>(jitter));

	}


	int32_t meanJitter = static_cast<int32_t>(sumJitter / MAX_ITERATION);

	printf("end\n");


	printf("\n------------------------------\n\n");

	printf("RESULTS\n\n");
	printf("\tmin:\t\t\t\t%d cycles\t(%d us)\n",
	       static_cast<int>(minJitter),
	       static_cast<int>(us_from_cycles(minJitter)));
	printf("\tmax:\t\t\t\t%d cycles\t(%d us)\n",
	       static_cast<int>(maxJitter),
	       static_cast<int>(us_from_cycles(maxJitter)));
	printf("\tmean abs:\t\t\t%d cycles\t(%d us)\n",
	       static_cast<int>(meanJitter),
	       static_cast<int>(us_from_cycles(meanJitter)));

	return 0;
}