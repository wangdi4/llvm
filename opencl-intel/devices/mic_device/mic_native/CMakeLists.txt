#
# Create native MIC server using icc compiler
#
function (SET_NATIVE_PATH ARG PATH1)
    file( TO_NATIVE_PATH "${PATH1}" MY_TEMP)
    if (WIN32)    
        set( ${ARG} \"${MY_TEMP}\" PARENT_SCOPE )
    endif (WIN32)
endfunction (SET_NATIVE_PATH)

get_filename_component( TARGET_NAME ${MIC_NATIVE_SERVER_EXE} NAME_WE )
get_filename_component( EXE_FILE_EXTENSION ${MIC_NATIVE_SERVER_EXE} EXT )

set( BIN_DIR ${CMAKE_CURRENT_BINARY_DIR} )
set( OBJ_FILE_EXTENSION .o )
set( LIB_FILE_EXTENSION .a )
set( LIB_FILE_PREFIX    lib)

set( DEVICE   MIC )

set ( TASK_EXECUTOR_FLAG __TBB_EXECUTOR__ )
set( OMP_COMPILER_FLAG  )
set( MIC_DA_OMP_FLAG  )

if (MIC_USE_OMP)
    set( TASK_EXECUTOR_FLAG __OMP_EXECUTOR__ )
    set( MIC_DA_OMP_FLAG -D__MIC_DA_OMP__ )
    set( OMP_COMPILER_FLAG -openmp)
endif(MIC_USE_OMP)

set( COI_BUFFER_ADDREF_FLAG )
if ( USE_COI_BUFFS_REF_NEW_API )
    set ( COI_BUFFER_ADDREF_FLAG -DMIC_USE_COI_BUFFS_REF_NEW_API )
endif ( USE_COI_BUFFS_REF_NEW_API )

#
# Set Backend dependencies
#
set( BACKEND_LIB            OclDeviceExecutor )
if (WIN32)
    set( BACKEND_LIB_DIR        ${OCL_BINARY_DIR}/${IMPLIB_SUBDIR}/%CONFIG_NAME% )
else()
    set( BACKEND_LIB_DIR        ${OCL_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR} )
endif (WIN32)

set( BACKEND_SVML_DIR       ${OCL_SOURCE_DIR}/backend/libraries/ocl_builtins/bin/svml/Linux64 )

set( UTILS_DIR              ${OCL_SOURCE_DIR}/utils )
set( SYS_UTILS_DIR          ${UTILS_DIR}/cl_sys_utils )
set( HW_UTILS_DIR           ${UTILS_DIR}/cl_hw_utils )
set( LOGGER_DIR             ${UTILS_DIR}/cl_logger )
set( ITT_DIR                ${OCL_SOURCE_DIR}/externals/itt )
set( MIC_TBB_PATH           ${OCL_SOURCE_DIR}/externals/tbb )
set( DEVICE_COMMON_DIR      ${OCL_SOURCE_DIR}/devices/common)

if (WIN32)
    set( MIC_TBB_LIB_PATH      ${MIC_TBB_PATH}/bin/Knc_%CONFIG_NAME% )
else()
    set( MIC_TBB_LIB_PATH_DEBUG      ${MIC_TBB_PATH}/bin/Knc_Debug )
    set( MIC_TBB_LIB_PATH_RELEASE    ${MIC_TBB_PATH}/bin/Knc_Release )
endif (WIN32)

set( BACKEND_FILES_TO_COPY ${BACKEND_SVML_DIR}/libsvml.so )

if (WIN32)
    # Windows
    set( FORCE_LINUX ON )
    set( ICC_INSTALL_DIR     "C:\\Program Files (x86)\\Intel\\Composer XE" )
    if ( NOT EXISTS ${ICC_INSTALL_DIR})
            message( FATAL_ERROR "ICC for Xeon Phi(MIC) can't be found at ${ICC_INSTALL_DIR}")
    endif ()
    
    set( ICC_COMPILER ${ICC_INSTALL_DIR}\\bin\\intel64_mic\\icc )
    
    set( DEVICE_INIT_ENV_SCRIPT "${ICC_INSTALL_DIR}\\bin\\compilervars.bat" )
    set( DEVICE_LIBS_DIR    ${ICC_INSTALL_DIR}/lib/mic )
    
    set( DEVICE_DEPENDENCY_SCRIPT   dependency.bat )
    set( DEVICE_COMPILER_SCRIPT     compile.bat )
    set( DEVICE_LINKER_SCRIPT       link.bat )
    set( DEVICE_ARCHIVE             "C:\\Program Files\\Intel\\MPSS\\x86_64-mpsssdk-linux\\usr\\bin\\k1om-mpss-linux\\k1om-mpss-linux-ar.exe" )
    set( DEVICE_ASSEMBLER           "C:\\Program Files\\Intel\\MPSS\\x86_64-mpsssdk-linux\\usr\\bin\\k1om-mpss-linux\\k1om-mpss-linux-as.exe" )
    set( DEVICE_PREPROCESSOR        ${ICC_COMPILER} )
    
    if (NOT EXISTS ${DEVICE_ARCHIVE})
        set( DEVICE_ARCHIVE "C:\\Program Files\\Intel\\MPSS\\sdk\\linux-k1om-4.7\\bin\\x86_64-k1om-linux-ar.exe" )
    endif()
    if (NOT EXISTS ${DEVICE_ARCHIVE} )
        message( FATAL_ERROR "Cannot find library archive ${FILE}!")
    endif()
   
else ()
    # Linux
    set( ICC_INSTALL_DIR /opt/intel/composerxe )
    if ( NOT EXISTS ${ICC_INSTALL_DIR})
            message( FATAL_ERROR "ICC for Xeon Phi(MIC) can't be found")
    endif ()

    set( ICC_COMPILER ${ICC_INSTALL_DIR}/bin/icc )

    set( DEVICE_INIT_ENV_SCRIPT "${ICC_INSTALL_DIR}/bin/compilervars.sh" )
    set( DEVICE_LIBS_DIR    ${ICC_INSTALL_DIR}/lib/mic )    
    
    set( DEVICE_DEPENDENCY_SCRIPT   dependency.sh )
    set( DEVICE_COMPILER_SCRIPT     compile.sh )
    set( DEVICE_LINKER_SCRIPT       link.sh )
    set( DEVICE_ARCHIVE             /usr/linux-k1om-4.7/bin/x86_64-k1om-linux-ar )
    set( DEVICE_ASSEMBLER           /usr/linux-k1om-4.7/bin/x86_64-k1om-linux-as )
    set( DEVICE_PREPROCESSOR        cpp)
endif (WIN32)

execute_process(COMMAND ${ICC_COMPILER} -dumpversion OUTPUT_VARIABLE MIC_ICC_VERSION)
if (  MIC_ICC_VERSION VERSION_LESS 13.0 )
    message(FATAL_ERROR "ICC ${MIC_ICC_VERSION} found, expected 13.x or higher")
endif()

message(STATUS "MIC C COMPILER: ${ICC_COMPILER}, version ${MIC_ICC_VERSION}" )

set( DEVICE_DEPENDENCY   ${BIN_DIR}/${DEVICE_DEPENDENCY_SCRIPT} )
set( DEVICE_COMPILER     ${BIN_DIR}/${DEVICE_COMPILER_SCRIPT} )
set( DEVICE_LINKER       ${BIN_DIR}/${DEVICE_LINKER_SCRIPT} )

##############################################################################################
##############################################################################################
##############################################################################################
#
# Check for file/dir existence
#    check_file(FILE)
#
function (check_file FILE )
    if (NOT EXISTS ${FILE})
        message( FATAL_ERROR "Cannot find ${FILE}!")
    endif()
endfunction( check_file )

#
# Usage
#     COMPILE_DEVICE_CXX_FILE( <cxx_file_full_path_name> <obj_file_full_path_name>)
#
function ( COMPILE_DEVICE_CXX_FILE FILE_FULL_PATH_NAME OBJ_FILE )
    get_filename_component( FILE_NAME ${FILE_FULL_PATH_NAME} NAME )

    execute_process(COMMAND ${DEVICE_DEPENDENCY} ${FILE_FULL_PATH_NAME}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/devices/mic_device/mic_native
                    OUTPUT_VARIABLE   DEPENDENCY_DATA
                    )

    # replace the spaces with ; in the shell script and remove the next command
    if (NOT WIN32)
        string( STRIP ${DEPENDENCY_DATA} DEPENDENCY_FILES )
        string( REGEX REPLACE " +" ";" DEPENDENCY_LIST ${DEPENDENCY_FILES} )
        set( FILE_FULL_PATH_NAME_NATIVE ${FILE_FULL_PATH_NAME} )
        set( OBJ_FILE_NATIVE ${OBJ_FILE} )
    else()
        string( REPLACE "#" ";" DEPENDENCY_LIST ${DEPENDENCY_DATA} )
        file( TO_NATIVE_PATH "${FILE_FULL_PATH_NAME}" FILE_FULL_PATH_NAME_NATIVE)
        file( TO_NATIVE_PATH "${OBJ_FILE}" OBJ_FILE_NATIVE)
    endif (NOT WIN32)
    #message( FATAL_ERROR "|${DEPENDENCY_LIST}|" )                
    add_custom_command( OUTPUT ${OBJ_FILE_NATIVE}

                        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
                        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}
                        COMMAND ${DEVICE_COMPILER} ${INSTALL_SUBDIR} ${FILE_FULL_PATH_NAME_NATIVE} ${OBJ_FILE_NATIVE}

                        MAIN_DEPENDENCY ${FILE_FULL_PATH_NAME_NATIVE}
                        IMPLICIT_DEPENDS CXX
                        DEPENDS ${BIN_DIR}/${DEVICE_COMPILER_SCRIPT} ${DEPENDENCY_LIST}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        COMMENT "Compiling ${FILE_NAME} for ${DEVICE}"
                        VERBATIM
                        )

