; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s

; Check parsing output for the loop
; CHECK: + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; CHECK: |   %0 = (%B)[i1];
; CHECK: |   %1 = (%A)[i1];
; CHECK: |   %add = %0  +  %1;
; CHECK: |   (%A)[i1] = %add;
; CHECK: + END LOOP

; Verify that %A, %B and %n are marked as livein to region and loop.
; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser -hir-details | FileCheck %s --check-prefix=LIVEIN

; LIVEIN: LiveIns: %n(%n), %A(%A), %B(%B)

; LIVEIN: LiveIn symbases: [[NSYM:.*]], [[BSYM:.*]], [[ASYM:.*]]

; LIVEIN: <BLOB> LINEAR i32 %n {sb:[[NSYM]]}
; LIVEIN: <BLOB> LINEAR float* %B {sb:[[BSYM]]}
; LIVEIN: <BLOB> LINEAR float* %A {sb:[[ASYM]]}

; ModuleID = 'float.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(float* nocapture %A, float* nocapture readonly %B, i32 %n) {
entry:
  %cmp.7 = icmp sgt i32 %n, 0
  br i1 %cmp.7, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds float, float* %B, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %1 = load float, float* %arrayidx2, align 4
  %add = fadd float %0, %1
  store float %add, float* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
