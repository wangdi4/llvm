##===----------------------------------------------------------------------===##
#
#                      The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.txt for details.
# 
##===----------------------------------------------------------------------===##
#
# Plugin for Nios(R) II
#
##===----------------------------------------------------------------------===##

# TODO: Should be removed once ics configuration for offload compiler is implemented.
if (NOT LIBOMPTARGET_DEP_NIOS2_SDK)
  set(LIBOMPTARGET_DEP_NIOS2_SDK /rdrive/ref/nios2/17.0)
endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64)$" AND CMAKE_SYSTEM_NAME MATCHES "Linux")
  if (LIBOMPTARGET_DEP_NIOS2_SDK)
    libomptarget_say("Building Nios2 offloading plugin.")

    # MSOF is a part of Nios2 SDK
    set(MSOF ${LIBOMPTARGET_DEP_NIOS2_SDK}/nios2offload/host)

    # Plugin library
    add_library(omptarget.rtl.nios2 SHARED src/rtl.cpp)

    target_include_directories(omptarget.rtl.nios2 PUBLIC
      ${MSOF}/include
    )
    target_link_libraries(omptarget.rtl.nios2
      "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports"
      -L${MSOF}/lib
      msof
    )

    # Install plugin under the lib destination folder.
    install(TARGETS omptarget.rtl.nios2
      LIBRARY DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})

    # TODO: Temporary copy MSOF to compiler libraries.
    set(src_file ${MSOF}/lib/libmsof.so)
    set(dst_file ${LIBOMPTARGET_LIBRARY_DIR}/libmsof.so)

    add_custom_target(omptarget.rtl.nios2.msof ALL DEPENDS ${dst_file})

    add_custom_command(
      OUTPUT ${dst_file}
      COMMAND ${CMAKE_COMMAND} -E copy ${src_file} ${dst_file}
      DEPENDS ${src_file}
      VERBATIM
    )
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${dst_file})

    install(FILES ${dst_file} DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})

    set(LIBOMPTARGET_SYSTEM_TARGETS "${LIBOMPTARGET_SYSTEM_TARGETS} nios2-intel-elfnios2" PARENT_SCOPE)
  else(LIBOMPTARGET_DEP_NIOS2_SDK)
    libomptarget_say("Not building Nios2 offloading plugin: Nios2 SDK not found.")
  endif(LIBOMPTARGET_DEP_NIOS2_SDK)
else()
  libomptarget_say("Not building Nios2 offloading plugin: supported on Linux x86_64 hosts only.")
endif()

