-- Copyright Microsoft and CHERIoT Contributors.
-- SPDX-License-Identifier: MIT

set_project("CHERIoT cyclictest benchmark");
sdkdir = "../../sdk"
includes(sdkdir)
set_toolchains("cheriot-clang")

-- Support libraries
includes(path.join(sdkdir, "lib/freestanding"),
         path.join(sdkdir, "lib/atomic"),
         path.join(sdkdir, "lib/crt"))

option("board")
    set_default("sail")

option("stressors")
    set_default(false)
    set_description("Enable stressor threads for wakeup latency benchmark")

option("interval-mode")
    set_default(0)
    set_description("Measurement interval mode")

option("interval")
    set_default(1000)
    set_description("Measurement interval in milliseconds")

option("ticks-mode")
    set_default(0)
    set_description("Measurement interval in cycles")

option("iterations")
    set_default(100)
    set_description("Maximum number of iterations for the measurement")

option("trace")
    set_default(false)
    set_description("Enable full logs during cyclictest measurement")

option("output")
    set_default(false)
    set_description("Enable logs during cyclictest measurement")

debugOption("cyclictest");
compartment("cyclictest")
    add_deps("crt", "freestanding", "stdio", "debug")
    add_defines("BOARD=" .. tostring(get_config("board")))
    add_defines("INTERVAL_MODE=" .. tostring(get_config("interval-mode")))
    add_defines("INTERVAL=" .. tostring(get_config("interval")))
    add_defines("SLEEP_TICKS_MODE=" .. tostring(get_config("ticks-mode")))
    add_defines("MAX_ITERATION=" .. tostring(get_config("iterations")))
    if get_config("stressors") then
        add_defines("STRESSORS_ENABLED")
    end
    if get_config("trace") then
        add_defines("VERBOSE_ENABLED")
    end
    if get_config("output") then
        add_defines("OUTPUT_ENABLED")
    end
    add_files("cyclictest.cc")

compartment("stressors")
    add_deps("crt", "freestanding", "stdio", "debug")
    add_files("stressors.cc")

firmware("cyclictest-benchmark")
    add_deps("cyclictest")
    add_deps("stressors")
    on_load(function(target)
        local threads = {
            {
                compartment = "cyclictest",
                priority = 1,
                entry_point = "entry_waiter",
                stack_size = 0x1000,
                trusted_stack_frames = 4
            },
        }
        if get_config("stressors") then
            table.insert(threads, {
                compartment = "stressors",
                priority = 1,
                entry_point = "entry_stressor",
                stack_size = 0x800,
                trusted_stack_frames = 4
            })
        end

        target:values_set("threads", threads, {expand = false})
    end)
