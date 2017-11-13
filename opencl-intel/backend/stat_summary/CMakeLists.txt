# remove the restriction of warning as error for this component
if (NOT WIN32)
  remove_definitions( -Werror )
endif (NOT WIN32)

add_opencl_executable(SumStats
    FileList.cpp
    Stats.cpp
    SumStats.cpp

    INCLUDE_DIRS
    ${LLVM_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/include/
    ${BACKEND_ROOT_DIR}/metadata_api

    LINK_LIBS
    metadataapi
    ${LLVM_MODULE_LIBS}
    )