endfunction ( COMPILE_DEVICE_CXX_FILE )

#
# Usage
#     COMPILE_DEVICE_ASM_FILE( <asm_file_full_path_name> <obj_file_full_path_name>)
#
function ( COMPILE_DEVICE_ASM_FILE FILE_FULL_PATH_NAME OBJ_FILE )
    get_filename_component( FILE_NAME ${FILE_FULL_PATH_NAME} NAME )

    add_custom_command( OUTPUT ${OBJ_FILE}

                        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
                        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}
                        COMMAND ${DEVICE_PREPROCESSOR} -E -DKMP_ARCH_X86_64=1 -D__MIC2__=1 -D__MIC__=1 ${FILE_FULL_PATH_NAME} | ${DEVICE_ASSEMBLER} --64 -march=k1om -o ${OBJ_FILE}
                        MAIN_DEPENDENCY ${FILE_FULL_PATH_NAME_NATIVE}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        COMMENT "Compiling ${FILE_NAME} for ${DEVICE}"
                        VERBATIM
                        )

endfunction ( COMPILE_DEVICE_ASM_FILE )

#
# Usage
#     LINK_DEVICE_EXE( <full_path_to_target_exe> <full_path_to_obj>.... )
#
function ( LINK_DEVICE_EXE TARGET_EXE )
    get_filename_component( FILE_NAME ${TARGET_EXE} NAME )

    if (${ARGC} GREATER  1)
        set( OBJ_FILES ${ARGN})
        if ( WIN32 )
            set ( OBJ_FILES_PARAM ${CMAKE_CURRENT_BINARY_DIR}/link_objects.txt)
            file(REMOVE ${OBJ_FILES_PARAM})
            foreach( obj_file_path IN LISTS OBJ_FILES)
                get_filename_component( obj_file_name ${obj_file_path} NAME )
                get_filename_component( obj_file_ext  ${obj_file_path} EXT )
                if ( obj_file_ext STREQUAL "" )
                    file(APPEND ${OBJ_FILES_PARAM} "lib${obj_file_name}.a ")
                else()
                    file(APPEND ${OBJ_FILES_PARAM} "${obj_file_name} ")
                endif()
            endforeach(obj_file_path)
        else()
            set ( OBJ_FILES_PARAM )
            foreach( obj_file_path IN LISTS OBJ_FILES)
                get_filename_component( obj_file_name ${obj_file_path} NAME )
                get_filename_component( obj_file_ext  ${obj_file_path} EXT )
                if ( obj_file_ext STREQUAL "" )
                    list(APPEND OBJ_FILES_PARAM "lib${obj_file_name}.a ")
                else()
                    list(APPEND OBJ_FILES_PARAM "${obj_file_name} ")
                endif()
            endforeach(obj_file_path)
        endif (WIN32)

        add_custom_command( OUTPUT ${TARGET_EXE}
                            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
                            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}
                            COMMAND ${DEVICE_LINKER} ${INSTALL_SUBDIR} ${TARGET_EXE} ${OBJ_FILES_PARAM}
                            DEPENDS  ${OBJ_FILES} ${BIN_DIR}/${DEVICE_LINKER_SCRIPT} ${BACKEND_LIB_DIR}/lib${BACKEND_LIB}.a ${TARGET_DEPEND_LIBRARIES_BINS}
                            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}
                            COMMENT "Linking ${FILE_NAME} for ${DEVICE}"
                            VERBATIM
                            )
    endif (${ARGC} GREATER 1)

