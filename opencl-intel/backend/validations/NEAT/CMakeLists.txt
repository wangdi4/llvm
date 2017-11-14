use_eh(TRUE)

set (TARGET_NAME "NEAT")

# To be sure that ICC compiler uses the correct presentation of 80-bit long doubles
# we have to set proper compiler options, for win32 only, for linux gcc(g++) is used and 
# we don't need to set such options
if (WIN32)
  set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ICC_AUXILIARY_CXX_FLAGS}" )
  set ( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${ICC_AUXILIARY_C_FLAGS}" )
endif(WIN32)

include_directories(
  ${LLVM_INCLUDE_DIRS}
  ${BACKEND_ROOT_DIR}/../cl_api
  ${CL_API_HEADERS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../REFALU
  ${CMAKE_CURRENT_SOURCE_DIR}/../ImathLibd
  ${CMAKE_CURRENT_SOURCE_DIR}/../Common
)

add_library(${TARGET_NAME} STATIC
  IntervalError.cpp
  NEATALU.cpp
  NEATValue.cpp
  NEATVector.cpp
  NEAT_WRAP.cpp
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_VALIDATION_FOLDER_NAME} )
