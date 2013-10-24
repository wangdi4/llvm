#
# Project variables:
#  OCL_SOURCE_DIR    - points to the top src directory
#  OCL_BINARY_DIR    - points to the directory outside of OCL_SOURCE_DIR - root of generated files
#

# set that name of the main output file as a target name
set( TARGET_NAME clang_compiler${BUILD_PLATFORM} )

include_directories( AFTER
            ${LLVM_INCLUDE_DIR}
            ${CMAKE_SOURCE_DIR}/backend/metadata_api/gen
            ${CMAKE_SOURCE_DIR}/backend/metadata_api
            ${CMAKE_SOURCE_DIR}/backend/plugin_manager
            ${CMAKE_SOURCE_DIR}/backend/ocl_cpu_backend/export
            ${CMAKE_SOURCE_DIR}/backend/dynamic_lib
            ${CMAKE_SOURCE_DIR}/utils/cl_sys_utils/export
            ${OCL_BINARY_DIR}
            )

            
if (NOT WIN32)
    set(HW_UTILS_HEADERS ../../utils/cl_hw_utils/export)
    set(HW_UTILS_PROJ_NAME cl_hw_utils)
endif (NOT WIN32)

if (WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        add_definitions( -DRESOURCE_X64 )
    endif (CMAKE_SIZEOF_VOID_P EQUAL 8) 
    
    if (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2008
        set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${IMPLIB_SUBDIR}/VS2008/${OCL_SYS_DEPENDENT_SUBDIR} )
    else (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2010
        set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${IMPLIB_SUBDIR}/VS2010/${OCL_SYS_DEPENDENT_SUBDIR} )
    endif (${MSVC_VERSION} EQUAL 1500)
else (WIN32)
    set( TBB_LIB_DIR ${OCL_SOURCE_DIR}/externals/tbb/${OCL_SYS_DEPENDENT_IMPLIB_SUBDIR} )
endif (WIN32)

link_directories( 
            ${TBB_LIB_DIR}
            ${LLVM_LIBRARY_DIR}
            )

set(FILES_TO_COPY 
        ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX} 
        )
            
FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} SHARED ${TARGET_SOURCES})

install(TARGETS ${TARGET_NAME} 
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION bin)

#COPY_GENERATED_FILES_POST_BUILD( ${TARGET_NAME} ${FILES_TO_COPY} )
#COPY_GENERATED_FILES_POST_BUILD( ${TARGET_NAME} ${FILES_TO_COPY} )

if (NOT WIN32) 
  list(APPEND LINK_LIBS
        LLVMCppBackendCodeGen
        LLVMCppBackendInfo
      )
  if (NOT ANDROID)
     list(APPEND LINK_LIBS pthread)
  endif (NOT ANDROID)
endif (NOT WIN32)


# The list of clang libraries is taken from clang makefile
# (build/tools/clang/tools/driver/CMakeFiles/clang.dir/build.make)
# All duplicate libraries are there on purpose 
target_link_libraries( ${TARGET_NAME} ${HW_UTILS_PROJ_NAME}
                                      cl_sys_utils
                                      cl_logger
                                      ${LINK_LIBS} 
                                      task_executor${BUILD_PLATFORM}
                                      clangFrontendTool
                                      clangAST
                                      clangAnalysis
                                      clangBasic
                                      clangCodeGen
                                      clangDriver
                                      clangEdit
                                      clangFrontend
                                      clangLex
                                      clangParse
                                      clangEdit
                                      clangARCMigrate
                                      clangRewriteCore
                                      clangRewriteFrontend
                                      clangSema
                                      clangSerialization
                                      clangStaticAnalyzerFrontend
                                      clangStaticAnalyzerCheckers
                                      clangStaticAnalyzerCore
                                      clangRewriteFrontend
                                      clangFrontend
                                      clangDriver
                                      clangSerialization
                                      clangRewriteCore
                                      clangParse
                                      clangSema
                                      clangAnalysis
                                      clangEdit
                                      clangAST
                                      clangLex
                                      clangBasic
                                      OclPluginManager
                                      metadataapi
                                      dynamic_load
                                      cl_sys_utils
                                      ${LLVM_MODULE_LIBS}
                                      )

