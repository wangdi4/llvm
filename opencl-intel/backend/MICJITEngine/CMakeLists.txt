add_llvm_library(LLVMMICCodeGenerationEngine
    MICCodeGenerationEngine.cpp
  )
set_target_properties( LLVMMICCodeGenerationEngine PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
include_directories (../)
