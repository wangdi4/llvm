# Find the native LLVM includes and library
#
#  LLVM_INCLUDE_DIR - where to find llvm include files
#  LLVM_LIBRARY_DIR - where to find llvm libs
#  LLVM_MODULE_LIBS - list of llvm libs for working with modules.
#  LLVM_FOUND       - True if llvm found.
if(BUILD_LLVM_FROM_SOURCE )
    set (LLVM_INCLUDE_DIR ${LLVM_SRC_ROOT}/include ${LLVM_SRC_ROOT}/tools/clang/include ${CMAKE_CURRENT_BINARY_DIR}/llvm/include ${CMAKE_CURRENT_BINARY_DIR}/llvm/tools/clang/include)
    set (LLVM_LIBRARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/llvm/lib/${CMAKE_CFG_INTDIR})
    set (LLVM_BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/llvm/bin/${CMAKE_CFG_INTDIR})
    set (LLVM_MODULE_LIBS ${STATIC_LLVM_MODULE_LIBS})
else(BUILD_LLVM_FROM_SOURCE )
    if (APPLE)
        set(LLVM_INCLUDE_DIR   "/System/Library/Frameworks/OpenGL.framework/PrivateHeaders/llvm/include")
        set(CVMS_INCLUDE_DIR   "/System/Library/Frameworks/OpenGL.framework/PrivateHeaders")
    else (APPLE)
        # detect where to look for the LLVM installation
        if( DEFINED LLVM_PATH )
            set(LLVM_INSTALL_PATH ${LLVM_PATH})
            message (STATUS "Using LLVM root location specified in LLVM_PATH variable: " ${LLVM_PATH})
        elseif( EXISTS $ENV{LLVM_PATH} )
            set(LLVM_INSTALL_PATH $ENV{LLVM_PATH})
            message (STATUS "Using LLVM location specified in LLVM_PATH environment variable: " $ENV{LLVM_PATH} )
        else()
            message( FATAL_ERROR "Can't find LLVM library. Please specify LLVM library location using either LLVM_PATH environment variable or defining LLVM_PATH parameter to CMAKE" )
        endif()
        
        # detect where the LLVMConfig should be found
        if( DEFINED CMAKE_BUILD_TYPE )
            set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LLVM_PATH}/share/llvm/cmake)
        else()
            # we need to include some file from LLVM share, the problem is
            # that for VS the LLVM_PATH is not full but contain the ${CMAKE_CFG_INTDIR} macro
            # we need to explicitly check for existence of either release or debug cmake then
            string( REPLACE  \${CMAKE_CFG_INTDIR} Debug LLVM_PATH_DEBUG "${LLVM_PATH}" )
            string( REPLACE  \${CMAKE_CFG_INTDIR} Release LLVM_PATH_RELEASE "${LLVM_PATH}" )
            set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LLVM_PATH_DEBUG}/share/llvm/cmake ${LLVM_PATH_RELEASE}/share/llvm/cmake)
        endif()
        message( STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
        set( CMAKE_MODULE_PATH_SAVE ${CMAKE_MODULE_PATH})
        include( LLVMConfig )
        
        #reset some LLVM internal variables that were set in LLVMConfig
        set(LLVM_TOOLS_BINARY_DIR ${LLVM_PATH}/bin)
        set(LLVM_INSTALL_PREFIX ${LLVM_PATH})
        set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH_SAVE})

        #set variable used further in the project
        set(LLVM_INCLUDE_DIR ${LLVM_PATH}/include)
        set(LLVM_LIBRARY_DIR ${LLVM_PATH}/lib)
        set(LLVM_BINARY_DIR  ${LLVM_PATH}/bin)
            
        #configure the string to include expanded ${CMAKE_CFG_INTDIR}
        string( CONFIGURE ${LLVM_INCLUDE_DIR} LLVM_INCLUDE_DIR)
        string( CONFIGURE ${LLVM_LIBRARY_DIR} LLVM_LIBRARY_DIR)
        string( CONFIGURE ${LLVM_BINARY_DIR}  LLVM_BINARY_DIR)
        
        #remove predefined unnecessary components
        get_property(temp_list GLOBAL PROPERTY LLVM_LIBS)
        list(REMOVE_ITEM temp_list gtest gtest_main profile_rt-static profile_rt-shared LTO LTO_static)
        set_property( GLOBAL PROPERTY LLVM_LIBS ${temp_list})

        #build the list of the llvm libraries in the right dependency order
        llvm_map_components_to_libraries(LLVM_MODULE_LIBS all)
    endif (APPLE)
endif(BUILD_LLVM_FROM_SOURCE )

message( STATUS "LLVM libs: ${LLVM_MODULE_LIBS}")
