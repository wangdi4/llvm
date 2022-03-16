# INTEL_CUSTOMIZATION
if (NOT EXISTS ${LLVM_EXTERNAL_XPTI_SOURCE_DIR})
  message (FATAL_ERROR "Undefined LLVM_EXTERNAL_XPTI_SOURCE_DIR variable: Must be set for tests to work!")
endif()
include_directories(${LLVM_EXTERNAL_XPTI_SOURCE_DIR}/include)
#end INTEL_CUSTOMIZATION

# Only download Google Test outside of LLVM tree.
if (NOT DEFINED LLVM_EXTERNAL_XPTIFW_SOURCE_DIR)
  # Download and unpack googletest at configure time
  configure_file(../CMakeLists.txt.in googletest-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
  if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
  if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
  endif()

  # Prevent overriding the parent project's compiler/linker
  # settings on Windows
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  # Add googletest directly to our build. This defines
  # the llvm_gtest and llvm_gtest_main targets.
  add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                  ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                  EXCLUDE_FROM_ALL)

  # The llvm_gtest/llvm_gtest_main targets carry header search
  # path dependencies automatically when using CMake 2.8.11 or
  # later. Otherwise we have to add them here ourselves.
  if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
  endif()
endif()

add_executable(XPTIFWUnitTests EXCLUDE_FROM_ALL
  xpti_api_tests.cpp
  xpti_correctness_tests.cpp
)

# Silence warnings from GTest
target_include_directories(XPTIFWUnitTests SYSTEM PRIVATE
  ${gtest_SOURCE_DIR}/include
  ${LLVM_MAIN_SRC_DIR}/utils/unittest/googletest/include
)

# INTEL_CUSTOMIZATION

# This piece is replaced with logic below
# if (TARGET LLVMSupport)
#   set(SUPPORT_LIB LLVMSupport)
# endif()

if (LLVM_LIBCXX_USED AND NOT SYCL_USE_LIBCXX)
  llvm_map_components_to_libnames(llvm_libs Support_stdcpp)
  set(SUPPORT_LIB ${llvm_libs} llvm_gtest_stdcpp llvm_gtest_main_stdcpp stdc++)
  string( REPLACE "-stdlib=libc++" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
  string( REPLACE "-stdlib=libc++" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" )
  string( REPLACE "-stdlib=libc++" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" )
  string( REPLACE "-static-libstdc++" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
  string( REPLACE "-static-libstdc++" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" )
  string( REPLACE "-static-libstdc++" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" )
  set(EXTRA_FLAGS -stdlib=libstdc++)
elseif(WIN32)
  llvm_map_components_to_libnames(llvm_libs Support_dyn)
  set(SUPPORT_LIB ${llvm_libs} llvm_gtest_dyn llvm_gtest_main_dyn)
  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_options(XPTIFWUnitTests PRIVATE "/MDd")
  else()
    target_compile_options(XPTIFWUnitTests PRIVATE "/MD")
  endif()
else()
  llvm_map_components_to_libnames(llvm_libs Support)
  set(SUPPORT_LIB ${llvm_libs} llvm_gtest llvm_gtest_main)
endif()
# end INTEL_CUSTOMIZATION

target_link_libraries(XPTIFWUnitTests PRIVATE
  # llvm_gtest INTEL_CUSTOMIZATION
  # llvm_gtest_main INTEL_CUSTOMIZATION
  xptifw
  ${SUPPORT_LIB}
)

if (EXTRA_FLAGS)
  target_compile_options(XPTIFWUnitTests PRIVATE ${EXTRA_FLAGS})
endif()

add_test(NAME XPTIFWTests COMMAND XPTIFWUnitTests)

add_custom_target(check-xptifw-unittest COMMAND
  $<TARGET_PROPERTY:XPTIFWUnitTests,RUNTIME_OUTPUT_DIRECTORY>/XPTIFWUnitTests)
add_dependencies(check-xptifw-unittest XPTIFWUnitTests)
add_dependencies(check-xptifw check-xptifw-unittest)
