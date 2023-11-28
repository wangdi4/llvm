
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; CHECK: Function: foo

; CHECK:      BEGIN REGION { }
; CHECK:            + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK:            |   (@B)[0][4 * i1] = 4 * i1 + %n;
; CHECK:            |   (@B)[0][4 * i1 + 1] = 4 * i1 + %n + 1;
; CHECK:            |   (@B)[0][4 * i1 + 2] = 4 * i1 + %n + 2;
; CHECK:            |   (@B)[0][4 * i1 + 3] = 4 * i1 + %n + 3;
; CHECK:            + END LOOP
; CHECK:      END REGION

; CHECK: Function: foo

; CHECK:      BEGIN REGION { }
; CHECK:            + DO i1 = 0, 4 * ((3 + sext.i32.i64(%n)) /u 4) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 8>
; CHECK:            |   (@B)[0][i1] = i1 + %n;
; CHECK:            + END LOOP
; CHECK:      END REGION

;Module Before HIR; ModuleID = 'iv-pattern.c'
source_filename = "iv-pattern.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp37 = icmp sgt i32 %n, 0
  br i1 %cmp37, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %1 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %1, %n
  %arrayidx = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %indvars.iv
  store i32 %add, ptr %arrayidx, align 16
  %2 = or i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %2
  %3 = trunc i64 %2 to i32
  %4 = add i32 %3, %n
  store i32 %4, ptr %arrayidx5, align 4
  %5 = or i64 %indvars.iv, 2
  %arrayidx10 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %5
  %6 = trunc i64 %5 to i32
  %7 = add i32 %6, %n
  store i32 %7, ptr %arrayidx10, align 8
  %8 = or i64 %indvars.iv, 3
  %arrayidx15 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %8
  %9 = trunc i64 %8 to i32
  %10 = add i32 %9, %n
  store i32 %10, ptr %arrayidx15, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}



