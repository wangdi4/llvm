cmake_minimum_required(VERSION 2.8.4)
#
#

set( LLVM_ENABLE_WERROR          OFF CACHE STRING "")
set( LLVM_BUILD_TESTS            ON  CACHE STRING "")
set( LLVM_BUILD_GFX              OFF CACHE STRING "")
set( LLVM_BUILD_VERIFICATION_LIB ON  CACHE STRING "")
set( OCL_BACKEND_STAND_ALONE     ON  CACHE STRING "")

set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set_property( GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "__BUILD__")

set(LLVM_TARGETS_TO_BUILD X86 CACHE STRING "Semicolon-separated list of targets to build, or \"all\".")
if (INCLUDE_MIC_DEVICE)
    list ( APPEND LLVM_TARGETS_TO_BUILD Y86)
endif ()

if (NOT MSVC)
    list( APPEND LLVM_TARGETS_TO_BUILD CBackend CppBackend)
endif ()

message ("LLVM_TARGETS_TO_BUILD: ${LLVM_TARGETS_TO_BUILD}")
set( BACKEND_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set( BACKEND_FOLDER_NAME       Backend )
set( BACKEND_TESTS_FOLDER_NAME  ${BACKEND_FOLDER_NAME}/Tests )
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
    if (APPLE)
        # Warning level
        set ( WARNING_LEVEL  "-Wall -Wextra -Wno-unknown-pragmas -Wno-strict-aliasing -Wno-variadic-macros -Wno-long-long -Wno-unused-parameter")
        add_definitions( ${WARNING_LEVEL} )
    else (APPLE)
        # Warning level
        set ( WARNING_LEVEL  "-pedantic -Wall -Wextra -Werror -Wno-unknown-pragmas -Wno-strict-aliasing -Wno-variadic-macros -Wno-long-long -Wno-unused-parameter")
        add_definitions( ${WARNING_LEVEL} )
    endif (APPLE)
    
    # Assembler setup - use private rules
    #set( CMAKE_ASM_FLAGS               ) # do not quote this!!!!
    #set( CMAKE_ASM_INCLUDE_DIR_FLAG    -I )
    #set( CMAKE_ASM_OUTPUT_NAME_FLAG    -o )
    #if (CVCC_ASM_PATH)
    #    set(CMAKE_ASM_COMPILER         ${CVCC_ASM_PATH})
    #else(CVCC_ASM_PATH)
    #    message(STATUS "Using system assembler, check that its version is 4.3.x")
    #    set( CMAKE_ASM_COMPILER        as )
    #endif(CVCC_ASM_PATH)    
    
endif (NOT WIN32)


function(ocl_replace_compiler_option var old new)
  # Replaces a compiler option or switch `old' in `var' by `new'.
  # If `old' is not in `var', appends `new' to `var'.
  # Example: llvm_replace_compiler_option(CMAKE_CXX_FLAGS_RELEASE "-O3" "-O2")
  # If the option already is on the variable, don't add it:
  if( "${${var}}" MATCHES "(^| )${new}($| )" )
    set(n "")
  else()
    set(n "${new}")
  endif()
  if( "${${var}}" MATCHES "(^| )${old}($| )" )
    string( REGEX REPLACE "(^| )${old}($| )" " ${n} " ${var} "${${var}}" )
  else()
    set( ${var} "${${var}} ${n}" )
  endif()
  set( ${var} "${${var}}" PARENT_SCOPE )
endfunction(ocl_replace_compiler_option)

function(use_rtti val)
    if( CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        if( ${val} )
            ocl_replace_compiler_option(CMAKE_CXX_FLAGS "-fno-rtti" "-frtti")
        else()
            ocl_replace_compiler_option(CMAKE_CXX_FLAGS "-frtti" "-fno-rtti" )
        endif()    
    else(MSVC)
        if( ${val} )
            ocl_replace_compiler_option(CMAKE_CXX_FLAGS "/GR-" "/GR")
        else()
            ocl_replace_compiler_option(CMAKE_CXX_FLAGS "/GR" "/GR-" )
        endif()    
    endif()
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE )
endfunction(use_rtti)
    
function(use_eh val)
    if( CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        if( ${val} )
            remove_definitions( -fno-exceptions )
        else()
            add_definitions( -fno-exceptions )
        endif()    
    else(MSVC)
        if( ${val} )
              llvm_replace_compiler_option(CMAKE_CXX_FLAGS "/EHs-c-" "/EHsc" )
              add_definitions( /D_HAS_EXCEPTIONS=1 )
        else()
              llvm_replace_compiler_option(CMAKE_CXX_FLAGS "/EHsc" "/EHs-c-")
              add_definitions( /D_HAS_EXCEPTIONS=0 )
        endif()    
    endif()
endfunction(use_eh)

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


###############################################################################################

option(LLVM_BUILD_APPLE_BACKEND "Build OpenCL CPU Apple backend plugin." OFF)
message("Build OpenCL CPU Apple backend plugin: ${LLVM_BUILD_APPLE_BACKEND}")

if (LLVM_BUILD_APPLE_BACKEND)
    project(Backend)
    
	find_package(LLVM REQUIRED)
	find_package(OpenCL REQUIRED)

    set_directory_properties(PROPERTIES INCLUDE_DIRECTORIES ${LLVM_INCLUDE_DIR})
    
    set(ADD_CXX_FLAGS   "-D  __STDC_LIMIT_MACROS  -D __STDC_CONSTANT_MACROS -fno-rtti")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${ADD_CXX_FLAGS}")
    
    add_subdirectory(utils)
    #add_subdirectory(libraries/ocl_builtins/utils/TableGen)
    #add_subdirectory(libraries/ocl_builtins)
    add_subdirectory(metadata_api)
    add_subdirectory(passes)
use_rtti(TRUE)
    add_subdirectory(name_mangling)
    add_subdirectory(reflection_module)
use_rtti(FALSE)
    add_subdirectory(optimizer)
    add_subdirectory(barrier)
    add_subdirectory(vectorizer)
    add_subdirectory(apple_plugin)
    
else(LLVM_BUILD_APPLE_BACKEND)
    project(Backend)
    add_subdirectory(llvm )


    set_target_properties( llc PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( lli PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    if( MSVC_IDE OR XCODE )
        set_target_properties( llvm_headers_do_not_build PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    endif()
    set_target_properties( llvm-ar PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( llvm-as PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( llvm-bcanalyzer PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( llvm-diff PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( llvm-dis PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( llvm-extract PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( llvm-ld PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( llvm-link PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( llvm-mc PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( llvm-nm PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( llvm-prof PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( llvm-ranlib PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( llvm-stub PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMAnalysis PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMArchive PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMAsmParser PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMAsmPrinter PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMBitReader PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMBitWriter PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMCodeGen PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMCore PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMExecutionEngine PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMHello PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMInstCombine PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMInstrumentation PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMInterpreter PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    #set_target_properties( OCLInterpreter PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMipa PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMipo PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMJIT PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMLinker PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMMC PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMMCDisassembler PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMMCJIT PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMMCParser PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMScalarOpts PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMSelectionDAG PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    #set_target_properties( LLVMSupport PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    #set_target_properties( LLVMSystem PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMTarget PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMTransformUtils PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMVectorizer PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMBarrier PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMX86AsmParser PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMX86AsmPrinter PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMX86CodeGen PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMX86Disassembler PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( LLVMX86Info PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    if ( INCLUDE_MIC_DEVICE )
        set_target_properties( LLVMY86AsmParser PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
        set_target_properties( LLVMY86AsmPrinter PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
        set_target_properties( LLVMY86CodeGen PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
        set_target_properties( LLVMY86Disassembler PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
        set_target_properties( LLVMY86Info PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    endif ()
    set_target_properties( ModuleMaker PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( not PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( opt PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    #set_target_properties( tblgen PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    #set_target_properties( X86CodeGenTable_gen PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( count PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( EnhancedDisassembly PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( Fibonacci PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( FileCheck PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    if (MSVC)
        #set_target_properties( hashGen PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    endif()
    set_target_properties( HowToUseJIT PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( intrinsics_gen PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( Kaleidoscope-Ch2 PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( Kaleidoscope-Ch3 PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( Kaleidoscope-Ch4 PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( Kaleidoscope-Ch5 PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( Kaleidoscope-Ch6 PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( Kaleidoscope-Ch7 PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( BrainF PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    #set_target_properties( c-index-test PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    set_target_properties( bugpoint PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    #set_target_properties( stress PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    #set_target_properties( COPY_BACKEND_CLANG PROPERTIES FOLDER ${BACKEND_FOLDER_NAME} )
    #set_target_properties( GoogleTest PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    #set_target_properties( LLVMUnitTests PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    #set_target_properties( UnitTestMain PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
    
    set_target_properties( check PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check.deps PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_barrier PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_barrier.deps PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_codegen PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_codegen.deps PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_images PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_images.deps PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_ocl_backend_passes PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_ocl_backend_passes.deps PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_ocl_ref PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_ocl_ref.deps PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_ocl_ref_conf PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_ocl_ref_conf.deps PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_regression PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_regression.deps PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_vectorizer PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )
    set_target_properties( check_vectorizer.deps PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/Tests )

    set_target_properties( Comparator PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( DataGenerator PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( DataManager PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    if(WIN32)
        set_target_properties( NEAT_wrapper PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
        set_target_properties( REFALU_wrapper PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    else ()
        set_target_properties( NEAT PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
        set_target_properties( REFALU PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    endif()
    set_target_properties( PlugInNEAT PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( OCLBuiltins PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( SATest PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( LLVMInterpreterPluggable PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( ValidationTests PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation/tests )
    set_target_properties( NEATChecker PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation/tests )
    set_target_properties( BackEndTests PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation/tests )
    set_target_properties( OCLSamplePlugin PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation/tests )
    set_target_properties( BackendPassesWrappers PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation )
    set_target_properties( FMAtest PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/validation/tests )
    set_target_properties( tinyxml_STL PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/externals )
    
    # copying MICRegression suite tests to install directory, excluding .svn
    # files 
    install(DIRECTORY ${BACKEND_ROOT_DIR}/tests/MIC/satest/mic/
        DESTINATION bin/validation/MICRegression/
        PATTERN ".svn" EXCLUDE)
endif(LLVM_BUILD_APPLE_BACKEND)