endfunction ( LINK_DEVICE_EXE )

#
# Usage
#     add_device_executable( <target_name> <input_cxx_files>.... )
#
function ( add_device_executable TARGET_NAME )

    set( EXE_FILE ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}_${TARGET_NAME}${EXE_FILE_EXTENSION} )
    set( OBJ_LIST )
    set( LIB_LIST )

    if (${ARGC} GREATER  1)
        foreach( SRC_FILE ${ARGN} )
            get_filename_component( FILE_NAME ${SRC_FILE} NAME_WE )
            get_filename_component( FILE_PATH ${SRC_FILE} ABSOLUTE )
            get_filename_component( FILE_EXT  ${SRC_FILE} EXT )
            
            if ( FILE_EXT STREQUAL  "" )
                list( APPEND LIB_LIST ${SRC_FILE} )
            else()
                set( OBJ_FILE ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}/${FILE_NAME}${OBJ_FILE_EXTENSION} )
                COMPILE_DEVICE_CXX_FILE( ${SRC_FILE} ${OBJ_FILE} )
                list( APPEND OBJ_LIST ${OBJ_FILE} )
            endif()
        endforeach( SRC_FILE )

        LINK_DEVICE_EXE( ${EXE_FILE} ${OBJ_LIST} ${LIB_LIST})

        add_custom_target( ${TARGET_NAME} ALL DEPENDS ${EXE_FILE} )
    else ()
        message( FATAL_ERROR "No input files specified for ${TARGET_NAME}" )
    endif (${ARGC} GREATER 1)

