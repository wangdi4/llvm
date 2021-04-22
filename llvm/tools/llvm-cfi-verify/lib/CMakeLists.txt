set(LLVM_LINK_COMPONENTS
    DebugInfoDWARF
    MC
    MCParser
    Object
    Support
    Symbolize
    )

add_llvm_library(LLVMCFIVerify
  STATIC
  FileAnalysis.cpp
  FileAnalysis.h
  GraphBuilder.cpp
  GraphBuilder.h
  )

# INTEL_CUSTOMIZATION
set_msvc_crt_flags(LLVMCFIVerify)
# end INTEL_CUSTOMIZATION
