; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Not in a right pattern

; CHECK: Function: foo

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK:           |   %1 = (@B)[0][4 * i1];
; CHECK:           |   (@A)[0][4 * i1] = (%n * %n * %1);
; CHECK:           |   %3 = (@B)[0][4 * i1 + 1];
; CHECK:           |   (@A)[0][4 * i1 + 1] = ((1 + %n) * %n * %3);
; CHECK:           |   %5 = (@B)[0][4 * i1 + 2];
; CHECK:           |   (@A)[0][4 * i1 + 2] = (%n * %5);
; CHECK:           |   %7 = (@B)[0][4 * i1 + 3];
; CHECK:           |   (@A)[0][4 * i1 + 3] = (%n * %n * %7);
; CHECK:           + END LOOP
; CHECK:     END REGION

; CHECK: Function: foo

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, (sext.i32.i64(%n) + -1)/u4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK:           |   %1 = (@B)[0][4 * i1];
; CHECK:           |   (@A)[0][4 * i1] = (%n * %n * %1);
; CHECK:           |   %3 = (@B)[0][4 * i1 + 1];
; CHECK:           |   (@A)[0][4 * i1 + 1] = ((1 + %n) * %n * %3);
; CHECK:           |   %5 = (@B)[0][4 * i1 + 2];
; CHECK:           |   (@A)[0][4 * i1 + 2] = (%n * %5);
; CHECK:           |   %7 = (@B)[0][4 * i1 + 3];
; CHECK:           |   (@A)[0][4 * i1 + 3] = (%n * %n * %7);
; CHECK:           + END LOOP
; CHECK:     END REGION

;Module Before HIR; ModuleID = 'one-store-invalid.c'
source_filename = "one-store-invalid.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %mul = mul nsw i32 %n, %n
  %add = add nsw i32 %n, 1
  %mul1 = mul nsw i32 %add, %n
  %cmp45 = icmp sgt i32 %n, 0
  br i1 %cmp45, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx, align 16
  %mul2 = mul nsw i32 %1, %mul
  %arrayidx4 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %indvars.iv
  store i32 %mul2, ptr %arrayidx4, align 16
  %2 = or i64 %indvars.iv, 1
  %arrayidx7 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %2
  %3 = load i32, ptr %arrayidx7, align 4
  %mul8 = mul nsw i32 %mul1, %3
  %arrayidx11 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %2
  store i32 %mul8, ptr %arrayidx11, align 4
  %4 = or i64 %indvars.iv, 2
  %arrayidx14 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %4
  %5 = load i32, ptr %arrayidx14, align 8
  %mul15 = mul nsw i32 %5, %n
  %arrayidx18 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %4
  store i32 %mul15, ptr %arrayidx18, align 8
  %6 = or i64 %indvars.iv, 3
  %arrayidx21 = getelementptr inbounds [10 x i32], ptr @B, i64 0, i64 %6
  %7 = load i32, ptr %arrayidx21, align 4
  %mul22 = mul nsw i32 %7, %mul
  %arrayidx25 = getelementptr inbounds [10 x i32], ptr @A, i64 0, i64 %6
  store i32 %mul22, ptr %arrayidx25, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup.loopexit
}



