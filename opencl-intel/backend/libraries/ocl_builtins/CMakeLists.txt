# Copyright (C) 2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this
# software or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express
# or implied warranties, other than those that are expressly stated in the
# License.

#--------------------------- Settings ----------------------------------------------------------
#-- All Win32/Win64/Linux32/Linux64-dependent definitions should be placed here

#
#  Variables which override defaults (used only by OCL SDK build)
#     OCL_INSTALL_DIR               overrides output directory. The default is where the rest of OCL CPU BE resides.
#     OCL_CLANG_HEADERS             overrides directory name for clang headers. The default is 'clang_headers' in Volcano root
#     OCL_GLOBAL_INCLUDE            overrides directory name for global headers. The default is 'cl_api' in Volcano root
#     OCL_SOURCE_DIR                TRUE when invoked by OCL SDK build
#     OCL_HOST_CLANG                overrides 'clang' tool selection. The default is to use 'clang' tool from the same build
#     OCL_HOST_OPT                  overrides 'opt' tool selection. The default is to use 'opt' tool from the same build
#     OCL_HOST_LLVM_LINK            overrides 'llvm-link' tool selection. The default is to use 'llvm-link' tool from the same build
#

project(cl_builtins)

# target name prefix to be used for generated binaries
set (TARGET_NAME_PREFIX clbltfn)

# target name for shared RTL
set (SHARED_RTL_NAME shared)
set (TARGET_NAME_SHARED ${TARGET_NAME_PREFIX}${SHARED_RTL_NAME})

if (WIN32)
  if (BUILD_X64)
    # 64 bit
    set (PLATFORM win64)
  else ()
    # 32 bit
    set (PLATFORM win32)
  endif ()
else ()
  set (PLATFORM linux)
endif()

add_subdirectory (utils/TableGen)

# Implicit:
#  cl_builtins_SOURCE_DIR - points to the top src directory
#  cl_builtins_BINARY_DIR - points to the build directory

