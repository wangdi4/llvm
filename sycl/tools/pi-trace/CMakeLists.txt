add_library(pi_trace SHARED pi_trace.cpp)
target_link_libraries(pi_trace PRIVATE xptifw)
# INTEL_CUSTOMIZATON
if (EXISTS ${XPTI_SOURCE_DIR})
  target_include_directories(pi_trace PRIVATE "${XPTI_SOURCE_DIR}/include")
elseif (EXISTS ${LLVM_EXTERNAL_XPTI_SOURCE_DIR})
  target_include_directories(pi_trace PRIVATE "${LLVM_EXTERNAL_XPTI_SOURCE_DIR}/include")
else()
  message (FATAL_ERROR "Undefined XPTI_SOURCE_DIR and LLVM_EXTERNAL_XPTI_SOURCE_DIR variables: Required for XPTI tracing")
endif()
# end INTEL_CUSTOMIZATON
target_include_directories(pi_trace PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../xpti_helpers/")
target_include_directories(pi_trace PRIVATE "${sycl_inc_dir}")
target_include_directories(pi_trace PRIVATE "${sycl_src_dir}")

if(UNIX)
  target_link_libraries(pi_trace PRIVATE dl)
endif()

if (XPTI_ENABLE_TBB)
  target_link_libraries(pi_trace PRIVATE tbb)
endif()
