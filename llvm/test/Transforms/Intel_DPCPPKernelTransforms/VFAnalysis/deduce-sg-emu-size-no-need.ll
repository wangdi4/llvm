; No need to do emulation if kernel has no subgroup.
; No need to do emulation if kernel with subgroup can be vectorized to VF > 1.

; RUN: opt -dpcpp-enable-subgroup-emulation -dpcpp-kernel-vf-analysis -analyze %s -S 2>&1 | FileCheck %s
; RUN: opt -dpcpp-enable-subgroup-emulation -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; RUN: opt -dpcpp-enable-subgroup-emulation=false -dpcpp-kernel-vf-analysis -analyze %s -S 2>&1 | FileCheck %s
; RUN: opt -dpcpp-enable-subgroup-emulation=false -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; CHECK-LABEL: Kernel --> SGEmuSize:
; CHECK-DAG: <none> : 0
; CHECK-DAG: <has_subgroup> : 0

define void @none() {
  ret void
}

define void @has_subgroup() "has-sub-groups" !kernel_has_sub_groups !{i1 true} {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void ()* @none, void ()* @has_subgroup}
