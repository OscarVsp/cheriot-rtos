-- Copyright Microsoft and CHERIoT Contributors.
-- SPDX-License-Identifier: MIT

set_project("CHERIoT uart-test benchmark");
sdkdir = "../../sdk"
includes(sdkdir)
set_toolchains("cheriot-clang")

-- Support libraries
includes(path.join(sdkdir, "lib/freestanding"),
         path.join(sdkdir, "lib/atomic"),
         path.join(sdkdir, "lib/crt"))

option("board")
    set_default("sonata")

debugOption("uart-test");
compartment("uart-test")
    add_deps("crt", "freestanding", "stdio", "debug")
    add_defines("BOARD=" .. tostring(get_config("board")))
    add_files("uart-test.cc")

firmware("uart-test-benchmark")
    add_deps("uart-test")
    on_load(function(target)
        local threads = {
            {
                compartment = "uart-test",
                priority = 2,
                entry_point = "entry_waiter",
                stack_size = 0x1000,
                trusted_stack_frames = 4
            },
        }
        target:values_set("threads", threads, {expand = false})
    end)