endfunction ( add_device_executable )

#
# Usage
#     CREATE_LIB_ARCHIVE( <full_path_to_target_exe> <full_path_to_obj>.... )
#
function ( CREATE_LIB_ARCHIVE LIB_TARGET_NAME )
    #get_filename_component( LIB_TARGET_NAME ${TARGET_LIB} NAME_WE )
    set( LIB_FILE ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}/${LIB_FILE_PREFIX}${LIB_TARGET_NAME}${LIB_FILE_EXTENSION} )

    if (${ARGC} GREATER  1)
    
        set( OBJ_FILES ${ARGN})
        
        add_custom_command( OUTPUT ${LIB_FILE}
                            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}
                            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}
                            COMMAND ${DEVICE_ARCHIVE} rsc --target=elf64-k1om ${LIB_FILE} ${OBJ_FILES}

                            DEPENDS  ${OBJ_FILES} 
                            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}
                            COMMENT "Archiving ${LIB_TARGET_NAME} for ${DEVICE}"
                            VERBATIM
                            )
                            
        add_custom_target ( ${LIB_TARGET_NAME} ALL DEPENDS ${LIB_FILE} )
        set_target_properties( ${LIB_TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME}/mic_device/mic_native )        
    endif (${ARGC} GREATER 1)

endfunction ( CREATE_LIB_ARCHIVE )

