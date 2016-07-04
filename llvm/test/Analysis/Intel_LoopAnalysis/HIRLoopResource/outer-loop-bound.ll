; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-loop-resource | FileCheck %s

; Src code-
; for(i=0; i<n; i++) {
;   t = 0;
;   for(j=0; j<n; j++) {
;     t += (A[j] + B[j]);
;   }
;   s += t;
; }

; HIR-
;  + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;  |   %t.024 = 0;
;  |   + DO i2 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
;  |   |   %t.024.out = %t.024;
;  |   |   %0 = {al:4}(%A)[i2];
;  |   |   %1 = {al:4}(%B)[i2];
;  |   |   %t.024 = %0 + %t.024.out  +  %1;
;  |   + END LOOP
;  |   %s.027 = %t.024  +  %s.027;
;  + END LOOP


; Check the loop resource for i1 and i2 loop.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:    Integer Operations: 3
; CHECK:    Integer Operations Cost: 2
; CHECK:    Integer Bound
; CHECK:     + DO i2 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK:    |   Integer Operations: 6
; CHECK:    |   Integer Operations Cost: 6
; CHECK:    |   Integer Memory Reads: 2
; CHECK:    |   Memory Operations Cost: 8
; CHECK:    |   Memory Bound
; CHECK:     + END LOOP
; CHECK: + END LOOP


; Verify that i1 loop's total resource is marked as memory bound because of i2 loop.
; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-loop-resource -hir-print-total-resource | FileCheck -check-prefix=TOTAL %s

; TOTAL: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; TOTAL:    Integer Operations: 9
; TOTAL:    Integer Operations Cost: 8
; TOTAL:    Integer Memory Reads: 2
; TOTAL:    Memory Operations Cost: 8
; TOTAL:    Memory Bound
; TOTAL:     + DO i2 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; TOTAL:    |   Integer Operations: 6
; TOTAL:    |   Integer Operations Cost: 6
; TOTAL:    |   Integer Memory Reads: 2
; TOTAL:    |   Memory Operations Cost: 8
; TOTAL:    |   Memory Bound
; TOTAL:     + END LOOP
; TOTAL: + END LOOP


; ModuleID = 'outer-loop-bound.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32* nocapture readonly %A, i32* nocapture readonly %B, i32 %n) {
entry:
  %cmp25 = icmp sgt i32 %n, 0
  br i1 %cmp25, label %for.body3.preheader.preheader, label %for.end10

for.body3.preheader.preheader:                    ; preds = %entry
  br label %for.body3.preheader

for.body3.preheader:                              ; preds = %for.body3.preheader.preheader, %for.end
  %s.027 = phi i32 [ %add7, %for.end ], [ 0, %for.body3.preheader.preheader ]
  %i.026 = phi i32 [ %inc9, %for.end ], [ 0, %for.body3.preheader.preheader ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.preheader
  %indvars.iv = phi i64 [ 0, %for.body3.preheader ], [ %indvars.iv.next, %for.body3 ]
  %t.024 = phi i32 [ 0, %for.body3.preheader ], [ %add6, %for.body3 ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx5 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx5, align 4
  %add = add i32 %0, %t.024
  %add6 = add i32 %add, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body3

for.end:                                          ; preds = %for.body3
  %add7 = add nsw i32 %add6, %s.027
  %inc9 = add nuw nsw i32 %i.026, 1
  %exitcond29 = icmp eq i32 %inc9, %n
  br i1 %exitcond29, label %for.end10.loopexit, label %for.body3.preheader

for.end10.loopexit:                               ; preds = %for.end
  br label %for.end10

for.end10:                                        ; preds = %for.end10.loopexit, %entry
  %s.0.lcssa = phi i32 [ 0, %entry ], [ %add7, %for.end10.loopexit ]
  ret i32 %s.0.lcssa
}
