# Copyright (C) 2017-2022 Intel Corporation
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express or
# implied warranties, other than those that are expressly stated in the License.

# Workadound to support both old and new files / directories location This is
# needed to avoid dependancy on tools team

# Define build and install directories
set(OCL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(OCL_LIBRARY_DIR ${OCL_BINARY_DIR}/lib)
set(OCL_INCLUDE_DIR ${OCL_BINARY_DIR}/include)
set(OCL_RUNTIME_DIR ${OCL_BINARY_DIR}/bin)
set(OCL_TOOLS_BINARY_DIR ${OCL_BINARY_DIR}/bin)
set(OCL_TESTS_BINARY_DIR ${OCL_BINARY_DIR}/tests)

# Define architecture and OS suffix for output dirs
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(OUTPUT_ARCH_SUFF "x86")
else() # x64
  set(OUTPUT_ARCH_SUFF "x64")
endif(CMAKE_SIZEOF_VOID_P EQUAL 4)

# Define output install path for the emulator if needed
set(OUTPUT_EMU_SUFF "emu")
add_definitions(-DOUTPUT_EMU_SUFF="_${OUTPUT_EMU_SUFF}")

# This macro sets OpenCL libraries version as 'x.y', where 'x' is a major
# version of LLVM and 'y' is an internally agreed digit.
macro(set_opencl_version)
  if(NOT OPENCL_INTREE_BUILD)
    if(NOT DEFINED LLVM_PATH_FE)
      message(FATAL_ERROR "LLVM_PATH_FE is not specified.")
    endif()
    set(LLVM_PATH ${LLVM_PATH_FE})
    find_package(LLVM REQUIRED)
  endif()

  math(EXPR LLVM_RELEASE_VER "${LLVM_VERSION_MAJOR} - 1")
  set(VERSIONSTRING
      "${PRODUCTVER_MAJOR}.${LLVM_RELEASE_VER}.${PRODUCTVER_MINOR}")
  # Get branch info
  if(("${OCL_RELEASE_BRANCH}" STREQUAL "xmain-rel") OR ("${OCL_RELEASE_BRANCH}"
                                                        STREQUAL ""))
    set(WSPROJECT "")
  elseif("${OCL_RELEASE_BRANCH}" STREQUAL "xmain")
    set(WSPROJECT ".prerelease")
  else()
    set(WSPROJECT ".${OCL_RELEASE_BRANCH}")
  endif()
  # Get dd_hhmmss info of changeset.
  string(LENGTH "${OCL_REVISION}" CHANGESET_LEN)
  # yyyymmdd or #yyyymmdd_hhmmss pattern
  if("${OCL_REVISION}" MATCHES "^[0-9]+(_[0-9]+)?")
    # Exclude the "yyyymm" value from the changeset info.
    math(EXPR LENTH_LEFT "${CHANGESET_LEN} - 6")
    string(SUBSTRING "${OCL_REVISION}" 6 ${LENTH_LEFT} WSDATE)
  else("${OCL_REVISION}" MATCHES "^[0-9]+(_[0-9]+)?")
    set(WSDATE "${OCL_REVISION}")
  endif()
  if("${WSDATE}" STREQUAL "")
    set(VERSION_EXT ${WSPROJECT})
  else()
    set(VERSION_EXT ".${WSDATE}${WSPROJECT}")
  endif()

  add_definitions(-DVERSIONSTRING="${VERSIONSTRING}")
  add_definitions(-DVERSIONSTRING_WITH_EXT="${VERSIONSTRING}${VERSION_EXT}")
  file(
    WRITE ${OCL_BINARY_DIR}/driverversion.h.txt
    "#ifndef VERSIONSTRING\n
          #define VERSIONSTRING \"${VERSIONSTRING}\"\n
        #endif\n
        #ifndef VERSIONSTRING_WITH_EXT\n
          #define VERSIONSTRING_WITH_EXT \"${VERSIONSTRING}${VERSION_EXT}\"\n
        #endif\n")
  execute_process(
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      ${OCL_BINARY_DIR}/driverversion.h.txt ${OCL_BINARY_DIR}/driverversion.h)
endmacro(set_opencl_version)

# Define output dirs
set(OCL_OUTPUT_BINARY_DIR ${OCL_RUNTIME_DIR}/${OUTPUT_ARCH_SUFF})
set(OCL_OUTPUT_LIBRARY_DIR ${OCL_LIBRARY_DIR}/${OUTPUT_ARCH_SUFF})

# add_opencl_library - binding over add_library for OpenCL needs
#   name             - defines library name
#   SHARED / STATIC  - defines library type
#   INCLUDE_DIRS     - defines include directories
#   COMPONENTS       - defines shipping OpenCL libraries to link
#   LINK_LIBS        - defines rest of libraries to link
#   INSTALL_PATH     - defines the last element of the install directory.
#   RC_TEMPLATE      - defines template for .rc files generation on Windows.
#                      No .rc file generated if omitted.
#                      Pass 'default' to use the default one.

function(add_opencl_library name)
  cmake_parse_arguments(
    ARG "SHARED;STATIC;EXCLUDE_FROM_ALL_BUILD" "RC_TEMPLATE"
    "INCLUDE_DIRS;COMPONENTS;LINK_LIBS;INSTALL_PATH" ${ARGN})

  set(sources ${ARG_UNPARSED_ARGUMENTS})

  # TODO: replace with target_include_directories
  include_directories(AFTER ${ARG_INCLUDE_DIRS})

  # Generate resource files for shared libs on Windows
  if(WIN32
     AND ARG_SHARED
     AND ARG_RC_TEMPLATE)
    string(TOUPPER ${ARG_RC_TEMPLATE} RC_TEMPLATE_UPPERCASE)
    if(${RC_TEMPLATE_UPPERCASE} STREQUAL "DEFAULT")
      set(rc_template ${OCL_SOURCE_DIR}/rc_template.rc.in) # default template
    else(${RC_TEMPLATE_UPPERCASE} STREQUAL "DEFAULT")
      set(rc_template ${ARG_RC_TEMPLATE}) # custom template
    endif(${RC_TEMPLATE_UPPERCASE} STREQUAL "DEFAULT")

    configure_file(${rc_template} ${OCL_BINARY_DIR}/${name}.rc @ONLY)
    list(APPEND sources ${OCL_BINARY_DIR}/${name}.rc)
  endif(
    WIN32
    AND ARG_SHARED
    AND ARG_RC_TEMPLATE)

  if(ARG_SHARED)
    add_library(${name} SHARED ${sources})
  else(ARG_SHARED)
    add_library(${name} STATIC ${sources})
  endif(ARG_SHARED)

  if(ARG_EXCLUDE_FROM_ALL_BUILD)
    set_target_properties(${name} PROPERTIES EXCLUDE_FROM_ALL ON)
  endif()

  if(WIN32)
    set_target_properties(
      ${name}
      PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OCL_OUTPUT_LIBRARY_DIR}
                 LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OCL_OUTPUT_LIBRARY_DIR}
                 ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OCL_OUTPUT_LIBRARY_DIR}
                 RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OCL_OUTPUT_LIBRARY_DIR}
                 LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OCL_OUTPUT_LIBRARY_DIR}
                 ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OCL_OUTPUT_LIBRARY_DIR})
  else(WIN32)
    set_target_properties(
      ${name}
      PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OCL_OUTPUT_LIBRARY_DIR}
                 LIBRARY_OUTPUT_DIRECTORY ${OCL_OUTPUT_LIBRARY_DIR}
                 ARCHIVE_OUTPUT_DIRECTORY ${OCL_OUTPUT_LIBRARY_DIR}
                 SOVERSION ${VERSIONSTRING})
  endif(WIN32)

  target_link_libraries(${name} ${ARG_LINK_LIBS} ${ARG_COMPONENTS})

  # Deals with pdb on Windows
  if(WIN32 AND ARG_SHARED)
    file(TO_NATIVE_PATH ${OCL_OUTPUT_LIBRARY_DIR}/${name}.pdb PDB_NAME)
    set_target_properties(
      ${name}
      PROPERTIES # The /DEBUG flag is required in order to create pdbs.
                 LINK_FLAGS_RELEASE
                 "${LINK_FLAGS_RELEASE} /DEBUG /PDB:${PDB_NAME}"
                 LINK_FLAGS_DEBUG "${LINK_FLAGS_DEBUG} /PDB:${PDB_NAME}")

    get_ics_build_type(ICS_BUILD_TYPE)
    # Disable PDB installation in release build
    if(NOT ICS_BUILD_TYPE STREQUAL "release")
      foreach(path ${ARG_INSTALL_PATH})
        install_to(${OCL_OUTPUT_LIBRARY_DIR}/${name}.pdb DESTINATION
                   lib/${path} COMPONENT ocl-${name})
      endforeach(path)
    endif()
  endif(WIN32 AND ARG_SHARED)

  foreach(path ${ARG_INSTALL_PATH})
    install_to(${name} DESTINATION lib/${path} COMPONENT ocl-${name})
  endforeach(path)

