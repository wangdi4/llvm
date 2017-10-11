# remove the restriction of warning as error for this component
if (NOT WIN32)
  remove_definitions( -Werror )
endif (NOT WIN32)

set( TARGET cpu_device_test_type )

include_directories( AFTER ${OCL_SOURCE_DIR}/utils/cl_sys_utils/export )

set( IGNORE_FILES cpu_device_test_type.cpp )

FIND_SOURCES( ${IGNORE_FILES} )
CALCULATE_TARGET_SOURCES()

link_directories( ${TBB_LIB_DIR} )

set( TARGET_LIBS cl_sys_utils
    cpu_device${BUILD_PLATFORM}${OPENCL_BINARIES_POSTFIX}
    task_executor${BUILD_PLATFORM}${OPENCL_BINARIES_POSTFIX} test_utils )

add_ocl_unittest( ${TARGET} SOURCE_FILES ${TARGET_SOURCES}
    LINK_LIBRARIES ${TARGET_LIBS} )

set( INCLUDE_HEADERS_PATH ${LLVM_INCLUDE_DIRS}/cclang)

set( INPUT_FILE test.cl )
set( OUTPUT_FILE test.bc )

if ( BUILD_X64 )
  set( SPIR_TRIPLE spir64-unknown-unknown )
else ( BUILD_X64 )
  set( SPIR_TRIPLE spir-unknown-unknown )
endif ( BUILD_X64 )

add_custom_command( TARGET ${TARGET}
    POST_BUILD
    COMMAND ${CLANG} -cc1 -mllvm -loopopt=false -x cl
    ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
    -I ${INCLUDE_HEADERS_PATH} -emit-llvm-bc -triple ${SPIR_TRIPLE}
    -include opencl-c.h -O3 -o ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE}
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE}
    DEPENDS ${INCLUDE_HEADERS_PATH}/opencl-c.h
    BYPRODUCTS ${OUTPUT_FILE}
    COMMENT "Generate optimized ${OUTPUT_FILE}"
    VERBATIM )

add_dependencies( ${TARGET} clang_compiler${BUILD_PLATFORM} )
