; Checks the deduced VF after analyzing VF constraints.

; RUN: opt -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; CHECK-LABEL: Kernel --> VF:

; heuristic VF is 4 (SSE arch).
; CHECK-DAG: <heuristic> : 4
; Overrides heuristic VF:
; CHECK-DAG: <vec_len_hint> : 16
; CHECK-DAG: <reqd_sg_size> : 8

; CHECK-LABEL: Kernel --> SGEmuSize:

define void @heuristic() {
  ret void
}

define void @vec_len_hint() !intel_vec_len_hint !{i32 16} {
  ret void
}

define void @reqd_sg_size() !intel_reqd_sub_group_size !{i32 8} {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @heuristic, ptr @vec_len_hint, ptr @reqd_sg_size}