#
# Usage
#     add_device_library( <target_name> <input_cxx_files>.... )
#
function ( add_device_library LIBRARY_NAME )
    set( OBJ_LIST )

    if (${ARGC} GREATER  1)
        foreach( SRC_FILE ${ARGN} )
            get_filename_component( FILE_NAME ${SRC_FILE} NAME_WE )
            get_filename_component( FILE_PATH ${SRC_FILE} ABSOLUTE)
            get_filename_component( FILE_EXT  ${SRC_FILE} EXT )

            set( OBJ_FILE ${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_SUBDIR}/${FILE_NAME}${OBJ_FILE_EXTENSION} )
            
            if ( FILE_EXT STREQUAL ".s" )
                COMPILE_DEVICE_ASM_FILE( ${FILE_PATH} ${OBJ_FILE} )
            else()
                COMPILE_DEVICE_CXX_FILE( ${FILE_PATH} ${OBJ_FILE} )
            endif()
            
            list( APPEND OBJ_LIST ${OBJ_FILE} )
        endforeach( SRC_FILE )

        CREATE_LIB_ARCHIVE ( ${LIBRARY_NAME} ${OBJ_LIST} )
    else ()
        message( FATAL_ERROR "No input files specified for ${TARGET_NAME}" )
    endif (${ARGC} GREATER 1)

endfunction ( add_device_library )


#################################################################################################

#check_file(${DEVICE_INIT_ENV_SCRIPT})
check_file(${CMAKE_CURRENT_SOURCE_DIR}/${DEVICE_DEPENDENCY_SCRIPT})
check_file(${CMAKE_CURRENT_SOURCE_DIR}/${DEVICE_COMPILER_SCRIPT})
check_file(${CMAKE_CURRENT_SOURCE_DIR}/${DEVICE_LINKER_SCRIPT})

# substitute vars in the scripts
if (NOT EXISTS ${BIN_DIR})
        file(MAKE_DIRECTORY "${BIN_DIR}")
endif()

# MIC compilation flags
set( MIC_FLAGS  -Werror
                #-std=c++0x
                # next entry - support converting Linux trap signals into C++ exceptions
                -fnon-call-exceptions
                # enable stack security check, influence performance
                #-fstack-security-check
                #enable MASM code inlining in C/C++ code
                -use-msasm
                
                # Save compilation option in the executable
                -sox
                ${OMP_COMPILER_FLAG}

                # allow exclusion of un-referenced code and data
                -fdata-sections
                -ffunction-sections
            )

set( MIC_DEBUG_FLAGS   -g -O0 -D_DEBUG -DDEBUG -DTBB_USE_DEBUG)
set( MIC_RELEASE_FLAGS -O3 -DNDEBUG )

