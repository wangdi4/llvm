set(TARGET_NAME CLElfLib)

set(TARGET_INCLUDE_FILES
    ElfReader.h
	ElfWriter.h
)

set(TARGET_SOURCE_FILES
	ElfReader.cpp
	ElfWriter.cpp
)

add_library(${TARGET_NAME}
    ${TARGET_INCLUDE_FILES}
    ${TARGET_SOURCE_FILES}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER clang_compiler)
