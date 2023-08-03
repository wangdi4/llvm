; Checks the finalized VF with vec_type_hint.

; RUN: opt -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; CHECK-COUNT-2: warning: kernel "{{unsupported_half|unsupported_i1}}": Kernel can't be vectorized due to unsupported vec type hint

; CHECK-LABEL: Kernel --> VF:

; CHECK-DAG: <unsupported_half> : 1
; CHECK-DAG: <unsupported_i1> : 1
; CHECK-DAG: <vec_len_hint> : 16

; CHECK-LABEL: Kernel --> SGEmuSize:

define void @unsupported_half() !vec_type_hint !{half undef, i32 0} {
  ret void
}

define void @unsupported_i1() !vec_type_hint !{i1 undef, i32 0} {
  ret void
}

; Doesn't check type hint if length hint is present.
define void @vec_len_hint() !intel_vec_len_hint !{i32 16} !vec_type_hint !{i1 undef, i32 0} {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @unsupported_half, ptr @unsupported_i1, ptr @vec_len_hint}
