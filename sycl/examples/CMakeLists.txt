if( INTEL_CUSTOMIZATION )
  # Skip building examples on xmain as add_sycl_executable does not
  # configure clang to use custom gcc toolchain so that clang
  # uses system installed gcc which may appear too outdated.
  message(STATUS "Skip building SYCL examples")
else()
  add_sycl_executable(simple-dpcpp-app SOURCES simple-dpcpp-app.cpp)
endif()
