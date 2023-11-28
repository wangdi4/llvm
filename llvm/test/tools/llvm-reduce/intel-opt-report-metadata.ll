; RUN: llvm-reduce %s -o %t --delta-passes=metadata --test FileCheck --test-arg %s --test-arg --input-file
; RUN: FileCheck %s --input-file %t

; Check that llvm-reduce doesn't crash when attempting to reduce IR with
; multiple functions containing opt-report metadata.

define void @foo(i64 %n) {
entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1 ]
  %i.next = add nuw nsw i64 %i, 1
  %cmp = icmp ne i64 %i.next, %n
  br i1 %cmp, label %L1, label %exit, !llvm.loop !0

exit:
  ret void
}

define void @bar(i64 %n) {
entry:
  br label %L1

L1:
  %i = phi i64 [ 0, %entry ], [ %i.next, %L1 ]
  %i.next = add nuw nsw i64 %i, 1
  %cmp = icmp ne i64 %i.next, %n
  br i1 %cmp, label %L1, label %exit, !llvm.loop !5

exit:
  ret void
}

!0 = distinct !{!0, !1}
!1 = distinct !{!"intel.optreport", !3}
!3 = !{!"intel.optreport.remarks", !4}
; CHECK: !{!"intel.optreport.remark", i32 0, !"Dummy remark"}
!4 = !{!"intel.optreport.remark", i32 0, !"Dummy remark"}
!5 = distinct !{!5, !6}
!6 = distinct !{!"intel.optreport", !3}
