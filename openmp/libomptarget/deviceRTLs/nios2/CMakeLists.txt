##===----------------------------------------------------------------------===##
# 
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.txt for details.
# 
##===----------------------------------------------------------------------===##
#
# Build the NiosII device runtime libraries - data_block, libomp
#
##===----------------------------------------------------------------------===##


##----------------------------------------------------------------------------##
# Macro for building a static library for the NiosII target
##----------------------------------------------------------------------------##

# variant - the name of library variant; separate library binary is created for
# each variant, which is a set of hardware features like supported instructions
# lib_name - static library binary name
macro(build_device_lib variant lib_name)
  # Destination directory for the library
  set(dst_dir ${LIBOMPTARGET_LIBRARY_DIR}/${variant})
  set(dst_lib ${dst_dir}/${lib_name})

  # CFLAGS for building library
  set(flags -I${DISPATCHER}/include -O2 ${ARGN})

  # Compile files
  set(obj_files "")
  foreach (src_file ${src_files})
    get_filename_component(src ${src_file} ABSOLUTE)
    get_filename_component(name ${src_file} NAME)

    set(obj ${variant}/${name}.o)

    add_custom_command(OUTPUT ${obj}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${variant}
      COMMAND ${NIOS2_GCC} ${flags} -c ${src} -o ${obj}
      DEPENDS ${src}
      IMPLICIT_DEPENDS C ${src}
      VERBATIM
    )
    set_property(DIRECTORY APPEND PROPERTY
      ADDITIONAL_MAKE_CLEAN_FILES ${obj})

    list(APPEND obj_files ${obj})
  endforeach()

  # Build archive
  add_custom_command(OUTPUT ${dst_lib}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${dst_dir}
    COMMAND ${NIOS2_AR} cr ${dst_lib} ${obj_files}
    DEPENDS ${obj_files}
    VERBATIM
  )
  set_property(DIRECTORY APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES ${dst_lib})

  # Add target for building the library
  string(REPLACE "/" "-" target_name ${lib_name}-${variant})
  add_custom_target(${target_name} ALL DEPENDS ${dst_lib})

  # Install device library under the destination folder.
  install(FILES ${dst_lib} DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX}/${variant})
endmacro()

##----------------------------------------------------------------------------##
# Macros for building all necessary library variants. The list of variants that
# are built here is similar to newlib variants included into Nios2 EDS (except
# for profile which seems redundant for offload runtime). Combinations of the
# following features are used for building library variants:
#   march-r2                 Nios II R2 ISA (otherwise it is Nios II R1 ISA)
#   mbmx                     Nios II R2 BMX instructions
#   mcdx                     Nios II R2 CDX instructions
#   mhw-mul                  Enable MUL instructions
#   mhw-mulx                 Enable MULX instructions
#   fstack-limit-register-et Stack overflow limit register checking
#   mno-hw-mul               No HW Multiplication
##----------------------------------------------------------------------------##
macro(build_device_libs1 variant lib_name)
  build_device_lib(${variant} ${lib_name} ${ARGN})
  #build_device_lib(${variant}/pg ${ARGN} -fprofile)
endmacro()

macro(build_device_libs2 variant)
  build_device_libs1(${variant}                          ${ARGN})
  build_device_libs1(${variant}/fstack-limit-register-et ${ARGN} -fstack-limit-register=et)
endmacro()

macro(build_device_libs3 variant)
  build_device_libs2(${variant}            ${ARGN})
  build_device_libs2(${variant}/mno-hw-mul ${ARGN} -mno-hw-mul)
  build_device_libs2(${variant}/mhw-mulx   ${ARGN} -mhw-mulx)
endmacro()

macro(build_device_libs4 variant)
  build_device_libs3(${variant}      ${ARGN})
  build_device_libs3(${variant}/mcdx ${ARGN} -mcdx)
endmacro()

macro(build_all_device_lib_variants lib_name)
  build_device_libs3(nios2 ${lib_name})
  build_device_libs4(nios2/march-r2 ${lib_name}      -march=r2)
  build_device_libs4(nios2/march-r2/mbmx ${lib_name} -march=r2 -mbmx)
endmacro()

##----------------------------------------------------------------------------##
# Paths
##----------------------------------------------------------------------------##

# TODO: Should be removed once ics configuration for offload compiler is implemented.
if (NOT LIBOMPTARGET_DEP_NIOS2_SDK)
  set(LIBOMPTARGET_DEP_NIOS2_SDK /rdrive/ref/nios2/17.0)
endif()

##----------------------------------------------------------------------------##
# Define the set of device libraries and build them
##----------------------------------------------------------------------------##

if (LIBOMPTARGET_DEP_NIOS2_SDK)
  libomptarget_say("Building Nios2 device runtime libraries")

  # EDS and Dispatcher are located in Nios2 SDK
  set(NIOS2_EDS  ${LIBOMPTARGET_DEP_NIOS2_SDK}/nios2eds)
  set(DISPATCHER ${LIBOMPTARGET_DEP_NIOS2_SDK}/nios2offload/target)

  # Nios2 gcc and ar for buildig target RTL
  set(NIOS2_GCC ${NIOS2_EDS}/bin/gnu/H-x86_64-pc-linux-gnu/bin/nios2-elf-gcc)
  set(NIOS2_AR  ${NIOS2_EDS}/bin/gnu/H-x86_64-pc-linux-gnu/bin/nios2-elf-ar)


  set(library_ids rtl block omp)

  set(lib_name_rtl   omptarget-nios2)
  set(lib_name_block datablock)
  set(lib_name_omp   omp)

  # Source files for the target RTL
  set(src_files_rtl   src/main.c)
  set(src_files_block src/data_block.cpp)
  set(src_files_omp   src/libomp.c)

  # Build all libraries
  foreach(lib_id IN LISTS library_ids)
    set(lib_name lib${lib_name_${lib_id}}.a)
    set(src_files ${src_files_${lib_id}})

    # Build library variants
    build_all_device_lib_variants(${lib_name})
  endforeach()

  # TODO: Temporary copy Dispatcher library to compiler libraries. So far we
  # have only one Dispatcher library variant.
  set(src_file ${DISPATCHER}/lib/libfpga_mc_dispatcher.a)
  set(dst_file ${LIBOMPTARGET_LIBRARY_DIR}/nios2/libfpga_mc_dispatcher.a)

  add_custom_target(omptarget.rtl.nios2.dispatcher ALL DEPENDS ${dst_file})

  add_custom_command(
    OUTPUT ${dst_file}
    COMMAND ${CMAKE_COMMAND} -E copy ${src_file} ${dst_file}
    DEPENDS ${src_file}
    VERBATIM
  )
  set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${dst_file})

  install(FILES ${dst_file} DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})
else(LIBOMPTARGET_DEP_NIOS2_SDK)
  libomptarget_say("Not building Nios2 device runtime libraries: Nios2 SDK not found.")
endif(LIBOMPTARGET_DEP_NIOS2_SDK)
