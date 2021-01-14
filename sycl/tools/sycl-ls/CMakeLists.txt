# INTEL_CUSTOMIZATION
foreach(flag_var
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
string(REGEX REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
endforeach()
# Don't use libcxx library to build sycl-ls if it is not explicitly stated to do so
if (NOT SYCL_USE_LIBCXX)
  string( REPLACE "-stdlib=libc++" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
  string( REPLACE "-stdlib=libc++" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" )
  string( REPLACE "-stdlib=libc++" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" )
endif()
# Don't link statically against standard C++ library
  string( REPLACE "-static-libstdc++" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
  string( REPLACE "-static-libstdc++" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" )
  string( REPLACE "-static-libstdc++" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}" )
# end INTEL_CUSTOMIZATION
add_executable(sycl-ls sycl-ls.cpp)
add_dependencies(sycl-ls sycl)
target_include_directories(sycl-ls PRIVATE "${sycl_inc_dir}")
# INTEL_CUSTOMIZATION
set(LIB_NAME sycl)
if (WIN32 AND (${CMAKE_BUILD_TYPE} MATCHES Debug))
  set(LIB_NAME sycld)
endif()
# end INTEL_CUSTOMIZATION
target_link_libraries(sycl-ls
  PRIVATE
# INTEL_CUSTOMIZATION
    ${LIB_NAME}
# end INTEL_CUSTOMIZATION
    OpenCL::Headers
)
install(TARGETS sycl-ls
  RUNTIME DESTINATION "bin" COMPONENT sycl-ls)
