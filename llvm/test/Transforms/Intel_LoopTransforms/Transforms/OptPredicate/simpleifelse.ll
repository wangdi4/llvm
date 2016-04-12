; Test for OptPredicate with single if else block inside a single level loop.

; Source code:
;        i= 10;
;        for (j=1; j < 1000; j++) {
;            A[j][i] = B[i][j] + C[j][i] + B[i+1][j] + B[i][j+1];
;            if(n>10) {
;                B[i+1][j+1] = A[j][i];
;            } else {
;                B[i+1][j] = A[j][i] + C[i][j];
;            }
;        }
;        C[i][2] = i;

; HIR Before Opt Predicate.
;          BEGIN REGION { }
; <33>         + DO i1 = 0, 998, 1   <DO_LOOP>
; <3>          |   %1 = {al:8}(@C)[0][i1 + 1][10];
; <4>          |   %add = %0  +  %1;
; <6>          |   %2 = {al:4}(@B)[0][11][i1 + 1];
; <7>          |   %add7 = %add  +  %2;
; <10>         |   %3 = {al:4}(@B)[0][10][i1 + 2];
; <11>         |   %add11 = %add7  +  %3;
; <13>         |   {al:8}(@A)[0][i1 + 1][10] = %add11;
; <14>         |   if (%n > 10)
; <14>         |   {
; <19>         |      {al:4}(@B)[0][11][i1 + 2] = %add11;
; <14>         |   }
; <14>         |   else
; <14>         |   {
; <29>         |      %4 = {al:4}(@C)[0][10][i1 + 1];
; <30>         |      %add25 = %add11  +  %4;
; <31>         |      {al:4}(@B)[0][11][i1 + 1] = %add25;
; <14>         |   }
; <23>         |   %0 = %3;
; <33>         + END LOOP
;          END REGION


; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -S < %s 2>&1 | FileCheck %s

; Check the then loop.
; CHECK: REGION { modified }
; CHECK: if (%n > 10)
; CHECK: DO i1 = 0, 998, 1
; CHECK-NEXT:  = {al:8}(@C)[0][i1 + 1][10];
; CHECK: {al:4}(@B)[0][11][i1 + 2] =
; CHECK: END LOOP

; Check else block.
; CHECK: else
; CHECK: DO i1 = 0, 998, 1
; CHECK-NEXT:  = {al:8}(@C)[0][i1 + 1][10];
; CHECK-NOT: {al:4}(@B)[0][11][i1 + 2] =
; CHECK: = {al:4}(@C)[0][10][i1 + 1];
; CHECK: {al:4}(@B)[0][11][i1 + 1] =
; CHECK: END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [1000 x [1000 x float]] zeroinitializer, align 16
@C = common global [1000 x [1000 x float]] zeroinitializer, align 16
@A = common global [1000 x [1000 x float]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub3(i64 %n) {
entry:
  %cmp14 = icmp sgt i64 %n, 10
  %.pre = load float, float* getelementptr inbounds ([1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 10, i64 1), align 4 
  br label %for.body

for.body:                                         ; preds = %for.cond.backedge, %entry
  %0 = phi float [ %.pre, %entry ], [ %3, %for.cond.backedge ]
  %j.052 = phi i64 [ 1, %entry ], [ %add8, %for.cond.backedge ]
  %arrayidx3 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 %j.052, i64 10
  %1 = load float, float* %arrayidx3, align 8
  %add = fadd float %0, %1
  %arrayidx6 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 11, i64 %j.052
  %2 = load float, float* %arrayidx6, align 4
  %add7 = fadd float %add, %2
  %add8 = add nuw nsw i64 %j.052, 1
  %arrayidx10 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 10, i64 %add8
  %3 = load float, float* %arrayidx10, align 4
  %add11 = fadd float %add7, %3
  %arrayidx13 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @A, i64 0, i64 %j.052, i64 10
  store float %add11, float* %arrayidx13, align 8
  br i1 %cmp14, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %arrayidx20 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 11, i64 %add8
  store float %add11, float* %arrayidx20, align 4
  br label %for.cond.backedge

for.cond.backedge:                                ; preds = %if.then, %if.else
  %exitcond = icmp eq i64 %add8, 1000
  br i1 %exitcond, label %for.end, label %for.body

if.else:                                          ; preds = %for.body
  %arrayidx24 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 10, i64 %j.052
  %4 = load float, float* %arrayidx24, align 4
  %add25 = fadd float %add11, %4
  store float %add25, float* %arrayidx6, align 4
  br label %for.cond.backedge

for.end:                                          ; preds = %for.cond.backedge
  store float 1.000000e+01, float* getelementptr inbounds ([1000 x [1000 x float]], [1000 x [1000 x float]]* @C, i64 0, i64 10, i64 2), align 8
  ret void
}

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.start(i64, i8* nocapture) 

; Function Attrs: nounwind argmemonly
declare void @llvm.lifetime.end(i64, i8* nocapture) 



