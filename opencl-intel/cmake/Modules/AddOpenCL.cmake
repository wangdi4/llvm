# Workadound to support both old and new files / directories location
# This is needed to avoid dependancy on tools team
# TODO: remove DRY_RUN variable when build script will be aligned with new
# files location

set (DRY_RUN ON)

# Define build and install directories
set(OCL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(OCL_LIBRARY_DIR  ${OCL_BINARY_DIR}/lib)
set(OCL_RUNTIME_DIR  ${OCL_BINARY_DIR}/bin)
set(OCL_TOOLS_BINARY_DIR ${OCL_BINARY_DIR}/bin)
set(OCL_TESTS_BINARY_DIR ${OCL_BINARY_DIR}/tests)

# Define architecture and OS suffix for output dirs
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(OUTPUT_ARCH_SUFF "ia32")
else () # x64
    set(OUTPUT_ARCH_SUFF "intel64")
endif (CMAKE_SIZEOF_VOID_P EQUAL 4)

if (WIN32)
    set(OUTPUT_OS_SUFF "_win")
else () # Linux
    set(OUTPUT_OS_SUFF "_lin")
endif (WIN32)

# Define output dirs
set(OCL_OUTPUT_BINARY_DIR ${OCL_RUNTIME_DIR}/${OUTPUT_ARCH_SUFF})
set(OCL_OUTPUT_LIBRARY_DIR ${OCL_LIBRARY_DIR}/${OUTPUT_ARCH_SUFF}${OUTPUT_OS_SUFF})


# add_opencl_library - binding over add_library for OpenCL needs
#       name            - defines library name
#       SHARED / STATIC - defines library type
#       INCLUDE_DIRS    - defines include directories
#       LINK_LIBS       - defines libraries to link

function(add_opencl_library name)
    cmake_parse_arguments(ARG
        "SHARED;STATIC"
        ""
        "INCLUDE_DIRS;LINK_LIBS"
        ${ARGN})

    set(sources ${ARG_UNPARSED_ARGUMENTS})

    # TODO: replace with target_include_directories
    include_directories(AFTER ${ARG_INCLUDE_DIRS})

    if (ARG_SHARED)
        add_library(${name} SHARED ${sources})
    else (ARG_SHARED)
        add_library(${name} STATIC ${sources})
    endif (ARG_SHARED)

    if (WIN32)
        set_target_properties(${name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OCL_OUTPUT_LIBRARY_DIR}
            LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OCL_OUTPUT_LIBRARY_DIR}
            ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OCL_OUTPUT_LIBRARY_DIR}
            RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OCL_OUTPUT_LIBRARY_DIR}
            LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OCL_OUTPUT_LIBRARY_DIR}
            ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OCL_OUTPUT_LIBRARY_DIR})
    else (WIN32)
        set_target_properties(${name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${OCL_OUTPUT_LIBRARY_DIR}
            LIBRARY_OUTPUT_DIRECTORY ${OCL_OUTPUT_LIBRARY_DIR}
            ARCHIVE_OUTPUT_DIRECTORY ${OCL_OUTPUT_LIBRARY_DIR})
    endif (WIN32)

    target_link_libraries(${name} ${ARG_LINK_LIBS})

    # Deals with pdb files on Windows
    if (WIN32 AND ARG_SHARED)
        file (TO_NATIVE_PATH ${OCL_OUTPUT_LIBRARY_DIR}/${name}_stripped.pdb PDB_NAME)
        set_target_properties (${name} PROPERTIES
            # The /DEBUG flag is required in order to create stripped pdbs.
            LINK_FLAGS_RELEASE "/PDBSTRIPPED:${PDB_NAME} /DEBUG"
            LINK_FLAGS_DEBUG "/PDBSTRIPPED:${PDB_NAME}")

        install_to (${OCL_OUTPUT_LIBRARY_DIR}/${name}_stripped.pdb DESTINATION lib)

        if (INSTALL_PDBS)
            install_to (${OCL_OUTPUT_LIBRARY_DIR}/${name}.pdb DESTINATION lib)
        endif (INSTALL_PDBS)
    endif (WIN32 AND ARG_SHARED)

    install_to (${name} DESTINATION lib)

endfunction(add_opencl_library name)

# add_opencl_executable - binding over add_executable for OpenCL needs
#       name            - defines executable name
#       INCLUDE_DIRS    - defines include directories
#       LINK_LIBS       - defines libraries to link
#

function(add_opencl_executable name)
    cmake_parse_arguments(ARG "" "" "INCLUDE_DIRS;LINK_LIBS" ${ARGN})

    # TODO: replace with target_include_directories
    include_directories(AFTER ${ARG_INCLUDE_DIRS})

    add_executable(${name} ${ARG_UNPARSED_ARGUMENTS})

    if (WIN32)
        set_target_properties(${name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OCL_OUTPUT_BINARY_DIR}
            LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OCL_OUTPUT_BINARY_DIR}
            ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OCL_OUTPUT_BINARY_DIR}
            RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OCL_OUTPUT_BINARY_DIR}
            LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OCL_OUTPUT_BINARY_DIR}
            ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OCL_OUTPUT_BINARY_DIR})
    else (WIN32)
        set_target_properties(${name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${OCL_OUTPUT_BINARY_DIR}
            LIBRARY_OUTPUT_DIRECTORY ${OCL_OUTPUT_BINARY_DIR}
            ARCHIVE_OUTPUT_DIRECTORY ${OCL_OUTPUT_BINARY_DIR})
    endif (WIN32)

    target_link_libraries(${name} ${ARG_LINK_LIBS})

    install_to (${name} DESTINATION bin)

endfunction(add_opencl_executable name)

# copy_to (<files, directories...> DESTINATION <dir>) -
#       simple utility to copy files during configuration step
#
#       DESTINATION - relative path from a root of build directory

function (copy_to)
    cmake_parse_arguments(ARG "" "DESTINATION" "" ${ARGN})
    set(files_to_copy ${ARG_UNPARSED_ARGUMENTS})

    if (${ARG_DESTINATION} STREQUAL "lib")
        set (output ${OCL_OUTPUT_LIBRARY_DIR})
    elseif (${ARG_DESTINATION} STREQUAL "bin")
        set (output ${OCL_OUTPUT_BINARY_DIR})
    else ()
        set (output ${OCL_BINARY_DIR}/${ARG_DESTINATION})
    endif ()

    file(COPY ${files_to_copy} DESTINATION ${output})
    install_to (${files_to_copy} DESTINATION ${ARG_DESTINATION})

endfunction (copy_to)

# install_to (<files, directories, targets...> DESTINATION <dir>) -
#       simple utility to install files, directoies or targets
#
#       DESTINATION - relative path from a root of build directory
#       NOTE: Targets should be specified by target name,
#       files and directories with full paths

function (install_to)
    cmake_parse_arguments(ARG "" "DESTINATION" "" ${ARGN})
    set(files_to_install ${ARG_UNPARSED_ARGUMENTS})

    if (NOT DRY_RUN AND (${CMAKE_INSTALL_PREFIX} STREQUAL ${OCL_BINARY_DIR}))
        return ()
    endif ()

    # In DRY_RUN mode 'install' copies all output to 'bin/' dir
    # TODO: remove this condition when DRY_RUN will be no longer needed
    if (DRY_RUN AND (${ARG_DESTINATION} STREQUAL "lib" OR ${ARG_DESTINATION} STREQUAL "bin"))

        # TODO: remove next line and uncomment next to it when
        # DRY_RUN will be no longer needed
        set (output ${CMAKE_INSTALL_PREFIX}/bin)
        # set (output ${CMAKE_INSTALL_PREFIX}/${destination})

        foreach (file ${files_to_install})
            if (TARGET ${file})
                install (TARGETS ${file}
                    RUNTIME DESTINATION ${output}
                    LIBRARY DESTINATION ${output}
                    ARCHIVE DESTINATION ${output})
            elseif (IS_DIRECTORY ${file})
                install (DIRECTORY ${file} DESTINATION ${output})
            else ()
                install (FILES ${file} DESTINATION ${output})
            endif ()
        endforeach (file)

    endif ()
endfunction (install_to)