endfunction(add_opencl_library name)

# add_opencl_executable  - binding over add_executable for OpenCL needs
#       name             - defines executable name
#       INCLUDE_DIRS     - defines include directories
#       COMPONENTS       - defines shipping OpenCL libraries to link
#       LINK_LIBS        - defines rest of libraries to link
#

function(add_opencl_executable name)
  cmake_parse_arguments(ARG "EXCLUDE_FROM_ALL_BUILD" ""
                        "INCLUDE_DIRS;COMPONENTS;LINK_LIBS" ${ARGN})

  # TODO: replace with target_include_directories
  include_directories(AFTER ${ARG_INCLUDE_DIRS})

  add_executable(${name} ${ARG_UNPARSED_ARGUMENTS})

  if(ARG_EXCLUDE_FROM_ALL_BUILD)
    set_target_properties(${name} PROPERTIES EXCLUDE_FROM_ALL ON)
  endif()

  if(WIN32)
    set_target_properties(
      ${name}
      PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OCL_OUTPUT_BINARY_DIR}
                 LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OCL_OUTPUT_BINARY_DIR}
                 ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OCL_OUTPUT_BINARY_DIR}
                 RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OCL_OUTPUT_BINARY_DIR}
                 LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OCL_OUTPUT_BINARY_DIR}
                 ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OCL_OUTPUT_BINARY_DIR})
  else(WIN32)
    set_target_properties(
      ${name}
      PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OCL_OUTPUT_BINARY_DIR}
                 LIBRARY_OUTPUT_DIRECTORY ${OCL_OUTPUT_BINARY_DIR}
                 ARCHIVE_OUTPUT_DIRECTORY ${OCL_OUTPUT_BINARY_DIR})
  endif(WIN32)

  target_link_libraries(${name} ${ARG_LINK_LIBS} ${ARG_COMPONENTS})

  install_to(${name} DESTINATION bin COMPONENT ocl-${name})

