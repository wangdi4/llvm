add_library(LLVMOpenCLAliasAnalysisSupport
    OpenCLAliasAnalysis.cpp
  )

set_target_properties( LLVMOpenCLAliasAnalysisSupport PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
include_directories (../)

