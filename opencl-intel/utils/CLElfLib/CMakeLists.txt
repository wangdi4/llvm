set(TARGET_NAME CLElfLib)

set(TARGET_INCLUDE_FILES
    ElfReader.h
    ElfWriter.h
)

set(TARGET_SOURCE_FILES
    ElfReader.cpp
    ElfWriter.cpp
)

include_directories( AFTER
    ../cl_sys_utils/export
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${UTILS_FOLDER_NAME})

install( TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)
