set(TARGET_NAME soft_math)

set(TARGET_INCLUDE_FILES
  fp_extend.h
  fp_lib.h
  fp_trunc.h
  int_endianness.h
  int_lib.h
  int_math.h
  int_types.h
  int_util.h
)

set(TARGET_SOURCE_FILES
  extendhfsf2.c
  int_util.c
  truncdfhf2.c
  truncsfhf2.c
)

if(NOT WIN32)
  list(APPEND TARGET_SOURCE_FILES extenddftf2.c extendsftf2.c trunctfdf2.c
                                  trunctfhf2.c trunctfsf2.c)
endif(NOT WIN32)

if(WIN32 AND BUILD_X64)
  list(APPEND TARGET_SOURCE_FILES divti3.c udivti3.c udivmodti4.c)
endif()

add_opencl_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_INCLUDE_FILES}
  ${TARGET_SOURCE_FILES}
)

# For a tidy Visual Studio solution layout
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME})
