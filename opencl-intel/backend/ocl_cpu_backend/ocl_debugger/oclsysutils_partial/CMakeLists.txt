# set that name of the main output file as a target name
set( TARGET_NAME oclsysutils_partial )

include (../CMakeFuncs.txt)

add_definitions( -D_SCL_SECURE_NO_WARNINGS )

FIND_SOURCES()
CALCULATE_TARGET_SOURCES()

if(NOT WIN32)
	set (ADD_COMMON_C_FLAGS  "-msse3 -mssse3 -fPIC -fdiagnostics-show-option -funsigned-bitfields")
	set (ADD_CXX_FLAGS       "${ADD_COMMON_C_FLAGS} ")
	set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} ${ADD_CXX_FLAGS}")
	remove_definitions( -Werror -pedantic )
endif(NOT WIN32)

SET_UNICODE_OFF()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../cl_api 
)

# add a target named ${TARGET_NAME}
add_library (${TARGET_NAME} STATIC ${TARGET_SOURCES})

