; Checks VF = 1 if there's optnone.

; RUN: opt -dpcpp-kernel-vf-analysis -analyze -enable-new-pm=0 %s -S 2>&1 | FileCheck %s
; RUN: opt -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; CHECK-LABEL: Kernel --> VF:

; optnone
; CHECK-DAG: <none> : 1
; CHECK-DAG: <vec_len_hint> : 1
; CHECK-DAG: <reqd_sg_size> : 1

; CHECK-LABEL: Kernel --> SGEmuSize:

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

!0 = !{void ()* @none, void ()* @vec_len_hint, void ()* @reqd_sg_size}
