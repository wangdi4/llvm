; RUN: SATest -BUILD -pass-manager-type=lto -llvm-option="-debug-pass-manager -enable-o0-vectorization" -config=%S/pipeline_lto_O0.tst.cfg 2>&1 | FileCheck %s
; RUN: SATest -BUILD -llvm-option="-debug-pass-manager -enable-o0-vectorization" -config=%S/pipeline_lto_O0.tst.cfg 2>&1 | FileCheck %s

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

; CHECK: Running pass: vpo::VPlanDriverPass on test (7 instructions)

; CHECK: Running pass: VPODirectiveCleanupPass on test (7 instructions)

; CHECK: Running pass: SYCLKernelPostVecPass on [module]

; CHECK-NOT: Running pass: SGBuiltinPass on [module]
; CHECK-NOT: Running analysis: SGSizeAnalysisPass on [module]
; CHECK-NOT: Running pass: SGBarrierPropagatePass on [module]
; CHECK-NOT: Running pass: SGBarrierSimplifyPass on [module]
; CHECK-NOT: Running pass: SGValueWidenPass on [module]
; CHECK-NOT: Running pass: SGLoopConstructPass on [module]
; CHECK-NOT: Invalidating analysis: SGSizeAnalysisPass on [module]
