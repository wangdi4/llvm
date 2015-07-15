add_llvm_library(LLVMIntel_LoopTransforms 
        Intel_LoopTransforms.cpp
        SSADeconstruction.cpp
        HIRPrinter.cpp
        HIRCodeGen.cpp
        HIRCompleteUnroll.cpp
        HIRGeneralUnroll.cpp
        )

add_subdirectory(Utils)
