; RUN: opt -disable-hir-create-fusion-regions=0 -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that "i1" loops are fused while "i2" loops are not fused because of the dependency A[i1][i2] --> A[i1][i2+1].

;     BEGIN REGION { }
;           + DO i1 = 0, 99, 1   <DO_LOOP>
;           |   + DO i2 = 0, 8, 1   <DO_LOOP>
;           |   |   (%A)[0][i1][i2] = i1 + i2;
;           |   + END LOOP
;           + END LOOP
;
;
;           + DO i1 = 0, 99, 1   <DO_LOOP>
;           |   + DO i2 = 0, 7, 1   <DO_LOOP>
;           |   |   %t1 = (%A)[0][i1][i2 + 1];
;           |   |   %t2 = (@D)[0][i2];
;           |   |   (@D)[0][i2] = %t1 + %t2;
;           |   + END LOOP
;           + END LOOP
;     END REGION

; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 8, 1   <DO_LOOP>
; CHECK:           |   |   (%A)[0][i1][i2] = i1 + i2;
; CHECK:           |   + END LOOP
; CHECK-NOT:       + DO i1 = 0, 99, 1
; CHECK:           |
; CHECK:           |
; CHECK:           |   + DO i2 = 0, 7, 1   <DO_LOOP>
; CHECK:           |   |   %t1 = (%A)[0][i1][i2 + 1];
; CHECK:           |   |   %t2 = (@D)[0][i2];
; CHECK:           |   |   (@D)[0][i2] = %t1 + %t2;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@D = dso_local local_unnamed_addr global [8 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  %A = alloca [100 x [9 x i32]], align 16
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.cond.cleanup3
  %i.052 = phi i32 [ 0, %entry ], [ %inc8, %for.cond.cleanup3 ]
  %idxprom = zext i32 %i.052 to i64
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %inc8 = add nuw nsw i32 %i.052, 1
  %exitcond55 = icmp eq i32 %inc8, 100
  br i1 %exitcond55, label %for.cond16.preheader.preheader, label %for.cond1.preheader

for.cond16.preheader.preheader:                   ; preds = %for.cond.cleanup3
  br label %for.cond16.preheader

for.body4:                                        ; preds = %for.cond1.preheader, %for.body4
  %j.051 = phi i32 [ 0, %for.cond1.preheader ], [ %inc, %for.body4 ]
  %add = add nuw nsw i32 %j.051, %i.052
  %idxprom5 = zext i32 %j.051 to i64
  %arrayidx6 = getelementptr inbounds [100 x [9 x i32]], ptr %A, i64 0, i64 %idxprom, i64 %idxprom5
  store i32 %add, ptr %arrayidx6, align 4
  %inc = add nuw nsw i32 %j.051, 1
  %exitcond54 = icmp eq i32 %inc, 9
  br i1 %exitcond54, label %for.cond.cleanup3, label %for.body4

for.cond16.preheader:                             ; preds = %for.cond16.preheader.preheader, %for.cond.cleanup18
  %i10.050 = phi i32 [ %inc32, %for.cond.cleanup18 ], [ 0, %for.cond16.preheader.preheader ]
  %idxprom20 = zext i32 %i10.050 to i64
  br label %for.body19

for.body19:                                       ; preds = %for.cond16.preheader, %for.body19
  %j15.049 = phi i32 [ 0, %for.cond16.preheader ], [ %add22, %for.body19 ]
  %add22 = add nuw nsw i32 %j15.049, 1
  %idxprom23 = zext i32 %add22 to i64
  %arrayidx24 = getelementptr inbounds [100 x [9 x i32]], ptr %A, i64 0, i64 %idxprom20, i64 %idxprom23
  %t1 = load i32, ptr %arrayidx24, align 4
  %idxprom25 = zext i32 %j15.049 to i64
  %arrayidx26 = getelementptr inbounds [8 x i32], ptr @D, i64 0, i64 %idxprom25
  %t2 = load i32, ptr %arrayidx26, align 4
  %add27 = add nsw i32 %t2, %t1
  store i32 %add27, ptr %arrayidx26, align 4
  %exitcond = icmp eq i32 %add22, 8
  br i1 %exitcond, label %for.cond.cleanup18, label %for.body19

for.cond.cleanup18:                               ; preds = %for.body19
  %inc32 = add nuw nsw i32 %i10.050, 1
  %exitcond53 = icmp eq i32 %inc32, 100
  br i1 %exitcond53, label %for.cond.cleanup13, label %for.cond16.preheader
  
for.cond.cleanup13:                               ; preds = %for.cond.cleanup18
  ret void
}

