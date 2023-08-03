; RUN: opt -passes=sycl-kernel-postvec %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-postvec %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

; If the kernel is not vectorized, then the cloned kernel is removed.

; CHECK-NOT: define void @_ZGVeN16___omp_offloading_804_230a44c__Z4main_l31
; CHECK-NOT: define void @_ZGVeM16___omp_offloading_804_230a44c__Z4main_l31

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @__omp_offloading_804_230a44c__Z4main_l31() #0 !vectorized_kernel !1 !vectorized_masked_kernel !2 {
entry:
  ret void
}

define void @_ZGVeN16___omp_offloading_804_230a44c__Z4main_l31() #1 {
entry:
  br label %simd.loop.header

simd.loop.header:                                 ; preds = %simd.loop.header, %entry
  br label %simd.loop.header, !llvm.loop !3
}

define void @_ZGVeM16___omp_offloading_804_230a44c__Z4main_l31() #1 {
entry:
  br label %simd.loop.header

simd.loop.header:                                 ; preds = %simd.loop.header, %entry
  br label %simd.loop.header, !llvm.loop !6
}

attributes #0 = { convergent noinline nounwind "has-sub-groups" "kernel-call-once" "kernel-convergent-call" "prefer-vector-width"="512" "vector-variants"="_ZGVeN16___omp_offloading_804_230a44c__Z4main_l31,_ZGVeM16___omp_offloading_804_230a44c__Z4main_l31" }
attributes #1 = { "vector-variant-failure"="Bailout" }
!sycl.kernels = !{!0}

!0 = !{ptr @__omp_offloading_804_230a44c__Z4main_l31}
!1 = !{ptr @_ZGVeN16___omp_offloading_804_230a44c__Z4main_l31}
!2 = !{ptr @_ZGVeM16___omp_offloading_804_230a44c__Z4main_l31}
!3 = distinct !{!3, !4, !5}
!4 = !{!"llvm.loop.unroll.disable"}
!5 = !{!"llvm.loop.vectorize.enable", i32 1}
!6 = distinct !{!6, !4, !5}

; DEBUGIFY-COUNT-4: WARNING: Missing line
; DEBUGIFY-COUNT-2: WARNING: Missing variable
; DEBUGIFY-NOT: WARNING
