; Checks VF = 1 if there's optnone and O0 vectorization is disabled.

; RUN: opt -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s
; RUN: opt -sycl-enable-o0-vectorization -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s --check-prefix=O0_VECTORIZATION

; CHECK-LABEL: Kernel --> VF:

; optnone
; CHECK-DAG: <none> : 1
; CHECK-DAG: <vec_len_hint> : 1
; CHECK-DAG: <reqd_sg_size> : 1

; CHECK-LABEL: Kernel --> SGEmuSize:

; O0 vectorization + optnone
; O0_VECTORIZATION-DAG: <vec_len_hint> : 16
; O0_VECTORIZATION-DAG: <reqd_sg_size> : 8

; O0_VECTORIZATION-LABEL: Kernel --> SGEmuSize:

define void @none() noinline optnone {
  ret void
}

define void @vec_len_hint() noinline optnone !intel_vec_len_hint !{i32 16} {
  ret void
}

define void @reqd_sg_size() noinline optnone !intel_reqd_sub_group_size !{i32 8} {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @none, ptr @vec_len_hint, ptr @reqd_sg_size}
