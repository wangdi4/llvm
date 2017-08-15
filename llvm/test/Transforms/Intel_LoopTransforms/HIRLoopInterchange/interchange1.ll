; Test for Loop Interchange					
;    for (i=1; i < 900; i++) {
;        for (j=1; j <= 96; j++) {
;            A[j][i] = A[j][i+n] + 1; 
;
; REQUIRES: asserts
; RUN: opt -debug -hir-ssa-deconstruction -hir-loop-interchange  < %s 2>&1 | FileCheck %s
; CHECK: Interchanged:
; CHECK-SAME:  ( 2 1 )  
; 
;
; ModuleID = 'interchange1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [1000 x [1000 x float]] zeroinitializer, align 16
@B = common global [1000 x [1000 x float]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub3(i64 %n) #0 {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.8, %entry
  %i.019 = phi i64 [ 1, %entry ], [ %inc9, %for.inc.8 ]
  %add = add nsw i64 %i.019, %n
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3, %for.cond.1.preheader
  %j.018 = phi i64 [ 1, %for.cond.1.preheader ], [ %inc, %for.body.3 ]
  %arrayidx4 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @A, i64 0, i64 %j.018, i64 %add
  %0 = load float, float* %arrayidx4, align 4, !tbaa !1
  %add5 = fadd float %0, 1.000000e+00
  %arrayidx7 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @A, i64 0, i64 %j.018, i64 %i.019
  store float %add5, float* %arrayidx7, align 4, !tbaa !1
  %inc = add nuw nsw i64 %j.018, 1
  %exitcond = icmp eq i64 %inc, 97
  br i1 %exitcond, label %for.inc.8, label %for.body.3

for.inc.8:                                        ; preds = %for.body.3
  %inc9 = add nuw nsw i64 %i.019, 1
  %exitcond20 = icmp eq i64 %inc9, 900
  br i1 %exitcond20, label %for.end.10, label %for.cond.1.preheader

for.end.10:                                       ; preds = %for.inc.8
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1250) (llvm/branches/loopopt 1301)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
