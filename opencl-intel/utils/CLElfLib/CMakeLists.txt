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

if (WIN32)
    include_directories( AFTER
        ../../fe_compilers/common/windows/headers
    )
else()
    include_directories( AFTER
        ../../fe_compilers/common/linux/headers
    )
endif()    

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER clang_compiler)

install( TARGETS ${TARGET_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib)