set(TARGET_NAME GenericAddressResolution)

set(TARGET_INCLUDE_FILES
    GenericAddressResolution.h
    GenericAddressStaticResolution.h
    GenericAddressDynamicResolution.h
)

set(TARGET_SOURCE_FILES
    GenericAddressResolutionUtils.cpp
    GenericAddressStaticResolution.cpp
    GenericAddressStaticResolutionCases.cpp
    GenericAddressDynamicResolution.cpp
    GenericAddressDynamicResolutionCases.cpp
)

include_directories(
    ${BACKEND_ROOT_DIR}/name_mangling
    ${BACKEND_ROOT_DIR}/metadata_api
    ${CMAKE_BINARY_DIR}/include
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

if (OPENCL_INTREE_BUILD)
  add_dependencies(${TARGET_NAME} intrinsics_gen)
endif()

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
