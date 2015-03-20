add_llvm_library(LLVMIntel_LoopTransforms 
        HIRCodeGen.cpp
        HIRCompleteUnroll.cpp
        )

add_subdirectory(Utils)
