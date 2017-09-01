cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(OPENMP_LLVM_TOOLS_DIR "" CACHE PATH "Path to LLVM tools for testing")

# INTEL - do not add runtime if Intel specific runtime is being used.
if (NOT INTEL_SPECIFIC_OPENMP)
  add_subdirectory(runtime)
endif()

# Currently libomptarget cannot be compiled on Windows or MacOS X.
# Since the device plugins are only supported on Linux anyway,
# there is no point in trying to compile libomptarget on other OSes.
if (NOT (WIN32 OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
  add_subdirectory(libomptarget)
endif()
