use_rtti( FALSE )

set(TARGET_NAME SPIRMetadataAdder)

include_directories( AFTER
            ${LLVM_INCLUDE_DIR}
            )

add_library(${TARGET_NAME}
    SPIRMetadataAdder.cpp
)

add_definitions( -D__STDC_LIMIT_MACROS )
add_definitions( -D__STDC_CONSTANT_MACROS )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER clang_compiler )
