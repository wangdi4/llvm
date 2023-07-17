; Emit error when subgroup semantics is really broken (because intel_reqd_sub_group_size = 1).

; RUN: not opt -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; CHECK: error: kernel "reqd_sg_size": Required subgroup size can't be 1 for subgroup calls

define void @reqd_sg_size() "has-sub-groups" !kernel_has_sub_groups !{i1 true} !intel_reqd_sub_group_size !{i32 1} {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @reqd_sg_size}
