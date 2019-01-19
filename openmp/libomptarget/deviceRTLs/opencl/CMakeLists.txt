# INTEL_COLLAB
##===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.txt for details.
#
##===----------------------------------------------------------------------===##
#
# Build the OpenCL device RTL if the OpenCL SDK is available
#
##===----------------------------------------------------------------------===##

if(LIBOMPTARGET_DEP_OPENCL_FOUND)
  libomptarget_say("Building OpenCL offloading device RTL.")

  # For now, there is only one source file.
  # FIXME: SPIRV tool (spirv-link) is required to build RTL with multiple sources.
  set(opencl_inc src/omptarget-opencl.h)
  set(opencl_src src/omptarget-opencl.cl)
  get_filename_component(opencl_src_abs ${opencl_src} ABSOLUTE)

  set(opencl_device_rtl ${LIBOMPTARGET_LIBRARY_DIR}/libomptarget-opencl.a)

  set(opencl_flags
    -cc1 -emit-spirv -triple spir64-unknown-unknown -cl-std=CL2.0
    -include opencl-c.h -D INTEL_COLLAB=1 -o ${opencl_device_rtl}
  )

  add_custom_target(omptarget-opencl ALL DEPENDS ${opencl_device_rtl})

  # Use the build tree's clang which can emit SPIRV.
  add_custom_command(
    OUTPUT ${opencl_device_rtl}
    COMMAND clang ${opencl_flags} ${opencl_src_abs}
    DEPENDS ${opencl_src} ${opencl_inc} clang
    VERBATIM
  )

  install(FILES ${opencl_device_rtl}
    COMPONENT omptarget-opencl-rtl
    DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX}
  )
else()
  libomptarget_say("Not building OpenCL offloading device RTL: OpenCL SDK not found")
endif()
# end INTEL_COLLAB