set( OPENCL_PCH_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

if( ANDROID )
  set( TRIPLE -triple i686-pc-linux )
endif( ANDROID )

set( COMPILER_OPTIONS -cc1
                      ${TRIPLE}
                      -x cl
                      -O0
                      -emit-pch 
                      -I ${OCL_CLANG_HEADERS}
                      -D __IMAGE2D_DEPTH__
                      -D __WRITE_IMAGE3D__
                      )

set( COMPILER_OPTIONS_OPENCL_12
					${COMPILER_OPTIONS}
                      )

set( COMPILER_OPTIONS_OPENCL_20 
					${COMPILER_OPTIONS}
					-cl-std=CL2.0
					-D__OPENCL_C_VERSION__=200
                      )

set( REDIRECTION_OPTION < ${OCL_CLANG_HEADERS}/opencl_.h
)

if( CMAKE_CROSSCOMPILING OR LLVM_USE_NATIVE OR ANDROID )
   set(TOOL_BINARY_DIR "${LLVM_BINARY_DIR}/../native/bin")
else()
   set(TOOL_BINARY_DIR "${LLVM_BINARY_DIR}" )
endif()
    
set( CLANG ${TOOL_BINARY_DIR}/clang )

add_custom_command( TARGET ${TARGET_NAME}
                    PRE_BUILD
                    COMMAND ${CLANG} ${COMPILER_OPTIONS_OPENCL_12} -o ${OPENCL_PCH_OUT_DIR}/opencl_.pch ${REDIRECTION_OPTION}
                    COMMENT "Generate non-optimized Intel OpenCL precompiled headers"
                    VERBATIM
                   )
add_custom_command( TARGET ${TARGET_NAME}
                    PRE_BUILD
                    COMMAND ${CLANG} ${COMPILER_OPTIONS_OPENCL_12} -triple i686-pc-linux -o ${OPENCL_PCH_OUT_DIR}/opencl_android_.pch ${REDIRECTION_OPTION}
                    COMMENT "Generate non-optimized Intel OpenCL precompiled headers"
                    VERBATIM
                   )

add_custom_command( TARGET ${TARGET_NAME}
                    PRE_BUILD
                    COMMAND ${CLANG} ${COMPILER_OPTIONS_OPENCL_20} -o ${OPENCL_PCH_OUT_DIR}/opencl20_.pch ${REDIRECTION_OPTION}
                    COMMENT "Generate non-optimized Intel OpenCL20 precompiled headers"
                    VERBATIM
                   )
add_custom_command( TARGET ${TARGET_NAME}
                    PRE_BUILD
                    COMMAND ${CLANG} ${COMPILER_OPTIONS_OPENCL_20} -triple i686-pc-linux -o ${OPENCL_PCH_OUT_DIR}/opencl20_android_.pch ${REDIRECTION_OPTION}
                    COMMENT "Generate non-optimized Intel OpenCL20 precompiled headers"
                    VERBATIM
                   )

add_dependencies( ${TARGET_NAME}
	clang
	${OCL_CLANG_HEADERS}/opencl_.h
)

install(FILES ${OPENCL_PCH_OUT_DIR}/opencl_.pch DESTINATION bin)
install(FILES ${OPENCL_PCH_OUT_DIR}/opencl20_.pch DESTINATION bin)
install(FILES ${OPENCL_PCH_OUT_DIR}/opencl_android_.pch DESTINATION bin)
install(DIRECTORY ${OCL_CLANG_HEADERS}/ DESTINATION include/fe_include PATTERN *.h PATTERN .svn EXCLUDE )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER clang_compiler )

if (WIN32)
    get_target_property(RT_OUTPUT_DIRECTORY ${TARGET_NAME} RUNTIME_OUTPUT_DIRECTORY)
    file(TO_NATIVE_PATH ${RT_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/${TARGET_NAME}_stripped.pdb PDB_NAME)
    if (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2008 
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS    "${LINK_FLAGS} /PDBSTRIPPED:${PDB_NAME}")        
    else (${MSVC_VERSION} EQUAL 1500)
        # Visual Studio 2010 (assumed if not Visual Studio 2008)
        # This is a fix due to a bug in CMake, Does not add the flag /DEBUG to the linker flags in Release mode.
        # The /DEBUG flag is required in order to create stripped pdbs.        
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG    "${LINK_FLAGS_DEBUG} /PDBSTRIPPED:${PDB_NAME}")
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE    "${LINK_FLAGS_RELEASE} /DEBUG /PDBSTRIPPED:${PDB_NAME}")
    endif (${MSVC_VERSION} EQUAL 1500)
    if (INSTALL_PDBS)    
        install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}.pdb DESTINATION bin)
    endif(INSTALL_PDBS)    
    install(FILES ${RT_OUTPUT_DIRECTORY}/\${BUILD_TYPE}/${TARGET_NAME}_stripped.pdb DESTINATION bin)
else (WIN32)
    set_target_properties( ${TARGET_NAME} PROPERTIES LINK_FLAGS "-Wl,-Bsymbolic")
endif(WIN32)

add_definitions( -D__STDC_LIMIT_MACROS )
add_definitions( -D__STDC_CONSTANT_MACROS )

set( CMAKE_CXX_FLAGS_DEBUG
  "${CMAKE_CXX_FLAGS_DEBUG} -D OCL_DEV_BACKEND_PLUGINS -D OCLFRONTEND_PLUGINS"
)

