; RUN: opt -passes=dpcpp-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=dpcpp-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s

; CHECK: DPCPPKernelAnalysisPass
; CHECK: Kernel <kernel_contains_matrix_call>:
; CHECK:   NoBarrierPath=1
; CHECK:   KernelHasMatrixCall=1
; CHECK:   KernelHasSubgroups=0
; CHECK:   KernelHasGlobalSync=0
; CHECK:   KernelExecutionLength=3


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
