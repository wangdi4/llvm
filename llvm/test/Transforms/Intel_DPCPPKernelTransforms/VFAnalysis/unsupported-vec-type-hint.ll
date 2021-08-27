; Checks the finalized VF with vec_type_hint.

; RUN: opt -dpcpp-kernel-vf-analysis -analyze %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-TYPE-HINT
; RUN: opt -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-TYPE-HINT

; CHECK-LABEL: Kernel --> VF:

; CHECK-TYPE-HINT-COUNT-2: <unsupported_{{.*}}> : 1

; CHECK-LABEL: Kernel --> SGEmuSize:

define void @unsupported_half() !vec_type_hint !{half undef, i32 0} {
  ret void
}

define void @unsupported_i1() !vec_type_hint !{i1 undef, i32 0} {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void ()* @unsupported_half, void ()* @unsupported_i1}
