#--------------------------- Settings ----------------------------------------------------------
#-- All Win32/Win64/Linux32/Linux64-dependent definitions should be placed here

#
#  Variables which override defaults (used only by OCL SDK build)
#      OCL_INSTALL_DIR               overrides output directory. The default is where the rest of OCL CPU BE resides.
#      OCL_CLANG_HEADERS             overrides directory name for clang headers. The default is 'clang_headers' in Volcano root
#     OCL_GLOBAL_INCLUDE         overrides directory name for global headers. The default is 'cl_api' in Volcano root
#     OCL_SOURCE_DIR                 TRUE when invoked by OCL SDK build
#     OCL_HOST_CLANG         overrides 'clang' tool selection. The default is to use 'clang' tool from the same build
#     OCL_HOST_OPT                     overrides 'opt' tool selection. The default is to use 'opt' tool from the same build
#     OCL_HOST_LLVM_LINK         overrides 'llvm-link' tool selection. The default is to use 'llvm-link' tool from the same build
#

#
# Global and platform settings
#
cmake_minimum_required(VERSION 2.8.4)
cmake_policy(SET CMP0001 NEW)
cmake_policy(SET CMP0011 OLD)

project(cl_builtins)

# Only MIC supports this flag so far. Remove this option as well as legacy code
# for OCL builtin support when both CPU and MIC move to the new solution.
option (AUTOGEN_OCL_BUILTIN "Generate OCL builtins" ON)

# target name prefix to be used for generated binaries 
set (TARGET_NAME_PREFIX clbltfn)

if (BUILD_X64)
  # 32 bit
  set (PLATFORM 64)
else ()
  # 64 bit
  set (PLATFORM 32)
endif ()

if (WIN32)
  set (PLATFORM Win${PLATFORM})
elseif(ANDROID)
  set (PLATFORM Android${PLATFORM})
else ()
  set (PLATFORM Linux${PLATFORM})
endif()
message (STATUS "OCL Builtins build: ${PLATFORM}")

add_subdirectory (utils/TableGen)

# Implicit:
#  cl_builtins_SOURCE_DIR - points to the top src directory
#  cl_builtins_BINARY_DIR - points to the build directory

