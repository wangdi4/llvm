add_llvm_library(LLVMMICModuleJIT ModuleJITHolder.cpp)
set_target_properties( LLVMMICModuleJIT PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
include_directories(../../)

