; RUN: opt < %s -analyze -enable-new-pm=0 -scalar-evolution | FileCheck %s
; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s

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

; Verify that we are able to compute trip count of shl recurrence
; with slt predicate.

; CHECK: foo2
; CHECK: Loop %do.body1: backedge-taken count is 8
; CHECK: Loop %do.body1: max backedge-taken count is 8

define void @foo2(i8 %x) {
entry:
  %conv = zext i8 %x to i32
  %or = or i32 %conv, 256
  br label %do.body1

do.body1:                                          ; preds = %do.body1, %entry
  %a.0 = phi i32 [ %or, %entry ], [ %shl, %do.body1 ]
  tail call void @bar()
  %shl = shl i32 %a.0, 1
  %cmp = icmp slt i32 %a.0, 65536
  br i1 %cmp, label %do.body1, label %do.end

do.end:                                           ; preds = %do.body1
  ret void
}

; Verify that we are able to compute max trip count when unsigned shl
; recurrence has appropriate no-wrap flag.

; CHECK: foo3
; CHECK: Loop %do.body1: Unpredictable backedge-taken count
; CHECK: Loop %do.body1: max backedge-taken count is 1

define void @foo3(i8 %x) {
entry:
  %conv = zext i8 %x to i32
  %or = or i32 %conv, 256
  br label %do.body1

do.body1:                                          ; preds = %do.body1, %entry
  %a.0 = phi i32 [ %or, %entry ], [ %shl, %do.body1 ]
  tail call void @bar()
  %shl = shl nuw i32 %a.0, 1
  %cmp = icmp ult i32 %a.0, 257
  br i1 %cmp, label %do.body1, label %do.end

do.end:                                           ; preds = %do.body1
  ret void
}

; Verify that we are able to compute max trip count when signed shl
; recurrence has appropriate no-wrap flags even when an llvm.ssa.copy() is
; involved.
; Also verify that we are able to compute the right range info for %a.0.

; CHECK: foo4
; CHECK:  -->  %a.0 U: [1,65) S: [1,65)
; CHECK: Loop %do.body1: Unpredictable backedge-taken count
; CHECK: Loop %do.body1: max backedge-taken count is 6

define void @foo4(i8 %x) {
entry:
  %div = sdiv i8 %x, 2
  br label %do.body1

do.body1:                                          ; preds = %do.body1, %entry
  %a.0 = phi i8 [ 1, %entry ], [ %shl, %do.body1 ]
  tail call void @bar()
  %a.copy = call i8 @llvm.ssa.copy.i8(i8 %a.0)
  %shl = shl nsw i8 %a.copy, 1
  %cmp = icmp slt i8 %a.0, %div
  br i1 %cmp, label %do.body1, label %do.end

do.end:                                           ; preds = %do.body1
  ret void
}

declare void @bar()

declare i8 @llvm.ssa.copy.i8(i8 returned)
