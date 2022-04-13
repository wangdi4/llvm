; Checks VF = 1 if dpcpp-force-optnone is enabled even when optnone is not present.

; RUN: opt -dpcpp-force-optnone=true -dpcpp-kernel-vf-analysis -analyze %s -S 2>&1 | FileCheck %s
; RUN: opt -dpcpp-force-optnone=true -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; CHECK-LABEL: Kernel --> VF:

; CHECK-DAG: <none> : 1
; CHECK-DAG: <vec_len_hint> : 1
; CHECK-DAG: <reqd_sg_size> : 1

; CHECK-LABEL: Kernel --> SGEmuSize:

define void @none() noinline {
  ret void
}

define void @vec_len_hint() noinline !intel_vec_len_hint !{i32 16} {
  ret void
}

define void @reqd_sg_size() noinline !intel_reqd_sub_group_size !{i32 8} {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void ()* @none, void ()* @vec_len_hint, void ()* @reqd_sg_size}
