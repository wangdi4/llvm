set(TARGET_NAME WrapSvmlPass)

include_directories(${BACKEND_ROOT_DIR}/utils)

set(WRAPSVML_SOURCE_FILES
    WrapSvml.cpp
    )

set (ASM_SOURCES 
    svml_thunk.asm
    )

CREATE_ASM_RULES( ASM_ADD_TO_SOURCES_LIST ${ASM_SOURCES} )

list(APPEND WRAPSVML_SOURCE_FILES
     ${ASM_ADD_TO_SOURCES_LIST}
    )

add_library(${TARGET_NAME}
    ${WRAPSVML_SOURCE_FILES}
    )
