
; Test for completely unrolling multi-dependent loop pattern.

; RUN: opt -hir-ssa-deconstruction -hir-complete-unroll -print-after=hir-complete-unroll 2>&1 < %s | FileCheck %s

; CHECK: BEGIN REGION { modified }
; CHECK: %0 = {al:8}(%B)[0]
; CHECK: %1 = {al:4}(%0)[1]
; CHECK: %2 = {al:8}(%A)[0]
; CHECK: {al:4}(%2)[0] = %1
; CHECK: END REGION

; Source Code
; int foo(int **A, int **B) {
;    for(int64_t k=0; k < 2; k++){
;     for(int64_t i = 0; i < 2; i++) {
;       for(int64_t j=i; j<k; j++)
;         A[i][j] = B[j][k];
;     }
;    }
;     return A[2][3];
; }


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @_Z3fooPPiS0_(i32** %A, i32** %B) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc13
  %k.04 = phi i64 [ 0, %entry ], [ %inc14, %for.inc13 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.inc10
  %i.03 = phi i64 [ 0, %for.body ], [ %inc11, %for.inc10 ]
  %cmp51 = icmp slt i64 %i.03, %k.04
  br i1 %cmp51, label %for.body6.lr.ph, label %for.end

for.body6.lr.ph:                                  ; preds = %for.body3
  br label %for.body6

for.body6:                                        ; preds = %for.body6.lr.ph, %for.inc
  %j.02 = phi i64 [ %i.03, %for.body6.lr.ph ], [ %inc, %for.inc ]
  %arrayidx = getelementptr inbounds i32*, i32** %B, i64 %j.02
  %0 = load i32*, i32** %arrayidx, align 8
  %arrayidx7 = getelementptr inbounds i32, i32* %0, i64 %k.04
  %1 = load i32, i32* %arrayidx7, align 4
  %arrayidx8 = getelementptr inbounds i32*, i32** %A, i64 %i.03
  %2 = load i32*, i32** %arrayidx8, align 8
  %arrayidx9 = getelementptr inbounds i32, i32* %2, i64 %j.02
  store i32 %1, i32* %arrayidx9, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body6
  %inc = add nsw i64 %j.02, 1
  %cmp5 = icmp slt i64 %inc, %k.04
  br i1 %cmp5, label %for.body6, label %for.cond4.for.end_crit_edge

for.cond4.for.end_crit_edge:                      ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond4.for.end_crit_edge, %for.body3
  br label %for.inc10

for.inc10:                                        ; preds = %for.end
  %inc11 = add nsw i64 %i.03, 1
  %cmp2 = icmp slt i64 %inc11, 2
  br i1 %cmp2, label %for.body3, label %for.end12

for.end12:                                        ; preds = %for.inc10
  br label %for.inc13

for.inc13:                                        ; preds = %for.end12
  %inc14 = add nsw i64 %k.04, 1
  %cmp = icmp slt i64 %inc14, 2
  br i1 %cmp, label %for.body, label %for.end15

for.end15:                                        ; preds = %for.inc13
  %arrayidx16 = getelementptr inbounds i32*, i32** %A, i64 2
  %3 = load i32*, i32** %arrayidx16, align 8
  %arrayidx17 = getelementptr inbounds i32, i32* %3, i64 3
  %4 = load i32, i32* %arrayidx17, align 4
  ret i32 %4
}


attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 3770) (llvm/branches/loopopt 6801)"}
