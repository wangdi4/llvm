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

set(__MAKEFUNCS_INCLUDED__ 1)

#
# Usage FILTER_SOURCES()
#
# Filters HEADER_FILES_1, EXPORT_HEADER_FILES_1 and SOURCE_FILES_1 variables to
# remove Win or Linux files Creates HEADER_FILES, EXPORT_HEADER_FILES and
# SOURCE_FILES variables
#
macro(FILTER_SOURCES FILER_OUT_REGEXP)
  list(APPEND HEADER_FILES_1 ${EXPORT_HEADER_FILES_1})

  if((WIN32) AND (NOT FORCE_LINUX))
    foreach(FILE ${HEADER_FILES_1})
      if(NOT FILE MATCHES _linux AND NOT FILE MATCHES ${FILER_OUT_REGEXP})
        list(APPEND HEADER_FILES_2 ${FILE})
      endif(NOT FILE MATCHES _linux AND NOT FILE MATCHES ${FILER_OUT_REGEXP})
    endforeach(FILE)

    foreach(FILE ${SOURCE_FILES_1})
      if(NOT FILE MATCHES ^stdafx[.]c.*$|_linux AND NOT FILE MATCHES
                                                    ${FILER_OUT_REGEXP})
        list(APPEND SOURCE_FILES_2 ${FILE})
      endif(NOT FILE MATCHES ^stdafx[.]c.*$|_linux AND NOT FILE MATCHES
                                                       ${FILER_OUT_REGEXP})
    endforeach(FILE)

    file(
      GLOB RECOURCE_FILES_1
      RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
      *.rc *.def)
    list(APPEND SOURCE_FILES_2 ${RECOURCE_FILES_1})

  else() # Linux

    foreach(FILE ${HEADER_FILES_1})
      if(NOT FILE MATCHES ^resource[.]h$|^stdafx[.]h$|_windows|_win32
         AND NOT FILE MATCHES ${FILER_OUT_REGEXP})
        list(APPEND HEADER_FILES_2 ${FILE})
      endif(NOT FILE MATCHES ^resource[.]h$|^stdafx[.]h$|_windows|_win32
            AND NOT FILE MATCHES ${FILER_OUT_REGEXP})
    endforeach(FILE)

    foreach(FILE ${SOURCE_FILES_1})
      if(NOT FILE MATCHES ^stdafx[.]c.*$|_windows|_win32
         AND NOT FILE MATCHES ${FILER_OUT_REGEXP})
        list(APPEND SOURCE_FILES_2 ${FILE})
      endif(NOT FILE MATCHES ^stdafx[.]c.*$|_windows|_win32
            AND NOT FILE MATCHES ${FILER_OUT_REGEXP})
    endforeach(FILE)

  endif((WIN32) AND (NOT FORCE_LINUX))

  if(DEFINED HEADER_FILES_2)
    set(HEADER_FILES
        ${HEADER_FILES_2}
        PARENT_SCOPE)
  endif(DEFINED HEADER_FILES_2)

  if(DEFINED SOURCE_FILES_2)
    set(SOURCE_FILES
        ${SOURCE_FILES_2}
        PARENT_SCOPE)
  endif(DEFINED SOURCE_FILES_2)

  if(DEFINED EXPORT_HEADER_FILES_1)
    set(EXPORT_HEADER_FILES
        ${EXPORT_HEADER_FILES_1}
        PARENT_SCOPE)
    include_directories(BEFORE export)
  endif(DEFINED EXPORT_HEADER_FILES_1)

endmacro(FILTER_SOURCES)

