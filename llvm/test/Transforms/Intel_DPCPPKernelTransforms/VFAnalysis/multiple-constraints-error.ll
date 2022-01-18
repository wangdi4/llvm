; Checks that only one of ForceVF, "intel_vec_len_hint" and "intel_reqd_sub_group_size" can be specified.
; If not, a diagnostic message is emitted.

; RUN: not opt -dpcpp-kernel-vf-analysis %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-NO-FORCE
; RUN: not opt -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-NO-FORCE
; RUN: not opt -dpcpp-kernel-vf-analysis -dpcpp-force-vf=4 %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-FORCE
; RUN: not opt -passes="print<dpcpp-kernel-vf-analysis>" -dpcpp-force-vf=4 %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-FORCE

define void @recommended() !recommended_vector_length !{i32 4} {
  ret void
}

define void @vec_len_hint() !intel_vec_len_hint !{i32 16} {
  ret void
}

define void @reqd_sg_size() !intel_reqd_sub_group_size !{i32 8} {
  ret void
}

define void @vec_len_hint_and_reqd_sg_size() !intel_vec_len_hint !{i32 16} !intel_reqd_sub_group_size !{i32 8} {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void ()* @recommended, void ()* @vec_len_hint, void ()* @reqd_sg_size, void ()* @vec_len_hint_and_reqd_sg_size}

; No diagnostics with zero or one constraint.
; CHECK-NO-FORCE-NOT: error: kernel "{{recommended|vec_len_hint|reqd_sg_size}}"
; CHECK-FORCE-NOT: error: kernel "recommended"

; Report error with multiple constraints.
; CHECK-NO-FORCE: error: kernel "vec_len_hint_and_reqd_sg_size": Only one of CL_CONFIG_CPU_VECTORIZER_MODE, intel_vec_len_hint and intel_reqd_sub_group_size can be specified
; CHECK-FORCE: error: kernel "vec_len_hint": Only one of CL_CONFIG_CPU_VECTORIZER_MODE, intel_vec_len_hint and intel_reqd_sub_group_size can be specified
; No other errors since the optimizer will early exit due to this fatal error.
