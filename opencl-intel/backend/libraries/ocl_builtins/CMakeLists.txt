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

# In this project we use binaries from OpenCL prject (e.g. oclopt), LLVM for
# front-end compiler (e.g. clang) and LLVM for back-end compiler (e.g. llc).
# oclopt utility is built with LLVM version for the back-end compiler, so when
# applies passes to the RTL it converts LLVM IR to newer version. This restrict
# the tools we use after oclopt to the tools from LLVM for back-end compiler.
# So we have to set path to tools directory for BE version of LLVM.
# We know that LLVM for the back-end compiler is already configured, so just
# save LLVM_BINARY_DIR for later use by LLVM tools running after oclopt.
# TODO: to make this file more self contained we should not rely on the
# assumption above and configure LLVM for the back-end right here.
set(BE_LLVM_BINARY_DIR ${LLVM_BINARY_DIR})

if( NOT DEFINED LLVM_PATH_FE )
  message( FATAL_ERROR "LLVM_PATH_FE is not specified. Please specify LLVM library location for fronted using LLVM_PATH_FE parameter to CMAKE" )
endif()

set(LLVM_PATH ${LLVM_PATH_FE})
find_package(LLVM REQUIRED)

project(cl_builtins)

# target name prefix to be used for generated binaries
set (TARGET_NAME_PREFIX clbltfn)

# target name for shared RTL
set (SHARED_RTL_NAME shared)
set (TARGET_NAME_SHARED ${TARGET_NAME_PREFIX}${SHARED_RTL_NAME})

if (BUILD_X64)
  # 64 bit
  set (PLATFORM 64)
else ()
  # 32 bit
  set (PLATFORM 32)
endif ()

if (WIN32)
  set (PLATFORM Win${PLATFORM})
else ()
  set (PLATFORM Linux${PLATFORM})
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

# global headers
if (DEFINED OCL_CLANG_HEADERS)
  set (CLANG_HEADERS ${OCL_CLANG_HEADERS})
else ()
  set (CLANG_HEADERS ${CL_BUILTIN_SOURCE_DIR}/../../clang_headers)
endif()
if (NOT EXISTS ${CLANG_HEADERS})
  message (FATAL_ERROR "Cannot find Clang headers (${CLANG_HEADERS})!")
endif()

if (NOT DEFINED OCL_GLOBAL_INCLUDE)
  message (FATAL_ERROR "CL API directory path is not set!")
endif ()
set (IMPORT_DIR ${OCL_GLOBAL_INCLUDE})

# svml directories
set (SVML_DLL_DIR ${CL_BUILTIN_SOURCE_DIR}/bin/svml/${PLATFORM})
if (NOT EXISTS ${SVML_DLL_DIR})
  message (FATAL_ERROR "Cannot find SVML binary directory (${SVML_DLL_DIR})!")
endif()
set (SVML_NAME_PREFIX __ocl_svml_)
set (SDE_NAME_PREFIX __sde_svml_)

set(TOOL_BINARY_DIR "${BE_LLVM_BINARY_DIR}" )

if (NOT OPENCL_INTREE_BUILD)
  # LLVM tools are assumed to be pre-built within the global project
  if (NOT DEFINED OCL_HOST_CLANG)
    set (CLANG "${LLVM_BINARY_DIR}/clang")
  else()
    set (CLANG ${OCL_HOST_CLANG})
  endif()
else()
  set(CLANG clang)
endif()

if (NOT DEFINED OCL_HOST_OPT)
  set (OPT oclopt)
  set (OPT_DEP oclopt)
  set (OCL-TBLGEN ocl-tblgen)
  set (OCL-TBLGEN-DEP ocl-tblgen)
else ()
  set (OPT OCL_HOST_OPT)
endif()
if (NOT DEFINED OCL_HOST_LLVM_LINK)
  set (LLVM_LINK "${TOOL_BINARY_DIR}/llvm-link")
else()
  set (LLVM_LINK OCL_HOST_LLVM_LINK)
endif()
if (NOT DEFINED OCL_HOST_LLC)
  set (LLC "${TOOL_BINARY_DIR}/llc")
else()
  set (LLC OCL_HOST_LLC)
endif()

