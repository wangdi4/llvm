; RUN: opt -passes=sycl-kernel-analysis -sycl-kernel-analysis-assume-isamx=true -sycl-kernel-analysis-assume-isamxfp16=true %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-analysis -sycl-kernel-analysis-assume-isamx=true -sycl-kernel-analysis-assume-isamxfp16=true %s -S -debug -disable-output 2>&1| FileCheck %s

; If assume sycl-kernel-analysis's isamx-fp16 is false, check if the diagnose is reported
; RUN: not opt -passes=sycl-kernel-analysis -sycl-kernel-analysis-assume-isamx=true -sycl-kernel-analysis-assume-isamxfp16=false %s -S -debug -disable-output 2>&1| FileCheck %s -check-prefixes=CHECK-SYCLKernelAnalysis-BROKEN

; CHECK: SYCLKernelAnalysisPass
; CHECK: Kernel <kernel_contains_matrix_call_fp16>:
; CHECK:   NoBarrierPath=1
; CHECK:   KernelHasMatrixCall=1
; CHECK:   KernelHasSubgroups=0
; CHECK:   KernelHasGlobalSync=0
; CHECK:   KernelExecutionLength=3

; CHECK-SYCLKernelAnalysis-BROKEN: error: llvm.experimental.matrix.load.v8f16.p0": AMX-FP16 matrix primitives are being used on an arch older than Granite Rapids! DPC++ joint matrix extension requires presence of AMX on Sapphire Rapids or later


define void @kernel_contains_matrix_call_fp16(ptr %ptr, ptr %dst, i64 %stride) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %0 = call <8 x half> @llvm.experimental.matrix.load.v8f16.p4.v2(ptr %ptr,  i64 %stride, i1 false, i32 4, i32 2, metadata !"matrix.use.b", metadata !"matrix.packed.b", metadata !"matrix.packed.b", metadata !"scope.subgroup")
  call void @llvm.experimental.matrix.store.v8f16.p4.v2(<8 x half> %0, ptr %dst,  i64 %stride, i1 false, i32 4, i32 2, metadata !"matrix.use.b", metadata !"matrix.packed.b", metadata !"matrix.packed.b", metadata !"scope.subgroup")
  ret void
}
declare <8 x half> @llvm.experimental.matrix.load.v8f16.p4.v2(ptr, i64, i1, i32, i32, metadata, metadata, metadata, metadata);
declare void @llvm.experimental.matrix.store.v8f16.p4.v2(<8 x half>, ptr, i64, i1, i32, i32, metadata, metadata, metadata, metadata);

attributes #0 = { convergent }

!sycl.kernels = !{!0}
!0 = !{ptr @kernel_contains_matrix_call_fp16}
!1 = !{!"int*", !"int*", !"long*"}
!2 = !{ptr null, ptr null, ptr null}

; DEBUGIFY-NOT: WARNING
