# Meeting

> 27/05

## CHERIO-RTOS scheduler and timeout

### Thread priority

- Static thread priority

> Can be dynamically adapted by priority boosting mechanism

![alt](scheduler_priority.svg)

### Round-Robin

- For threads with the same priority
    - Timer freq = CPU freq = 40 MHz (20ns)
    - Tickrate is variable (100 by default)
    - Cycles per tick = CPU_FREQ / TICKRATE (400 000 cycles -> 10ms by default)

![alt](scheduler_rr.svg)

### Thread sleep 

- Sleep is relative, using ticks level precision

> Currently being rework to allow absolute time concept

![alt](scheduler_sleep_ticks.svg)

- When target is time or cycles, we need to round somehow
    - Nearest:
        - Average = 0 (absolute average = 1/4 tick)
        - Th. Max = +1/2 tick
    - Round up:
        - Average = +1/2 tick
        - Th. Max = +1 tick
    - Round down:
        - Average = -1/2 tick
        - Th. Max = -1 tick

![alt](scheduler_sleep_cycles.svg)

## Existing Benchmarks

### Compartment call

A bunch of different calls from one compartment to another, with variable stack size

- no-op cross-calls: ~260 cycles, not depending on stack size
- cross call dereference: ~356 cycles
- cross call derefecence: ~430 cycles
- cross call using a stack: depend on stack in callee
- same compartment: 7 cycles
- ...

> A bit too much infos here, but everything seems deterministic here

### Interrupt-latency

A bunch of high prio threads waiting on a low prio one.

> Same compartment. Seems to be bugged here currently. Might raise an issue on github.

### Interrupt-latency-ext

A single thread waiting on an external event (timer) for wakeup, measuring the time between the last even and the actual wakeup

- Result: between 950 and 1150 cycles

> Pretty deterministic.

## My benchmarks

### Simple compartment switch

Measuring the compartment switch time (calling a function from another compartment) for different stack size

- Result: 270 cycles

> Coherent with above results

### UART test

Testing the UART duration depending on the msg size

- results:
    - 4 bytes:  1756 cycles
    - 8 bytes:  3492 cycles
    - 16 bytes: 6964 cycles
    - 32 bytes: 13908 cycles

-> ~ 430 cycle/byte

> Very deterministic

### Cyclictest

A single sleep-and-measure thread and a few stressor threads

#### Ticks modes

- Nearest:
    - Abs. Average = 1/4 tick
    - Th. Max = +1/2 tick
    > Interval > 1 tick
- Round up:
    - Average = +1/2 tick
    - Th. Max = +1 tick
- Round down:
    - Average = -1/2 tick
    - Th. Max = -1 tick

![alt](cyclictest_ticks_mode.svg)

- **Tickrate = 1000 => 1 tick = 1ms**

| Test | 1 Tick (cycles) | Interval (cycles) | Abs. Mean (cycles) | Max (cycles) | Warning |
|------|--------------|-------------------|-------------------|--------------|---------|
| 1 | 40 000 | **100 000** | 10 054 | 21 725 | — |
| 2 | 40 000 | **60 000** | 10 048 | 21 793 | — |
| 2 | 40 000 | **50 000** | 10 052 | 21 599 | — |
| 3 | 40 000 | **40 000** | 10 105 | 21 483 | — |
| 4 | 40 000 | **30 000** | 10 093 | 21 559 | — |
| 5 | 40 000 | **20 000** | 10 402 | 21 709 | ⚠️ *3.46% frame skipped* |
| 6 | 40 000 | **10 000** | 11 341 | 21 225 | ⚠️ *33.4% frame skipped* |



### Availability

- Higher priority thread that does not yield

> Indeed, not progress on the lower priority threads

- Same prio

> Progress is divided (with a slight overhead, that will probably increase with tickrate)

- Low prio thread with interrupt disabled

> Interrupt disabled not working currently...


## Sumary

- Sleep resolution depend on tickrate (higher the better)
    - Will probably be modified soon
    - Does cycles/subtick sleep would help?
- Progress division overhead depend on tickrate (lower the better)
- No progress garantee with higher priority thread
- Interrupt disable section are not limited in duration

> What is needed to ensure the schedulability?