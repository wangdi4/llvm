set(TARGET_NAME CacheBinaryHandler)

set(TARGET_INCLUDE_FILES
    cache_binary_handler.h
)

set(TARGET_SOURCE_FILES
    cache_binary_handler.cpp
)

include_directories( AFTER
    ${CMAKE_SOURCE_DIR}/utils/CLElfLib
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
