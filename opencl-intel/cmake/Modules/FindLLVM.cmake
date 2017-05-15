# Find the native LLVM includes and library
#
#  LLVM_LIBRARY_DIRS - where to find llvm libs
#  LLVM_INCLUDE_DIRS - where to find llvm include files
#  LLVM_MODULE_LIBS - list of llvm libs for working with modules.

if(BUILD_LLVM_FROM_SOURCE )
  set (LLVM_INCLUDE_DIRS ${LLVM_SRC_ROOT}/include ${LLVM_SRC_ROOT}/tools/clang/include ${CMAKE_CURRENT_BINARY_DIR}/llvm/include ${CMAKE_CURRENT_BINARY_DIR}/llvm/tools/clang/include)
  set (LLVM_LIBRARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}/llvm/lib/${CMAKE_CFG_INTDIR})
  set (LLVM_BINARY_DIR  ${CMAKE_CURRENT_BINARY_DIR}/llvm/bin/${CMAKE_CFG_INTDIR})
  set (LLVM_MODULE_LIBS ${STATIC_LLVM_MODULE_LIBS})
else(BUILD_LLVM_FROM_SOURCE )
  if (APPLE)
    set(LLVM_INCLUDE_DIRS   "/System/Library/Frameworks/OpenGL.framework/PrivateHeaders/llvm/include")
    set(CVMS_INCLUDE_DIR   "/System/Library/Frameworks/OpenGL.framework/PrivateHeaders")
  else (APPLE)
    # reset the LLVM related variables
    set( LLVM_MODULE_LIBS )
    if( DEFINED CMAKE_MODULE_PATH_ORIGINAL)
      set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH_ORIGINAL})
    else()
      set(CMAKE_MODULE_PATH_ORIGINAL ${CMAKE_MODULE_PATH})
    endif()

    # detect where to look for the LLVM installation
    if( DEFINED LLVM_PATH )
        set(LLVM_INSTALL_PATH ${LLVM_PATH})
    else()
      message( FATAL_ERROR "Can't find LLVM library. Please specify LLVM library location using LLVM_PATH parameter to CMAKE" )
    endif()

    # detect where the LLVMConfig should be found
    if( DEFINED CMAKE_BUILD_TYPE )
      set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LLVM_PATH}/lib/cmake/llvm)
    else()
      # we need to include some file from LLVM share, the problem is
      # that for VS the LLVM_PATH is not full but contain the ${CMAKE_CFG_INTDIR} macro
      # we need to explicitly check for existence of either release or debug cmake then
      string( REPLACE  \${CMAKE_CFG_INTDIR} Debug LLVM_PATH_DEBUG "${LLVM_PATH}" )
      string( REPLACE  \${CMAKE_CFG_INTDIR} Release LLVM_PATH_RELEASE "${LLVM_PATH}" )
      set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LLVM_PATH_DEBUG}/share/llvm/cmake ${LLVM_PATH_RELEASE}/share/llvm/cmake)
    endif()
    set( CMAKE_MODULE_PATH_SAVE ${CMAKE_MODULE_PATH})
    include( LLVMConfig )
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH_SAVE})

    if( DEFINED LLVM_PACKAGE_VERSION)
        message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    else()
        message(FATAL_ERROR "Can't find LLVM in supplied path: ${LLVM_PATH}")
    endif()

    if (${LLVM_PACKAGE_VERSION} VERSION_LESS "3.5")
      #reset some LLVM internal variables that were set in LLVMConfig
      set(LLVM_TOOLS_BINARY_DIR ${LLVM_PATH}/bin)
      set(LLVM_INSTALL_PREFIX ${LLVM_PATH})

      #set variable used further in the project
      set(LLVM_INCLUDE_DIRS  ${LLVM_PATH}/include)
      set(LLVM_LIBRARY_DIRS  ${LLVM_PATH}/lib)
      set(LLVM_BINARY_DIR  ${LLVM_PATH}/bin)

      #configure the string to include expanded ${CMAKE_CFG_INTDIR}
      string( CONFIGURE ${LLVM_INCLUDE_DIRS} LLVM_INCLUDE_DIRS)
      string( CONFIGURE ${LLVM_LIBRARY_DIRS} LLVM_LIBRARY_DIRS)
      string( CONFIGURE ${LLVM_BINARY_DIR}  LLVM_BINARY_DIR)

      #remove predefined unnecessary components
      get_property(temp_list GLOBAL PROPERTY LLVM_LIBS)
      list(REMOVE_ITEM temp_list gtest gtest_main profile_rt-static profile_rt-shared LTO LTO_static)
      set_property( GLOBAL PROPERTY LLVM_LIBS ${temp_list})
    else()
      #LLVM_BINARY_DIR is set only if AddLLVM.cmake is used.
      set(LLVM_BINARY_DIR ${LLVM_INSTALL_PREFIX}/bin)

      #remove predefined unnecessary components
      list(REMOVE_ITEM LLVM_AVAILABLE_LIBS gtest gtest_main profile_rt-static profile_rt-shared LTO LTO_static)
    endif()

    #build the list of the llvm libraries in the right dependency order
    if (${LLVM_PACKAGE_VERSION} VERSION_LESS "3.5")
      llvm_map_components_to_libraries(LLVM_MODULE_LIBS_SHORT all)
    else()
      llvm_map_components_to_libnames(LLVM_MODULE_LIBS all)
    endif()

    if (${LLVM_PACKAGE_VERSION} VERSION_LESS "3.5")
      #patch all the libraries to contain the full path
      get_system_libs(SYSTEM_LIBS)
      foreach(LLVM_LIB ${LLVM_MODULE_LIBS_SHORT})
        list(FIND SYSTEM_LIBS ${LLVM_LIB} IDX)
        if (IDX EQUAL -1)
          list(APPEND LLVM_MODULE_LIBS ${LLVM_LIBRARY_DIRS}/${CMAKE_STATIC_LIBRARY_PREFIX}${LLVM_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX})
        else()
          list(APPEND LLVM_MODULE_LIBS ${LLVM_LIB})
        endif()
      endforeach(LLVM_LIB)
    endif()
  endif (APPLE)
endif(BUILD_LLVM_FROM_SOURCE )

#kept as comment for the debug purposes.
#message( STATUS "LLVM libs: ${LLVM_MODULE_LIBS}")
