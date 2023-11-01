; RUN: CL_CONFIG_CPU_O0_VECTORIZATION=1 SATest -BUILD -pass-manager-type=lto -llvm-option="-debug-pass-manager" -config=%S/pipeline_lto_O0.tst.cfg 2>&1 | FileCheck %s
; RUN: CL_CONFIG_CPU_O0_VECTORIZATION=1 SATest -BUILD -llvm-option="-debug-pass-manager" -config=%S/pipeline_lto_O0.tst.cfg 2>&1 | FileCheck %s

; CHECK: Running pass: ReqdSubGroupSizePass
; CHECK-NEXT: Running pass: SetVectorizationFactorPass
; CHECK-NEXT: Running analysis: VFAnalysis on [module]
; CHECK-NEXT: Running analysis: WeightedInstCountAnalysis on test

; CHECK: Running pass: VectorVariantLowering on [module]
; CHECK-NEXT: Running pass: CreateSimdVariantPropagation on [module]
; CHECK-NEXT: Running pass: SGSizeCollectorPass on [module] 
; CHECK-NEXT: Running pass: SGSizeCollectorIndirectPass on [module]

; CHECK: Running pass: SYCLKernelVecClonePass on [module] 

; CHECK: Running pass: VectorVariantFillIn on [module]
; CHECK-NEXT: Running pass: UpdateCallAttrs on [module]

; CHECK: Running pass: VPOCFGRestructuringPass on test (7 instructions)

; CHECK: Running pass: vpo::VPlanDriverLLVMPass on test (7 instructions)

; CHECK: Running pass: VPODirectiveCleanupPass on test (7 instructions)

; CHECK: Running pass: SYCLKernelPostVecPass on [module]
