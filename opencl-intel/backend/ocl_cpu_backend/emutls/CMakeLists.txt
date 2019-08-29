set(TARGET_NAME emutls)

set(TARGET_SOURCE_FILES
  emutls.c
)

# Compile emutls built-in without debug information unconditionally
# The use of the built-in is always implicit and should be hidden from the user
set( CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g0")
set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g0")
set( CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g0")


add_library(${TARGET_NAME}
    ${TARGET_SOURCE_FILES}
)
