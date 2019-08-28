; RUN: opt -hir-temp-cleanup -hir-loop-distribute-loopnest -S -print-after=hir-loop-distribute-loopnest -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-temp-cleanup,hir-loop-distribute-loopnest,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; See description below

; BEGIN REGION { }
;       + DO i1 = 0, 31, 1   <DO_LOOP>
;       |   if (%n == 8)
;       |   {
;       |      %add = (@B)[0][i1]  +  (@C)[0][i1];
;       |      (@A)[0][i1] = %add;
;       |
;       |      + DO i2 = 0, 31, 1   <DO_LOOP>
;       |      |   %add13 = (@A)[0][i1 + 1]  +  1.000000e+00;
;       |      |   (@D)[0][i2] = %add13;
;       |      + END LOOP
;       |   }
;       + END LOOP
; END REGION

; Check that loopnest will not be distributed over the IF statement.
; Like the following-

; BEGIN REGION { modified }
;       + DO i1 = 0, 31, 1   <DO_LOOP>
;       |   if (%n == 8)
;       |   {
;       |      + DO i2 = 0, 31, 1   <DO_LOOP>
;       |      |   %add13 = (@A)[0][i1 + 1]  +  1.000000e+00;
;       |      |   (@D)[0][i2] = %add13;
;       |      + END LOOP
;       |   }
;       + END LOOP
;
;
;       + DO i1 = 0, 31, 1   <DO_LOOP>
;       |   if (%n == 8)
;       |   {
;       |      %add = (@B)[0][i1]  +  (@C)[0][i1];
;       |      (@A)[0][i1] = %add;
;       |   }
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION
; CHECK-NOT: modified
; CHECK: DO i1
; CHECK: END LOOP
; CHECK-NOT: DO i1
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@D = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

; Function Attrs: nofree noinline norecurse nounwind uwtable
define dso_local void @foo(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp1 = icmp eq i32 %n, 8
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc16
  ret void

for.body:                                         ; preds = %for.inc16, %entry
  %indvars.iv31 = phi i64 [ 0, %entry ], [ %indvars.iv.next32, %for.inc16 ]
  br i1 %cmp1, label %if.then, label %for.inc16

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @B, i64 0, i64 %indvars.iv31
  %0 = load float, float* %arrayidx, align 4
  %arrayidx3 = getelementptr inbounds [100 x float], [100 x float]* @C, i64 0, i64 %indvars.iv31
  %1 = load float, float* %arrayidx3, align 4
  %add = fadd float %0, %1
  %arrayidx5 = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %indvars.iv31
  store float %add, float* %arrayidx5, align 4
  %2 = add nuw nsw i64 %indvars.iv31, 1
  %arrayidx12 = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %2
  br label %for.body9

for.body9:                                        ; preds = %for.body9, %if.then
  %indvars.iv = phi i64 [ 0, %if.then ], [ %indvars.iv.next, %for.body9 ]
  %3 = load float, float* %arrayidx12, align 4
  %add13 = fadd float %3, 1.000000e+00
  %arrayidx15 = getelementptr inbounds [100 x float], [100 x float]* @D, i64 0, i64 %indvars.iv
  store float %add13, float* %arrayidx15, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 32
  br i1 %exitcond, label %for.inc16.loopexit, label %for.body9

for.inc16.loopexit:                               ; preds = %for.body9
  br label %for.inc16

for.inc16:                                        ; preds = %for.inc16.loopexit, %for.body
  %indvars.iv.next32 = add nuw nsw i64 %indvars.iv31, 1
  %exitcond34 = icmp eq i64 %indvars.iv.next32, 32
  br i1 %exitcond34, label %for.cond.cleanup, label %for.body
}

