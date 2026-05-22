-- Copyright Microsoft and CHERIoT Contributors.
-- SPDX-License-Identifier: MIT

set_project("CHERIoT compartement-switch-latency benchmark");
sdkdir = "../../sdk"
includes(sdkdir)
set_toolchains("cheriot-clang")

-- Support libraries
includes(path.join(sdkdir, "lib/freestanding"),
         path.join(sdkdir, "lib/atomic"),
         path.join(sdkdir, "lib/crt"))

option("board")
    set_default("sail")

option("iterations")
    set_default(100)
    set_description("Maximum number of iterations for the measurement")

option("trace")
    set_default(false)
    set_description("Enable full logs during compartement-switch-latency measurement")

debugOption("compartement-switch-latency");
compartment("caller")
    add_deps("crt", "freestanding", "stdio", "debug")
    add_defines("BOARD=" .. tostring(get_config("board")))
    add_defines("MAX_ITERATION=" .. tostring(get_config("iterations")))
    add_files("caller.cc")

compartment("callee")
    add_deps("crt", "freestanding", "stdio", "debug")
    add_files("callee.cc")

firmware("compartement-switch-latency-benchmark")
    add_deps("caller")
    add_deps("callee")
    on_load(function(target)
        local threads = {
            {
                compartment = "caller",
                priority = 1,
                entry_point = "caller_entry",
                stack_size = 0x200,
                trusted_stack_frames = 4
            },
        }

        target:values_set("threads", threads, {expand = false})
    end)
