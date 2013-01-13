set(TARGET_NAME SPIR)

include_directories(${BACKEND_ROOT_DIR}/llvm/include)

set(SPIR_INCLUDE_FILES
	SPIRVerifier.h
    SPIRMaterializer.h
)

set(SPIR_SOURCE_FILES
    SPIRVerifier.cpp
	SPIRMaterializer.cpp
)

add_library(${TARGET_NAME}
    ${SPIR_INCLUDE_FILES}
    ${SPIR_SOURCE_FILES}
)

add_dependencies(${TARGET_NAME} intrinsics_gen )

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_PASSES_FOLDER_NAME})
