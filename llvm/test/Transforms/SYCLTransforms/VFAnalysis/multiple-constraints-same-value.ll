; Checks that diagnostic message isn't emitted if ForceVF, "intel_vec_len_hint"
; and "intel_reqd_sub_group_size" have the same value.

; RUN: opt -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s
; RUN: opt -passes="print<sycl-kernel-vf-analysis>" -sycl-force-vf=8 %s -S 2>&1 | FileCheck %s

define void @vec_len_hint() !intel_vec_len_hint !{i32 8} {
  ret void
}

define void @reqd_sg_size() !intel_reqd_sub_group_size !{i32 8} {
  ret void
}

define void @vec_len_hint_and_reqd_sg_size() !intel_vec_len_hint !{i32 8} !intel_reqd_sub_group_size !{i32 8} {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @vec_len_hint, ptr @reqd_sg_size, ptr @vec_len_hint_and_reqd_sg_size}

; CHECK-NOT: error: kernel
