set(TARGET_NAME CacheBinaryHandler)

set(TARGET_INCLUDE_FILES
    cache_binary_handler.h
    elf_binary.h
)

set(TARGET_SOURCE_FILES
    cache_binary_handler.cpp
    elf_binary.cpp
)

include_directories( AFTER
    ${CMAKE_SOURCE_DIR}/utils/CLElfLib
    ${CMAKE_SOURCE_DIR}/cl_api
    ${CL_API_HEADERS}
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME})

install( TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)
