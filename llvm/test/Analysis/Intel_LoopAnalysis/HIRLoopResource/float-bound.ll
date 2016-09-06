; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-loop-resource | FileCheck %s

; Src code-
; float t = 0;
; for(i=0; i<n; i++)
;  t += *p;

; HIR-
; + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; |   %t.07 = %t.07  +  %0;
; + END LOOP

; Check the loop resource and verify that it is float bound.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:    Floating Point Operations: 1
; CHECK:    Floating Point Operations Cost: 1
; CHECK:    Floating Point Bound
; CHECK: + END LOOP

; Total resouce should be same as self resource for innermost loops.
; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-loop-resource -hir-print-total-resource | FileCheck %s


; ModuleID = 'float-bound.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @foo(i32* nocapture readnone %A, i32 %n, float* nocapture readonly %p) {
entry:
  %cmp5 = icmp sgt i32 %n, 0
  br i1 %cmp5, label %for.body.lr.ph, label %for.end

for.body.lr.ph:                                   ; preds = %entry
  %0 = load float, float* %p, align 4
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %t.07 = phi float [ 0.000000e+00, %for.body.lr.ph ], [ %add, %for.body ]
  %i.06 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %add = fadd float %t.07, %0
  %inc = add nuw nsw i32 %i.06, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %t.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %add, %for.end.loopexit ]
  ret float %t.0.lcssa
}
