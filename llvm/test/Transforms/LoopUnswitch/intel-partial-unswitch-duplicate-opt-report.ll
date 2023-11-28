; RUN: opt -bugpoint-enable-legacy-pm -loop-unswitch -intel-ir-optreport-emitter -intel-opt-report=low -intel-opt-report-file=stdout -disable-output < %s | FileCheck %s

; Verify that we do not duplicate loop opt report during partial unswitching.
; Only one of the loop versions contains the original remark.

; CHECK:      Global optimization report for : partial_unswitch_true_successor

; CHECK:      LOOP BEGIN
; CHECK-NEXT: LOOP END

; CHECK:      LOOP BEGIN
; CHECK-NEXT:     remark #99999: Dummy remark for testing
; CHECK-NEXT:     remark #25422: Invariant Condition hoisted out of this loop
; CHECK-NEXT: LOOP END

declare void @clobber()

define i32 @partial_unswitch_true_successor(i32* %ptr, i32 %N) {
entry:
  br label %loop.header

loop.header:
  %iv = phi i32 [ 0, %entry ], [ %iv.next, %loop.latch ]
  %lv = load i32, i32* %ptr
  %sc = icmp eq i32 %lv, 100
  br i1 %sc, label %noclobber, label %clobber

noclobber:
  br label %loop.latch

clobber:
  call void @clobber()
  br label %loop.latch

loop.latch:
  %c = icmp ult i32 %iv, %N
  %iv.next = add i32 %iv, 1
  br i1 %c, label %loop.header, label %exit, !llvm.loop !0

exit:
  ret i32 10
}

!0 = distinct !{!0, !1}
!1 = distinct !{!"intel.optreport", !2}
!2 = !{!"intel.optreport.remarks", !3}
!3 = !{!"intel.optreport.remark", i32 99999}

