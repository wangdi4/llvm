; RUN: opt < %s -analyze -scalar-evolution | FileCheck %s

; We should be able to prove this loop body executes 8 times.

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

; We should be able to prove this loop body executes (8+1) = 9 times because
; compare is using the IV phi instead of the shifted value.

; CHECK: Loop %do.body1: backedge-taken count is 8
; CHECK: Loop %do.body1: max backedge-taken count is 8

define void @foo1(i8 %x) {
entry:
  %conv = zext i8 %x to i32
  %or = or i32 %conv, 256
  br label %do.body1

do.body1:                                          ; preds = %do.body1, %entry
  %a.0 = phi i32 [ %or, %entry ], [ %shl, %do.body1 ]
  tail call void @bar()
  %shl = shl i32 %a.0, 1
  %cmp = icmp ult i32 %a.0, 65536
  br i1 %cmp, label %do.body1, label %do.end

do.end:                                           ; preds = %do.body1
  ret void
}

declare void @bar()
