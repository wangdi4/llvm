##===----------------------------------------------------------------------===##
#
#                      The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.txt for details.
# 
##===----------------------------------------------------------------------===##
#
# Build a plugin for Intel(R) Xeon Phi(TM) on top of COI.
#
##===----------------------------------------------------------------------===##

if(NOT LIBOMPTARGET_DEP_MPSS_SDK)
  # Note: this default MPSS path should be replaced with /opt/mpss/x200/minsdk
  # before upstreaming to LLORG.
  if (IS_DIRECTORY /rdrive/ref/lrb/mpss/x200/linux/minsdk)
    set(LIBOMPTARGET_DEP_MPSS_SDK /rdrive/ref/lrb/mpss/x200/linux/minsdk)
  endif()
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64)$" AND CMAKE_SYSTEM_NAME MATCHES "Linux")
  if(LIBOMPTARGET_DEP_MPSS_SDK)
    libomptarget_say("Building Intel(R) Xeon Phi(TM) offloading plugin.")

    # Target main
    add_executable(target_main src/target_main.cpp)

    target_include_directories(target_main PUBLIC
      ${LIBOMPTARGET_DEP_MPSS_SDK}/include
    )
    target_compile_definitions(target_main PUBLIC
      COI_LIBRARY_VERSION=2
    )
    target_link_libraries(target_main
      -rdynamic
      -Wl,--enable-new-dtags
      -L${LIBOMPTARGET_DEP_MPSS_SDK}/lib
      coi_device
    )

    set(TARGET_MAIN_DUMP ${CMAKE_CURRENT_BINARY_DIR}/target_main.dump)

    # Create a hex dump of target_main binary
    add_custom_command(
      OUTPUT ${TARGET_MAIN_DUMP}
      COMMAND xxd -i < $<TARGET_FILE:target_main> > ${TARGET_MAIN_DUMP}
      DEPENDS target_main
      COMMENT "Building Target Binary Dump target_main.dump"
      VERBATIM
    )
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
      ${TARGET_MAIN_DUMP})

    # plugin library
    add_library(omptarget.rtl.x86_64_mic SHARED src/rtl.cpp)

    # target_main.dump is included by rtl.cpp
    set_source_files_properties(src/rtl.cpp PROPERTIES OBJECT_DEPENDS
      ${TARGET_MAIN_DUMP})

    target_include_directories(omptarget.rtl.x86_64_mic PUBLIC
      ${LIBOMPTARGET_DEP_MPSS_SDK}/include
    )
    target_compile_definitions(omptarget.rtl.x86_64_mic PUBLIC
      COI_LIBRARY_VERSION=2
    )
    target_link_libraries(omptarget.rtl.x86_64_mic 
      "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports"
      dl
      pthread
    )
    
    # Install plugin under the lib destination folder.
    install(TARGETS omptarget.rtl.x86_64_mic LIBRARY
      DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})

    # Report to the parent scope that we are building a plugin
    # for Intel(R) Xeon Phi(TM).
    set(LIBOMPTARGET_SYSTEM_TARGETS "${LIBOMPTARGET_SYSTEM_TARGETS} x86_64-intel-linux-gnu" PARENT_SCOPE)
  else(LIBOMPTARGET_DEP_MPSS_SDK)
    libomptarget_say("Not building Intel(R) Xeon Phi(TM) offloading plugin: MPSS SDK not found.")
  endif(LIBOMPTARGET_DEP_MPSS_SDK)
else()
  libomptarget_say("Not building Intel(R) Xeon Phi(TM) offloading plugin: supported on Linux x86_64 hosts only.")
endif()

