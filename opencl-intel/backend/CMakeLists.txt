cmake_minimum_required(VERSION 2.8.7)
# LLVM

if( NOT DEFINED LLVM_PATH_BE )
    message( FATAL_ERROR "LLVM_PATH_BE is not specified. Please specify LLVM library location for backend using LLVM_PATH_BE parameter to CMAKE" )
endif()

set(LLVM_PATH ${LLVM_PATH_BE})
find_package(LLVM REQUIRED)
message( STATUS "Backend LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}" )

if( ANDROID )
    set( BACKEND_BUILD_TESTS            OFF  CACHE STRING "")
    set( BACKEND_BUILD_VERIFICATION_LIB OFF  CACHE STRING "")
else( ANDROID )
    set( BACKEND_BUILD_TESTS            ON  CACHE STRING "")
    set( BACKEND_BUILD_VERIFICATION_LIB ON  CACHE STRING "")
endif( ANDROID )

set( OCL_BACKEND_STAND_ALONE      ON  CACHE STRING "")

set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set_property( GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "__BUILD__")

set( BACKEND_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set( BACKEND_FOLDER_NAME       Backend )
set( BACKEND_TESTS_FOLDER_NAME  ${BACKEND_FOLDER_NAME}/tests )
set( BACKEND_PASSES_FOLDER_NAME  ${BACKEND_FOLDER_NAME}/passes )
set( BACKEND_VALIDATION_FOLDER_NAME  ${BACKEND_FOLDER_NAME}/validation )

# Set OCL Tuning to ON. ON by default. can be turned off for release.
set( OCLT ON)
if ( OCLT )
  add_definitions( -DOCLT )
endif (OCLT)

set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES "")

remove_definitions( -D_UNICODE -DUNICODE )

# Define a macro which can be used to hide MIC symbols when we do not want to
# expose IP
if (INCLUDE_MIC_DEVICE)
  add_definitions( -DINCLUDE_MIC_DEVICE )
endif (INCLUDE_MIC_DEVICE)

#
# Settings for Linux compilation
#
if (NOT WIN32)
    set( CMAKE_CXX_FLAGS_RELEASE                "${CMAKE_CXX_FLAGS_RELEASE} -D_FORTIFY_SOURCE=2")
    set( CMAKE_C_FLAGS_RELEASE                  "${CMAKE_C_FLAGS_RELEASE}   -D_FORTIFY_SOURCE=2")
    set ( WARNING_LEVEL  "-pedantic -Wall -Wextra -Wno-unknown-pragmas -Wno-strict-aliasing -Wno-variadic-macros -Wno-long-long -Wno-unused-parameter -Wno-unused-function")
    if( CMAKE_CROSSCOMPILING OR LLVM_USE_NATIVE )
      set( WARNING_LEVEL "${WARNING_LEVEL} -w" )
    else( )
      set( WARNING_LEVEL "${WARNING_LEVEL} -Werror" )
    endif( )
    add_definitions( ${WARNING_LEVEL} )
endif (NOT WIN32)

if(WIN32)
#
# Usage
#     CREATE_ASM_RULES( <ADD_TO_SOURCES_LIST_VAR> ...<asm-files-list>...)
#
# Creates ADD_TO_SOURCES_LIST_VAR that should be added to the source files list
#
macro( CREATE_ASM_RULES ADD_TO_SOURCES_LIST_VAR )

    if (${ARGC} GREATER  1)
        foreach( FILE ${ARGN} )
            get_filename_component( FILE_NAME ${FILE} NAME_WE )
            set( BIN_DIR  ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR} )
            set( OBJ_FILE ${BIN_DIR}/${FILE_NAME}${CMAKE_C_OUTPUT_EXTENSION} )
            set( SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${FILE} )

            if (CMAKE_ASM_OUTPUT_NAME_FLAG)
              set( OBJ_OUTPUT_NAME_FLAG ${CMAKE_ASM_OUTPUT_NAME_FLAG} ${FILE_NAME}${CMAKE_C_OUTPUT_EXTENSION} )
            endif (CMAKE_ASM_OUTPUT_NAME_FLAG)

            add_custom_command(OUTPUT ${OBJ_FILE}
              COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_DIR}
              COMMAND ${CMAKE_COMMAND} -E chdir ${BIN_DIR}
                      ${CMAKE_ASM_COMPILER} ${CMAKE_ASM_FLAGS}
                      ${CMAKE_ASM_INCLUDE_DIR_FLAG} ${CMAKE_CURRENT_SOURCE_DIR}
                      ${OBJ_OUTPUT_NAME_FLAG}
                      ${SRC_FILE}
              MAIN_DEPENDENCY ${SRC_FILE}
              VERBATIM
            )

            list(APPEND OBJ_FILES ${OBJ_FILE})
            list(APPEND SRC_FILES ${SRC_FILE})
        endforeach( FILE )
    endif (${ARGC} GREATER 1)

    if (DEFINED OBJ_FILES)
        set (${ADD_TO_SOURCES_LIST_VAR} ${SRC_FILES} ${OBJ_FILES})
        set_source_files_properties( ${OBJ_FILES} PROPERTIES GENERATED TRUE EXTERNAL_OBJECT TRUE )
    endif(DEFINED OBJ_FILES)

