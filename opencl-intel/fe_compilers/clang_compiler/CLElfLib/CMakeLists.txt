set(TARGET_NAME CLElfLib)

set(TARGET_INCLUDE_FILES
    ElfReader.h
    ElfWriter.h
)

set(TARGET_SOURCE_FILES
    ElfReader.cpp
    ElfWriter.cpp
)

if (WIN32)
    include_directories( AFTER
        ../../common/windows/headers
    )
else()
    include_directories( AFTER
        ../../common/linux/headers
    )
endif()    

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER clang_compiler)
