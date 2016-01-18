; RUN: opt -S -jump-threading -distant-jump-threading -conservative-jump-threading=false < %s | FileCheck %s
;
target triple = "x86_64-unknown-linux-gnu"

; Check the most basic form of distant jump threading where the conditional
; branch depends on a phi from an earlier block. The thread region is B3-B4.
; Also check that the branch weights are updated properly. All the branch weight
; should be 2-to-1 except for the original b4 
;
; CHECK-LABEL: f1
; CHECK-LABEL: b3.thread
; CHECK-LABEL: b4.thread
; CHECK !1 = !{!"branch_weights", i32 2, i32 1}
; CHECK !2 = !{!"branch_weights", i32 4, i32 1}
;
define void @f1(i1 %arg1, i1 %arg2, i1 %arg3) !prof !0 {
b1:
  br i1 %arg1, label %b3, label %b2, !prof !1

b2:
  call void @f0()
  br label %b3

b3:
  %cond = phi i1 [ 1, %b1 ], [ %arg2, %b2 ]
  br i1 %arg3, label %b4, label %b5, !prof !1

b4:
  call void @f0()
  br i1 %cond, label %b5, label %b6, !prof !2

b5:
  call void @f0()
  br label %b6

b6:
  ret void
}

declare void @f0()

!0 = !{!"function_entry_count", i64 1}
!1 = !{!"branch_weights", i32 2, i32 1}
!2 = !{!"branch_weights", i32 10, i32 1}