endmacro( CREATE_ASM_RULES )
endif(WIN32)

###############################################################################################

project(Backend C CXX)

add_definitions(${LLVM_DEFINITIONS})
use_rtti(FALSE)

# LLVM device backend library
option(LLVM_BUILD_OCL_CPU_BACKEND "Build OpenCL CPU backend DLL." ON)
message(STATUS "Build OpenCL CPU backend DLL: ${LLVM_BUILD_OCL_CPU_BACKEND}")

if (LLVM_BUILD_OCL_CPU_BACKEND)
    add_subdirectory(vectorizer        "${CMAKE_CURRENT_BINARY_DIR}/vectorizer")
    add_subdirectory(barrier           "${CMAKE_CURRENT_BINARY_DIR}/barrier")
    add_subdirectory(utils             "${CMAKE_CURRENT_BINARY_DIR}/utils")
    add_subdirectory(metadata_api      "${CMAKE_CURRENT_BINARY_DIR}/metadata_api")
    add_subdirectory(passes            "${CMAKE_CURRENT_BINARY_DIR}/passes")
    add_subdirectory(optimizer         "${CMAKE_CURRENT_BINARY_DIR}/optimizer")
    add_subdirectory(ocl_cpu_backend   "${CMAKE_CURRENT_BINARY_DIR}/ocl_cpu_backend")
    add_subdirectory(ocl_cpu_debugging "${CMAKE_CURRENT_BINARY_DIR}/ocl_cpu_debugging")
    add_subdirectory(ocl_opt           "${CMAKE_CURRENT_BINARY_DIR}/ocl_opt")
    add_subdirectory(ocl_opt_img       "${CMAKE_CURRENT_BINARY_DIR}/ocl_opt_img")
    add_subdirectory(libraries/ocl_builtins "${CMAKE_CURRENT_BINARY_DIR}/ocl_builtins")
    add_subdirectory(plugin_manager    "${CMAKE_CURRENT_BINARY_DIR}/plugin_manager")
    add_subdirectory(dynamic_lib       "${CMAKE_CURRENT_BINARY_DIR}/dynamic_lib")
    add_subdirectory(name_mangling     "${CMAKE_CURRENT_BINARY_DIR}/name_mangling")
    add_subdirectory(reflection_module "${CMAKE_CURRENT_BINARY_DIR}/reflection_module")
    add_subdirectory(stat_summary      "${CMAKE_CURRENT_BINARY_DIR}/stat_summary")
    if (INCLUDE_MIC_DEVICE)
        add_subdirectory(ocl_mic_executor "${CMAKE_CURRENT_BINARY_DIR}/ocl_mic_executor")
        add_subdirectory(MICJITEngine     "${CMAKE_CURRENT_BINARY_DIR}/MICJITEngine")
    endif ()

    # Tests
    add_subdirectory(tests/type_conversion)
    add_subdirectory(tests/recorder)
    add_subdirectory(tests/debugger_test_type)
endif (LLVM_BUILD_OCL_CPU_BACKEND)

# Validation utilities
option(BACKEND_BUILD_VERIFICATION_LIB "Build back-end validation tools." ON)
message(STATUS "Build OpenCL CPU backend validation tools: ${LLVM_BUILD_OCL_CPU_BACKEND}")

if (BACKEND_BUILD_VERIFICATION_LIB)
    add_subdirectory(external/tinyxml  "${CMAKE_CURRENT_BINARY_DIR}/tinyxml")
    add_subdirectory(validations       "${CMAKE_CURRENT_BINARY_DIR}/validations")

    # copying MICRegression suite tests to install directory
    install(DIRECTORY ${BACKEND_ROOT_DIR}/tests/MIC/satest/mic/
            DESTINATION tests/MICRegression/)

    set_target_properties( DataVersion PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( Comparator PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( DataGenerator PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( DataManager PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    if (NOT WIN32)
        set_target_properties( NEAT PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
        set_target_properties( REFALU PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    endif ()
    set_target_properties( PlugInNEAT PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( OCLBuiltins PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( SATest PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( LLVMInterpreterPluggable PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( ValidationTests PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation/tests )
    set_target_properties( NEATChecker PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation/tests )
    set_target_properties( BackEndTests PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation/tests )
    set_target_properties( OCLSamplePlugin PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation/tests )
    set_target_properties( FMAtest PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation/tests )
    set_target_properties( tinyxml_STL PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/externals )
endif (BACKEND_BUILD_VERIFICATION_LIB)

# copy the Clang conformance tests to the validation directory
install(FILES ${BACKEND_ROOT_DIR}/tests/Conformance.7z
        DESTINATION tests)

