add_llvm_library(LLVMIntel_LoopTransforms 
        Intel_LoopTransforms.cpp
        HIRPrinter.cpp
        HIRCodeGen.cpp
        HIRCompleteUnroll.cpp
        )

add_subdirectory(Utils)
