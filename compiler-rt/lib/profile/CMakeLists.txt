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

CHECK_CXX_SOURCE_COMPILES("
#ifdef _WIN32
#include <intrin.h> /* Workaround for PR19898. */
#include <windows.h>
#endif
int main() {
#ifdef _WIN32
        volatile LONG val = 1;
        MemoryBarrier();
        InterlockedCompareExchange(&val, 0, 1);
        InterlockedIncrement(&val);
        InterlockedDecrement(&val);
#else
        volatile unsigned long val = 1;
        __sync_synchronize();
        __sync_val_compare_and_swap(&val, 1, 0);
        __sync_add_and_fetch(&val, 1);
        __sync_sub_and_fetch(&val, 1);
#endif
        return 0;
      }
" COMPILER_RT_TARGET_HAS_ATOMICS)

CHECK_CXX_SOURCE_COMPILES("
#if defined(__linux__)
#include <unistd.h>
#endif
#include <fcntl.h>
int fd;
int main() {
 struct flock s_flock;

 s_flock.l_type = F_WRLCK;
 fcntl(fd, F_SETLKW, &s_flock);
 return 0;
}

" COMPILER_RT_TARGET_HAS_FCNTL_LCK)

CHECK_CXX_SOURCE_COMPILES("
#include <sys/utsname.h>
int main() {
 return 0;
}

" COMPILER_RT_TARGET_HAS_UNAME)

add_compiler_rt_component(profile)

set(PROFILE_SOURCES
  GCDAProfiling.c
  InstrProfiling.c
  InstrProfilingInternal.c
  InstrProfilingValue.c
  InstrProfilingBuffer.c
  InstrProfilingFile.c
  InstrProfilingMerge.c
  InstrProfilingMergeFile.c
  InstrProfilingNameVar.c
  InstrProfilingVersionVar.c
  InstrProfilingWriter.c
  InstrProfilingPlatformDarwin.c
  InstrProfilingPlatformFuchsia.c
  InstrProfilingPlatformLinux.c
  InstrProfilingPlatformOther.c
  InstrProfilingPlatformWindows.c
  InstrProfilingRuntime.cpp
  InstrProfilingUtil.c
  )

set(PROFILE_HEADERS
  InstrProfiling.h
  InstrProfilingInternal.h
  InstrProfilingPort.h
  InstrProfilingUtil.h
  WindowsMMap.h
  )

if(WIN32)
  list(APPEND PROFILE_SOURCES
    WindowsMMap.c
  )
endif()

include_directories(..)
include_directories(../../include)

if(FUCHSIA OR UNIX)
 set(EXTRA_FLAGS
     -fPIC
     -Wno-pedantic)
endif()

if(COMPILER_RT_TARGET_HAS_ATOMICS)
 set(EXTRA_FLAGS
     ${EXTRA_FLAGS}
     -DCOMPILER_RT_HAS_ATOMICS=1)
endif()

if(COMPILER_RT_TARGET_HAS_FCNTL_LCK)
 set(EXTRA_FLAGS
     ${EXTRA_FLAGS}
     -DCOMPILER_RT_HAS_FCNTL_LCK=1)
endif()

if(COMPILER_RT_TARGET_HAS_UNAME)
 set(EXTRA_FLAGS
     ${EXTRA_FLAGS}
     -DCOMPILER_RT_HAS_UNAME=1)
endif()

# INTEL_CUSTOMIZATION
if(COMPILER_RT_CCOV_BUILD)
 set(EXTRA_FLAGS
     ${EXTRA_FLAGS}
     ${CCOV_BUILD_FLAGS})
endif()
# end INTEL_CUSTOMIZATION

# We don't use the C++ Standard Library here, so avoid including it by mistake.
append_list_if(COMPILER_RT_HAS_NOSTDINCXX_FLAG -nostdinc++ EXTRA_FLAGS)
# XRay uses C++ standard library headers.
string(REGEX REPLACE "-stdlib=[a-zA-Z+]*" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# This appears to be a C-only warning banning the use of locals in aggregate
# initializers. All other compilers accept this, though.
# nonstandard extension used : 'identifier' : cannot be initialized using address of automatic variable
append_list_if(COMPILER_RT_HAS_WD4221_FLAG /wd4221 EXTRA_FLAGS)

# Disable 'nonstandard extension used: translation unit is empty'.
append_list_if(COMPILER_RT_HAS_WD4206_FLAG /wd4206 EXTRA_FLAGS)

if(APPLE)
  add_compiler_rt_runtime(clang_rt.profile
    STATIC
    OS ${PROFILE_SUPPORTED_OS}
    ARCHS ${PROFILE_SUPPORTED_ARCH}
    CFLAGS ${EXTRA_FLAGS}
    SOURCES ${PROFILE_SOURCES}
    ADDITIONAL_HEADERS ${PROFILE_HEADERS}
    PARENT_TARGET profile)
else()
  add_compiler_rt_runtime(clang_rt.profile
    STATIC
    ARCHS ${PROFILE_SUPPORTED_ARCH}
    CFLAGS ${EXTRA_FLAGS}
    SOURCES ${PROFILE_SOURCES}
    ADDITIONAL_HEADERS ${PROFILE_HEADERS}
    PARENT_TARGET profile)
endif()