# remember OCL builtins source dir
set (CL_BUILTIN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# target directory for OCL .rtl, and .dll/.so of SVML library
set (BUILTINS_OUTPUT_DIR ${OCL_OUTPUT_LIBRARY_DIR})
set (BUILTINS_SHARED_OUTPUT_DIR ${OCL_LIBRARY_DIR})

# svml directories
set (SVML_DLL_DIR ${OCL_SOURCE_DIR}/externals/ocl_svml/${PLATFORM})
if (NOT EXISTS ${SVML_DLL_DIR})
  message (FATAL_ERROR "Cannot find SVML binary directory (${SVML_DLL_DIR})!")
endif()
set (SVML_NAME_PREFIX __ocl_svml_)

#
# Tool environment and options
#

# options for clang to compile OpenCL 2.0 code

# change -ffp-contract=off to fast to fix precision issue in built-in libary code.
# opencl.enable.FP_CONTRACT will not appear with  open source frontend. So in order to keep FMA
# folding in built-ins this attribute is required. It places 'contract' attribute that will perform
# FMAs even in 'standard' CodeGen setting.
set(CLANG_CL_COMMON_COMPILER_FLAGS
  ${CLANG_CL_COMMON_FLAGS}
  -I ${CL_BUILTIN_SOURCE_DIR} -include long_vector_types.h -include opencl_cth_pre_release.h
  -fopencl-force-vector-abi -fblocks -faddress-space-map-mangling=yes -mstackrealign -ffp-contract=fast
  -cl-ext=+cl_khr_fp16
  -D float_atomics_enable
  -Werror -Wall -Wno-psabi
  )

set (CLANG_C_COMPILER_PARAMS
  -cc1 -triple ${DEVICE_TRIPLE} -emit-llvm-bc -fwrapv -O3 -disable-intel-proprietary-opts
  -I ${IMPORT_DIR} ${INCLUDE_OCL_CLANG_HEADERS} -I ${CL_BUILTIN_SOURCE_DIR} ${CLANG_LLVM_INCLUDE_DIRS} -mstackrealign
  -no-opaque-pointers
  # Pretend we use LTO as Intel's clang version doesn't allows emitting LLVM
  # bytecode for C/C++ files w/o LTO flag.
  -flto
  )

# options for clang to compile OpenCL 2.0 image callbacks

# -stack-protector is enabled only on Linux since it is no possible to do so on
# Windows w/o linking to MS VS runtime (see, __security_check_cookie)
if (WIN32)
  set (CLANG_CL_CBK_COMPILER_FLAGS ${CLANG_CL_COMMON_COMPILER_FLAGS})
else (WIN32)
  set (CLANG_CL_CBK_COMPILER_FLAGS ${CLANG_CL_COMMON_COMPILER_FLAGS} -stack-protector 2)
endif (WIN32)

#------------------- Utility functions ---------------------------

#
# Compile single source file
#
# compile( COMPILE_OPTIONS OUTPUT_FILE_VAR PROC INPUT_FILE OUTPUT_DIR )
#
#   COMPILE_OPTIONS - compile options for clang
#   OUTPUT_FILE_VAR - output, contains full path to object file
#   PROC            - target CPU
#   INPUT_FILE      - full path to input file
#   OUTPUT_DIR      - directory for output file
#   DISABLE_LLVM_PASSES - whether to add '--disable-llvm-passes' option to clang
#

function( compile COMPILE_OPTIONS OUTPUT_FILE_VAR PROC INPUT_FILE OUTPUT_DIR DISABLE_LLVM_PASSES )

  get_filename_component (NAME ${INPUT_FILE} NAME_WE )
  set (OUTPUT_FILE ${NAME}${OCL_OUTPUT_EXTENSION} )

  # Provide an option to explicitly disable llvm optimization pipeline when we
  # compile OpenCL sources, especially when the output bitcode will later be
  # optimized by `opt -O3`, avoiding running duplication optimizations.
  if (${DISABLE_LLVM_PASSES})
    set(COMPILE_OPTIONS ${COMPILE_OPTIONS} -disable-llvm-passes)
  endif (${DISABLE_LLVM_PASSES})

  add_custom_command (OUTPUT ${OUTPUT_DIR}/${OUTPUT_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
    COMMAND ${CLANG} ${COMPILE_OPTIONS} -target-cpu ${PROC} -o ${OUTPUT_DIR}/${OUTPUT_FILE} ${INPUT_FILE}
    MAIN_DEPENDENCY ${INPUT_FILE}
    DEPENDS ${HEADER_FILES} ${CLANG} process-intrin-headers
    IMPLICIT_DEPENDS CXX ${INPUT_FILE}
    WORKING_DIRECTORY ${CL_BUILTIN_SOURCE_DIR}
    COMMENT "Compiling ${INPUT_FILE}"
    VERBATIM
    )

  set (${OUTPUT_FILE_VAR} ${OUTPUT_DIR}/${OUTPUT_FILE} PARENT_SCOPE )

endfunction( compile )

#
# add OpenCL library built from set of OpenCL source files
#
# add_ocl_library( OUTPUT_FILE_VAR TARGET_NAME ARCH PROC USE_OPT OUTPUT_DIR C_FILES ...FILES... )
#
#   OUTPUT_FILE_VAR             - output, contains generated library (rtl file)
#   PRECOMPILED_OUTPUT_FILE_VAR - output, contains generated library (obj file)
#   CLANG_COMPILE_FLAGS         - compiler flags
#   TARGET_NAME                 - name of the target and base name of the lib. Extension is OCL_OUTPUT_EXTENSION
#   TARGET_DIR                  - directory of target-*.td file
#   ARCH                        - target SIMD arch type
#   PROC                        - target CPU
#   USE_OPT                     - Optimize output library with llvm opt tool
#   ENABLE_PRECOMPILE           - enable precompiling (to native machine code)
#   OUTPUT_DIR                  - output directory for the intermediate files
#   C_FILES                     - input file list to compile as C99 (may be set to "IGNORE")
#   FILES                       - input file list to compile as OpenCL C 2.0
#

function( add_ocl_library OUTPUT_FILE_VAR PRECOMPILED_OUTPUT_FILE_VAR CLANG_COMPILE_FLAGS TARGET_NAME TARGET_DIR ARCH PROC USE_OPT ENABLE_PRECOMPILE BI_LIB_NAME OUTPUT_DIR C_FILES )

  if (${ARGC} LESS 10)
    message (FATAL_ERROR "add_ocl_library(${TARGET_NAME}) - no sources given")
  endif ()

  set (TMP_LINKED_RTL ${TARGET_NAME}_linked_tmp)
  set (TMP_LINKED_RTL_OUTPUT ${OUTPUT_DIR}/${TMP_LINKED_RTL}${OCL_OUTPUT_EXTENSION})

  set (OUTPUT_FILE ${TARGET_NAME}${OCL_OUTPUT_EXTENSION})
  set(LIBRARY_OUTPUT_FILE ${OCL_LIBRARY_TARGET_NAME}${OCL_OUTPUT_EXTENSION})
  set(PRECOMPILED_OUTPUT_FILE ${OCL_LIBRARY_TARGET_NAME}${ARCH}${OCL_PRECOMPILED_OUTPUT_EXTENSION})

  # Compile all files and prepare a 'TMP_FILES' list of .rtl files

  foreach (FILE ${ARGN})
    compile( "${CLANG_COMPILE_FLAGS}" TMP_RTL ${PROC} ${FILE} ${OUTPUT_DIR} TRUE)
    list (APPEND TMP_FILES ${TMP_RTL})
  endforeach (FILE)

  if(NOT ("${C_FILES}" STREQUAL "IGNORE"))
    foreach (FILE ${C_FILES})
      compile( "${CLANG_C_COMPILER_PARAMS}" TMP_RTL ${PROC} ${FILE} ${OUTPUT_DIR} TRUE)
      list (APPEND TMP_FILES ${TMP_RTL})
    endforeach (FILE)
  endif()

  if (EXISTS ${CL_BUILTIN_SOURCE_DIR}/${TARGET_DIR}/${ARCH}_intrinsics.ll)
    list (APPEND TMP_FILES ${CL_BUILTIN_SOURCE_DIR}/${TARGET_DIR}/${ARCH}_intrinsics.ll)
  endif()

  if(${USE_OPT})
    # link OpenCL image callback library (adding also intrinsic BI IR )
    add_custom_command (OUTPUT ${OUTPUT_DIR}/${OUTPUT_FILE}_noopt
      COMMENT "Linking ${OUTPUT_FILE}_noopt"
      COMMAND ${LLVM_LINK} ${LLVM_LINK_PARAMS} -o ${OUTPUT_DIR}/${OUTPUT_FILE}_noopt ${TMP_FILES}
      DEPENDS ${LLVM_LINK} ${TMP_FILES}
      VERBATIM
      )
    # link shared rtl and particular BI rtl
    # image callback rtl must be linked with built-ins RTL statically, we have two built-ins rtls: shared (clbltfnshared.rtl)
    # and particular lib (for  example, clbltfnl9.rtl), but image optimizer takes only one runtime lib as argument, so we have to
    # link clbltfnshared.rtl and particular (clbltfnl9.rtl, for instance) together to temporary TMP_LINKED_RTL_OUTPUT and then
    # link image callback rtl (${OUTPUT_FILE}_noopt) with this TMP_LINKED_RTL_OUTPUT
    # We also allow symbols from target-specific rtl (e.g. clbltfnl9.rtl) to override defined symbol in shared rtl.
    #
    add_custom_command (OUTPUT ${TMP_LINKED_RTL_OUTPUT}
      COMMENT "Linking tmp rtl ${TMP_LINKED_RTL}"
      COMMAND ${LLVM_LINK} ${LLVM_LINK_PARAMS} -o ${TMP_LINKED_RTL_OUTPUT} --override=${BUILTINS_OUTPUT_DIR}/${BI_LIB_NAME}${OCL_OUTPUT_EXTENSION} ${BUILTINS_SHARED_OUTPUT_DIR}/${TARGET_NAME_SHARED}${OCL_OUTPUT_EXTENSION}
      DEPENDS ${LLVM_LINK} ${BUILTINS_OUTPUT_DIR}/${BI_LIB_NAME}${OCL_OUTPUT_EXTENSION} ${TARGET_NAME_SHARED}
      VERBATIM
      )
    set (RTL_TO_LINK ${TMP_LINKED_RTL_OUTPUT})

    add_custom_command(OUTPUT ${OUTPUT_DIR}/${OUTPUT_FILE}_opt
      COMMENT "Optimizing ${OUTPUT_FILE}_noopt"
      COMMAND ${OPT} ${OUTPUT_DIR}/${OUTPUT_FILE}_noopt -infer-address-spaces -dpcpp-kernel-builtin-lib=${RTL_TO_LINK} -dpcpp-kernel-cpu-prefix=${ARCH} -dpcpp-kernel-builtin-import -dpcpp-kernel-builtin-call-to-inst -verify -o ${OUTPUT_DIR}/${OUTPUT_FILE}_noopt_gas
      COMMAND ${OPT} ${OUTPUT_DIR}/${OUTPUT_FILE}_noopt_gas -O3 -disable-intel-proprietary-opts -o ${OUTPUT_DIR}/${OUTPUT_FILE}_opt
      DEPENDS ${OPT} ${OUTPUT_DIR}/${OUTPUT_FILE}_noopt ${RTL_TO_LINK}
      VERBATIM
      )

    # link with kernel rtl
    add_custom_command (OUTPUT ${OUTPUT_DIR}/${LIBRARY_OUTPUT_FILE}
      COMMENT "Linking ${OUTPUT_FILE}_opt with ${KERNEL_RTL}"
      COMMAND ${LLVM_LINK} ${LLVM_LINK_PARAMS} -o ${OUTPUT_DIR}/${LIBRARY_OUTPUT_FILE} ${OUTPUT_DIR}/${OUTPUT_FILE}_opt ${KERNEL_RTL}
      DEPENDS ${LLVM_LINK} ${OUTPUT_DIR}/${OUTPUT_FILE}_opt ${KERNEL_RTL}
      VERBATIM
      )
  else()
    # link OpenCL library (adding also BI IR for vectorizer)
    add_custom_command (OUTPUT ${BUILTINS_OUTPUT_DIR}/${OUTPUT_FILE}
      COMMENT "Linking and optimizing ${BUILTINS_OUTPUT_DIR}/${OUTPUT_FILE}"
      COMMAND ${LLVM_LINK} ${LLVM_LINK_PARAMS} -o ${OUTPUT_DIR}/${OUTPUT_FILE}_link ${TMP_FILES} ${CL_BUILTIN_SOURCE_DIR}/${TARGET_DIR}/${ARCH}_vectorizer.ll
      COMMAND ${OPT} ${OUTPUT_DIR}/${OUTPUT_FILE}_link -O3 -disable-intel-proprietary-opts -o ${BUILTINS_OUTPUT_DIR}/${OUTPUT_FILE}
      DEPENDS ${LLVM_LINK} ${OPT} ${TMP_FILES} ${CL_BUILTIN_SOURCE_DIR}/${TARGET_DIR}/${ARCH}_vectorizer.ll
      VERBATIM
      )
  endif()

  # compile the rtl file to native objects
  if(${ENABLE_PRECOMPILE})
    add_custom_command(OUTPUT ${BUILTINS_OUTPUT_DIR}/${PRECOMPILED_OUTPUT_FILE}
      COMMENT "Building static object ${PRECOMPILED_OUTPUT_FILE}"
      COMMAND ${LLC} ${LLC_PARAMS} -mcpu=${PROC} -o
              ${BUILTINS_OUTPUT_DIR}/${PRECOMPILED_OUTPUT_FILE} <
              ${OUTPUT_DIR}/${LIBRARY_OUTPUT_FILE}
      DEPENDS ${LLC} ${OUTPUT_DIR}/${LIBRARY_OUTPUT_FILE}
      VERBATIM
      )
  endif(${ENABLE_PRECOMPILE})

  set (${OUTPUT_FILE_VAR} ${OUTPUT_FILE} PARENT_SCOPE)
  set (${PRECOMPILED_OUTPUT_FILE_VAR} ${PRECOMPILED_OUTPUT_FILE} PARENT_SCOPE)

endfunction( add_ocl_library )

#
# Install SVML library (and SDE library) for given CPU arch
#
# install_svml( ARCH )
#
#   ARCH            - target SIMD arch type
#

function( install_svml ARCH )

  # SVML binary
  set (SVML_DLL ${SVML_DLL_DIR}/${SVML_NAME_PREFIX}${ARCH}${CMAKE_SHARED_LIBRARY_SUFFIX}${ADDITIONAL_SHARED_LIBRARY_SUFFIX})
  if (NOT EXISTS ${SVML_DLL})
    message( FATAL_ERROR "Cannot find ${SVML_DLL}!")
  endif()

  copy_to (${SVML_DLL} DESTINATION lib)
  install_to (${SVML_DLL} DESTINATION lib/${OUTPUT_ARCH_SUFF} COMPONENT ocl-svml)

endfunction( install_svml )

function(tblgenoclblt OUTPUT_VAR OUTPUT_DIR INPUT_DIR IFN OFN)
    add_custom_command(
        OUTPUT ${OUTPUT_DIR}/${OFN}
        COMMAND ${OCL_TBLGEN} ${ARGN} -I=${CL_BUILTIN_SOURCE_DIR} -o ${OUTPUT_DIR}/${OFN} ${CL_BUILTIN_SOURCE_DIR}/${INPUT_DIR}/${IFN}
        DEPENDS ${OCL_TBLGEN} ${OCL_ALL_TD_FILES}
        COMMENT "Generating ${OFN}"
        )
    set(${OUTPUT_VAR} ${${OUTPUT_VAR}} ${OUTPUT_DIR}/${OFN} PARENT_SCOPE)
endfunction(tblgenoclblt)

function(tblgenllblt OUTPUT_VAR OUTPUT_DIR INPUT_FILE)
  get_filename_component(RELATIVE_DIR ${INPUT_FILE} DIRECTORY)
  make_directory(${OUTPUT_DIR}/${RELATIVE_DIR})
  set(JSON_FILE ${OUTPUT_DIR}/${INPUT_FILE}.json)
  set(LL_FILE ${OUTPUT_DIR}/${INPUT_FILE}.ll)
  add_custom_command(
    OUTPUT ${JSON_FILE}
    COMMAND ${LLVM_TBLGEN} ${ARGN} -I=${CL_BUILTIN_SOURCE_DIR} -dump-json -o ${JSON_FILE} ${CL_BUILTIN_SOURCE_DIR}/${INPUT_FILE}
    DEPENDS ${LLVM_TBLGEN} ${CL_BUILTIN_SOURCE_DIR}/${INPUT_FILE}
    COMMENT "Generating ${JSON_FILE}"
  )
  add_custom_command(
    OUTPUT ${LL_FILE}
    COMMAND python ${CL_BUILTIN_SOURCE_DIR}/utils/LLTableGen/emit_ll.py ${JSON_FILE} -o ${LL_FILE}
    DEPENDS ${LLVM_TBLGEN} ${JSON_FILE} ${CL_BUILTIN_SOURCE_DIR}/utils/LLTableGen/emit_ll.py
    COMMENT "Generating ${LL_FILE}"
  )
  set_source_files_properties(${LL_FILE} PROPERTIES GENERATED 1)
  set(${OUTPUT_VAR} ${LL_FILE} PARENT_SCOPE)
endfunction(tblgenllblt)

#
# create_target( PROC ARCH PREBUILD_IMAGE_LIBRARY)
#
# Uses:
#   TARGET_NAME_PREFIX - base name of the target OpenCL library
#   OCL_SOURCE_FILES   - list of sources for OpenCL library
# Parameters
#   PROC                    - target CPU
#   ARCH                    - target SIMD arch type
#   TARGET_DIR              - directory of target-*.td file
#   PREBUILD_IMAGE_LIBRARY  - flag to control creation of object files for image callback library

function( create_target PROC ARCH TARGET_DIR PREBUILD_IMAGE_LIBRARY)

  set (TARGET_NAME ${TARGET_NAME_PREFIX}${ARCH})

  # intermediate directory for OpenCL compilation/link
  set (INTERMEDIATE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

  # generate OCL builtin implementation
  # TODO: remove this tblgenoclblt invocation when we migrate all builtins to
  # source/main.td
  tblgenoclblt(OCL_TABLEGEN_SOURCE
    ${INTERMEDIATE_OUTPUT_DIR}
    ${TARGET_DIR}
    target-${ARCH}.td
    target-${ARCH}.cl
    --gen-ocl-bi
  )
  # The newly added builtins are implemented in souce/main.td
  tblgenoclblt(OCL_TABLEGEN_SOURCE
    ${INTERMEDIATE_OUTPUT_DIR}
    source
    main.td
    target-${ARCH}-new.cl
    # args for ocl-tblgen
    --gen-ocl-bi --handle-impl-target-inheritance --emit-target-name=${TARGET_DIR}
  )
  list (APPEND OCL_20_SOURCE_FILES ${OCL_TABLEGEN_SOURCE})

  # build .rtl file out of OCL code (by clang) and copy it to install directory
  add_ocl_library( OUTPUT_OCL_LIB IGNORE "${CLANG_CL_COMMON_COMPILER_FLAGS}" ${TARGET_NAME} ${TARGET_DIR} ${ARCH} ${PROC}  FALSE FALSE IGNORE ${INTERMEDIATE_OUTPUT_DIR} "${C_SOURCE_FILES}" ${OCL_20_SOURCE_FILES})

  # Build .o files out of OCL image callbacks rtl file and kernel rtl file.
  # Copy to install directory
  add_ocl_library(IGNORE OUTPUT_OCL_LIBRARY_OBJ "${CLANG_CL_CBK_COMPILER_FLAGS}" ${TARGET_NAME}_img_cbk ${TARGET_DIR} ${ARCH} ${PROC} TRUE ${PREBUILD_IMAGE_LIBRARY} ${TARGET_NAME} ${INTERMEDIATE_OUTPUT_DIR} IGNORE ${OCL_IMAGE_CALLBACK_SOURCE_FILE})

  # copy SVML (and SDE) libaries to install directory
  install_svml( ${ARCH} )

  # Unconditional dependent targets
  set (TARGET_DEPENDENCIES ${BUILTINS_OUTPUT_DIR}/${OUTPUT_OCL_LIB} ${BUILTINS_OUTPUT_DIR}/${SVML_NAME_PREFIX}${ARCH}${CMAKE_SHARED_LIBRARY_SUFFIX}${ADDITIONAL_SHARED_LIBRARY_SUFFIX})

  # If prebuilt library is built, depend on it so it gets built.
  if( PREBUILD_IMAGE_LIBRARY )
    set (TARGET_DEPENDENCIES ${TARGET_DEPENDENCIES} ${BUILTINS_OUTPUT_DIR}/${OUTPUT_OCL_LIBRARY_OBJ}) 
  endif( PREBUILD_IMAGE_LIBRARY)

  # top-level target for OCL target
  add_custom_target (${TARGET_NAME} ALL
    DEPENDS ${TARGET_DEPENDENCIES} ${OCL_20_SOURCE_FILES} ${OCL_ALL_TD_FILES} ${OCL_CLANG_HEADER_DIR}
    COMMENT "Target ${TARGET_NAME} build completed"
    VERBATIM
    SOURCES ${OCL_20_SOURCE_FILES} ${OCL_IMAGE_CALLBACK_SOURCE_FILE} ${HEADER_FILES}
  )

  set_target_properties (${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/cl_builtins)

  add_dependencies(${TARGET_NAME} ${OCL_TBLGEN} ${OCL_LIBRARY_TARGET_NAME} ${OPT})

  set_property(GLOBAL APPEND PROPERTY OCL_BUILTIN_LIBS ${TARGET_NAME})

  set (INSTALL_LIST ${BUILTINS_OUTPUT_DIR}/${OUTPUT_OCL_LIB})
  if (PREBUILD_IMAGE_LIBRARY)
    set (INSTALL_LIST ${INSTALL_LIST} ${BUILTINS_OUTPUT_DIR}/${OUTPUT_OCL_LIBRARY_OBJ})
  endif (PREBUILD_IMAGE_LIBRARY)

  install_to (${INSTALL_LIST} DESTINATION lib/${OUTPUT_ARCH_SUFF} COMPONENT ocl-builtins)

endfunction( create_target)

#------------------ Main -----------------------

# OpenCL source code
list (APPEND OCL_20_SOURCE_FILES
  generic-builtin-defines.c
  cpu-builtin-defines.c
  image_functions.cpp
  extended_execution_functions.cpp
  generic_functions.cl
  kmp_acquire_release_lock.cl
)

list (APPEND OCL_20_SOURCE_FILES
  pipes.cl
  pipes-info.cl
  pipe_functions.c
)

list (APPEND C_SOURCE_FILES
  ${CL_BUILTIN_SOURCE_DIR}/cbk_helper.c
  )


list (APPEND OCL_IMAGE_CALLBACK_SOURCE_FILE
  ${CL_BUILTIN_SOURCE_DIR}/image_callback_functions.cpp
  )

# project header files
list (APPEND HEADER_FILES
  cl_image_declaration.h
  cpu-builtin-defines.h
  long_vector_types.h
  opencl_cth_pre_release.h
  svml_ia32.h
  svml_ia32e.h
  svml_shared.h
  transpose_functions.h
  )

# convert everything to full paths
set (SAVE_OCL_20_SOURCE_FILES ${OCL_20_SOURCE_FILES})
unset (OCL_20_SOURCE_FILES)
foreach (FILE ${SAVE_OCL_20_SOURCE_FILES})
  list (APPEND OCL_20_SOURCE_FILES ${CL_BUILTIN_SOURCE_DIR}/${FILE})
endforeach (FILE)

set (SAVE_HEADER_FILES ${HEADER_FILES})
unset (HEADER_FILES)
foreach (FILE ${SAVE_HEADER_FILES})
  list (APPEND HEADER_FILES ${CL_BUILTIN_SOURCE_DIR}/${FILE})
endforeach (FILE)

# running build for all supported SIMD architectures
add_subdirectory (sse42)
add_subdirectory (avx)
add_subdirectory (avx2)
# includes AVX-512 F, CD, BW, DQ, VL.
add_subdirectory (avx512)
add_subdirectory (amx)
add_subdirectory (shared)
if (ENABLE_KNL AND BUILD_X64 AND NOT WIN32)
    # includes AVX-512 F, CD, ER, PF.
  add_subdirectory (avx512phi)
endif()

# algorithm from compiler-rt for soft types
add_subdirectory(soft_math)

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(MINIMAL_SUFFIX n8)
else ()
  set(MINIMAL_SUFFIX h8)
endif()

set(RTL_MINIMAL
    ${BUILTINS_OUTPUT_DIR}/${TARGET_NAME_PREFIX}${MINIMAL_SUFFIX}.rtl)
set(TARGET_NAME_MINIMAL ${TARGET_NAME_PREFIX}${MINIMAL_SUFFIX})

set(RTL_SHARED ${BUILTINS_SHARED_OUTPUT_DIR}/${TARGET_NAME_PREFIX}shared.rtl)

# Pipe built-ins for FPGA are used (linked into) in Backend and Runtime:
#
#   - ChannelPipeTransformation uses 1 built-in to get a size of pipe
#     object (because it allocates memory for pipe object)
#
#   - Pipe object in runtime uses several functions to initialize pipe
#     object from runtime (for OpenCL 2.0 pipes)
#
add_custom_target(clbltpipe_target
  DEPENDS clbltpipe.obj)

add_custom_command(OUTPUT clbltpipe.obj
  DEPENDS ${TARGET_NAME_MINIMAL} ${TARGET_NAME_SHARED}
  DEPENDS pipes.cl pipes-info.cl pipes-internal.h pipes-defines.h
  DEPENDS ${CLANG} process-intrin-headers

  # Use host triple to let llc produce object file in a native format.
  COMMAND ${CLANG} ${CLANG_CL_COMMON_COMPILER_FLAGS} -triple ${HOST_TRIPLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/pipes.cl -o pipes1.bc

  COMMAND ${CLANG} ${CLANG_CL_COMMON_COMPILER_FLAGS} -triple ${HOST_TRIPLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/pipes-info.cl -o pipes2.bc

  COMMAND ${LLVM_LINK} pipes1.bc pipes2.bc -o pipes.bc

  COMMAND ${OPT} -dpcpp-kernel-builtin-lib=${RTL_MINIMAL},${RTL_SHARED}
  -dpcpp-kernel-builtin-import pipes.bc -o pipes-builtin.bc

  COMMAND ${OPT} -O3 -disable-intel-proprietary-opts pipes-builtin.bc -o pipes-opt.bc

  COMMAND ${LLC} ${LLC_PARAMS} -mcpu=corei7 pipes-opt.bc
  -o clbltpipe.obj)

add_library(clbltpipe STATIC IMPORTED GLOBAL)
add_dependencies(clbltpipe clbltpipe_target)
set_target_properties(clbltpipe
  PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/clbltpipe.obj)
