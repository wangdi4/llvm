; Checks that ForceVF, "intel_vec_len_hint" and "intel_reqd_sub_group_size"
; can't have different values. If not, a diagnostic message is emitted.

; RUN: not opt -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-NO-FORCE
; RUN: not opt -passes="print<sycl-kernel-vf-analysis>" -sycl-force-vf=4 %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-FORCE

define void @heuristic() {
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

!0 = !{ptr @heuristic, ptr @vec_len_hint, ptr @reqd_sg_size, ptr @vec_len_hint_and_reqd_sg_size}

; No diagnostics with zero or one constraint.
; CHECK-NO-FORCE-NOT: error: kernel "{{heuristic|vec_len_hint|reqd_sg_size}}"
; CHECK-FORCE-NOT: error: kernel "heuristic"

; Report error with multiple conflicting constraints.
; CHECK-NO-FORCE: error: kernel "vec_len_hint_and_reqd_sg_size": Conflicting CL_CONFIG_CPU_VECTORIZER_MODE, intel_vec_len_hint and intel_reqd_sub_group_size are specified
; CHECK-FORCE: error: kernel "vec_len_hint": Conflicting CL_CONFIG_CPU_VECTORIZER_MODE, intel_vec_len_hint and intel_reqd_sub_group_size are specified
; No other errors since the optimizer will early exit due to this fatal error.
