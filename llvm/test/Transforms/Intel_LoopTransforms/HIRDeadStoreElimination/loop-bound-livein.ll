; RUN: opt -hir-create-function-level-region -hir-details -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination 2>&1 < %s | FileCheck %s

; Verify that we are able to eliminate load/store to %bound which is a region
; local alloca and the propagate %n to the loop upper via copy propagation
; and successfully add %n as livein to the loop.

; CHECK: Dump Before

; CHECK: (%bound)[0] = %n;
; CHECK: <RVAL-REG> LINEAR i64 %n {sb:[[NSYM:.*]]}

; CHECK: %ld = (%bound)[0];

; CHECK: + DO i64 i1 = 0, %ld + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   (@A)[0][i1] = i1;
; CHECK: + END LOOP


; CHECK: Dump After


; CHECK: %ld = %n;

; CHECK: LiveIn symbases: 
; CHECK-SAME: [[NSYM]]
; CHECK: + DO i64 i1 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
; CHECK: |   (@A)[0][i1] = i1;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x i64] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable writeonly
define dso_local void @foo(i64 %n) local_unnamed_addr #0 {
entry:
  %bound = alloca i64
  br label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  store i64 %n, ptr %bound, align 4
  %ld = load i64, ptr %bound, align 4
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv21 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next22, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i64], ptr @A, i64 0, i64 %indvars.iv21
  store i64 %indvars.iv21, ptr %arrayidx, align 4
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, 1
  %exitcond24 = icmp eq i64 %indvars.iv.next22, %ld
  br i1 %exitcond24, label %for.end8, label %for.body

for.end8:                                         ; preds = %for.end8.loopexit, %for.cond1.preheader
  ret void
}

