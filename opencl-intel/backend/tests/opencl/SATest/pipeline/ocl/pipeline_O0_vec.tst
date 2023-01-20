; RUN: SATest -BUILD -pass-manager-type=ocl -llvm-option="-debug-pass-manager -enable-o0-vectorization" -config=%s.cfg 2>&1 | FileCheck %s

; CHECK: Running pass: ReqdSubGroupSizePass
; CHECK-NEXT: Running pass: SetVectorizationFactorPass
; CHECK-NEXT: Running analysis: VFAnalysis
; CHECK-NEXT: Running analysis: WeightedInstCountAnalysis on test

; CHECK: Running pass: VectorVariantLowering
; CHECK-NEXT: Running pass: CreateSimdVariantPropagation
; CHECK-NEXT: Running pass: SGSizeCollectorPass
; CHECK-NEXT: Running pass: SGSizeCollectorIndirectPass
; CHECK-NEXT: Running pass: DPCPPKernelVecClonePass

; CHECK: Running pass: VectorVariantFillIn
; CHECK-NEXT: Running pass: UpdateCallAttrs

; CHECK: Running pass: VPOCFGRestructuringPass on test (7 instructions)

; CHECK: Running pass: vpo::VPlanDriverPass on test (7 instructions)

; CHECK: Running pass: DPCPPKernelPostVecPass

; CHECK: Running pass: VPODirectiveCleanupPass on test (7 instructions)

; CHECK-NOT: Running pass: SGBuiltinPass
; CHECK-NOT: Running analysis: SGSizeAnalysisPass
; CHECK-NOT: Running pass: SGBarrierPropagatePass
; CHECK-NOT: Running pass: SGBarrierSimplifyPass
; CHECK-NOT: Running pass: SGValueWidenPass
; CHECK-NOT: Running pass: SGLoopConstructPass
; CHECK-NOT: Invalidating analysis: SGSizeAnalysisPass
