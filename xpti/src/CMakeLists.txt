# INTEL_CUSTOMIZATION
#
# INTEL CONFIDENTIAL
#
# Modifications, Copyright (C) 2021 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may not
# use, modify, copy, publish, distribute, disclose or transmit this software or
# the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.
#
# end INTEL_CUSTOMIZATION
include(GNUInstallDirs)

macro(add_xpti_lib target_name)
  add_library(${target_name} STATIC ${ARGN})
  target_compile_definitions(${target_name} PRIVATE -DXPTI_STATIC_LIBRARY)
  target_include_directories(${target_name} PRIVATE ${XPTI_DIR}/include)

  if (MSVC)
    target_compile_options(${target_name} PRIVATE /EHsc)
  endif()

  # Set the location of the library installation
  install(TARGETS ${target_name}
    RUNTIME DESTINATION bin COMPONENT xpti
    LIBRARY DESTINATION lib${LLVM_LIBDIR_SUFFIX} COMPONENT xpti
    ARCHIVE DESTINATION lib${LLVM_LIBDIR_SUFFIX} COMPONENT xpti
  )
endmacro()

set(SOURCES
  xpti_proxy.cpp
)

add_xpti_lib(xpti ${SOURCES})

# Handle the debug version for the Microsoft compiler as a special case by
# creating a debug version of the static library that uses the flags used by
# the SYCL runtime
if (MSVC)
  add_xpti_lib(xptid STATIC ${SOURCES})
# INTEL_CUSTOMIZATION
  target_compile_options(xptid PRIVATE ${XPTI_CXX_FLAGS_DEBUG})
  target_compile_options(xpti PRIVATE ${XPTI_CXX_FLAGS_RELEASE})
else()
  intel_clear_stdlib()
# end INTEL_CUSTOMIZATION
endif()

