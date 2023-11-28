; REQUIRES: asserts

; RUN: opt -passes='hir-ssa-deconstruction,print<hir-parvec-analysis>' -debug-only=parvec-analysis -disable-output < %s 2>&1 | FileCheck %s

; Verify that we are able to refine the output edge for the inner loop and mark
; the loop as vectorizable.

; HIR-
; + DO i1 = 0, 99, 1   <DO_LOOP>
; |   + DO i2 = 0, 98, 1   <DO_LOOP>
; |   |   (@A)[0][i2 + 1] = 5;
; |   |   (@A)[0][i2] = 10;
; |   + END LOOP
; + END LOOP


; CHECK: (@A)[0][i2] --> (@A)[0][i2 + 1] OUTPUT (* <)
; CHECK-NEXT: is safe to vectorize (indep)

; CHECK: loop is vectorizable


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc6
  %i.014 = phi i32 [ 0, %entry ], [ %inc7, %for.inc6 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 1, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %arrayidx = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %indvars.iv
  store i32 5, ptr %arrayidx, align 4
  %0 = add nsw i64 %indvars.iv, -1
  %arrayidx5 = getelementptr inbounds [100 x i32], ptr @A, i64 0, i64 %0
  store i32 10, ptr %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.body3
  %inc7 = add nuw nsw i32 %i.014, 1
  %exitcond16.not = icmp eq i32 %inc7, 100
  br i1 %exitcond16.not, label %for.end8, label %for.cond1.preheader

for.end8:                                         ; preds = %for.inc6
  ret void
}

