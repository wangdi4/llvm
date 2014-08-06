# remove the restriction of warning as error for this component
if (NOT WIN32)
remove_definitions( -Werror )
endif (NOT WIN32)

#include helping functions of cmake.
include (../../cmake_utils/CMakeFuncs.txt)

set( TARGET_NAME SumStats )

include_directories(
    ${CMAKE_BINARY_DIR}/include/
	${BACKEND_ROOT_DIR}/metadata_api
	)

set( TARGET_SOURCES
     FileList.cpp
     Stats.cpp
     SumStats.cpp)

add_executable( ${TARGET_NAME} ${TARGET_SOURCES} )

link_directories (${LLVM_LIBRARY_DIR})

target_link_libraries (${TARGET_NAME}
    metadataapi
    ${LLVM_MODULE_LIBS}
    )

if (NOT WIN32)
  if(NOT ANDROID)
    target_link_libraries(${TARGET_NAME} pthread dl)
  endif(NOT ANDROID)
endif (NOT WIN32)

set( CREATE_DIR_NAME bin)

# Copy generated executable to appropriate test directory in 'Release' area.
install(TARGETS ${TARGET_NAME} DESTINATION ${CREATE_DIR_NAME})
