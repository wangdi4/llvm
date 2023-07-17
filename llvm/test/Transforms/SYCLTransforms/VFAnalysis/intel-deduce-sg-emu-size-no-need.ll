; No need to do emulation if kernel has no subgroup.
; No need to do emulation if kernel with subgroup can be vectorized to VF > 1.

; RUN: opt -sycl-enable-subgroup-emulation -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; RUN: opt -sycl-enable-subgroup-emulation=false -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; CHECK-LABEL: Kernel --> SGEmuSize:
; CHECK-DAG: <none> : 0
; CHECK-DAG: <has_subgroup> : 0
; CHECK-DAG: <has_subgroup_false> : 0

define void @none() {
  ret void
}

define void @has_subgroup() "has-sub-groups" !kernel_has_sub_groups !{i1 true} {
  ret void
}

define void @has_subgroup_false() !intel_vec_len_hint !{i32 1} !kernel_has_sub_groups !{i1 false} {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @none, ptr @has_subgroup, ptr @has_subgroup_false}
