; RUN: opt -disable-output "-passes=print<scalar-evolution>" < %s 2>&1 | FileCheck %s

; ScalarEvolution should be able to use maximum trip counts specified in
; metadata as an upper bound when calculating backedge-taken counts.

; Check that llvm.loop.intel.loopcount_maximum is recognized.
; CHECK-LABEL: @loopcount_maximum
; CHECK: Loop %L1: max backedge-taken count is 7
define void @loopcount_maximum(i64 %n, i64 %s) {
entry:
  br label %L1

L1:
  %i = phi i64 [ %i.next, %L1 ], [ 1, %entry ]
  %i.next = add nsw i64 %i, %s
  %cond = icmp sle i64 %i.next, %n
  br i1 %cond, label %L1, label %exit, !llvm.loop !0 ; loopcount_maximum=8

exit:
  ret void
}

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.intel.loopcount_maximum", i32 8}

; Check that llvm.loop.intel.max.trip_count is recognized.
; CHECK-LABEL: @max_trip_count
; CHECK: Loop %L1: max backedge-taken count is 15
define void @max_trip_count(i64 %n, i64 %s) {
entry:
  br label %L1

L1:
  %i = phi i64 [ %i.next, %L1 ], [ 1, %entry ]
  %i.next = add nsw i64 %i, %s
  %cond = icmp sle i64 %i.next, %n
  br i1 %cond, label %L1, label %exit, !llvm.loop !2 ; max.trip_count=16

exit:
  ret void
}

!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.intel.max.trip_count", i32 16}

; Check that if both metadata are present the smaller one is used to bound the
; trip count.
; CHECK-LABEL: @both_metadata
; CHECK: Loop %L3: max backedge-taken count is 3
; CHECK: Loop %L2: max backedge-taken count is 7
; CHECK: Loop %L1: max backedge-taken count is 7
define void @both_metadata(i64 %n, i64 %s) {
entry:
  br label %L1

L1:
  %i = phi i64 [ %i.next, %L1 ], [ 1, %entry ]
  %i.next = add nsw i64 %i, %s
  %cond = icmp sle i64 %i.next, %n
  br i1 %cond, label %L1, label %L2.ph, !llvm.loop !4 ; loopcount_maximum=8, max.trip_count=16

L2.ph:
  br label %L2

L2:
  %i.L2 = phi i64 [ %i.next.L2, %L2 ], [ 1, %L2.ph ]
  %i.next.L2 = add nsw i64 %i.L2, %s
  %cond.L2 = icmp sle i64 %i.next.L2, %n
  br i1 %cond.L2, label %L2, label %L3.ph, !llvm.loop !5 ; max.trip_count=16, loopcount_maximum=8

L3.ph:
  br label %L3

L3:
  %i.L3 = phi i64 [ %i.next.L3, %L3 ], [ 1, %L3.ph ]
  %i.next.L3 = add nsw i64 %i.L3, %s
  %cond.L3 = icmp sle i64 %i.next.L3, %n
  br i1 %cond.L3, label %L3, label %exit, !llvm.loop !6 ; loopcount_maximum=8, max.trip_count=4

exit:
  ret void
}

!4 = distinct !{!4, !1, !3}
!5 = distinct !{!5, !3, !1}
!6 = distinct !{!6, !1, !7}
!7 = !{!"llvm.loop.intel.max.trip_count", i32 4}

; Check that this min operation is correctly unsigned.
; CHECK-LABEL: @both_metadata_unsigned
; CHECK: Loop %L1: max backedge-taken count is 7
define void @both_metadata_unsigned(i64 %n, i64 %s) {
entry:
  br label %L1

L1:
  %i = phi i64 [ %i.next, %L1 ], [ 1, %entry ]
  %i.next = add nsw i64 %i, %s
  %cond = icmp sle i64 %i.next, %n
  br i1 %cond, label %L1, label %exit, !llvm.loop !8 ; loopcount_maximum=8, max.trip_count=UINT_MAX

exit:
  ret void
}

!8 = distinct !{!8, !1, !9}
!9 = !{!"llvm.loop.intel.max.trip_count", i32 -1}

; Check that it can also handle differing types correctly.
; CHECK-LABEL: @both_metadata_types
; CHECK: Loop %L1: max backedge-taken count is 4294967294
define void @both_metadata_types(i64 %n, i64 %s) {
entry:
  br label %L1

L1:
  %i = phi i64 [ %i.next, %L1 ], [ 1, %entry ]
  %i.next = add nsw i64 %i, %s
  %cond = icmp sle i64 %i.next, %n
  br i1 %cond, label %L1, label %exit, !llvm.loop !10 ; loopcount_maximum=i64 UINT32_MAX+5, max.trip_count=UINT32_MAX

exit:
  ret void
}

!10 = distinct !{!10, !11, !9}
!11 = !{!"llvm.loop.intel.loopcount_maximum", i64 4294967300}

; Check that if the max trip count can be determined by other means, it will
; also be used instead of the metadata values if it is smaller.
; CHECK-LABEL: @known_max_trip_count
; CHECK: Loop %L2: max backedge-taken count is 3
; CHECK: Loop %L1: max backedge-taken count is 6
define void @known_max_trip_count(i64 %n) {
entry:
  %n.mask = and i64 %n, 7
  br label %L1

L1:
  %i = phi i64 [ %i.next, %L1 ], [ 1, %entry ]
  %i.next = add nsw i64 %i, 1
  %cond = icmp sle i64 %i.next, %n.mask
  br i1 %cond, label %L1, label %L2.ph, !llvm.loop !12 ; max.trip_count=16

L2.ph:
  br label %L2

L2:
  %i.L2 = phi i64 [ %i.next.L2, %L2 ], [ 1, %L2.ph ]
  %i.next.L2 = add nsw i64 %i.L2, 1
  %cond.L2 = icmp sle i64 %i.next.L2, %n.mask
  br i1 %cond.L2, label %L2, label %exit, !llvm.loop !13 ; max.trip_count=4

exit:
  ret void
}

!12 = distinct !{!12, !3}
!13 = distinct !{!13, !7}

; Check that a metadata max trip count of zero is handled correctly and doesn't
; underflow.
; CHECK-LABEL: @zero_trip_count
; CHECK: Loop %L1: max backedge-taken count is 0
define void @zero_trip_count(i64 %n, i64 %s) {
entry:
  br label %L1

L1:
  %i = phi i64 [ %i.next, %L1 ], [ 1, %entry ]
  %i.next = add nsw i64 %i, %s
  %cond = icmp sle i64 %i.next, %n
  br i1 %cond, label %L1, label %exit, !llvm.loop !14 ; loopcount_maximum=0

exit:
  ret void
}

!14 = distinct !{!14, !15}
!15 = !{!"llvm.loop.intel.loopcount_maximum", i32 0}