# LLVM triple is per OS and Arch (keep consistent with Materializer),
# also, note that -elf postfix is added to windows triples later in the file.
# adding "-elf" postfix to the triple for Win32 to codegen ELF with llc later (to be consistent with Backend Materializer)
if (WIN32)
  if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set (DEVICE_TRIPLE i686-pc-win32-gnu-elf)
    set (HOST_TRIPLE   i686-pc-win32-gnu)
  else ()
    set (DEVICE_TRIPLE x86_64-pc-win32-gnu-elf)
    set (HOST_TRIPLE   x86_64-pc-win32-gnu)
  endif (CMAKE_SIZEOF_VOID_P EQUAL 4)
else (WIN32)
  if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set (DEVICE_TRIPLE i686-pc-linux)
    set (HOST_TRIPLE   i686-pc-linux)
  else ()
    set (DEVICE_TRIPLE x86_64-pc-linux)
    set (HOST_TRIPLE   x86_64-pc-linux)
  endif (CMAKE_SIZEOF_VOID_P EQUAL 4)
endif (WIN32)

#
# Tool environment and options
#

# clang
set (OCL_OUTPUT_EXTENSION .rtl)
set (OCL_PRECOMPILED_OUTPUT_EXTENSION .o)
set (CLANG_LLVM_INCLUDE_DIRS)
string( REPLACE  \${CMAKE_CFG_INTDIR} ${CMAKE_CFG_INTDIR} LLVM_INCLUDE_DIRS "${LLVM_INCLUDE_DIRS}" )
foreach( LLVM_INCLUDE_DIR_ITEM ${LLVM_INCLUDE_DIRS} )
  set(CLANG_LLVM_INCLUDE_DIRS ${CLANG_LLVM_INCLUDE_DIRS} -I ${LLVM_INCLUDE_DIR_ITEM})
endforeach( LLVM_INCLUDE_DIR_ITEM)

# options for clang to compile OpenCL 2.0 code

# change -ffp-contract=off to fast to fix precision issue in built-in libary code.
# opencl.enable.FP_CONTRACT will not appear with  open source frontend. So in order to keep FMA
# folding in built-ins this attribute is required. It places 'contract' attribute that will perform
# FMAs even in 'standard' CodeGen setting.
set (CLANG_CL_COMMON_COMPILER_FLAGS
  -cc1 -triple ${DEVICE_TRIPLE} -emit-llvm-bc -fwrapv -O3 -disable-intel-proprietary-opts
  -x cl -I ${IMPORT_DIR} -I ${CLANG_HEADERS} -I ${CCLANG_USER_INCLUDE_DIRS} -I ${CL_BUILTIN_SOURCE_DIR} ${CLANG_LLVM_INCLUDE_DIRS} -include opencl-c.h -include long_vector_types.h -include opencl_cth_pre_release.h
  -cl-std=CL2.0 -D __OPENCL_C_VERSION__=200 -fopencl-force-vector-abi
  -fblocks -ffake-address-space-map -faddress-space-map-mangling=yes -mstackrealign -ffp-contract=fast
  -cl-ext=+cl_khr_fp16
  -D float_atomics_enable
  )

