; RUN: opt < %s -enable-new-pm=0 -loop-unswitch -S < %s 2>&1 | FileCheck %s

; Verify that we do not duplicate loop opt report during partial unswitching.
; Only one of the loop versions contains the original remark.

; CHECK: !llvm.loop [[ID1:!.*]]
; CHECK: !llvm.loop [[ID2:!.*]]

; CHECK: [[ID1]] = distinct !{[[ID1]], [[DISABLE:.*]]}
; CHECK: [[DISABLE]] = !{!"llvm.loop.unswitch.partial.disable"}
; CHECK: [[ID2]] = distinct !{[[ID2]], [[OPTREPORT:.*]]}
; CHECK: [[OPTREPORT]] = distinct !{!"intel.optreport.rootnode"

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
!1 = distinct !{!"intel.optreport.rootnode", !2}
!2 = distinct !{!"intel.optreport", !3}
!3 = !{!"intel.optreport.remarks", !4}
!4 = !{!"intel.optreport.remark", !"dummy opt report"}

