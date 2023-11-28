# INTEL_COPYRIGHT_BEGIN
# Copyright (c) 2019, Intel Corporation.  All rights reserved.
# INTEL_COPYRIGHT_END

set(SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_driver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_device.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_context.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_cmdqueue.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_cmdlist.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_buffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_kernel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_module.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_event.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cpu_kernel_launch.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/cm_printf_host.cpp
  )

set (API_LOADER_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_core_loader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_tools_loader.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_sysman_loader.cpp
)

set(EMPTY_API_SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/ze_api_empty.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/zes_api_empty.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/zet_api_empty.cpp
)

add_library(ze_intel_cpu SHARED ze_api.cpp ${API_LOADER_SRC} ${EMPTY_API_SRC} ${SRC})
set_target_properties(ze_intel_cpu PROPERTIES FOLDER ze_intel_cpu)

include_directories(${CMAKE_SOURCE_DIR}/)

install(TARGETS ze_intel_cpu DESTINATION lib)

