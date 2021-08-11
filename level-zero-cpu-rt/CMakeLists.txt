# INTEL_COPYRIGHT_BEGIN
# Copyright (c) 2021, Intel Corporation.  All rights reserved.
# INTEL_COPYRIGHT_END

cmake_minimum_required(VERSION 3.14)

include(FetchContent)

# Options. Turn on with 'cmake -Dtest=ON'.
option(test "Build all tests." OFF) # Makes boolean 'test' available.

set (PROJECT_NAME CPULevel0)

project(${PROJECT_NAME})

set( CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE )

set (CMAKE_CXX_STANDARD 17)

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-error=cast-qual")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-strict-aliasing")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    link_libraries(stdc++fs)
  endif()
else()
  # the folowing to "fix build warning
  # source: https://stackoverflow.com/questions/14172856/compile-with-mt-instead-of-md-using-cmake
  set(CompilerFlags
     CMAKE_CXX_FLAGS
     CMAKE_CXX_FLAGS_DEBUG
     CMAKE_CXX_FLAGS_RELEASE
     CMAKE_C_FLAGS
     CMAKE_C_FLAGS_DEBUG
     CMAKE_C_FLAGS_RELEASE
     )
  foreach(CompilerFlag ${CompilerFlags})
    string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
  endforeach()

  # now set compilation flags
  set(CMAKE_CXX_FLAGS           "${CMAKE_CXX_FLAGS} /Od /Zi /WX")
  set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS}")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()

# Step 1: Fetch level 0 headers from github and add it to include path.

set(LEVEL_ZERO_HEADER_REPO "https://github.com/oneapi-src/level-zero.git")
set(LEVEL_ZERO_HEADER_TAG "v1.4.1")
message(STATUS "Will fetch Level0 headers from ${LEVEL_ZERO_HEADER_REPO}")

FetchContent_Declare(level-zero
  GIT_REPOSITORY    ${LEVEL_ZERO_HEADER_REPO}
  GIT_TAG           ${LEVEL_ZERO_HEADER_TAG}
  SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/level_zero
)

FetchContent_GetProperties(level-zero)

if(NOT level-zero_POPULATED)
  FetchContent_Populate(level-zero)
endif()

set(level_zero_header
    ${level-zero_SOURCE_DIR} CACHE PATH "Path to Level zero Headers")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# step 2: execute extract_empty_func.py to generate dummy API implementation.
find_package(PythonInterp 3 REQUIRED)
execute_process(COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/extract_empty_func.py
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Step 3: Build ze_intel_cpu library
add_subdirectory(src)

# Step 3: Run Test. (optional) 
if (test)
  include(CTest)
  enable_testing()

  add_subdirectory(unit_tests)
endif()

#set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Step 4: Deploy library.
