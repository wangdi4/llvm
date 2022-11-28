; RUN: SATest -BUILD -llvm-option="-debug-pass-manager -opt-bisect-limit=0" -config=%s.cfg 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-DEFAULT
; RUN: SATest -BUILD -llvm-option="-debug-pass-manager -opt-bisect-limit=0" -config=%s.O0.cfg 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-DEFAULT
; RUN: SATest -BUILD -llvm-option="-debug-pass-manager -opt-bisect-limit=0" -stop-before-jit -config=%s.dbg.cfg 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-DBG
; RUN: SATest -BUILD -llvm-option="-debug-pass-manager -opt-bisect-limit=0" -stop-before-jit -config=%s.O0.dbg.cfg 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-DBG

; stop-before-jit for -g test because the test crashes due to CodeGen
; LowerEmuTLS pass being skipped.

; TODO add subgroup kernel when O0 vectorization is enabled by default.

; CHECK: Running pass: DPCPPEqualizerPass
; CHECK: Running pass: LinearIdResolverPass
; CHECK: Running pass: ResolveVarTIDCallPass
; CHECK: Running pass: SGRemapWICallPass
; CHECK: Running pass: BuiltinCallToInstPass
; CHECK: Running pass: DetectRecursionPass
; CHECK: Running pass: DuplicateCalledKernelsPass
; CHECK: Running pass: DPCPPKernelAnalysisPass
; CHECK: Running pass: ResolveSubGroupWICallPass
; CHECK: Running pass: PreventDivCrashesPass
; CHECK: Running pass: DPCPPKernelWGLoopCreatorPass
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
