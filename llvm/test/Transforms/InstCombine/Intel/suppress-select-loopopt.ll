; RUN: opt -passes="instcombine" < %s -S | FileCheck %s

; This test checks if the following transform is suppressed when the
; function has "pre_loopopt" attribute and the 'or' is inside a loop:
;   or(sext(A), B) / or(B, sext(A)) --> A ? -1 : B
; On success there should be no 'select' instruction.

; Function Attrs: nounwind
define i32 @suppress_or_sext_opt(ptr %pa) local_unnamed_addr "pre_loopopt" {
; CHECK-LABEL: @suppress_or_sext_opt(
; CHECK:         %1 = icmp sgt i32 %0, 0
; CHECK-NEXT:    %slct = sext i1 %1 to i32
; CHECK-NEXT:    %or = or i32 %"var$0", %slct

entry:
  br label %bb0

bb0:                                          ; preds = %entry, %bb1
  %"var$0" = phi i32 [ 0, %entry ], [ %or, %bb1 ]
  %"var$1" = phi i64 [ 1, %entry ], [ %add, %bb1 ]
  %rel0 = icmp sle i64 %"var$1", 9
  br i1 %rel0, label %bb1, label %bb2

bb1:                                          ; preds = %bb0
  %0 = load i32, ptr %pa
  %1 = icmp sgt i32 %0, 0
  %isext = sext i1 %1 to i32
  %rel1 = icmp ne i32 %isext, 0
  %slct = select i1 %rel1, i32 -1, i32 0
  %or = or i32 %slct, %"var$0"
  %add = add nsw i64 %"var$1", 1
  br label %bb0

bb2:                                          ; preds = %bb0
  ret i32 %"var$0"
}
