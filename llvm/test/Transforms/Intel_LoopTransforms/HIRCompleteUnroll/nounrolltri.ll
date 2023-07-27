
; Test for not performing complete unrolling due to large trip count for triangular loops.

; RUN: opt -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll,print<hir>" -hir-complete-unroll-loopnest-trip-threshold=50 2>&1 < %s | FileCheck %s

; CHECK: BEGIN REGION { }
; CHECK: END REGION

; Source Code
; int foo(int **A, int **B) {
;     for(int64_t i = 0; i < 30; i++) {
;       for(int64_t j=0; j<i; j++)
;         A[i][j] = B[j][i];
;     }
;     return A[2][3];
; }


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @_Z3fooPPiS0_(ptr %A, ptr %B) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc7
  %i.03 = phi i64 [ 0, %entry ], [ %inc8, %for.inc7 ]
  %cmp21 = icmp slt i64 0, %i.03
  br i1 %cmp21, label %for.body3.lr.ph, label %for.end

for.body3.lr.ph:                                  ; preds = %for.body
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.inc
  %j.02 = phi i64 [ 0, %for.body3.lr.ph ], [ %inc, %for.inc ]
  %arrayidx = getelementptr inbounds ptr, ptr %B, i64 %j.02
  %0 = load ptr, ptr %arrayidx, align 8
  %arrayidx4 = getelementptr inbounds i32, ptr %0, i64 %i.03
  %1 = load i32, ptr %arrayidx4, align 4
  %arrayidx5 = getelementptr inbounds ptr, ptr %A, i64 %i.03
  %2 = load ptr, ptr %arrayidx5, align 8
  %arrayidx6 = getelementptr inbounds i32, ptr %2, i64 %j.02
  store i32 %1, ptr %arrayidx6, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %inc = add nsw i64 %j.02, 1
  %cmp2 = icmp slt i64 %inc, %i.03
  br i1 %cmp2, label %for.body3, label %for.cond1.for.end_crit_edge

for.cond1.for.end_crit_edge:                      ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond1.for.end_crit_edge, %for.body
  br label %for.inc7

for.inc7:                                         ; preds = %for.end
  %inc8 = add nsw i64 %i.03, 1
  %cmp = icmp slt i64 %inc8, 30
  br i1 %cmp, label %for.body, label %for.end9

for.end9:                                         ; preds = %for.inc7
  %arrayidx10 = getelementptr inbounds ptr, ptr %A, i64 2
  %3 = load ptr, ptr %arrayidx10, align 8
  %arrayidx11 = getelementptr inbounds i32, ptr %3, i64 3
  %4 = load i32, ptr %arrayidx11, align 4
  ret i32 %4
}


attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 2121) (llvm/branches/loopopt 3770)"}
