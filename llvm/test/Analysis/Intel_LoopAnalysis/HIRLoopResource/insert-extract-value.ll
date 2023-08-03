; RUN: opt -passes="hir-ssa-deconstruction,print<hir-loop-resource>" -disable-output < %s 2>&1 | FileCheck %s

; Verify that we are able to compute loop resource of loops containing
; insertvalue/extractvalue instructions.

; HIR-
; + DO i1 = 0, 9, 1   <DO_LOOP>
; |   %ld = (%ptr)[i1];
; |   %agg = insertvalue undef,  %ld, 1;
; |   %res = extractvalue %agg, 1;
; |   %t.07 = %t.07  +  %res;
; + END LOOP

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK:    Integer Operations: 3
; CHECK:    Integer Operations Cost: 3
; CHECK:    Floating Point Operations: 2
; CHECK:    Floating Point Operations Cost: 3
; CHECK:    Floating Point Reads: 1
; CHECK:    Memory Operations Cost: 4
; CHECK:    Total Cost: 10
; CHECK:    Memory Bound
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define float @foo(ptr nocapture readonly %ptr) {
for.body.lr.ph:                                   
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %t.07 = phi float [ 0.000000e+00, %for.body.lr.ph ], [ %add, %for.body ]
  %i.06 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds float, ptr %ptr, i64 %i.06
  %ld = load float, ptr %arrayidx, align 4
  %agg = insertvalue {i32, float} undef, float %ld, 1
  %res = extractvalue {i32, float} %agg, 1
  %add = fadd float %t.07, %res
  %inc = add nuw nsw i64 %i.06, 1
  %exitcond = icmp eq i64 %inc, 10
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  %t.0.lcssa = phi float [ %add, %for.body ]
  ret float %t.0.lcssa
}
