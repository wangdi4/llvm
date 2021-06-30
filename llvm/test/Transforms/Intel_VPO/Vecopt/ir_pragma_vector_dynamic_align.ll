; RUN: opt %s -S -vplan-vec --debug-only="VPlan_pragma_metadata" 2>&1 | FileCheck %s
; REQUIRES: asserts
;
; Checks if the code with #pragma vector [no]dynamic_align (!{!"llvm.loop.vectorize.dynamic_align", i1 true|false}) metadata
; and #pragma omp simd is being vectorized correctly with LLVM-IR
;
; CHECK: Dynamic Align was set by the user's #pragma vector dynamic_align
; CHECK-NEXT: No dynamic Align was set by the user's #pragma vector nodynamic_align
;
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
!8 = !{!"llvm.loop.intel.vector.dynamic_align", !"true"}
!9 = !{!"llvm.loop.intel.vector.nodynamic_align", !"true"}
!10 =  distinct !{!10, !9}
