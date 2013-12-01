add_library(LLVMOpenCLAliasAnalysisSupport
    OpenCLAliasAnalysis.cpp
    OpenCLAliasAnalysis.h
  )

set_target_properties( LLVMOpenCLAliasAnalysisSupport PROPERTIES FOLDER ${BACKEND_FOLDER_NAME}/llvm )
include_directories (../)

