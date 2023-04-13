; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-loop-resource>" -disable-output 2>&1 | FileCheck %s

; Src code-
; for(i=0; i<n; i++)
;  A[i] = B[i] + C[i];

; HIR-
; + DO i1 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; |   %0 = (%B)[i1];
; |   %1 = (%C)[i1];
; |   (%A)[i1] = %0 + %1;
; + END LOOP

; Check the loop resource and verify that it is memory bound.

; CHECK: + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK:    Integer Operations: 7
; CHECK:    Integer Operations Cost: 7
; CHECK:    Integer Memory Reads: 2
; CHECK:    Integer Memory Writes: 1
; CHECK:    Memory Operations Cost: 12
; CHECK:    Memory Bound
; CHECK: + END LOOP

; Total resouce should be same as self resource for innermost loops.
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-loop-resource>" -hir-print-total-resource -disable-output 2>&1 | FileCheck %s


; ModuleID = 'mem-bound.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %A, i32* nocapture readonly %B, i32* nocapture readonly %C, i32 %n) {
entry:
  %cmp10 = icmp sgt i32 %n, 0
  br i1 %cmp10, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %C, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4
  %add = add nsw i32 %1, %0
  %arrayidx4 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %add, i32* %arrayidx4, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
