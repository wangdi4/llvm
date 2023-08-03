; RUN: opt -passes=sycl-kernel-postvec %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-postvec %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

; If the kernel is not vectorized, then the cloned kernel is removed.

; CHECK-NOT: define void @_ZGVeN16_test

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define dso_local void @test() #0 !vectorized_kernel !1 {
entry:
  ret void
}

define void @_ZGVeN16_test() #2 {
entry:
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() #1

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "vector-variants"="_ZGVeN16_test" }
attributes #1 = { nounwind }
attributes #2 = { "vector-variant-failure"="Bailout" }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{ptr @_ZGVeN16_test}

; DEBUGIFY-COUNT-3: WARNING: Missing line
; DEBUGIFY-COUNT-1: WARNING: Missing variable
; DEBUGIFY-NOT: WARNING
