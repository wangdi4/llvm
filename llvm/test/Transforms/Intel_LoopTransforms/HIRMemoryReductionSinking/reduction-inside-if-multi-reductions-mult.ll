; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking,print<hir>" 2>&1 < %s | FileCheck %s

; This test case checks that multiple reductions inside the same If are
; identified and guarded under the same If condition when we exit the loop.
; This case handles when the reduction is a multiplication.

; HIR before transformation:

; BEGIN REGION { }
;       + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;       |   if ((%t14)[i1] >u 1.000000e+01)
;       |   {
;       |      %mul59 = %mul52  *  (%t13)[%t];
;       |      (%t13)[%t] = %mul59;
;       |      %mul60 = %mul52  *  (%t15)[%t];
;       |      (%t15)[%t] = %mul60;
;       |      %mul61 = %mul52  *  (%t16)[%t];
;       |      (%t16)[%t] = %mul61;
;       |   }
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       %tmp6 = 1.000000e+00;
; CHECK:       %tmp7 = 1.000000e+00;
; CHECK:       %tmp10 = 1.000000e+00;
; CHECK:       + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:       |   if ((%t14)[i1] >u 1.000000e+01)
; CHECK:       |   {
; CHECK:       |      %tmp10 = %tmp10  *  %mul52;
; CHECK:       |      %tmp7 = %tmp7  *  %mul52;
; CHECK:       |      %tmp6 = %tmp6  *  %mul52;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK:       %cmp = %tmp6 !=u 1.000000e+00;
; CHECK:       %cmp9 = %tmp7 !=u 1.000000e+00;
; CHECK:       %or = %cmp  |  %cmp9;
; CHECK:       %cmp12 = %tmp10 !=u 1.000000e+00;
; CHECK:       %or13 = %or  |  %cmp12;
; CHECK:       if (%or13 != 0)
; CHECK:       {
; CHECK:          %mul59 = %tmp10  *  (%t13)[%t];
; CHECK:          (%t13)[%t] = %mul59;
; CHECK:          %mul60 = %tmp7  *  (%t15)[%t];
; CHECK:          (%t15)[%t] = %mul60;
; CHECK:          %mul61 = %tmp6  *  (%t16)[%t];
; CHECK:          (%t16)[%t] = %mul61;
; CHECK:       }
; CHECK: END REGION


;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr noalias %t13, ptr noalias %t14, ptr noalias %t15, ptr noalias %t16, float %mul52, i64 %n, i64 %t) {
entry:
  br label %for.body56

for.body56:                                       ; preds = %for.body56, %entry
  %indvars.iv158 = phi i64 [ 0, %entry ], [ %indvars.iv.next159, %if.end ]
  %arrayidx58 = getelementptr inbounds float, ptr %t13, i64 %t
  %arrayidx2 = getelementptr inbounds float, ptr %t15, i64 %t
  %arrayidx3 = getelementptr inbounds float, ptr %t16, i64 %t
  %arrayidx = getelementptr inbounds float, ptr %t14, i64 %indvars.iv158
  %0 = load float, ptr %arrayidx
  %cmp1 = fcmp ugt float %0, 10.0
  br i1 %cmp1, label %if.then, label %if.end

if.then:
  %tmp = load float, ptr %arrayidx58, align 4
  %mul59 = fmul fast float %mul52, %tmp
  store float %mul59, ptr %arrayidx58, align 4
  %tmp2 = load float, ptr %arrayidx2, align 4
  %mul60 = fmul fast float %mul52, %tmp2
  store float %mul60, ptr %arrayidx2, align 4
  %tmp3 = load float, ptr %arrayidx3, align 4
  %mul61 = fmul fast float %mul52, %tmp3
  store float %mul61, ptr %arrayidx3, align 4
  br label %if.end

if.end:
  %indvars.iv.next159 = add nuw nsw i64 %indvars.iv158, 1
  %exitcond161 = icmp eq i64 %indvars.iv.next159, %n
  br i1 %exitcond161, label %for.end62.loopexit, label %for.body56

for.end62.loopexit:
  ret void
}