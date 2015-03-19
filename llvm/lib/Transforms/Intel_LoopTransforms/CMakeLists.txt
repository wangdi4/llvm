add_llvm_library(LLVMIntel_LoopTransforms 
        MockHIR.cpp
        HIRCodeGen.cpp
        HIRCompleteUnroll.cpp
        )

add_subdirectory(Utils)
