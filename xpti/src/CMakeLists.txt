cmake_minimum_required(VERSION 2.8.9)
project (xpti)
file(GLOB SOURCES *.cpp)
include_directories(${XPTI_DIR}/include)
add_definitions(-DXPTI_STATIC_LIBRARY)
add_library(xpti STATIC ${SOURCES})

# Handle the debug version for the Microsoft compiler as a special case by
# creating a debug version of the static library that uses the flags used by
# the SYCL runtime
if (MSVC)
  add_library(xptid STATIC ${SOURCES})
  target_compile_options(xptid PRIVATE ${XPTI_CXX_FLAGS_DEBUG})
  target_compile_options(xpti PRIVATE ${XPTI_CXX_FLAGS_RELEASE})
# INTEL_CUSTOMIZATION
  add_definitions(-D_HAS_EXCEPTIONS=0)
else()
# Don't use libcxx library to build sycl library if it is not explicitly stated to do so
  string( REPLACE "-stdlib=libc++" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )

# Don't link statically against standard C++ library
  string( REPLACE "-static-libstdc++" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )

# end INTEL_CUSTOMIZATION
endif()

# Set the location of the library installation
install(TARGETS xpti DESTINATION ${CMAKE_BINARY_DIR})
