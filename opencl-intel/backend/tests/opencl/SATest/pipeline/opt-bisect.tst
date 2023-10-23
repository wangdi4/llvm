; RUN: SATest -BUILD -llvm-option="-debug-pass-manager -opt-bisect-limit=0" -config=%s.cfg 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-DEFAULT,CHECK-O3
; RUN: SATest -BUILD -llvm-option="-debug-pass-manager -opt-bisect-limit=0" -config=%s.O0.cfg 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-DEFAULT
; RUN: SATest -BUILD -llvm-option="-debug-pass-manager -opt-bisect-limit=0" -stop-before-jit -config=%s.dbg.cfg 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-DBG,CHECK-O3
; RUN: SATest -BUILD -llvm-option="-debug-pass-manager -opt-bisect-limit=0" -stop-before-jit -config=%s.O0.dbg.cfg 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-DBG

; RUN: SATest -BUILD -llvm-option="-debug-pass-manager -opt-bisect-limit=0" -config=%S/opt-bisect-sycl.tst.cfg 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-DEFAULT,CHECK-O3,CHECK-SYCL

; stop-before-jit for -g test because the test crashes due to CodeGen
; LowerEmuTLS pass being skipped.

; TODO add subgroup kernel when O0 vectorization is enabled by default.

; CHECK: Running pass: KernelTargetExtTypeLowerPass
; CHECK: Running pass: SYCLEqualizerPass
; CHECK: Running pass: LinearIdResolverPass
; CHECK: Running pass: ResolveVarTIDCallPass
; CHECK: Running pass: SGRemapWICallPass
; CHECK: Running pass: BuiltinCallToInstPass
; CHECK: Running pass: DetectRecursionPass
; INTEL_CUSTOMIZATION
; CHECK-SYCL: Running pass: TaskSeqAsyncHandling
; CHECK-SYCL: Running pass: ResolveMatrixFillPass
; CHECK-SYCL: Running pass: ResolveMatrixWISlicePass
; end INTEL_CUSTOMIZATION
; CHECK: Running pass: DuplicateCalledKernelsPass
; CHECK: Running pass: SYCLKernelAnalysisPass
; CHECK: Running pass: MathFuncSelectPass
; CHECK: Running pass: ReqdSubGroupSizePass
; CHECK: Running pass: SetVectorizationFactorPass
; CHECK-O3: Running pass: VectorVariantLowering
; CHECK-O3: Running pass: CreateSimdVariantPropagation
; CHECK-O3: Running pass: SGSizeCollectorPass
; CHECK-O3: Running pass: SGSizeCollectorIndirectPass
; CHECK-O3: Running pass: SYCLKernelVecClonePass
; CHECK-O3: Running pass: VectorVariantFillIn
; CHECK-O3: Running pass: UpdateCallAttrs
; CHECK-O3: Running pass: SYCLKernelPostVecPass
; CHECK-O3: Running pass: HandleVPlanMask
; CHECK-SYCL: Running pass: ResolveMatrixLayoutPass ; INTEL
; CHECK: Running pass: ResolveSubGroupWICallPass
; CHECK: Running pass: PreventDivCrashesPass
; CHECK: Running pass: SYCLKernelWGLoopCreatorPass
; CHECK: Running pass: IndirectCallLowering
; CHECK: Running pass: GroupBuiltinPass
; CHECK: Running pass: BarrierInFunction
; CHECK: Running pass: SplitBBonBarrier
; CHECK: Running pass: KernelBarrier
; CHECK-DEFAULT: Running pass: AddImplicitArgsPass
; CHECK-DBG: Running pass: ResolveWICallPass
; CHECK: Running pass: LocalBuffersPass
; CHECK: Running pass: BuiltinImportPass
; CHECK: Running pass: PatchCallbackArgsPass
; CHECK: Running pass: PrepareKernelArgsPass
