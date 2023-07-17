;; Have a byval function with kernel-call-once attribute, checks that vectorization is disabled.
; RUN: opt -passes="sycl-kernel-add-function-attrs,print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefixes=CHECK-NO-VECTORIZATION

; If vectorization on byval/byref function is enabled, no need to do emulation then.
; RUN: opt -sycl-enable-byval-byref-function-call-vectorization -passes="sycl-kernel-add-function-attrs,print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefixes=CHECK-VECTORIZATION

; CHECK-LABEL: Kernel --> VF:
; CHECK-NO-VECTORIZATION: <kernel> : 1
; CHECK-VECTORIZATION: <kernel> : 16

; CHECK-LABEL: Kernel --> SGEmuSize:

%struct.A = type { float, i32, double, i64 }

; sycl-kernel-add-function-attrs will add "kernel-call-once" to all barrier-calling functions.
define void @f1(ptr byval(%struct.A) align 8 %arg) !kernel_arg_base_type !2 !arg_type_null_val !3 {
entry:
  call void @_Z18work_group_barrierj12memory_scope(i32 1, i32 1)
  ret void
}

define void @kernel(ptr nocapture readonly %arr) !intel_reqd_sub_group_size !1 !kernel_arg_base_type !4 !arg_type_null_val !5 {
entry:
  call void @f1(ptr nonnull byval(%struct.A) align 8 %arr)
  ret void
}

declare void @_Z18work_group_barrierj12memory_scope(i32, i32)

!sycl.kernels = !{!0}

!0 = !{ptr @kernel}
!1 = !{i32 16}
!2 = !{!"%struct.A"}
!3 = !{ptr null}
!4 = !{!"%struct.A*"}
!5 = !{ptr null}