# include path
list( APPEND MIC_FLAGS
                -I ${CMAKE_CURRENT_SOURCE_DIR}
                -I ${CMAKE_CURRENT_SOURCE_DIR}/..
                -I ${SYS_UTILS_DIR}/export
                -I ${HW_UTILS_DIR}/export
                -I ${LOGGER_DIR}/export
                -I ${OCL_SOURCE_DIR}/cl_api
                -I ${MIC_TBB_PATH}/include
                -I ${MIC_TBB_PATH}/enhancements
                -I ${COI_DEVICE_INCLUDE_PATH}
                -I ${COI_YOCTO_DEVICE_INCLUDE_PATH}
                -I ${ITT_DIR}/include
                -I ${ITT_DIR}/ittnotify
                -I ${CMAKE_SOURCE_DIR}/fe_compilers/clang_compiler
                -I ${DEVICE_COMMON_DIR}/builtin_kernels
                -DDEVICE_NATIVE
                -DDISABLE_NUMA_SUPPORT
                -DUSE_ITT
                -I ${CMAKE_SOURCE_DIR}/externals/tbb/include
                -D${TASK_EXECUTOR_FLAG} -DTBB_PREVIEW_LOCAL_OBSERVER=1 -DTBB_PREVIEW_TASK_ARENA=1
                ${MIC_DA_OMP_FLAG}
                ${COI_BUFFER_ADDREF_FLAG}
                -DKNC_CARD
                -fPIC
                )

if (USE_HARD_TRAPPING)
	list( APPEND MIC_FLAGS -D__HARD_TRAPPING__ )
endif()

# DEBUG flags
if (USE_ITT_INTERNAL)
    list( APPEND MIC_FLAGS -DUSE_ITT_INTERNAL )
endif()

if ( MIC_TESTING_MODE )
    list( APPEND MIC_FLAGS -DNDRANGE_UNIT_TEST )
endif ( MIC_TESTING_MODE )

if ( ENABLE_MIC_TRACER )
    list( APPEND MIC_FLAGS -DENABLE_MIC_TRACER )
endif ( ENABLE_MIC_TRACER )

list( APPEND MIC_FLAGS -I ${UTILS_DIR}/task_executor )

option( ENABLE_MIC_TBB_TRACER OFF )
if ( ENABLE_MIC_TBB_TRACER )
    list( APPEND MIC_FLAGS -DENABLE_MIC_TBB_TRACER )
endif( ENABLE_MIC_TBB_TRACER )

set( TBB_LIB_TO_LINK tbb_preview )
set( TBBMALLOC_LIB_TO_LINK tbbmalloc )
if (NOT WIN32)
    if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
        set( MIC_TBB_LIB_PATH ${MIC_TBB_LIB_PATH_DEBUG} )
    else (${CMAKE_BUILD_TYPE} STREQUAL Debug)
            set( MIC_TBB_LIB_PATH ${MIC_TBB_LIB_PATH_RELEASE} )
    endif (${CMAKE_BUILD_TYPE} STREQUAL Debug)
endif (NOT WIN32)
    
# MIC linking flags
set( MIC_LINK_FLAGS      )

SET_NATIVE_PATH(COI_DEVICE_LIB_PATH ${COI_DEVICE_LIB_PATH})

# lib path
list( APPEND MIC_LINK_FLAGS
                ${OMP_COMPILER_FLAG}
                # next entry - support converting Linux trap signals into C++ exceptions
                -fnon-call-exceptions
                 # library search path at link
                -L ${COI_DEVICE_LIB_PATH}
                -L ${BACKEND_LIB_DIR}
                -L ${MIC_TBB_LIB_PATH}
                ${BACKEND_VTUNE_DIR}

                 # libs to link
                -lscif
                -lcoi_device
                -l${BACKEND_LIB}
                -l${TBBMALLOC_LIB_TO_LINK}
                -l${TBB_LIB_TO_LINK}
                -lpthread
                -lrt
                
                ############ Begin of statically linked libraries
                # The following line affects all libraries mentioned after it, add here
                # only libraries which must be linked statically
                -Wl,-Bstatic
                -lirc
                ############ End of statically linked libraries

                # allow exclusion of un-referenced sections (code and data)
                -Wl,--gc-sections

                # add unused defined symbols to the dynamic symbol table - they will be used dynamically
                -rdynamic

                # perform static linkage for internally defined symbols if possible
                -Bsymbolic

                # statically link all intel-specific libs
                -static-intel

                # hide all defined symbols from the loader, except specified in the file
                -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/MicNative_linux_exports.txt

                # look for required shared libs in the same dir as exe resides
                #  \\\$ will result in passing \$ to the linker script
                -Wl,-rpath=\\\$ORIGIN -Wl,--enable-new-dtags
                )

