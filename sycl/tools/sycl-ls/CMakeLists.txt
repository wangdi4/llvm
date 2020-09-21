# INTEL_CUSTOMIZATION
foreach(flag_var
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
string(REGEX REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
endforeach()
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
