
; Test for completely unrolling multiple triangular loops.

; RUN: opt -hir-ssa-deconstruction -hir-complete-unroll -print-after=hir-complete-unroll 2>&1 < %s | FileCheck %s

; CHECK: BEGIN REGION { modified }
; CHECK: %0 = {al:8}(%B)[0]
; CHECK: %1 = {al:4}(%0)[1]
; CHECK: %2 = {al:8}(%A)[1]
; CHECK: {al:4}(%2)[0] = %1
; CHECK: %4 = {al:4}(%3)[0]
; CHECK: {al:4}(%3)[0] = sext.i32.i64(%4) + 1
; CHECK: %4 = {al:4}(%3)[0]
; CHECK: {al:4}(%3)[0] = sext.i32.i64(%4) + 2
; CHECK: {al:4}(%3)[1] = sext.i32.i64(%4) + 2
; CHECK: END REGION


; Source Code
; int foo(int **A, int **B) {
;    for(int64_t i = 0; i < 3; i++) {
;      for(int64_t j=0; j<i; j++) {
;        A[i][j] = B[j][i];
;      }
;      for(int64_t j=0; j<i; j++) {
;        A[i][j] += i;
;      }
;    }
;    return A[2][3];
;}


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @_Z3fooPPiS0_(i32** %A, i32** %B) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc17
  %i.05 = phi i64 [ 0, %entry ], [ %inc18, %for.inc17 ]
  %cmp21 = icmp slt i64 0, %i.05
  br i1 %cmp21, label %for.body3.lr.ph, label %for.end

for.body3.lr.ph:                                  ; preds = %for.body
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.inc
  %j.02 = phi i64 [ 0, %for.body3.lr.ph ], [ %inc, %for.inc ]
  %arrayidx = getelementptr inbounds i32*, i32** %B, i64 %j.02
  %0 = load i32*, i32** %arrayidx, align 8
  %arrayidx4 = getelementptr inbounds i32, i32* %0, i64 %i.05
  %1 = load i32, i32* %arrayidx4, align 4
  %arrayidx5 = getelementptr inbounds i32*, i32** %A, i64 %i.05
  %2 = load i32*, i32** %arrayidx5, align 8
  %arrayidx6 = getelementptr inbounds i32, i32* %2, i64 %j.02
  store i32 %1, i32* %arrayidx6, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %inc = add nsw i64 %j.02, 1
  %cmp2 = icmp slt i64 %inc, %i.05
  br i1 %cmp2, label %for.body3, label %for.cond1.for.end_crit_edge

for.cond1.for.end_crit_edge:                      ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond1.for.end_crit_edge, %for.body
  %cmp93 = icmp slt i64 0, %i.05
  br i1 %cmp93, label %for.body10.lr.ph, label %for.end16

for.body10.lr.ph:                                 ; preds = %for.end
  br label %for.body10

for.body10:                                       ; preds = %for.body10.lr.ph, %for.inc14
  %j7.04 = phi i64 [ 0, %for.body10.lr.ph ], [ %inc15, %for.inc14 ]
  %arrayidx11 = getelementptr inbounds i32*, i32** %A, i64 %i.05
  %3 = load i32*, i32** %arrayidx11, align 8
  %arrayidx12 = getelementptr inbounds i32, i32* %3, i64 %j7.04
  %4 = load i32, i32* %arrayidx12, align 4
  %conv = sext i32 %4 to i64
  %add = add nsw i64 %conv, %i.05
  %conv13 = trunc i64 %add to i32
  store i32 %conv13, i32* %arrayidx12, align 4
  br label %for.inc14

for.inc14:                                        ; preds = %for.body10
  %inc15 = add nsw i64 %j7.04, 1
  %cmp9 = icmp slt i64 %inc15, %i.05
  br i1 %cmp9, label %for.body10, label %for.cond8.for.end16_crit_edge

for.cond8.for.end16_crit_edge:                    ; preds = %for.inc14
  br label %for.end16

for.end16:                                        ; preds = %for.cond8.for.end16_crit_edge, %for.end
  br label %for.inc17

for.inc17:                                        ; preds = %for.end16
  %inc18 = add nsw i64 %i.05, 1
  %cmp = icmp slt i64 %inc18, 3
  br i1 %cmp, label %for.body, label %for.end19

for.end19:                                        ; preds = %for.inc17
  %arrayidx20 = getelementptr inbounds i32*, i32** %A, i64 2
  %5 = load i32*, i32** %arrayidx20, align 8
  %arrayidx21 = getelementptr inbounds i32, i32* %5, i64 3
  %6 = load i32, i32* %arrayidx21, align 4
  ret i32 %6
}


attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 3770) (llvm/branches/loopopt 6801)"}