# remember OCL builtins source dir
set (CL_BUILTIN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message (STATUS "Built-ins sources are in: ${CL_BUILTIN_SOURCE_DIR}")

# target directory for OCL .rtl, and .dll/.so of SVML library
set (BUILTINS_INSTALL_DIR ${OCL_TOOLS_BINARY_DIR}/${CMAKE_CFG_INTDIR})
message (STATUS "Built-ins libraries will be in: ${BUILTINS_INSTALL_DIR}")

# global headers
if (DEFINED OCL_CLANG_HEADERS)
  set (CLANG_HEADERS ${OCL_CLANG_HEADERS})
else ()
  set (CLANG_HEADERS ${CL_BUILTIN_SOURCE_DIR}/../../clang_headers)
endif()
if (NOT EXISTS ${CLANG_HEADERS})
  message (FATAL_ERROR "Cannot find Clang headers (${CLANG_HEADERS})!")
endif()
message (STATUS "Clang headers are in: ${CLANG_HEADERS}")

if (DEFINED OCL_GLOBAL_INCLUDE)
  set (IMPORT_DIR ${OCL_GLOBAL_INCLUDE})
else ()
  set (IMPORT_DIR ${BACKEND_ROOT_DIR}/../cl_api )
endif ()
if (NOT EXISTS ${IMPORT_DIR})
  message (FATAL_ERROR "Cannot find OCL headers (${IMPORT_DIR})!")
endif()
message (STATUS "Global headers are in: ${IMPORT_DIR}")

# svml directories
set (SVML_DLL_DIR ${CL_BUILTIN_SOURCE_DIR}/bin/svml/${PLATFORM})
if (NOT EXISTS ${SVML_DLL_DIR})
  message (FATAL_ERROR "Cannot find SVML binary directory (${SVML_DLL_DIR})!")
endif()
set (SVML_NAME_PREFIX __ocl_svml_)
set (SDE_NAME_PREFIX __sde_svml_)
message (STATUS "SVML binary dir: ${SVML_DLL_DIR}")

if( CMAKE_CROSSCOMPILING OR LLVM_USE_NATIVE OR ANDROID )
  set(TOOL_BINARY_DIR "${LLVM_BINARY_DIR}/../native/bin")
else()
  set(TOOL_BINARY_DIR "${LLVM_BINARY_DIR}" )
endif()

# LLVM tools are assumed to be pre-built within the global project
if (NOT DEFINED OCL_HOST_CLANG) 
  set (CLANG "${TOOL_BINARY_DIR}/clang")
else()
  set (CLANG OCL_HOST_CLANG)
endif()
if (NOT DEFINED OCL_HOST_OPT)
  if(CMAKE_CROSSCOMPILING OR LLVM_USE_NATIVE OR ANDROID) 
    set (OPT ${CMAKE_BINARY_DIR}/native/bin/oclopt)
    set (OPT_DEP oclopt_native)
    set (OCL-TBLGEN ${CMAKE_BINARY_DIR}/native/bin/ocl-tblgen)
    set (OCL-TBLGEN-DEP ocl-tblgen-native)
  else()
    set (OPT oclopt)
    set (OPT_DEP oclopt)
    set (OCL-TBLGEN ocl-tblgen)
    set (OCL-TBLGEN-DEP ocl-tblgen)
  endif()
else ()
  set (LLVM_OPT OCL_HOST_OPT)
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

# LLVM triple is per OS and Arch (keep consistent with SetTargetTriple() in Compiler.cpp)
if (WIN32)    
  if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set (TRIPLE i686-pc-win32-elf)
  else ()
    set (TRIPLE x86_64-pc-win32-elf) 
  endif (CMAKE_SIZEOF_VOID_P EQUAL 4)    
else (WIN32)
  if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set    (TRIPLE i686-pc-Linux)
  else ()
    set (TRIPLE amd64)
  endif (CMAKE_SIZEOF_VOID_P EQUAL 4) 
endif (WIN32)

#
# external tools
#

message (STATUS "CLANG:     ${CLANG}")
message (STATUS "LLVM_OPT:  ${LLVM_OPT}")
message (STATUS "LLVM_LINK: ${LLVM_LINK}")
message (STATUS "LLC:       ${LLC}")
message (STATUS "TRIPLE:    ${TRIPLE}")

# 
# Tool environment and options
#

# clang
set (OCL_OUTPUT_EXTENSION .rtl)
set (OCL_PRECOMPILED_OUTPUT_EXTENSION .o)

set (CLANG_COMPILER_PARAMS 
  -cc1 -triple ${TRIPLE} -emit-llvm-bc -O3 -opencl-builtins -fwrapv
  -x cl -I ${IMPORT_DIR} -I ${CLANG_HEADERS} -I ${CL_BUILTIN_SOURCE_DIR} -include opencl_.h
  )

# llvm-opt
set (LLVM_OPT_PARAMS -shuffle-call-to-inst -scalarrepl -functionattrs)

# images optimization options
set (LLVM_IMAGES_OPT_PARAMS -O3)

# llvm-link parameters
set (LLVM_LINK_PARAMS -f)

# llc parameters
set (LLC_PARAMS -filetype=obj)
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
  set (LLC_PARAMS ${LLC_PARAMS} -code-model=default -relocation-model=static)
else (CMAKE_SIZEOF_VOID_P EQUAL 4)
  set (LLC_PARAMS ${LLC_PARAMS} -code-model=large -relocation-model=pic)
endif (CMAKE_SIZEOF_VOID_P EQUAL 4)
#------------------- Utility functions ---------------------------

#
# Compile single OpenCL source file
#
# compile_ocl( OUTPUT_FILE_VAR PROC INPUT_FILE OUTPUT_DIR )
#
#   OUTPUT_FILE_VAR - output, contains full path to object file
#   PROC            - target CPU
#   INPUT_FILE      - full path to input file
#   OUTPUT_DIR      - directory for output file
#

function( compile_ocl OUTPUT_FILE_VAR PROC INPUT_FILE OUTPUT_DIR )

  get_filename_component (NAME ${INPUT_FILE} NAME_WE )
  set (OUTPUT_FILE ${NAME}${OCL_OUTPUT_EXTENSION} )

  add_custom_command (OUTPUT ${OUTPUT_DIR}/${OUTPUT_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
    COMMAND ${CLANG} ${CLANG_COMPILER_PARAMS} -target-cpu ${PROC} -o ${OUTPUT_DIR}/${OUTPUT_FILE} ${INPUT_FILE}
    COMMAND ${OPT}  ${LLVM_OPT_PARAMS} -o ${OUTPUT_DIR}/${OUTPUT_FILE} ${OUTPUT_DIR}/${OUTPUT_FILE}
    MAIN_DEPENDENCY ${INPUT_FILE}
    DEPENDS ${HEADER_FILES} ${OPT_DEP}
    IMPLICIT_DEPENDS CXX ${INPUT_FILE}
    WORKING_DIRECTORY ${CL_BUILTIN_SOURCE_DIR}
    COMMENT "OCL Compiling ${INPUT_FILE}: ${CLANG} ${CLANG_COMPILER_PARAMS} -target-cpu ${PROC} -o ${OUTPUT_DIR}/${OUTPUT_FILE} ${INPUT_FILE}"
    COMMENT "OCL Optimize  ${OUTPUT_FILE}: ${OPT}  ${LLVM_OPT_PARAMS} -o ${OUTPUT_DIR}/${OUTPUT_FILE} ${OUTPUT_DIR}/${OUTPUT_FILE}"
    VERBATIM
    )

  set (${OUTPUT_FILE_VAR} ${OUTPUT_DIR}/${OUTPUT_FILE} PARENT_SCOPE )

endfunction( compile_ocl )

#
# add OpenCL library built from set of OpenCL source files
#
# add_ocl_library( OUTPUT_FILE_VAR TARGET_NAME ARCH PROC USE_OPT OUTPUT_DIR ...FILES... )
#
#   OUTPUT_FILE_VAR   - output, contains generated library (rtl file)
#   PRECOMPILED_OUTPUT_FILE_VAR - output, contains generated library (obj file)
#   TARGET_NAME       - name of the target and base name of the lib. Extension is OCL_OUTPUT_EXTENSION
#   TARGET_DIR        - directory of target-*.td file
#   ARCH              - target SIMD arch type
#   PROC              - target CPU
#   USE_OPT           - Optimize output library with llvm opt tool
#   ENABLE_PRECOMPILE - enable precompiling (to native machine code)
#   OUTPUT_DIR        - output directory for the intermediate files
#   FILES             - input files list
#

function( add_ocl_library OUTPUT_FILE_VAR PRECOMPILED_OUTPUT_FILE_VAR TARGET_NAME TARGET_DIR ARCH PROC USE_OPT ENABLE_PRECOMPILE BI_LIB_NAME OUTPUT_DIR)

  if (${ARGC} LESS 10)
    message (FATAL_ERROR "add_ocl_library(${TARGET_NAME}) - no sources given")
  endif ()

  set (OUTPUT_FILE ${TARGET_NAME}${OCL_OUTPUT_EXTENSION})
  set (PRECOMPILED_OUTPUT_FILE ${TARGET_NAME}${OCL_PRECOMPILED_OUTPUT_EXTENSION})

  # Compile all files and prepare a 'TMP_FILES' list of .rtl files
  foreach (FILE ${ARGN})
    compile_ocl( TMP_RTL ${PROC} ${FILE} ${OUTPUT_DIR})
    list (APPEND TMP_FILES ${TMP_RTL})
  endforeach (FILE)

  if(${USE_OPT})
  # link OpenCL library (adding also BI IR for vectorizer)
    add_custom_command (OUTPUT ${OUTPUT_FILE}_noopt
      COMMENT "OCL Linking -o ${OUTPUT_FILE}_noopt"
      COMMAND ${LLVM_LINK} ${LLVM_LINK_PARAMS} -o ${OUTPUT_FILE}_noopt ${TMP_FILES} ${CL_BUILTIN_SOURCE_DIR}/${TARGET_DIR}/${ARCH}_vectorizer.ll ${CL_BUILTIN_SOURCE_DIR}/GENERIC/ll_intrinsics.ll
      DEPENDS ${LLVM_LINK} ${TMP_FILES} ${CL_BUILTIN_SOURCE_DIR}/${TARGET_DIR}/${ARCH}_vectorizer.ll
      VERBATIM
      )
    add_custom_command(OUTPUT ${BUILTINS_INSTALL_DIR}/${OUTPUT_FILE}
      COMMENT "Opt optimizing ${OUTPUT_FILE}_noopt"
      COMMAND ${OPT} ${OUTPUT_FILE}_noopt ${LLVM_IMAGES_OPT_PARAMS} -builtins-module=${BUILTINS_INSTALL_DIR}/${BI_LIB_NAME}${OCL_OUTPUT_EXTENSION} -o ${BUILTINS_INSTALL_DIR}/${OUTPUT_FILE}
      DEPENDS ${OPT_DEP} ${OUTPUT_FILE}_noopt ${BUILTINS_INSTALL_DIR}/${BI_LIB_NAME}${OCL_OUTPUT_EXTENSION}
      VERBATIM
      )
  else()
    add_custom_command (OUTPUT ${BUILTINS_INSTALL_DIR}/${OUTPUT_FILE}
      COMMENT "OCL Linking ${BUILTINS_INSTALL_DIR}/${OUTPUT_FILE}"
      COMMAND ${LLVM_LINK} ${LLVM_LINK_PARAMS} -o ${BUILTINS_INSTALL_DIR}/${OUTPUT_FILE} ${TMP_FILES} ${CL_BUILTIN_SOURCE_DIR}/${TARGET_DIR}/${ARCH}_vectorizer.ll ${CL_BUILTIN_SOURCE_DIR}/GENERIC/ll_intrinsics.ll
      DEPENDS ${LLVM_LINK} ${TMP_FILES} ${CL_BUILTIN_SOURCE_DIR}/${TARGET_DIR}/${ARCH}_vectorizer.ll
      VERBATIM
      )
  endif()

  # compile the rtl file to native objects
  if(${ENABLE_PRECOMPILE})
    message(STATUS "Building ${BUILTINS_INSTALL_DIR}/${PRECOMPILED_OUTPUT_FILE}")

    # Initialize the llc parameter "-mattr" based on PROC
    SET (MATTR "-mattr=")
    if (${PROC} MATCHES ".*avx2$")
      set (MATTR "${MATTR}+avx2")
    else()
      if (${PROC} MATCHES ".*avx$")
        set (MATTR "${MATTR}+avx")
      endif ()
    endif ()

    add_custom_command(OUTPUT ${BUILTINS_INSTALL_DIR}/${PRECOMPILED_OUTPUT_FILE}
      COMMENT "OCL Building Static Object ${PRECOMPILED_OUTPUT_FILE}"
      COMMAND ${LLC} ${LLC_PARAMS} ${MATTR} -mcpu=${PROC} -o ${BUILTINS_INSTALL_DIR}/${PRECOMPILED_OUTPUT_FILE} < ${BUILTINS_INSTALL_DIR}/${OUTPUT_FILE}
      DEPENDS ${LLC} ${BUILTINS_INSTALL_DIR}/${OUTPUT_FILE}
      VERBATIM
      )
  else (${ENABLE_PRECOMPILE})
    message (STATUS "Not pre-compiling library, outputting direct ${OUTPUT_FILE}")
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

  set(IS_MIC OFF)
  if(ARCH STREQUAL b1 OR ARCH STREQUAL b2)
    set(IS_MIC ON)
    if(ARCH STREQUAL b2)
      set(ADDITIONAL_SHARED_LIBRARY_SUFFIX .3.0)
    endif()
  endif()
  if(ENABLE_SDE AND IS_MIC)
    set(SDE_NAME_PREFIX ${SDE_NAME_PREFIX})
    set (SDE_DLL ${SVML_DLL_DIR}/${SDE_NAME_PREFIX}${ARCH}${CMAKE_SHARED_LIBRARY_SUFFIX}${ADDITIONAL_SHARED_LIBRARY_SUFFIX})
  endif ()

  # SVML binary
  set (SVML_DLL ${SVML_DLL_DIR}/${SVML_NAME_PREFIX}${ARCH}${CMAKE_SHARED_LIBRARY_SUFFIX}${ADDITIONAL_SHARED_LIBRARY_SUFFIX})
  if (NOT EXISTS ${SVML_DLL})
    message( FATAL_ERROR "Cannot find ${SVML_DLL}!")
  endif()

  # install SVML (and SDE) libraries
  if(ENABLE_SDE AND IS_MIC)
    add_custom_command (OUTPUT ${BUILTINS_INSTALL_DIR}/${SDE_NAME_PREFIX}${ARCH}${CMAKE_SHARED_LIBRARY_SUFFIX}${ADDITIONAL_SHARED_LIBRARY_SUFFIX}
      COMMAND ${CMAKE_COMMAND} -E copy ${SDE_DLL} ${BUILTINS_INSTALL_DIR} 
      VERBATIM
    )
  endif()
  add_custom_command (OUTPUT ${BUILTINS_INSTALL_DIR}/${SVML_NAME_PREFIX}${ARCH}${CMAKE_SHARED_LIBRARY_SUFFIX}${ADDITIONAL_SHARED_LIBRARY_SUFFIX}
    COMMAND ${CMAKE_COMMAND} -E copy ${SVML_DLL} ${BUILTINS_INSTALL_DIR} 
    VERBATIM
  )
  
  install( FILES ${SVML_DLL} DESTINATION bin )
  
  if(ENABLE_SDE AND IS_MIC)
    install( FILES ${SDE_DLL} DESTINATION bin )
  endif()
endfunction( install_svml )

function(tblgenoclblt OUTPUT_VAR OUTPUT_DIR INPUT_DIR IFN)
    string(REPLACE ".td" ".cl" OFN ${IFN})
    file(GLOB_RECURSE depend_tds "${CL_BUILTIN_SOURCE_DIR}/*.td")
    add_custom_command(
        OUTPUT ${OUTPUT_DIR}/${OFN}
        COMMAND ${OCL-TBLGEN} ${ARGN} -I=${CL_BUILTIN_SOURCE_DIR} -o ${OUTPUT_DIR}/${OFN} ${CL_BUILTIN_SOURCE_DIR}/${INPUT_DIR}/${IFN}
        DEPENDS ${OCL-TBLGEN-DEP} ${depend_tds}
        COMMENT "Generating ${OFN}..."
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
  if(ARCH STREQUAL b2)
    set(ADDITIONAL_SHARED_LIBRARY_SUFFIX .3.0)
  endif()

  message (STATUS "Building target:   ${TARGET_NAME}")

  # intermediate directory for OpenCL compilation/link
  set (INTERMEDIATE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR})
  
  # generate OCL builtin implementation
	tblgenoclblt(OCL_TABLEGEN_SOURCE ${INTERMEDIATE_OUTPUT_DIR} ${TARGET_DIR} target-${ARCH}.td -gen-ocl-bi)
    list (APPEND OCL_SOURCE_FILES ${OCL_TABLEGEN_SOURCE})
    
  # build .rtl file out of OCL code (by clang) and copy it to install directory
  add_ocl_library( OUTPUT_OCL_LIB IGNORE ${TARGET_NAME} ${TARGET_DIR} ${ARCH} ${PROC}  FALSE FALSE IGNORE ${INTERMEDIATE_OUTPUT_DIR} ${OCL_SOURCE_FILES})

  # build .rtl (and .o) files out of OCL image callbacks (by clang) and copy to install directory
  add_ocl_library( OUTPUT_OCL_IMAGE_CALLBACK_LIB OUTPUT_OCL_IMAGE_CALLBACK_OBJ ${TARGET_NAME}_img_cbk ${TARGET_DIR} ${ARCH} ${PROC} TRUE ${PREBUILD_IMAGE_LIBRARY} ${TARGET_NAME} ${INTERMEDIATE_OUTPUT_DIR} ${OCL_IMAGE_CALLBACK_SOURCE_FILE})

  # copy SVML (and SDE) libaries to install directory
  install_svml( ${ARCH} )

  # Unconditional dependent targets
  set (TARGET_DEPENDENCIES ${BUILTINS_INSTALL_DIR}/${OUTPUT_OCL_LIB} ${BUILTINS_INSTALL_DIR}/${OUTPUT_OCL_IMAGE_CALLBACK_LIB} ${BUILTINS_INSTALL_DIR}/${SVML_NAME_PREFIX}${ARCH}${CMAKE_SHARED_LIBRARY_SUFFIX}${ADDITIONAL_SHARED_LIBRARY_SUFFIX})

  # If prebuilt library is built, depend on it so it gets built.
  if( PREBUILD_IMAGE_LIBRARY )
    set (TARGET_DEPENDENCIES ${TARGET_DEPENDENCIES} ${BUILTINS_INSTALL_DIR}/${OUTPUT_OCL_IMAGE_CALLBACK_OBJ})
  endif( PREBUILD_IMAGE_LIBRARY)

  # top-level target for OCL target
  add_custom_target (${TARGET_NAME} ALL
    DEPENDS ${TARGET_DEPENDENCIES}
    COMMENT "Target ${TARGET_NAME} build completed"
    VERBATIM
    SOURCES ${OCL_SOURCE_FILES} ${OCL_IMAGE_CALLBACK_SOURCE_FILE} ${HEADER_FILES}
  )

  set_target_properties (${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/cl_builtins)

  if (WIN32)
    set (INSTALL_LIST ${OCL_TOOLS_BINARY_DIR}/\${BUILD_TYPE}/${OUTPUT_OCL_LIB} ${OCL_TOOLS_BINARY_DIR}/\${BUILD_TYPE}/${OUTPUT_OCL_IMAGE_CALLBACK_LIB})
    if( PREBUILD_IMAGE_LIBRARY)
      set (INSTALL_LIST ${INSTALL_LIST} ${OCL_TOOLS_BINARY_DIR}/\${BUILD_TYPE}/${OUTPUT_OCL_IMAGE_CALLBACK_OBJ})
    endif( PREBUILD_IMAGE_LIBRARY)
    install( FILES ${INSTALL_LIST} DESTINATION bin)
  else(WIN32)
    set (INSTALL_LIST ${OCL_TOOLS_BINARY_DIR}/${OUTPUT_OCL_LIB} ${OCL_TOOLS_BINARY_DIR}/${OUTPUT_OCL_IMAGE_CALLBACK_LIB} )
    if( PREBUILD_IMAGE_LIBRARY)
      set (INSTALL_LIST ${INSTALL_LIST} ${OCL_TOOLS_BINARY_DIR}/${OUTPUT_OCL_IMAGE_CALLBACK_OBJ})
    endif( PREBUILD_IMAGE_LIBRARY)
    install( FILES ${INSTALL_LIST} DESTINATION bin)
  endif(WIN32)

endfunction( create_target)

#------------------ Main -----------------------

# OpenCL source code
list (APPEND OCL_SOURCE_FILES
  image_functions.cpp
  masked_load_store_functions.cpp
  transpose_functions.cpp
  )


list (APPEND OCL_IMAGE_CALLBACK_SOURCE_FILE
  ${CL_BUILTIN_SOURCE_DIR}/image_callback_functions.cpp
  )

# project header files
list (APPEND HEADER_FILES
  cl_image_declaration.h
  masked_load_store_functions.h
  transpose_functions.h
  )

# convert everything to full paths
set (SAVE_OCL_SOURCE_FILES ${OCL_SOURCE_FILES})
unset (OCL_SOURCE_FILES)
foreach (FILE ${SAVE_OCL_SOURCE_FILES})
  list (APPEND OCL_SOURCE_FILES ${CL_BUILTIN_SOURCE_DIR}/${FILE})
endforeach (FILE)

set (SAVE_HEADER_FILES ${HEADER_FILES})
unset (HEADER_FILES)
foreach (FILE ${SAVE_HEADER_FILES})
  list (APPEND HEADER_FILES ${CL_BUILTIN_SOURCE_DIR}/${FILE})
endforeach (FILE)

# running build for all supported SIMD architectures 
if( INCLUDE_ATOM )
  add_subdirectory (ssse3)
else( INCLUDE_ATOM )
  if( WIN32 AND NOT BUILD_X64 )
    # no corresponding SVML for Win64; hence
    # we aren't building ssse3 for Win64
    add_subdirectory (ssse3)
  endif( WIN32 AND NOT BUILD_X64 )
  add_subdirectory (sse42)
  add_subdirectory (avx)
  add_subdirectory (avx2)
endif( INCLUDE_ATOM )

if (INCLUDE_MIC_DEVICE)
  add_subdirectory (KNCni)
endif()