#
# Usage FIND_SOURCES( optional FILER_OUT_REGEXP )
#
# Creates HEADER_FILES, EXPORT_HEADER_FILES and SOURCE_FILES variables by
# globbing CMAKE_CURRENT_SOURCE_DIR
#
function(FIND_SOURCES)
  file(
    GLOB HEADER_FILES_1
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.h *.hpp)
  file(
    GLOB EXPORT_HEADER_FILES_1
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    export/*.h export/*.hpp)
  file(
    GLOB SOURCE_FILES_1
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.c *.cpp *.cc)

  if(${ARGC} EQUAL 0)
    set(FILER_OUT_REGEXP "not used regexp")
  else()
    set(FILER_OUT_REGEXP ${ARGV0})
  endif(${ARGC} EQUAL 0)

  filter_sources(${FILER_OUT_REGEXP})
endfunction(FIND_SOURCES)

#
# Usage FIND_SOURCES_EX( SOURCE_DIRECTORY optional FILER_OUT_REGEXP )
#
# Creates HEADER_FILES, EXPORT_HEADER_FILES and SOURCE_FILES variables by
# globbing SOURCE_DIRECTORY
#
function(FIND_SOURCES_EX SOURCE_DIRECTORY)
  file(
    GLOB HEADER_FILES_1
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${SOURCE_DIRECTORY}/*.h ${SOURCE_DIRECTORY}/*.hpp)
  file(
    GLOB EXPORT_HEADER_FILES_1
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${SOURCE_DIRECTORY}/export/*.h ${SOURCE_DIRECTORY}/export/*.hpp)
  file(
    GLOB SOURCE_FILES_1
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${SOURCE_DIRECTORY}/*.c ${SOURCE_DIRECTORY}/*.cpp ${SOURCE_DIRECTORY}/*.cc)

  if(${ARGC} EQUAL 1)
    set(FILER_OUT_REGEXP "not used regexp")
  else()
    set(FILER_OUT_REGEXP ${ARGV1})
  endif(${ARGC} EQUAL 1)

  filter_sources(${FILER_OUT_REGEXP})
endfunction(FIND_SOURCES_EX)

#
# Usage FIND_SOURCES_IN_FOLDER( optional FOLDER_ARG optional FILER_OUT_REGEXP )
#
# Creates HEADER_FILES, EXPORT_HEADER_FILES and SOURCE_FILES variables by
# globbing CMAKE_CURRENT_SOURCE_DIR
#
function(FIND_SOURCES_IN_FOLDER)
  if(${ARGC} EQUAL 0)
    set(FOLDER_ARG "not used folder")
  else()
    set(FOLDER_ARG ${ARGV0})
  endif(${ARGC} EQUAL 0)

  file(GLOB HEADER_FILES_1 ${FOLDER_ARG}/*.h ${FOLDER_ARG}/*.hpp)
  file(GLOB EXPORT_HEADER_FILES_1 ${FOLDER_ARG}/export/*.h
       ${FOLDER_ARG}/export/*.hpp)
  file(GLOB SOURCE_FILES_1 ${FOLDER_ARG}/*.c ${FOLDER_ARG}/*.cpp
       ${FOLDER_ARG}/*.cc)

  if(${ARGC} EQUAL 1 OR ${ARGC} EQUAL 0)
    set(FILER_OUT_REGEXP "not used regexp")
  else()
    set(FILER_OUT_REGEXP ${ARGV1})
  endif(${ARGC} EQUAL 1 OR ${ARGC} EQUAL 0)

  filter_sources(${FILER_OUT_REGEXP})
endfunction(FIND_SOURCES_IN_FOLDER)

#
# Usage FIND_SOURCES_RECURSE( optional FILER_OUT_REGEXP )
#
# Creates HEADER_FILES and SOURCE_FILES variables by globbing
# CMAKE_CURRENT_SOURCE_DIR recursively
#
function(FIND_SOURCES_RECURSE)
  file(
    GLOB_RECURSE HEADER_FILES_1
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.h *.hpp)
  file(
    GLOB_RECURSE SOURCE_FILES_1
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.c *.cpp *.cc)

  if(${ARGC} EQUAL 0)
    set(FILER_OUT_REGEXP "not used regexp")
  else()
    set(FILER_OUT_REGEXP ${ARGV0})
  endif(${ARGC} EQUAL 0)

  filter_sources(${FILER_OUT_REGEXP})
endfunction(FIND_SOURCES_RECURSE)

#
# Usage CALCULATE_TARGET_SOURCES()
#
# Uses HEADER_FILES, EXPORT_HEADER_FILES and SOURCE_FILES variables and creates
# TARGET_SOURCES
#
function(CALCULATE_TARGET_SOURCES)
  if((WIN32) AND (NOT FORCE_LINUX))
    set(TARGET_SOURCES_1 ${HEADER_FILES} ${SOURCE_FILES})

    source_group("Header Files" FILES ${HEADER_FILES})
    source_group("Source Files" FILES ${SOURCE_FILES})

    foreach(FILE ${HEADER_FILES} ${SOURCE_FILES})
      if(FILE MATCHES ^.*[.]rc$|^resource[.]h$)
        source_group("Resource Files" FILES ${FILE})
      elseif(FILE MATCHES ^stdafx[.]|[.]def$)
        source_group("Extra Files" FILES ${FILE})
      elseif(FILE MATCHES "^(.+)/[^/]+$")
        source_group(${CMAKE_MATCH_1} FILES ${FILE})
      endif(FILE MATCHES ^.*[.]rc$|^resource[.]h$)
    endforeach(FILE)

    if(DEFINED EXPORT_HEADER_FILES)
      source_group("Export Header Files" FILES ${EXPORT_HEADER_FILES})
    endif(DEFINED EXPORT_HEADER_FILES)

  else()
    set(TARGET_SOURCES_1 ${SOURCE_FILES})
  endif((WIN32) AND (NOT FORCE_LINUX))

  if(DEFINED TARGET_SOURCES_1)
    set(TARGET_SOURCES
        ${TARGET_SOURCES_1}
        PARENT_SCOPE)
  endif(DEFINED TARGET_SOURCES_1)

endfunction(CALCULATE_TARGET_SOURCES)

#
# Usage SET_LINUX_EXPORTS_FILE( TARGET_NAME_ FILE_NAME )
#
function(SET_LINUX_EXPORTS_FILE TARGET_NAME_ FILE_NAME)
  if((NOT WIN32) OR (FORCE_LINUX))
    get_target_property(SOURCE_FILES ${TARGET_NAME_} SOURCES)
    list(GET SOURCE_FILES 0 FIRST_SOURCE)
    set_source_files_properties(
      ${FIRST_SOURCE} PROPERTIES OBJECT_DEPENDS
                                 ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME})
    set_property(
      TARGET ${TARGET_NAME_}
      APPEND_STRING
      PROPERTY
        LINK_FLAGS
        "-Wl,-Bsymbolic -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME}"
    )
  endif((NOT WIN32) OR (FORCE_LINUX))
endfunction(SET_LINUX_EXPORTS_FILE)

#
# Usage CREATE_ASM_RULES( <ADD_TO_SOURCES_LIST_VAR> ...<asm-files-list>...)
#
# Creates ADD_TO_SOURCES_LIST_VAR that should be added to the source files list
#
macro(CREATE_ASM_RULES ADD_TO_SOURCES_LIST_VAR)

  if(${ARGC} GREATER 1)
    foreach(FILE ${ARGN})
      get_filename_component(FILE_NAME ${FILE} NAME_WE)
      set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR})
      set(OBJ_FILE ${BIN_DIR}/${FILE_NAME}${CMAKE_C_OUTPUT_EXTENSION})
      set(SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${FILE})

      if(CMAKE_ASM_OUTPUT_NAME_FLAG)
        set(OBJ_OUTPUT_NAME_FLAG ${CMAKE_ASM_OUTPUT_NAME_FLAG}
                                 ${FILE_NAME}${CMAKE_C_OUTPUT_EXTENSION})
      endif(CMAKE_ASM_OUTPUT_NAME_FLAG)

      if(WIN32)
        # CMAKE_ASM_FLAGS is a list on Windows, so we can use it with
        # add_custom_command( VERBATIM )
        set(ASM_FLAGS_LIST ${CMAKE_ASM_FLAGS})
      else()
        # Transform string CMAKE_ASM_FLAGS into a list to work properly with
        # add_custom_command( VERBATIM )
        separate_arguments(ASM_FLAGS_LIST UNIX_COMMAND ${CMAKE_ASM_FLAGS})
      endif()

      add_custom_command(
        OUTPUT ${OBJ_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_DIR}
        COMMAND
          ${CMAKE_COMMAND} -E chdir ${BIN_DIR} ${OPENCL_ASM_COMPILER}
          ${ASM_FLAGS_LIST} ${CMAKE_ASM_INCLUDE_DIR_FLAG}
          ${CMAKE_CURRENT_SOURCE_DIR} ${OBJ_OUTPUT_NAME_FLAG}
          ${CMAKE_ASM_COMPILE_TO_OBJ_FLAG} ${SRC_FILE}
        MAIN_DEPENDENCY ${SRC_FILE}
        VERBATIM)

      list(APPEND OBJ_FILES ${OBJ_FILE})
      list(APPEND SRC_FILES ${SRC_FILE})
    endforeach(FILE)
  endif(${ARGC} GREATER 1)

  if(DEFINED OBJ_FILES)
    set(${ADD_TO_SOURCES_LIST_VAR} ${SRC_FILES} ${OBJ_FILES})
    set_source_files_properties(${OBJ_FILES} PROPERTIES GENERATED TRUE
                                                        EXTERNAL_OBJECT TRUE)
  endif(DEFINED OBJ_FILES)

endmacro(CREATE_ASM_RULES)

#
# Usage: SET_UNICODE_ON() SET_UNICODE_OFF()
#
macro(SET_UNICODE_ON)
  add_definitions(-D_UNICODE -DUNICODE)
endmacro(SET_UNICODE_ON)

macro(SET_UNICODE_OFF)
  remove_definitions(-D_UNICODE -DUNICODE)
endmacro(SET_UNICODE_OFF)

#
# Usage: PRINT_TOOLS()
#
function(PRINT_TOOLS)
  message(STATUS CMAKE_C_FLAGS " - " ${CMAKE_C_FLAGS})
  message(STATUS CMAKE_C_FLAGS_DEBUG " - " ${CMAKE_C_FLAGS_DEBUG})
  message(STATUS CMAKE_C_FLAGS_RELEASE " - " ${CMAKE_C_FLAGS_RELEASE})
  message(STATUS CMAKE_CXX_FLAGS " - " ${CMAKE_CXX_FLAGS})
  message(STATUS CMAKE_CXX_FLAGS_DEBUG " - " ${CMAKE_CXX_FLAGS_DEBUG})
  message(STATUS CMAKE_CXX_FLAGS_RELEASE " - " ${CMAKE_CXX_FLAGS_RELEASE})
  message(STATUS CMAKE_EXE_LINKER_FLAGS " - " ${CMAKE_EXE_LINKER_FLAGS})
  message(STATUS CMAKE_EXE_LINKER_FLAGS_DEBUG " - "
                 ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
  message(STATUS CMAKE_EXE_LINKER_FLAGS_RELEASE " - "
                 ${CMAKE_EXE_LINKER_FLAGS_RELEASE})
  message(STATUS CMAKE_SHARED_LINKER_FLAGS " - " ${CMAKE_SHARED_LINKER_FLAGS})
  message(STATUS CMAKE_SHARED_LINKER_FLAGS_DEBUG " - "
                 ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
  message(STATUS CMAKE_SHARED_LINKER_FLAGS_RELEASE " - "
                 ${CMAKE_SHARED_LINKER_FLAGS_RELEASE})
endfunction(PRINT_TOOLS)

#
# Removes the given path from the PATH environment Usage: remove_path( PATH )
#
function(remove_path PATHARG)
  foreach(PATH $ENV{PATH})
    string(TOUPPER ${PATHARG} UPATHARG)
    string(TOUPPER ${PATH} UPATH)
    if(NOT "${UPATH}" STREQUAL "")
      string(COMPARE NOTEQUAL ${UPATH} ${UPATHARG} PATH_NOTFOUND)
      if(PATH_NOTFOUND)
        set(NEW_PATH ${NEW_PATH} ${PATH})
      endif()
    endif()
  endforeach(PATH)
  set(ENV{PATH} "${NEW_PATH}")
endfunction(remove_path)

#
# Adds the given path to the PATH environment Usage: add_path( PATH )
#
function(add_path PATHARG)
  if(NOT WIN32)
    set(PATH_DELIMITER ":")
  endif(NOT WIN32)

  set(NEW_PATH ${PATHARG}${PATH_DELIMITER} $ENV{PATH})
  set(ENV{PATH} "${NEW_PATH}")
endfunction(add_path)

#
# Replaces a compiler option or switch `old' in `var' by `new'. If `old' is not
# in `var', appends `new' to `var'. If the option already is on the variable,
# don't add it Usage: llvm_replace_compiler_option(CMAKE_CXX_FLAGS_RELEASE "-O3"
# "-O2")
#
function(ocl_replace_compiler_option var old new)
  if("${${var}}" MATCHES "(^| )${new}($| )")
    set(n "")
  else()
    set(n "${new}")
  endif()
  if("${${var}}" MATCHES "(^| )${old}($| )")
    string(REGEX REPLACE "(^| )${old}($| )" " ${n} " ${var} "${${var}}")
  else()
    set(${var} "${${var}} ${n}")
  endif()
  set(${var}
      "${${var}}"
      PARENT_SCOPE)
endfunction(ocl_replace_compiler_option)

#
# Replaces compiler options to dynamic linking
#
function(ocl_replace_compiler_option_to_dynamic)
  ocl_replace_compiler_option(CMAKE_C_FLAGS_RELEASE "/MT" "/MD")
  ocl_replace_compiler_option(CMAKE_C_FLAGS_DEBUG "/MTd" "/MDd")
  ocl_replace_compiler_option(CMAKE_CXX_FLAGS_RELEASE "/MT" "/MD")
  ocl_replace_compiler_option(CMAKE_CXX_FLAGS_DEBUG "/MTd" "/MDd")
  ocl_replace_compiler_option(CMAKE_EXE_LINKER_FLAGS_RELEASE
                              "/NODEFAULTLIB:LIBCMTD" "")
  ocl_replace_compiler_option(CMAKE_EXE_LINKER_FLAGS_RELEASE
                              "/NODEFAULTLIB:LIBCPMTD" "")
  ocl_replace_compiler_option(CMAKE_EXE_LINKER_FLAGS_DEBUG
                              "/NODEFAULTLIB:LIBCMT" "")
  ocl_replace_compiler_option(CMAKE_EXE_LINKER_FLAGS_DEBUG
                              "/NODEFAULTLIB:LIBCPMT" "")

  set(CMAKE_C_FLAGS_RELEASE
      ${CMAKE_C_FLAGS_RELEASE}
      PARENT_SCOPE)
  set(CMAKE_C_FLAGS_DEBUG
      ${CMAKE_C_FLAGS_DEBUG}
      PARENT_SCOPE)
  set(CMAKE_CXX_FLAGS_RELEASE
      ${CMAKE_CXX_FLAGS_RELEASE}
      PARENT_SCOPE)
  set(CMAKE_CXX_FLAGS_DEBUG
      ${CMAKE_CXX_FLAGS_DEBUG}
      PARENT_SCOPE)
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE
      ${CMAKE_EXE_LINKER_FLAGS_RELEASE}
      PARENT_SCOPE)
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG
      ${CMAKE_EXE_LINKER_FLAGS_DEBUG}
      PARENT_SCOPE)
endfunction(ocl_replace_compiler_option_to_dynamic)

#
# Set compiler RTTI options according to the given flag
#
macro(use_rtti val)
  # !!! Don't change the order! ICX matches both MSVC and Clang, but it uses
  # MSVC flags !!!
  if(MSVC) # ICX/MSVC
    if(${val})
      ocl_replace_compiler_option(CMAKE_CXX_FLAGS "/GR-" "")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR")
    else()
      ocl_replace_compiler_option(CMAKE_CXX_FLAGS "/GR" "")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
    endif()
  else() # GCC/Clang
    if(${val})
      ocl_replace_compiler_option(CMAKE_CXX_FLAGS "-fno-rtti" "-frtti")
    else()
      ocl_replace_compiler_option(CMAKE_CXX_FLAGS "-frtti" "-fno-rtti")
    endif()
  endif()
endmacro(use_rtti)

#
# Set compiler Exception Handling options according to the given flag
#
macro(use_eh val)
  # !!! Don't change the order! ICX matches both MSVC and Clang, but it uses
  # MSVC flag !!!
  if(MSVC) # ICX/MSVC
    if(${val})
      ocl_replace_compiler_option(CMAKE_CXX_FLAGS "/EHs-c-" "")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
      add_definitions(/D_HAS_EXCEPTIONS=1)
    else()
      ocl_replace_compiler_option(CMAKE_CXX_FLAGS "/EHsc" "")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHs-c-")
      add_definitions(/D_HAS_EXCEPTIONS=0)
    endif()
  else() # GCC/Clang
    if(${val})
      remove_definitions(-fno-exceptions)
    else()
      add_definitions(-fno-exceptions)
    endif()
  endif()
endmacro(use_eh)

# Set of functions used to add unittests TODO: reuse in backend

# Adds new executable from SOURCE_FILES and links it to LINK_LIBRARIES
function(add_ocl_unittest test_name)
  cmake_parse_arguments("ARG" "" "" "SOURCE_FILES;LINK_LIBRARIES" ${ARGN})
  if(NOT OPENCL_RT_BUILD_TESTS)
    set(EXCLUDE_FROM_ALL ON)
  endif(NOT OPENCL_RT_BUILD_TESTS)

  include_directories(SYSTEM
                      ${OCL_SOURCE_DIR}/externals/gtest/googletest/include)
  include_directories(${CL_API_HEADERS})

  if(EXCLUDE_FROM_ALL)
    add_executable(${test_name} EXCLUDE_FROM_ALL ${ARG_SOURCE_FILES})
  else(EXCLUDE_FROM_ALL)
    add_executable(${test_name} ${ARG_SOURCE_FILES})
  endif(EXCLUDE_FROM_ALL)

  target_link_libraries(${test_name} llvm_gtest ${PTHREAD_LIB})
  if(NOT ARG_LINK_LIBRARIES STREQUAL "")
    target_link_libraries(${test_name} ${ARG_LINK_LIBRARIES})
  endif(NOT ARG_LINK_LIBRARIES STREQUAL "")

  if(WIN32)
    # Explicitly define OUTPUT_DIRECTORY_${Configuration} for generators
    # supporting more than one configuration type to avoid output to
    # ${OUTPUT_DIRECTORY}/${Configuration} subdirs
    get_filename_component(FNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    set_target_properties(
      ${test_name}
      PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OCL_BINARY_DIR}/tests/${FNAME}
                 LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OCL_BINARY_DIR}/tests/${FNAME}
                 ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OCL_BINARY_DIR}/tests/${FNAME}
                 RUNTIME_OUTPUT_DIRECTORY_RELEASE
                 ${OCL_BINARY_DIR}/tests/${FNAME}
                 LIBRARY_OUTPUT_DIRECTORY_RELEASE
                 ${OCL_BINARY_DIR}/tests/${FNAME}
                 ARCHIVE_OUTPUT_DIRECTORY_RELEASE
                 ${OCL_BINARY_DIR}/tests/${FNAME})
  else(WIN32)
    set_target_properties(
      ${test_name}
      PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                 LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                 ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  endif(WIN32)

  set_target_properties(${test_name} PROPERTIES FOLDER "validation")

  string(TOLOWER ${test_name} test_name_lowercase)
  set(LIT_TARGET check-ocl-${test_name_lowercase})
  add_lit_testsuite(
    ${LIT_TARGET}
    "Running the OpenCL ${test_name} unittests"
    ${CMAKE_CURRENT_SOURCE_DIR}
    PARAMS
    ${RT_TEST_PARAMS}
    DEPENDS
    ${test_name}
    ARGS
    ${RT_TEST_ARGS})
  set_target_properties(${LIT_TARGET} PROPERTIES FOLDER "Tests")
endfunction()

# Copies files to current binary dir on build time
function(add_ocl_unittest_artifacts)
  cmake_parse_arguments("ARG" "CONFIGURE" "FILE;RENAME" "FILES" ${ARGN})

  set(dest_dir ${CMAKE_CURRENT_BINARY_DIR})

  if(NOT "${ARG_FILE}" STREQUAL "")
    if("${ARG_RENAME}" STREQUAL "")
      get_filename_component(FILE_NAME ${ARG_FILE} NAME)
      set(dest_name ${FILE_NAME})
    else("${ARG_RENAME}" STREQUAL "")
      set(dest_name ${ARG_RENAME})
    endif("${ARG_RENAME}" STREQUAL "")

    if(ARG_CONFIGURE)
      configure_file(${ARG_FILE} ${dest_dir}/${dest_name} @ONLY)
    else(ARG_CONFIGURE)
      file(COPY ${ARG_FILE} DESTINATION ${dest_dir})
      file(RENAME ${dest_dir}/${ARG_FILE} ${dest_dir}/${dest_name})
    endif(ARG_CONFIGURE)
  endif(NOT "${ARG_FILE}" STREQUAL "")

  foreach(filename ${ARG_FILES})
    get_filename_component(FILE_NAME ${filename} NAME)
    file(COPY ${filename} DESTINATION ${dest_dir})
  endforeach(filename)
endfunction()

# Get ICS build type: debug | prod | release TODO: support self-build type
# detection Usage: GET_ICS_BUILD_TYPE(result)
function(GET_ICS_BUILD_TYPE result)
  # INTEL_PRODUCT_RELEASE for release build
  if(CMAKE_C_FLAGS MATCHES "-DINTEL_PRODUCT_RELEASE=1")
    set(${result}
        "release"
        PARENT_SCOPE)
  else()
    if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
      set(${result}
          "debug"
          PARENT_SCOPE)
    else()
      set(${result}
          "prod"
          PARENT_SCOPE)
    endif()
  endif()
endfunction()
