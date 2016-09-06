; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-loop-resource | FileCheck %s

; Src code-
;  for(i=0; i<n; i++)
;   t += (i*i);

; HIR-
; + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; |   %mul = i1  *  i1;
; |   %t.09 = %t.09  +  %mul;
; + END LOOP

; Check the loop resource and verify that it is int bound.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:    Integer Operations: 2
; CHECK:    Integer Operations Cost: 2
; CHECK:    Integer Bound
; CHECK: + END LOOP

; Total resouce should be same as self resource for innermost loops.
; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-loop-resource -hir-print-total-resource | FileCheck %s

; ModuleID = 'int-bound.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32* nocapture readnone %A, i32 %n, i32* nocapture readnone %p) {
entry:
  %cmp7 = icmp sgt i32 %n, 0
  br i1 %cmp7, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %t.09 = phi i32 [ %add, %for.body ], [ 0, %for.body.preheader ]
  %i.08 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %mul = mul nsw i32 %i.08, %i.08
  %add = add nsw i32 %t.09, %mul
  %inc = add nsw i32 %i.08, 1
  %cmp = icmp slt i32 %inc, %n
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %t.0.lcssa = phi i32 [ 0, %entry ], [ %add, %for.end.loopexit ]
  ret i32 %t.0.lcssa
}
