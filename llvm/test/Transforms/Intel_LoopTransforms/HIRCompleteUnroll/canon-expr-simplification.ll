; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-post-vec-complete-unroll,print<hir>" -S 2>&1 < %s | FileCheck %s

; Verify that we do not simplify the unsigned division of the canon expr because the numerator may be negative.

; Incoming HIR-
; + DO i1 = 0, 1, 1   <DO_LOOP>
; |   %1 = (-39 * %1)/u13  +  1;
; + END LOOP

; CHECK: BEGIN REGION { modified }
; CHECK: %1 = (-39 * %1)/u13  +  1;
; CHECK: %1 = (-39 * %1)/u13  +  1;
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global i32 1, align 4

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr {
entry:
  %0 = load i32, ptr @a, align 4
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %1 = phi i32 [ %0, %entry ], [ %add, %for.body ]
  %i.03 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %mul = mul i32 %1, -39
  %div = udiv i32 %mul, 13
  %add = add nuw nsw i32 %div, 1
  %inc = add nuw nsw i32 %i.03, 1
  %exitcond = icmp eq i32 %inc, 2
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  store i32 %add.lcssa, ptr @a, align 4
  ret i32 0
}

