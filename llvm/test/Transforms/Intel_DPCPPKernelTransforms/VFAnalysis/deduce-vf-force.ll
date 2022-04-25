; Checks the deduced VF after analyzing VF constraints.

; RUN: opt -dpcpp-force-vf=32 -dpcpp-kernel-vf-analysis -analyze -enable-new-pm=0 %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-FORCE
; RUN: opt -dpcpp-force-vf=32 -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-FORCE

; CHECK-LABEL: Kernel --> VF:

; Force VF to 32:
; CHECK-FORCE: <none> : 32

; CHECK-LABEL: Kernel --> SGEmuSize:

define void @none() {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{void ()* @none}
