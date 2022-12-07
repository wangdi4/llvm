; RUN: opt -passes=dpcpp-kernel-analysis -dpcpp-kernel-analysis-assume-isamx=true %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=dpcpp-kernel-analysis -dpcpp-kernel-analysis-assume-isamx=true %s -S -debug -disable-output 2>&1| FileCheck %s

; If assume dpcpp-kernel-analysis's isamx is false, check if the diagnose is reported
; RUN: not opt -passes=dpcpp-kernel-analysis -dpcpp-kernel-analysis-assume-isamx=false %s -S -debug -disable-output 2>&1| FileCheck %s -check-prefixes=CHECK-DPCPPKernelAnalysis-BROKEN

; CHECK: DPCPPKernelAnalysisPass
; CHECK: Kernel <kernel_contains_matrix_call>:
; CHECK:   NoBarrierPath=1
; CHECK:   KernelHasMatrixCall=1
; CHECK:   KernelHasSubgroups=0
; CHECK:   KernelHasGlobalSync=0
; CHECK:   KernelExecutionLength=3

; CHECK-DPCPPKernelAnalysis-BROKEN: error: llvm.experimental.matrix.load.v8i8.p0i32": AMX matrix primitives are being used on an arch older than Sapphire Rapids! DPC++ joint matrix extension requires presence of AMX on Sapphire Rapids or later


define void @kernel_contains_matrix_call(i32* %ptr, i32* %dst, i64 %stride) {
entry:
  %0 = call <8 x i8> @llvm.experimental.matrix.load.v8i8.p4i8.v2(i32* %ptr,  i64 %stride, i1 false, i32 4, i32 2, metadata !"matrix.use.b", metadata !"matrix.packed.b", metadata !"matrix.packed.b", metadata !"scope.subgroup")
  call void @llvm.experimental.matrix.store.v8i8.p4i8.v2(<8 x i8> %0, i32* %dst,  i64 %stride, i1 false, i32 4, i32 2, metadata !"matrix.use.b", metadata !"matrix.packed.b", metadata !"matrix.packed.b", metadata !"scope.subgroup")
  ret void
}
declare <8 x i8> @llvm.experimental.matrix.load.v8i8.p4i8.v2(i32*, i64, i1, i32, i32, metadata, metadata, metadata, metadata);
declare void @llvm.experimental.matrix.store.v8i8.p4i8.v2(<8 x i8>, i32*, i64, i1, i32, i32, metadata, metadata, metadata, metadata);

attributes #0 = { convergent }

!sycl.kernels = !{!0}
!0 = !{void (i32* , i32* , i64)* @kernel_contains_matrix_call}

; DEBUGIFY-NOT: WARNING
