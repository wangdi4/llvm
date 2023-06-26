# Copyright (C) 2023 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express or
# implied warranties, other than those that are expressly stated in the License.

include(CheckCSourceCompiles)
include(CheckIncludeFile)
include(CheckSymbolExists)
include(CheckTypeSize)
include(FetchContent)

#set repo
if(INTEL_CUSTOMIZATION)
  if(DEFINED ENV{ICS_GIT_MIRROR} AND NOT "$ENV{ICS_GIT_MIRROR}" STREQUAL "")
    STRING(REGEX REPLACE "\\\\" "/" GITSERVER "$ENV{ICS_GIT_MIRROR}")
	set(HWLOC_REPO "${GITSERVER}/hwloc.git")
  else()
    message(FATAL_ERROR "NO HWLOC MIRROR IN ICS_GIT_MIRROR!")
  endif()
else()
  set(HWLOC_REPO "https://github.com/open-mpi/hwloc.git")
endif(INTEL_CUSTOMIZATION)

#set hwloc version
set(HWLOC_TAG hwloc-2.9.0)
set(hwloc_patch git checkout -- . && git apply ${CMAKE_CURRENT_SOURCE_DIR}/hwloc_refine.patch)

message(STATUS "Fetch HWLOC project from ${HWLOC_REPO}")

#set args for download hwloc project
FetchContent_Declare(hwloc
  GIT_REPOSITORY      ${HWLOC_REPO}
  GIT_TAG             ${HWLOC_TAG}
  GIT_SHALLOW         TRUE
  PATCH_COMMAND       ${hwloc_patch}
  UPDATE_DISCONNECTED 1
 )

 FetchContent_GetProperties(hwloc)
 if(NOT hwloc_POPULATED)
  # Fetch the content using previously declared details
  FetchContent_Populate(hwloc)

  #check x86_32 or x86_64
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(SIZEOF_VOID_P 4)
    set(HWLOC_X86_32_ARCH 1)
  else()
    set(SIZEOF_VOID_P 8)
    set(HWLOC_X86_64_ARCH 1)
  endif()

  if(WIN32)
    set(HWLOC_WIN_SYS 1)
  else(WIN32)
    set(HWLOC_LINUX_SYS 1)
    set(HWLOC_HAVE_FFS 1)
    set(HWLOC_HAVE_FFSL 1)
    set(HWLOC_HAVE_DECL_FFS 1)
    set(HWLOC_HAVE_DECL_FFSL 1)
  endif(WIN32)

  check_include_file("dirent.h" HAVE_DIRENT_H)
  check_include_file("pthread.h" HWLOC_HAVE_PTHREAD_MUTEX)
  check_include_file("sched.h" HWLOC_HAVE_CPU_SET)
  check_include_file("strings.h" HAVE_STRINGS_H)
  check_include_file("stdint.h" HWLOC_HAVE_STDINT_H)
  check_include_file("sys/mman.h" HAVE_SYS_MMAN_H)
  check_include_file("sys/param.h" HAVE_SYS_PARAM_H)
  check_include_file("sys/utsname.h" HAVE_SYS_UTSNAME_H)
  check_include_file("unistd.h" HAVE_UNISTD_H)
  check_include_file("windows.h" HWLOC_HAVE_WINDOWS_H)

  check_type_size("unsigned int" HWLOC_SIZEOF_UNSIGNED_INT 
    BUILTIN_TYPES_ONLY LANGUAGE C)

  check_type_size("unsigned long" HWLOC_SIZEOF_UNSIGNED_LONG 
    BUILTIN_TYPES_ONLY LANGUAGE C)

  check_symbol_exists(_SC_PAGESIZE "unistd.h" HAVE_DECL__SC_PAGESIZE)
  check_symbol_exists(_SC_PAGE_SIZE "unistd.h" HAVE_DECL__SC_PAGE_SIZE)
  check_symbol_exists(memalign "malloc.h" HAVE_MEMALIGN)
  check_symbol_exists(strncasecmp "strings.h" HWLOC_HAVE_DECL_STRNCASECMP)
  
  check_c_source_compiles(
    "int main(void) {int cpuinfo[4]; __cpuidex(cpuinfo,0,0); return 0;}"
    HWLOC_HAVE_MSVC_CPUIDEX)

  if(HWLOC_HAVE_CPU_SET)
    set(HWLOC_HAVE_CPU_SET_S 1)
  endif()

  if(HWLOC_HAVE_WINDOWS_H)
    set(HAVE_DECL_GETMODULEFILENAME 1)
  endif()

  if(HAVE_SYS_UTSNAME_H)
    set(HAVE_UNAME 1)
    set(HAVE_USELOCALE 1)
  endif()
  
  if(MSVC AND HWLOC_HAVE_DECL_STRNCASECMP)
      set(hwloc_strncasecmp_fcn strncasecmp)
  else()
      set(hwloc_strncasecmp_fcn strncmp)
  endif()
  
  #generate the private_config.h and hwloc_config.h
  configure_file(hwloc_config.h.in 
  ${hwloc_SOURCE_DIR}/include/hwloc/autogen/config.h)
  
  configure_file(private_config.h.in 
  ${hwloc_SOURCE_DIR}/include/private/autogen/config.h)

  #copy CMakeLists.txt and static-components.h to repo
  configure_file(CMakeLists.txt.in ${hwloc_SOURCE_DIR}/CMakeLists.txt COPYONLY)
  configure_file(static-components.h.in 
  ${hwloc_SOURCE_DIR}/hwloc/static-components.h COPYONLY)

  #which is needed in hwloc project
  add_definitions(-DRUNSTATEDIR="${hwloc_SOURCE_DIR}/var/run")

  #set hwloc include path
  set(HWLOC_INCLUDE_DIR ${hwloc_SOURCE_DIR}/include CACHE STRING "")

  #Bring the populated content into the build
  add_subdirectory(${hwloc_SOURCE_DIR} ${hwloc_BINARY_DIR})
 endif()
