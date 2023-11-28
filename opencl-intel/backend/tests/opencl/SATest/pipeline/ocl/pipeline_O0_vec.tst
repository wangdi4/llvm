; RUN: CL_CONFIG_CPU_O0_VECTORIZATION=1 SATest -BUILD -pass-manager-type=ocl -llvm-option="-debug-pass-manager" -config=%s.cfg 2>&1 | FileCheck %s

; CHECK: Running pass: ReqdSubGroupSizePass
; CHECK-NEXT: Running pass: SetVectorizationFactorPass
; CHECK-NEXT: Running analysis: VFAnalysis
; CHECK-NEXT: Running analysis: WeightedInstCountAnalysis on test

; CHECK: Running pass: VectorVariantLowering
; CHECK-NEXT: Running pass: CreateSimdVariantPropagation
; CHECK-NEXT: Running pass: SGSizeCollectorPass
; CHECK-NEXT: Running pass: SGSizeCollectorIndirectPass
; CHECK-NEXT: Running pass: SYCLKernelVecClonePass

; CHECK: Running pass: VectorVariantFillIn
; CHECK-NEXT: Running pass: UpdateCallAttrs

; CHECK: Running pass: VPOCFGRestructuringPass on test (7 instructions)

; CHECK: Running pass: vpo::VPlanDriverLLVMPass on test (7 instructions)

; CHECK: Running pass: SYCLKernelPostVecPass

; CHECK: Running pass: VPODirectiveCleanupPass on test (7 instructions)