if (NOT WIN32)
    if (${CMAKE_BUILD_TYPE} STREQUAL Debug)
        # Debug
        list( APPEND MIC_FLAGS      ${MIC_DEBUG_FLAGS} )
    else ()
        # Release
        list( APPEND MIC_FLAGS      ${MIC_RELEASE_FLAGS} )
    endif (${CMAKE_BUILD_TYPE} STREQUAL Debug)
endif (NOT WIN32)

set ( TARGET_DEPEND_LIBRARIES native_builtin_kernels native_task_executor native_hw_utils native_sys_utils native_logger)
if (INCLUDE_MKL)
    list( APPEND TARGET_DEPEND_LIBRARIES native_mkl_kernels )
endif (INCLUDE_MKL)

# Append link flags with internal libraries
foreach( depend_library IN LISTS TARGET_DEPEND_LIBRARIES)
    set( FULL_PATH_BIN ${CMAKE_CURRENT_BINARY_DIR}/${depend_library}/${INSTALL_SUBDIR} )
    list( APPEND MIC_LINK_FLAGS "-L ${FULL_PATH_BIN} -l${depend_library}" )
    list( APPEND TARGET_DEPEND_LIBRARIES_BINS ${FULL_PATH_BIN}/${LIB_FILE_PREFIX}${depend_library}${LIB_FILE_EXTENSION} )
endforeach(depend_library)

