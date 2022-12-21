; RUN: opt %s -S -passes=vplan-vec --debug-only="VPlan_pragma_metadata" 2>&1 | FileCheck %s
; REQUIRES: asserts
;
; Checks if the code with #pragma [no]vecremainder (!{!"llvm.loop.vector.vecremainder", i1 true|false}) metadata
; and #pragma omp simd is being vectorized correctly with LLVM-IR
;
; CHECK: Vector Remainder was set by the user's #pragma vecremainder
; CHECK: Scalar Remainder was set by the user's #pragma novecremainder
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @main1() {
entry:
  br label %preheader

preheader:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header
header:
  %iv = phi i32 [ 0, %preheader ], [ %iv.next, %latch ]
  %iv.next = add nsw i32 %iv, 1
  br label %latch

latch:
  %t = sext i32 %iv to i64
  %x = add nsw i64 %t, 1
  %bottom_test = icmp eq i32 %iv.next, 131
  br i1 %bottom_test, label %loopexit, label %header, !llvm.loop !7

loopexit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  ret void
}

define void @main2() {
entry:
  br label %preheader

preheader:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header
header:
  %iv = phi i32 [ 0, %preheader ], [ %iv.next, %latch ]
  %iv.next = add nsw i32 %iv, 1
  br label %latch

latch:
  %t = sext i32 %iv to i64
  %x = add nsw i64 %t, 1
  %bottom_test = icmp eq i32 %iv.next, 131
  br i1 %bottom_test, label %loopexit, label %header, !llvm.loop !10

loopexit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind

!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.intel.vector.vecremainder", !"true"}
!9 = !{!"llvm.loop.intel.vector.novecremainder", !"true"}
!10 = distinct !{!10, !9}
