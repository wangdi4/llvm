add_llvm_library(LLVMIntel_LoopTransforms 
        Intel_LoopTransforms.cpp
        SSADeconstruction.cpp
        HIRPrinter.cpp
        HIRCodeGen.cpp
        HIROptPredicate.cpp
        HIRCompleteUnroll.cpp
        HIRGeneralUnroll.cpp
        HIRDummyTransformation.cpp
        HIRLoopInterchange.cpp
        )

add_subdirectory(Utils)
