; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check for parsing of complex predicates in HLIf

;for (i=0;  i< n; i++) {
;  q[i] += 1.0;
;  if (M1 == 2  &&  M2 == 3) {
;    p[i] = q[i]+1.0;
;  }
;  q[i] += p[i];
;}

; CHECK: if (%or.cond != 0)


; ModuleID = 'compare.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @foo(float* nocapture %p, float* nocapture %q, i32 %M1, i32 %M2, i64 %n) {
entry:
  %cmp.24 = icmp sgt i64 %n, 0
  br i1 %cmp.24, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %cmp2 = icmp eq i32 %M1, 2
  %cmp4 = icmp eq i32 %M2, 3
  %or.cond = and i1 %cmp2, %cmp4
  br label %for.body

for.body:                                         ; preds = %if.end, %for.body.lr.ph
  %i.025 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %if.end ]
  %arrayidx = getelementptr inbounds float, float* %q, i64 %i.025
  %0 = load float, float* %arrayidx, align 4
  %conv1 = fadd float %0, 1.000000e+00
  store float %conv1, float* %arrayidx, align 4
  br i1 %or.cond, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %conv9 = fadd float %conv1, 1.000000e+00
  %arrayidx10 = getelementptr inbounds float, float* %p, i64 %i.025
  store float %conv9, float* %arrayidx10, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %arrayidx11 = getelementptr inbounds float, float* %p, i64 %i.025
  %1 = load float, float* %arrayidx11, align 4
  %2 = load float, float* %arrayidx, align 4
  %add13 = fadd float %1, %2
  store float %add13, float* %arrayidx, align 4
  %inc = add nuw nsw i64 %i.025, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %if.end
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret float undef
}
