add_opencl_executable(SumStats
    FileList.cpp
    Stats.cpp
    SumStats.cpp

    INCLUDE_DIRS
    ${LLVM_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/include/
    ${BACKEND_ROOT_DIR}/metadata_api

    LINK_LIBS
    ${LLVM_MODULE_LIBS}
    )
