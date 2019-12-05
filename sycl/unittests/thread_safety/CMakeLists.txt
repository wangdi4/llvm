# INTEL_CUSTOMIZATION
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable exception handling for these unit tests
set(LLVM_REQUIRES_EH 1)

add_llvm_unittest(ThreadSafetyTests
    HostAccessorDeadLock.cpp
)

add_dependencies(ThreadSafetyTests sycl)
target_link_libraries(ThreadSafetyTests PRIVATE sycl LLVMTestingSupport)
# end INTEL_CUSTOMIZATION
