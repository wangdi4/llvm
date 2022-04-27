; Checks the deduced VF after analyzing VF constraints.

; RUN: opt -dpcpp-kernel-vf-analysis -analyze -enable-new-pm=0 %s -S 2>&1 | FileCheck %s
; RUN: opt -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; CHECK-LABEL: Kernel --> VF:

; VF is default to 4 (SSE arch).
; CHECK-DAG: <none> : 4
; Overrides recommended_vector_length:
; CHECK-DAG: <vec_len_hint> : 16
; CHECK-DAG: <reqd_sg_size> : 8
; Inherits from existing recommended_vector_length:
; CHECK-DAG: <recommended_vector_length> : 16

; CHECK-LABEL: Kernel --> SGEmuSize:

define void @none() {
  ret void
}

define void @vec_len_hint() !intel_vec_len_hint !{i32 16} {
  ret void
}

define void @reqd_sg_size() !intel_reqd_sub_group_size !{i32 8} {
  ret void
}

define void @recommended_vector_length() !recommended_vector_length !{i32 16} {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void ()* @none, void ()* @vec_len_hint, void ()* @reqd_sg_size, void ()* @recommended_vector_length}
