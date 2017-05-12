
; Test for completely unrolling loop with if conditions.

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s

; CHECK: BEGIN REGION { modified }

; Iteration: {1, 0}
; CHECK:   %3 = (%A)[1];
; CHECK:  (%3)[0] = 1;

; Iteration: {2, 0} 
; CHECK:   %3 = (%A)[2];
; CHECK:  (%3)[0] = 2;

; Iteration: {2, 1}
; CHECK:   %0 = (%B)[1];
; CHECK:   %1 = (%0)[2];
; CHECK:   %2 = (%A)[2];
; CHECK:  (%2)[1] = %1;

; CHECK: END REGION

; Source Code
; int foo(int **A, int **B) {
;     for(int64_t i = 0; i < 3; i++) {
;       for(int64_t j=0; j<i; j++)
;         if((i+j) > 2) {
;          A[i][j] = B[j][i];
;         } else {
;          A[i][j] = i+j;
;         }
;     }
;     return A[2][3];
; }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @_Z3fooPPiS0_(i32** %A, i32** %B) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc11
  %i.03 = phi i64 [ 0, %entry ], [ %inc12, %for.inc11 ]
  %cmp21 = icmp slt i64 0, %i.03
  br i1 %cmp21, label %for.body3.lr.ph, label %for.end

for.body3.lr.ph:                                  ; preds = %for.body
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.inc
  %j.02 = phi i64 [ 0, %for.body3.lr.ph ], [ %inc, %for.inc ]
  %add = add nsw i64 %i.03, %j.02
  %cmp4 = icmp sgt i64 %add, 2
  br i1 %cmp4, label %if.then, label %if.else

if.then:                                          ; preds = %for.body3
  %arrayidx = getelementptr inbounds i32*, i32** %B, i64 %j.02
  %0 = load i32*, i32** %arrayidx, align 8
  %arrayidx5 = getelementptr inbounds i32, i32* %0, i64 %i.03
  %1 = load i32, i32* %arrayidx5, align 4
  %arrayidx6 = getelementptr inbounds i32*, i32** %A, i64 %i.03
  %2 = load i32*, i32** %arrayidx6, align 8
  %arrayidx7 = getelementptr inbounds i32, i32* %2, i64 %j.02
  store i32 %1, i32* %arrayidx7, align 4
  br label %if.end

if.else:                                          ; preds = %for.body3
  %add8 = add nsw i64 %i.03, %j.02
  %conv = trunc i64 %add8 to i32
  %arrayidx9 = getelementptr inbounds i32*, i32** %A, i64 %i.03
  %3 = load i32*, i32** %arrayidx9, align 8
  %arrayidx10 = getelementptr inbounds i32, i32* %3, i64 %j.02
  store i32 %conv, i32* %arrayidx10, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i64 %j.02, 1
  %cmp2 = icmp slt i64 %inc, %i.03
  br i1 %cmp2, label %for.body3, label %for.cond1.for.end_crit_edge

for.cond1.for.end_crit_edge:                      ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond1.for.end_crit_edge, %for.body
  br label %for.inc11

for.inc11:                                        ; preds = %for.end
  %inc12 = add nsw i64 %i.03, 1
  %cmp = icmp slt i64 %inc12, 3
  br i1 %cmp, label %for.body, label %for.end13

for.end13:                                        ; preds = %for.inc11
  %arrayidx14 = getelementptr inbounds i32*, i32** %A, i64 2
  %4 = load i32*, i32** %arrayidx14, align 8
  %arrayidx15 = getelementptr inbounds i32, i32* %4, i64 3
  %5 = load i32, i32* %arrayidx15, align 4
  ret i32 %5
}


attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 2121) (llvm/branches/loopopt 3770)"}
