set(TARGET_NAME name_mangle)

set(TARGET_SOURCE_FILES
  ParameterType.cpp
  Demangler.cpp
  Mangler.cpp
  FunctionDescriptor.cpp
  DemangleParser.cpp
  Utils.cpp
  ParameterType.h
  NameMangleAPI.h
  FunctionDescriptor.h
  Refcount.h
  DemangleParser.h
  Utils.h
)

add_library(
  ${TARGET_NAME}
  STATIC
  ${TARGET_SOURCE_FILES}
)

include_directories(
  ${LLVM_INCLUDE_DIRS}
)

set_target_properties( ${TARGET_NAME} PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/reflection )
