
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

  LINK_LIBRARIES
    OpenCL
    test_utils
)
