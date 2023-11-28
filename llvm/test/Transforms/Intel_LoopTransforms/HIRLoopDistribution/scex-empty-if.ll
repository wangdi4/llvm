; RUN: opt -xmain-opt-level=3 -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec" -print-before=hir-loop-distribute-memrec -print-after=hir-loop-distribute-memrec < %s 2>&1 | FileCheck %s

; The tests verifies that empty nodes be removed after scalar expansion.

; Here is an example-

; CHUNK 1:
; %t = a[i];

; CHUNK 2:
; if (... %t ...) {
;   <EMPTY>
; }

; CHECK: Dump Before

; CHECK: + DO i1
; CHECK-SAME: <MAX_TC_EST = 100>

; CHECK: Dump After

; CHECK: modified

; Verify that the inner i2 loops acquire the MAX_TC_EST of stripmine size.

; CHECK: + DO i1
; CHECK-SAME: <MAX_TC_EST = 100>

; CHECK: + DO i2
; CHECK-SAME: <MAX_TC_EST = 64>

; CHECK: + DO i2
; CHECK-SAME: <MAX_TC_EST = 64>

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@d = external dso_local local_unnamed_addr global [20 x i32], align 16
@a = external dso_local local_unnamed_addr global i32, align 4
@f = external dso_local local_unnamed_addr global [100 x i32], align 4
@g = external dso_local local_unnamed_addr global [100 x i32], align 4
@h = external dso_local global [100 x i32], align 4

define dso_local void @main() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %cond.end31, %entry
  %indvars.iv = phi i64 [ undef, %entry ], [ %indvars.iv.next, %cond.end31 ]
  %0 = load i32, ptr getelementptr inbounds ([20 x i32], ptr @d, i64 0, i64 3), align 4
  %tobool1 = icmp eq i32 %0, 0
  br i1 %tobool1, label %if.end, label %if.then

if.then:                                          ; preds = %for.body
  %1 = load i32, ptr @a, align 4
  %idx.ext = sext i32 %1 to i64
  %add.ptr = getelementptr inbounds [100 x i32], ptr @h, i64 0, i64 %idx.ext
  %2 = ptrtoint ptr %add.ptr to i64
  %conv = trunc i64 %2 to i32
  store i32 %conv, ptr @a, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %3 = add nsw i64 %indvars.iv, 2
  %arrayidx = getelementptr inbounds [100 x i32], ptr @g, i64 0, i64 %3
  %4 = load i32, ptr %arrayidx, align 4
  %tobool2 = icmp eq i32 %4, 0
  br i1 %tobool2, label %if.end.cond.end31_crit_edge, label %cond.true

if.end.cond.end31_crit_edge:                      ; preds = %if.end
  %.pre = load i32, ptr @a, align 4
  br label %cond.end31

cond.true:                                        ; preds = %if.end
  %5 = load i32, ptr @a, align 4
  %tobool3 = icmp eq i32 %5, 0
  br i1 %tobool3, label %cond.false25, label %cond.true6

cond.true6:                                       ; preds = %cond.true
  %6 = load i32, ptr getelementptr inbounds ([100 x i32], ptr @g, i64 0, i64 3), align 4
  %tobool7 = icmp eq i32 %6, 0
  br i1 %tobool7, label %cond.false, label %cond.end31

cond.false:                                       ; preds = %cond.true6
  %7 = load i32, ptr getelementptr inbounds ([100 x i32], ptr @h, i64 0, i64 3), align 4
  %8 = load i32, ptr getelementptr inbounds ([100 x i32], ptr @f, i64 0, i64 2), align 4
  %mul = mul nsw i32 %8, %7
  %9 = load i32, ptr getelementptr inbounds ([100 x i32], ptr @h, i64 0, i64 2), align 4
  %mul12 = mul nsw i32 %9, %4
  %add13 = add nsw i32 %mul12, %mul
  %10 = load i32, ptr getelementptr inbounds ([100 x i32], ptr @f, i64 0, i64 5), align 4
  %cmp = icmp sgt i32 %add13, %10
  br i1 %cmp, label %cond.end31, label %cond.false16

cond.false16:                                     ; preds = %cond.false
  %11 = load i32, ptr getelementptr inbounds ([100 x i32], ptr @g, i64 0, i64 5), align 4
  br label %cond.end31

cond.false25:                                     ; preds = %cond.true
  %12 = load i32, ptr getelementptr inbounds ([100 x i32], ptr @f, i64 0, i64 5), align 4
  %13 = load i32, ptr getelementptr inbounds ([20 x i32], ptr @d, i64 0, i64 5), align 4
  %14 = load i32, ptr getelementptr inbounds ([100 x i32], ptr @g, i64 0, i64 5), align 4
  %mul26 = mul nsw i32 %14, %13
  %add27 = add nsw i32 %mul26, %12
  br label %cond.end31

cond.end31:                                       ; preds = %cond.false25, %cond.false16, %cond.false, %cond.true6, %if.end.cond.end31_crit_edge
  %15 = phi i32 [ 0, %cond.false25 ], [ %5, %cond.false16 ], [ %5, %cond.false ], [ %5, %cond.true6 ], [ %.pre, %if.end.cond.end31_crit_edge ]
  %cond32 = phi i32 [ %add27, %cond.false25 ], [ %11, %cond.false16 ], [ 1, %cond.false ], [ undef, %cond.true6 ], [ undef, %if.end.cond.end31_crit_edge ]
  %arrayidx34 = getelementptr inbounds [100 x i32], ptr @h, i64 0, i64 %indvars.iv
  %16 = load i32, ptr %arrayidx34, align 4
  %mul35 = mul nsw i32 %16, %15
  %add36 = add nsw i32 %mul35, %cond32
  %arrayidx38 = getelementptr inbounds [100 x i32], ptr @g, i64 0, i64 %indvars.iv
  store i32 undef, ptr %arrayidx38, align 4
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %arrayidx41 = getelementptr inbounds [100 x i32], ptr @h, i64 0, i64 %indvars.iv.next
  store i32 undef, ptr %arrayidx41, align 4
  %17 = load i32, ptr undef, align 4
  %sub = add nsw i32 %17, -3
  store i32 %sub, ptr undef, align 4
  %18 = trunc i64 %indvars.iv.next to i32
  %tobool = icmp eq i32 %18, 0
  br i1 %tobool, label %for.cond.for.end_crit_edge, label %for.body

for.cond.for.end_crit_edge:                       ; preds = %cond.end31
  %add36.lcssa = phi i32 [ %add36, %cond.end31 ]
  ret void
}

