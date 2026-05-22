#include <cstdint>
#include <interrupt.h>
#include <futex.h>
#include <platform-rdcycle.h>
#include <thread.h>
#include <timeout.h>


uint32_t __cheri_compartment("callee") callee_entry()
{
    auto readCycles = []() -> uint32_t {
#ifndef SIMULATION
    return rdcycle();
#else
    return rdcycle64();
#endif
};

    return readCycles();
}
