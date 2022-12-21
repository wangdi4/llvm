; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output -hir-cost-model-throttling=0 < %s 2>&1 | FileCheck %s

; Verify that two loops will be fused with a safe call in between.


; BEGIN REGION { }
;      + DO i1 = 0, 1, 1   <DO_LOOP>
;      |   + DO i2 = 0, 99, 1   <DO_LOOP>
;      |   |   %0 = (@A)[0][i2];
;      |   |   (@B)[0][i2] = i1 + i2 + %0;
;      |   + END LOOP
;      |
;      |   @bar();
;      |
;      |   + DO i2 = 0, 99, 1   <DO_LOOP>
;      |   |   %3 = (@B)[0][i2];
;      |   |   (@C)[0][i2] = i2 + %3;
;      |   + END LOOP
;      + END LOOP
; END REGION

; CHECK: DO i1
; CHECK: DO i2 = 0, 99
; CHECK-NOT: DO i2 = 0, 99

;Module Before HIR; ModuleID = '/export/iusers/pgprokof/loopopt-6/fus1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup11, %entry
  %indvars.iv44 = phi i64 [ 0, %entry ], [ %indvars.iv.next45, %for.cond.cleanup11 ]
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup11
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  tail call void (...) @bar()
  br label %for.body12

for.body4:                                        ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %1 = add nuw nsw i64 %indvars.iv, %indvars.iv44
  %2 = trunc i64 %1 to i32
  %add5 = add i32 %0, %2
  %arrayidx7 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  store i32 %add5, i32* %arrayidx7, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4

for.cond.cleanup11:                               ; preds = %for.body12
  %indvars.iv.next45 = add nuw nsw i64 %indvars.iv44, 1
  %exitcond46 = icmp eq i64 %indvars.iv.next45, 2
  br i1 %exitcond46, label %for.cond.cleanup, label %for.cond1.preheader

for.body12:                                       ; preds = %for.body12, %for.cond.cleanup3
  %indvars.iv41 = phi i64 [ 0, %for.cond.cleanup3 ], [ %indvars.iv.next42, %for.body12 ]
  %arrayidx14 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv41
  %3 = load i32, i32* %arrayidx14, align 4
  %4 = trunc i64 %indvars.iv41 to i32
  %add15 = add nsw i32 %3, %4
  %arrayidx17 = getelementptr inbounds [100 x i32], [100 x i32]* @C, i64 0, i64 %indvars.iv41
  store i32 %add15, i32* %arrayidx17, align 4
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond43 = icmp eq i64 %indvars.iv.next42, 100
  br i1 %exitcond43, label %for.cond.cleanup11, label %for.body12
}

declare dso_local void @bar(...) local_unnamed_addr #0

attributes #0 = { nounwind readnone }

