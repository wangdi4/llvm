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
    ${BACKEND_ROOT_DIR}/passes
    ${BACKEND_ROOT_DIR}/utils
    ${BACKEND_ROOT_DIR}/name_mangling
    ${BACKEND_ROOT_DIR}/../cl_api
    ${BACKEND_ROOT_DIR}/metadata_api
    ${BACKEND_ROOT_DIR}/metadata_api/gen
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)
set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})

add_dependencies(${TARGET_NAME} metadataapi)

if(APPLE)
  file(RELATIVE_PATH SOURCE_RELATIVE_DIR ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  install( FILES ${TARGET_INCLUDE_FILES} ${TARGET_SOURCE_FILES}
           DESTINATION src/${SOURCE_RELATIVE_DIR}
         )
endif(APPLE)
