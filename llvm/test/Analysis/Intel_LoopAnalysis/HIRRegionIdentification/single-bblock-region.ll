; RUN: opt < %s -passes='print<hir-region-identification>' -disable-output 2>&1 | FileCheck %s

; Check that we create one region each for functon foo and bar.
; CHECK: Region 1
; CHECK-NEXT: EntryBB
; CHECK-SAME: for.body
; CHECK-NEXT: ExitBB
; CHECK-NEXT: Member
; CHECK-SAME: for.body

; CHECK: Region 1
; CHECK-NEXT: EntryBB
; CHECK-SAME: for.body
; CHECK-NEXT: ExitBB
; CHECK-NEXT: Member
; CHECK-SAME: for.body


; Verify that region creation is disabled based on command line option.

; RUN: opt < %s -passes='print<hir-region-identification>' -disable-hir-regions-func-list=foo 2>&1 | FileCheck %s --check-prefix=ONEFUNC
; RUN: opt < %s -passes='print<hir-region-identification>' -disable-hir-regions-func-list=bar 2>&1 | FileCheck %s --check-prefix=ONEFUNC
; RUN: opt < %s -passes='print<hir-region-identification>' -disable-hir-regions-func-list=foo,bar 2>&1 | FileCheck %s --check-prefix=TWOFUNC

; ONEFUNC: Region 1

; ONEFUNC-NOT: Region 1

; TWOFUNC-NOT: Region 1

; ModuleID = 'q1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [10 x i32] zeroinitializer, align 16
@A = common global [10 x i32] zeroinitializer, align 16

define void @foo() {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.05 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %i.05
  %0 = load i32, ptr %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %i.05
  store i32 %0, ptr %arrayidx1, align 4
  %inc = add nuw nsw i64 %i.05, 1
  %exitcond = icmp eq i64 %inc, 6
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

define void @bar() {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.05 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %i.05
  %0 = load i32, ptr %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %i.05
  store i32 %0, ptr %arrayidx1, align 4
  %inc = add nuw nsw i64 %i.05, 1
  %exitcond = icmp eq i64 %inc, 6
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
