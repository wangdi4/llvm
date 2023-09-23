; RUN: opt -passes="hir-ssa-deconstruction,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Verify that we include the preheader of the small trip innermost loop in the
; region. This allows complete unroller's profitability analysis to deduce
; elimination of loads inside the loop after unrolling due to dominating stores
; in the preheader. In the case below, complete unroll (optimistically) assumes all
; loads to (%p)[i1] can be eliminated due to store of (%p)[0].


; CHECK: BEGIN REGION { }
; CHECK: (%p)[0] = 0;
; CHECK: %t = 0;

; CHECK: + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |   %ld = (%p)[i1];
; CHECK: |   %t = %ld  +  %t;
; CHECK: + END LOOP
; CHECK: END REGION


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(ptr nocapture readonly %p) {
entry:
  br label %for.body.pre

for.body.pre:                              ; preds = %entry
  store i32 0, ptr %p 
  br label %for.body

for.body:                                      ; preds = %6, %for.body.pre
  %t = phi i32 [ 0, %for.body.pre ], [ %add, %for.body ]
  %indvars.iv = phi i64 [ 0, %for.body.pre ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %indvars.iv
  %ld = load i32, ptr %arrayidx, align 4
  %add = add i32 %ld, %t
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                      ; preds = %6
  %.lcssa = phi i32 [ %add, %for.body ]
  ret i32 %.lcssa
}

