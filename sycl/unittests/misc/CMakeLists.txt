# INTEL_CUSTOMIZATION
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(sycl_lib_dir $<TARGET_FILE_DIR:sycl>)
add_definitions(-DSYCL_LIB_DIR="${sycl_lib_dir}")

# Enable exception handling for these unit tests
set(LLVM_REQUIRES_EH 1)
add_llvm_unittest(MiscTests
  OsUtils.cpp
)

add_dependencies(MiscTests sycl)
target_link_libraries(MiscTests PRIVATE sycl LLVMTestingSupport)
# end INTEL_CUSTOMIZATION