if (INCLUDE_MKL)
    if (WIN32)
        set( MKL_DIR \"${ICC_INSTALL_DIR}\"/mkl)
    else()
        set( MKL_DIR ${ICC_INSTALL_DIR}/mkl)
    endif(WIN32)
    list( APPEND MIC_FLAGS -D__INCLUDE_MKL__ -I ${DEVICE_COMMON_DIR}/mkl_kernels/export -I ${MKL_DIR}/include  -DMKL_ILP64)
    if( USE_OMP2TBB )
        list( APPEND TARGET_DEPEND_LIBRARIES native_omp2tbb )
        list( APPEND MIC_FLAGS -D__OMP2TBB__ -I ${DEVICE_COMMON_DIR}/omp2tbb)
        set( FULL_PATH_BIN ${CMAKE_CURRENT_BINARY_DIR}/native_omp2tbb/${INSTALL_SUBDIR}/libnative_omp2tbb.a )
        list( APPEND MIC_LINK_FLAGS -Wl,--start-group ${MKL_DIR}/lib/mic/libmkl_intel_ilp64.a ${MKL_DIR}/lib/mic/libmkl_core.a ${MKL_DIR}/lib/mic/libmkl_intel_thread.a ${FULL_PATH_BIN} -Wl,--end-group -lm)
        list( APPEND TARGET_DEPEND_LIBRARIES_BINS ${FULL_PATH_BIN} )
    else()
        list( APPEND MIC_FLAGS -openmp )
        list( APPEND MIC_LINK_FLAGS -Wl,--start-group ${MKL_DIR}/lib/mic/libmkl_intel_ilp64.a ${MKL_DIR}/lib/mic/libmkl_core.a ${MKL_DIR}/lib/mic/libmkl_intel_thread.a -Wl,--end-group -lm)
    endif( USE_OMP2TBB )
endif (INCLUDE_MKL)

# replace ; with spaces
foreach (FLAG ${MIC_FLAGS})
    set( FINAL_MIC_FLAGS "${FINAL_MIC_FLAGS} ${FLAG}")
endforeach( FLAG )

foreach (FLAG ${MIC_LINK_FLAGS})
    set( FINAL_MIC_LINK_FLAGS "${FINAL_MIC_LINK_FLAGS} ${FLAG}")
endforeach( FLAG )

if (WIN32)
    string( REPLACE  / \\ FINAL_MIC_FLAGS ${FINAL_MIC_FLAGS} )
    string( REPLACE  / \\ FINAL_MIC_LINK_FLAGS ${FINAL_MIC_LINK_FLAGS} )
    string( REPLACE  $(Configuration) %CONFIG_NAME% FINAL_MIC_LINK_FLAGS ${FINAL_MIC_LINK_FLAGS} )
endif(WIN32)

# FINAL_MIC_FLAGS and FINAL_MIC_LINK_FLAGS are used now
configure_file(${DEVICE_DEPENDENCY_SCRIPT}  ${DEVICE_DEPENDENCY} @ONLY)
configure_file(${DEVICE_COMPILER_SCRIPT}    ${DEVICE_COMPILER} @ONLY)
configure_file(${DEVICE_LINKER_SCRIPT}      ${DEVICE_LINKER} @ONLY)

if (MIC_USE_OMP)
    set(FILES_TO_COPY ${DEVICE_LIBS_DIR}/libiomp5.so ${DEVICE_LIBS_DIR}/libcilkrts.so.5 )
else (MIC_USE_OMP)
    set(FILES_TO_COPY )
endif ( MIC_USE_OMP)

foreach( depend_library IN LISTS TARGET_DEPEND_LIBRARIES)
    add_subdirectory ( ${depend_library} )
endforeach(depend_library)

# find main sources
set( MAIN_EXCEPTIONS )
FIND_SOURCES( ${MAIN_EXCEPTIONS} )
list(APPEND SOURCE_FILES ../mic_tracer.cpp)
list(APPEND HEADER_FILES ../mic_tracer_counters.h ../mic_tracer.h)
CALCULATE_TARGET_SOURCES()

#Add VTune sources ittnotify interface
list( APPEND TARGET_SOURCES ${ITT_DIR}/ittnotify/ittnotify_static.c )
list( APPEND TARGET_SOURCES ${ITT_DIR}/ittnotify/jitprofiling.c )

add_device_executable( ${TARGET_NAME} ${TARGET_SOURCES} )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${DEVICES_FOLDER_NAME}/mic_device/mic_native )

install(PROGRAMS ${FILES_TO_COPY} ${BACKEND_FILES_TO_COPY} DESTINATION bin/${MIC_NATIVE_SUBDIR_NAME} )
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/Debug_${TARGET_NAME}${EXE_FILE_EXTENSION} DESTINATION bin/${MIC_NATIVE_SUBDIR_NAME}
           CONFIGURATIONS Debug RENAME mic_server  )
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/Release_${TARGET_NAME}${EXE_FILE_EXTENSION} DESTINATION bin/${MIC_NATIVE_SUBDIR_NAME}
           CONFIGURATIONS Release RENAME mic_server  )

#add_dependencies( ${TARGET_NAME} ${BACKEND_LIB} cl_sys_utils)
add_dependencies( ${TARGET_NAME} ${BACKEND_LIB} ${TARGET_DEPEND_LIBRARIES})

if (WIN32)
    install(DIRECTORY ${MIC_TBB_PATH}/bin/Knc_\${CMAKE_INSTALL_CONFIG_NAME}/ DESTINATION bin/${MIC_NATIVE_SUBDIR_NAME})
else ()
    file( GLOB MIC_TBB_LIBS_TO_COPY ${MIC_TBB_LIB_PATH}/*.so ${MIC_TBB_LIB_PATH}/*.so.2 )
    install(FILES ${MIC_TBB_LIBS_TO_COPY} DESTINATION bin/${MIC_NATIVE_SUBDIR_NAME} )
endif (WIN32)
