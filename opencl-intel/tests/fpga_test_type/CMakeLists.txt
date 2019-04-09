
include_directories(include)

add_ocl_unittest(
  fpga_test_type

  SOURCE_FILES
    main.cpp
    pretty_printers.cpp

    device_info.cpp
    test_pipes.cpp
    channel_by_value.cpp
    wg_ordering.cpp
    infinite_loops.cpp
    autorun.cpp
    channel_multiple_call_site.cpp
    host_side_pipes.cpp
    autorun_profiling.cpp
    io_attribute.cpp
    channel_depth_emulation.cpp
    local_mem_size_attribute.cpp
    uses_global_work_offset_attribute.cpp
    tbb_num_workers.cpp
    multiple_devices.cpp
    fpga_features.cpp

  LINK_LIBRARIES
    OpenCL
    test_utils
)
