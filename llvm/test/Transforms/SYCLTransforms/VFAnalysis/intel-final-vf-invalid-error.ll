; Checks that the final VF must be power of 2.

; RUN: not opt -sycl-force-vf=42 -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

; CHECK: error: kernel "test": Vectorization width 42 is not a power of 2

define void @test() {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @test}
