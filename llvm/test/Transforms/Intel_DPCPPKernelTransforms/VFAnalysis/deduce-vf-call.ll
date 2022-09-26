; Checks the deduced VF after analyzing VF constraints.

; RUN: opt -dpcpp-kernel-vf-analysis -analyze -enable-new-pm=0 %s -S 2>&1 | FileCheck %s
; RUN: opt -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; CHECK: Kernel --> VF:
; CHECK-DAG: <call_reqd_sg_size> : 8
; CHECK-DAG: <call_vec_len_hint> : 1

; CHECK: Kernel --> SGEmuSize:
; CHECK-DAG: <call_reqd_sg_size> : 0
; CHECK-DAG: <call_vec_len_hint> : 0

define void @call_vec_len_hint() {
  call void @vec_len_hint()
  ret void
}

define void @vec_len_hint() !intel_vec_len_hint !{i32 1} {
  ret void
}

define void @call_reqd_sg_size() {
  call void @reqd_sg_size()
  ret void
}

define void @reqd_sg_size() !intel_reqd_sub_group_size !{i32 8} {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void ()* @call_vec_len_hint, void ()* @call_reqd_sg_size}