set (CLANG_C_COMPILER_PARAMS
  -cc1 -triple ${DEVICE_TRIPLE} -emit-llvm-bc -fwrapv -O3 -disable-intel-proprietary-opts
  -I ${IMPORT_DIR} -I ${CLANG_HEADERS} -I ${CCLANG_USER_INCLUDE_DIRS} -I ${CL_BUILTIN_SOURCE_DIR} ${CLANG_LLVM_INCLUDE_DIRS} -mstackrealign
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

# llvm-opt
set (LLVM_OPT_PARAMS -verify -sroa -functionattrs)

# images optimization options
set (LLVM_IMAGES_OPT_PARAMS -verify -builtin-import -builtin-call-to-inst -verify -O3 -disable-intel-proprietary-opts)

# llvm-link parameters
set (LLVM_LINK_PARAMS -suppress-warnings)

# llc parameters
set (LLC_PARAMS -filetype=obj)
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  set (LLC_PARAMS ${LLC_PARAMS} -relocation-model=static)
else (CMAKE_SIZEOF_VOID_P EQUAL 4)
  set (LLC_PARAMS ${LLC_PARAMS} -code-model=large -relocation-model=pic)
endif (CMAKE_SIZEOF_VOID_P EQUAL 4)
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
#

function( compile COMPILE_OPTIONS OUTPUT_FILE_VAR PROC INPUT_FILE OUTPUT_DIR )

  get_filename_component (NAME ${INPUT_FILE} NAME_WE )
  set (OUTPUT_FILE ${NAME}${OCL_OUTPUT_EXTENSION} )

  add_custom_command (OUTPUT ${OUTPUT_DIR}/${OUTPUT_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
    COMMAND ${CLANG} ${COMPILE_OPTIONS} -target-cpu ${PROC} -o ${OUTPUT_DIR}/${OUTPUT_FILE} ${INPUT_FILE}
    COMMAND ${OPT}  ${LLVM_OPT_PARAMS} -o ${OUTPUT_DIR}/${OUTPUT_FILE} ${OUTPUT_DIR}/${OUTPUT_FILE}
    MAIN_DEPENDENCY ${INPUT_FILE}
    DEPENDS ${HEADER_FILES} ${OPT_DEP} ${CLANG}
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
  set (TMP_LINKED_RTL_OUTPUT ${TMP_LINKED_RTL}${OCL_OUTPUT_EXTENSION})

  set (OUTPUT_FILE ${TARGET_NAME}${OCL_OUTPUT_EXTENSION})
  set (PRECOMPILED_OUTPUT_FILE ${TARGET_NAME}${OCL_PRECOMPILED_OUTPUT_EXTENSION})

  # Compile all files and prepare a 'TMP_FILES' list of .rtl files

  foreach (FILE ${ARGN})
    compile( "${CLANG_COMPILE_FLAGS}" TMP_RTL ${PROC} ${FILE} ${OUTPUT_DIR})
    list (APPEND TMP_FILES ${TMP_RTL})
  endforeach (FILE)

  if(NOT ("${C_FILES}" STREQUAL "IGNORE"))
    foreach (FILE ${C_FILES})
      compile( "${CLANG_C_COMPILER_PARAMS}" TMP_RTL ${PROC} ${FILE} ${OUTPUT_DIR})
      list (APPEND TMP_FILES ${TMP_RTL})
    endforeach (FILE)
  endif()

  if(${USE_OPT})
    # link OpenCL image callback library (adding also intrinsic BI IR )
    add_custom_command (OUTPUT ${OUTPUT_FILE}_noopt
      COMMENT "Linking ${OUTPUT_FILE}_noopt"
      COMMAND ${LLVM_LINK} ${LLVM_LINK_PARAMS} -o ${OUTPUT_FILE}_noopt ${TMP_FILES} ${CL_BUILTIN_SOURCE_DIR}/GENERIC/ll_intrinsics.ll
      DEPENDS ${LLVM_LINK} ${TMP_FILES} ${CL_BUILTIN_SOURCE_DIR}/GENERIC/ll_intrinsics.ll
      VERBATIM
      )
    # link shared rtl and particular BI rtl
    # image callback rtl must be linked with built-ins RTL statically, we have two built-ins rtls: shared (clbltfnshared.rtl)
    # and particular lib (for  example, clbltfnl9.rtl), but image optimizer takes only one runtime lib as argument, so we have to
    # link clbltfnshared.rtl and particular (clbltfnl9.rtl, for instance) together to temporary TMP_LINKED_RTL_OUTPUT and then
    # link image callback rtl (${OUTPUT_FILE}_noopt) with this TMP_LINKED_RTL_OUTPUT
    #
    add_custom_command (OUTPUT ${TMP_LINKED_RTL_OUTPUT}
      COMMENT "Linking tmp rtl ${TMP_LINKED_RTL}"
      COMMAND ${LLVM_LINK} ${LLVM_LINK_PARAMS} -o ${TMP_LINKED_RTL_OUTPUT} ${BUILTINS_OUTPUT_DIR}/${BI_LIB_NAME}${OCL_OUTPUT_EXTENSION} ${BUILTINS_SHARED_OUTPUT_DIR}/${TARGET_NAME_SHARED}${OCL_OUTPUT_EXTENSION}
      DEPENDS ${LLVM_LINK} ${BUILTINS_OUTPUT_DIR}/${BI_LIB_NAME}${OCL_OUTPUT_EXTENSION} ${TARGET_NAME_SHARED}
      VERBATIM
      )
    set (RTL_TO_LINK ${TMP_LINKED_RTL_OUTPUT})

    add_custom_command(OUTPUT ${BUILTINS_OUTPUT_DIR}/${OUTPUT_FILE}
      COMMENT "Optimizing ${OUTPUT_FILE}_noopt"
      COMMAND ${OPT} ${OUTPUT_FILE}_noopt -generic-addr-static-resolution -o ${OUTPUT_FILE}_noopt_gas
      COMMAND ${OPT} ${OUTPUT_FILE}_noopt_gas ${LLVM_IMAGES_OPT_PARAMS} -runtimelib=${RTL_TO_LINK} -arch=${ARCH} -o ${BUILTINS_OUTPUT_DIR}/${OUTPUT_FILE}
      DEPENDS ${OPT_DEP} ${OUTPUT_FILE}_noopt ${RTL_TO_LINK}
      VERBATIM
      )
  else()
    # link OpenCL library (adding also BI IR for vectorizer)
    add_custom_command (OUTPUT ${BUILTINS_OUTPUT_DIR}/${OUTPUT_FILE}
      COMMENT "Linking ${BUILTINS_OUTPUT_DIR}/${OUTPUT_FILE}"
      COMMAND ${LLVM_LINK} ${LLVM_LINK_PARAMS} -o ${BUILTINS_OUTPUT_DIR}/${OUTPUT_FILE} ${TMP_FILES} ${CL_BUILTIN_SOURCE_DIR}/${TARGET_DIR}/${ARCH}_vectorizer.ll ${CL_BUILTIN_SOURCE_DIR}/GENERIC/ll_intrinsics.ll
      DEPENDS ${LLVM_LINK} ${TMP_FILES} ${CL_BUILTIN_SOURCE_DIR}/${TARGET_DIR}/${ARCH}_vectorizer.ll ${CL_BUILTIN_SOURCE_DIR}/GENERIC/ll_intrinsics.ll
      VERBATIM
      )
  endif()

  # compile the rtl file to native objects
  if(${ENABLE_PRECOMPILE})
    # Initialize the llc parameter "-mattr" based on PROC
    SET (MATTR "-mattr=")
    if (${PROC} MATCHES ".*avx2$")
      set (MATTR "${MATTR}+avx2")
    else()
      if (${PROC} MATCHES ".*avx$")
        set (MATTR "${MATTR}+avx")
      endif ()
    endif ()

    add_custom_command(OUTPUT ${BUILTINS_OUTPUT_DIR}/${PRECOMPILED_OUTPUT_FILE}
      COMMENT "Building static object ${PRECOMPILED_OUTPUT_FILE}"
      COMMAND ${LLC} ${LLC_PARAMS} ${MATTR} -mcpu=${PROC} -o ${BUILTINS_OUTPUT_DIR}/${PRECOMPILED_OUTPUT_FILE} < ${BUILTINS_OUTPUT_DIR}/${OUTPUT_FILE}
      DEPENDS ${LLC} ${BUILTINS_OUTPUT_DIR}/${OUTPUT_FILE}
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
    file(GLOB_RECURSE depend_tds "${CL_BUILTIN_SOURCE_DIR}/*.td")
    add_custom_command(
        OUTPUT ${OUTPUT_DIR}/${OFN}
        #COMMAND ${OCL-TBLGEN} ${ARGN} -gen-ocl-Exclude=true -gen-ocl-Prefix="__cl_" -I=${CL_BUILTIN_SOURCE_DIR} -o ${OUTPUT_DIR}/${OFN} ${CL_BUILTIN_SOURCE_DIR}/${INPUT_DIR}/${IFN}
        COMMAND ${OCL-TBLGEN} ${ARGN} -I=${CL_BUILTIN_SOURCE_DIR} -o ${OUTPUT_DIR}/${OFN} ${CL_BUILTIN_SOURCE_DIR}/${INPUT_DIR}/${IFN}
        DEPENDS ${OCL-TBLGEN-DEP} ${depend_tds}
        COMMENT "Generating ${OFN}"
        )
    set_source_files_properties(${OUTPUT_DIR}/${OFN} PROPERTIES GENERATED 1)
    set(${OUTPUT_VAR} ${OUTPUT_DIR}/${OFN} PARENT_SCOPE)
endfunction(tblgenoclblt)

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

  list(APPEND CMD_ARGS -gen-ocl-bi)
  # generate OCL builtin implementation
  tblgenoclblt(OCL_TABLEGEN_SOURCE
    ${INTERMEDIATE_OUTPUT_DIR}
    ${TARGET_DIR}
    target-${ARCH}.td
    target-${ARCH}.cl
    ${CMD_ARGS}
  )
  list (APPEND OCL_20_SOURCE_FILES ${OCL_TABLEGEN_SOURCE})

  # build .rtl file out of OCL code (by clang) and copy it to install directory
  add_ocl_library( OUTPUT_OCL_LIB IGNORE "${CLANG_CL_COMMON_COMPILER_FLAGS}" ${TARGET_NAME} ${TARGET_DIR} ${ARCH} ${PROC}  FALSE FALSE IGNORE ${INTERMEDIATE_OUTPUT_DIR} "${C_SOURCE_FILES}" ${OCL_20_SOURCE_FILES})

  # build .rtl (and .o) files out of OCL image callbacks (by clang) and copy to install directory
  add_ocl_library( OUTPUT_OCL_IMAGE_CALLBACK_LIB OUTPUT_OCL_IMAGE_CALLBACK_OBJ "${CLANG_CL_CBK_COMPILER_FLAGS}" ${TARGET_NAME}_img_cbk ${TARGET_DIR} ${ARCH} ${PROC} TRUE ${PREBUILD_IMAGE_LIBRARY} ${TARGET_NAME} ${INTERMEDIATE_OUTPUT_DIR} IGNORE ${OCL_IMAGE_CALLBACK_SOURCE_FILE})

  # copy SVML (and SDE) libaries to install directory
  install_svml( ${ARCH} )

  # Unconditional dependent targets
  set (TARGET_DEPENDENCIES ${BUILTINS_OUTPUT_DIR}/${OUTPUT_OCL_LIB} ${BUILTINS_OUTPUT_DIR}/${OUTPUT_OCL_IMAGE_CALLBACK_LIB} ${BUILTINS_OUTPUT_DIR}/${SVML_NAME_PREFIX}${ARCH}${CMAKE_SHARED_LIBRARY_SUFFIX}${ADDITIONAL_SHARED_LIBRARY_SUFFIX})

  # If prebuilt library is built, depend on it so it gets built.
  if( PREBUILD_IMAGE_LIBRARY )
    set (TARGET_DEPENDENCIES ${TARGET_DEPENDENCIES} ${BUILTINS_OUTPUT_DIR}/${OUTPUT_OCL_IMAGE_CALLBACK_OBJ})
  endif( PREBUILD_IMAGE_LIBRARY)

  # top-level target for OCL target
  add_custom_target (${TARGET_NAME} ALL
    DEPENDS ${TARGET_DEPENDENCIES} ${OCL_20_SOURCE_FILES}
    COMMENT "Target ${TARGET_NAME} build completed"
    VERBATIM
    SOURCES ${OCL_20_SOURCE_FILES} ${OCL_IMAGE_CALLBACK_SOURCE_FILE} ${HEADER_FILES}
  )

  set_target_properties (${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/cl_builtins)

  set (INSTALL_LIST ${BUILTINS_OUTPUT_DIR}/${OUTPUT_OCL_LIB} ${BUILTINS_OUTPUT_DIR}/${OUTPUT_OCL_IMAGE_CALLBACK_LIB})
  if (PREBUILD_IMAGE_LIBRARY)
     set (INSTALL_LIST ${INSTALL_LIST} ${BUILTINS_OUTPUT_DIR}/${OUTPUT_OCL_IMAGE_CALLBACK_OBJ})
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
  half.cl
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
add_subdirectory (shared)
if (ENABLE_KNL AND BUILD_X64 AND NOT WIN32)
    # includes AVX-512 F, CD, ER, PF.
  add_subdirectory (avx512phi)
endif()

if(FALSE) #temporarily disabled
  # Configuring and adding the EyeQ builtins
  set(EYEQ_BUILTINS_ARCHS "x86-64" CACHE STRING "Architecture to compile the eyeq builtins for (-march)")
  set(EYEQ_BUILTINS_TARGETS "eyeqemu")
  set(EYEQ_BUILTINS_TARGET_SPECIFIC_ARGS "-D__INTELEYEQ_CL__")
  set(EYEQ_BUILTINS_OUTPUT_DIR ${BUILTINS_OUTPUT_DIR}/eyeq_builtins)
  set(EYEQ_BUILTINS_INSTALL_DIR bin)
  option(EYEQ_BUILTINS_BITCODE_OUTPUT_REQUIRES_LTO "Is bitcode output only supported with LTO" ON)
  option(EYEQ_BUILTINS_OCL_FAKE_ADDRESS_SPACE_MAP_MANGLING "Does ocl bitcode compliation requires fake address space map mangling" ON)
  option(EYEQ_BUILTINS_OCL_FORCE_VECTOR_ABI "Does ocl bitcode compliation requires forcing vector ABI for compatibility" ON)
  add_subdirectory (eyeq_builtins)

  install_to(${EYEQ_BUILTINS_OUTPUT_DIR} DESTINATION lib COMPONENT ocl-eyeq-builtins)
endif()

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
  DEPENDS ${CLANG}

  # Use host triple to let llc produce object file in a native format.
  COMMAND ${CLANG} ${CLANG_CL_COMMON_COMPILER_FLAGS} -triple ${HOST_TRIPLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/pipes.cl -o pipes1.bc

  COMMAND ${CLANG} ${CLANG_CL_COMMON_COMPILER_FLAGS} -triple ${HOST_TRIPLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/pipes-info.cl -o pipes2.bc

  COMMAND ${LLVM_LINK} pipes1.bc pipes2.bc -o pipes.bc

  COMMAND ${OPT} -runtimelib=${RTL_MINIMAL},${RTL_SHARED}
  -builtin-import pipes.bc
  -o pipes-opt.bc

  COMMAND ${LLC} ${LLC_PARAMS} -mcpu=corei7 pipes-opt.bc
  -o clbltpipe.obj)

add_library(clbltpipe STATIC IMPORTED GLOBAL)
add_dependencies(clbltpipe clbltpipe_target)
set_target_properties(clbltpipe
  PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/clbltpipe.obj)

# We have another object for several functions that return pipe size:
# that is because this object file will be linked into oclopt
# (ChannelPipeTransformation pass), but the main pipe object file
# depends on oclopt, resulting in circular dependency:
#
# oclopt -> ChannelPipeTransformation -> clbltpipe.obj
#        -> clbltshared.rtl -> oclopt
#
# Functions that required for ChannelPipeTransformation are not using
# any built-ins, so they are split into separate object file. This
# breaks the circular dependency.
#
add_custom_target(clbltpipeinfo_target
  DEPENDS clbltpipeinfo.obj)

if (OPENCL_INTREE_BUILD)
  add_dependencies(clbltpipeinfo_target common_clang${BUILD_PLATFORM})
endif()

add_custom_command(OUTPUT clbltpipeinfo.obj
  DEPENDS pipes-info.cl pipes-internal.h pipes-defines.h
  DEPENDS ${CLANG}

  # Use host triple to let llc produce object file in a native format.
  COMMAND ${CLANG} ${CLANG_CL_COMMON_COMPILER_FLAGS} -triple ${HOST_TRIPLE}
  ${CMAKE_CURRENT_SOURCE_DIR}/pipes-info.cl -o pipes-info.bc

  COMMAND ${LLC} ${LLC_PARAMS} -mcpu=corei7 pipes-info.bc
  -o clbltpipeinfo.obj)

add_library(clbltpipeinfo STATIC IMPORTED GLOBAL)
add_dependencies(clbltpipeinfo clbltpipeinfo_target)
set_target_properties(clbltpipeinfo
  PROPERTIES
  IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/clbltpipeinfo.obj)
