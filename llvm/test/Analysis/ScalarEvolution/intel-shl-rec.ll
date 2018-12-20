; RUN: opt < %s -analyze -scalar-evolution | FileCheck %s

; We should be able to prove this loop body executs 8 times.

; CHECK: Loop %do.body: backedge-taken count is 7
; CHECK: Loop %do.body: max backedge-taken count is 7

define void @foo(i8 %x) {
entry:
  %conv = zext i8 %x to i32
  %or = or i32 %conv, 256
  br label %do.body

do.body:                                          ; preds = %do.body, %entry
  %a.0 = phi i32 [ %or, %entry ], [ %shl, %do.body ]
  tail call void @bar()
  %shl = shl i32 %a.0, 1
  %cmp = icmp ult i32 %shl, 65536
  br i1 %cmp, label %do.body, label %do.end

do.end:                                           ; preds = %do.body
  ret void
}

declare void @bar()
