; REQUIRES: asserts

; RUN: opt -passes=sycl-kernel-postvec %s -S -debug-only=vplan-vec 2>&1 | FileCheck %s
; RUN: opt -passes=sycl-kernel-postvec %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

; If the main loop of clone function does not have a single exit, vectorizer would bail out.
; Remove the clone in this case.

; CHECK-NOT: define void @_ZGVeN16_test

define void @test() !vectorized_kernel !1 {
entry:
  ret void
}

define void @_ZGVeN16_test() #0 {
entry:
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16) ]
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  br label %simd.loop.header

simd.loop.header:                                 ; preds = %simd.loop.header, %simd.loop.preheader
  br label %simd.loop.header

simd.end.region:                                  ; No predecessors!
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

attributes #0 = { "vector-variant-failure"="Bailout" }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{ptr @_ZGVeN16_test}

; DEBUGIFY-COUNT-7: WARNING: Missing line
; DEBUGIFY: WARNING: Missing variable
; DEBUGIFY-NOT: WARNING