endfunction(add_opencl_executable name)

# copy_to (<files, directories...> DESTINATION <dir>) - simple utility to copy
# files during configuration step
#
# DESTINATION - relative path from a root of build directory

function(copy_to)
  cmake_parse_arguments(ARG "" "DESTINATION" "" ${ARGN})
  set(files_to_copy ${ARG_UNPARSED_ARGUMENTS})

  if(${ARG_DESTINATION} STREQUAL "lib")
    set(output ${OCL_OUTPUT_LIBRARY_DIR})
  elseif(${ARG_DESTINATION} STREQUAL "bin")
    set(output ${OCL_OUTPUT_BINARY_DIR})
  else()
    set(output ${OCL_BINARY_DIR}/${ARG_DESTINATION})
  endif()

  file(COPY ${files_to_copy} DESTINATION ${output})

endfunction(copy_to)

# install_to (<files, directories, targets...> [COMPONENT <name>] DESTINATION
# <dir>) - simple utility to install files, directoies or targets
#
#       COMPONENT - name of a CMake install component
#       DESTINATION - relative path from a root of build directory
#       NOTE: Targets should be specified by target name,
#       files and directories with full paths

function(install_to)
  cmake_parse_arguments(ARG "" "COMPONENT;DESTINATION" "" ${ARGN})
  set(install_namelist ${ARG_UNPARSED_ARGUMENTS})

  if(NOT ARG_DESTINATION)
    message(
      FATAL_ERROR
        "Missed destination location argument for install_to function.")
  endif()

  set(output ${ARG_DESTINATION})
  if(NOT ("${OUTPUT_ARCH_SUFF}" STREQUAL "x86" AND "${output}" STREQUAL
                                                   "lib/${OUTPUT_EMU_SUFF}"))
    foreach(name ${install_namelist})
      if(TARGET ${name})
        if(ARG_COMPONENT)
          set(component "${ARG_COMPONENT}")
        else()
          set(component "ocl-${name}")
        endif(ARG_COMPONENT)

        install(
          TARGETS ${name}
          RUNTIME DESTINATION ${output} COMPONENT ${component}
          LIBRARY DESTINATION ${output} COMPONENT ${component}
          ARCHIVE DESTINATION ${output} COMPONENT ${component})
      else()
        # name is a directory or a file

        if(ARG_COMPONENT)
          set(component "${ARG_COMPONENT}")
        else()
          set(component "ocl-Unspecified")
        endif()

        if(IS_DIRECTORY ${name})
          install(
            DIRECTORY ${name}
            DESTINATION ${output}
            COMPONENT ${component})
        else()
          install(
            FILES ${name}
            DESTINATION ${output}
            COMPONENT ${component})
        endif()
      endif()
    endforeach(name)
  endif()
endfunction(install_to)
