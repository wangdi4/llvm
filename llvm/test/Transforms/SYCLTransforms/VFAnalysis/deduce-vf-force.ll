; Checks the deduced VF after analyzing VF constraints.

; RUN: opt -sycl-force-vf=32 -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-FORCE

; CHECK-LABEL: Kernel --> VF:

; Force VF to 32:
; CHECK-FORCE: <none> : 32

; CHECK-LABEL: Kernel --> SGEmuSize:

define void @none() {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @none}
