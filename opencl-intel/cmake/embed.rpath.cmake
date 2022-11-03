#
# INTEL CONFIDENTIAL
#
# Copyright (C) 2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express or
# implied warranties, other than those that are expressly stated in the License.
#

if(NOT WIN32)
  # Linker switches
  set(ADD_RPATH_LINKER_FLAGS "-Wl,--enable-new-dtags")

  # embed RPATH and RUNPATH to the binaries that assumes that everything is
  # installed in the same directory
  #
  # Description: RPATH is used to locate dynamically load shared
  # libraries/objects (DLLs) for the non-standard OS locations without need of
  # relinking DLLs during installation. The algorithm is the following:
  #
  # 1. If RPATH is present in the EXE/DLL and RUNPATH is NOT present, search
  #    through it.
  # 2. If LD_LIBRARY_PATH env variable is present, search through it
  # 3. If RUNPATH is present in the EXE/DLL, search through it
  # 4. Search through locations, configured by system admin and cached in
  #    /etc/ld.so.cache
  # 5. Search through /lib and /usr/lib
  #
  # RUNPATH influences only the immediate dependencies, while RPATH influences
  # the whole subtree of dependencies RPATH is concidered deprecated in favor of
  # RUNPATH, but RUNPATH does not supported by some Linux systems. If RUNPATH is
  # not supported, system loader may report error - remove
  # "-Wl,--enable-new-dtags" above to disable RUNPATH generation.
  #
  # If RPATH or RUNPATH contains string $ORIGIN it is substituted by the full
  # path to the containing EXE/DLL. Security issue 1: if EXE/DLL is marked as
  # set-uid or set-gid, $ORIGIN is ignored. Security issue 2: if RPATH/RUNPATH
  # references relative subdirs, intruder may fool it by using softlinks
  #
  if(BUILD_FOR_COMPILATION_PURPOSES_ONLY)
    set(CMAKE_SKIP_BUILD_RPATH FALSE) # add pointers to the build tree, so it
                                      # can be used during compilation
  else()
    set(CMAKE_SKIP_BUILD_RPATH TRUE) # do not add pointers to the build tree
  endif(BUILD_FOR_COMPILATION_PURPOSES_ONLY)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) # build rpath as if already installed
  set(CMAKE_INSTALL_RPATH
      "$ORIGIN::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
  )# the rpath to use - search through installation dir only
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE) # do not use static link paths as
                                               # rpath

  # Linker switches - DLL
  set(CMAKE_SHARED_LINKER_FLAGS
      "${CMAKE_SHARED_LINKER_FLAGS}         ${ADD_RPATH_LINKER_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG
      "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}   ${ADD_RPATH_LINKER_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE
      "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} ${ADD_RPATH_LINKER_FLAGS}")
endif(NOT WIN32)
