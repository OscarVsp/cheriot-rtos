//Interval mode
#define INTERVAL_US         0
#define INTERVAL_MS         1
#define INTERVAL_CYCLES     2
#ifndef INTERVAL_MODE
#   define INTERVAL_MODE    INTERVAL_US
#endif
#ifndef INTERVAL
#   define INTERVAL         1000
#endif

//Sleep ticks mode
#define NEAREST_TICK        0
#define UPPER_TICK          1
#define LOWER_TICK          2
#ifndef SLEEP_TICKS_MODE
#   define SLEEP_TICKS_MODE NEAREST_TICK
#endif

//Max iteration
#ifndef MAX_ITERATION
#	define MAX_ITERATION    100
#endif