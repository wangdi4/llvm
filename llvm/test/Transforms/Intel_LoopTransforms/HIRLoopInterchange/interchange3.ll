; Test for interchange
;
;   for (i1=1; i1 <= 99; i1++) {
;      for (i2=1; i2 <= 98; i2++) {
;	  	for (i3=1; i3 <= 97; i3++) {
;	         for (i4=1; i4 <= 96; i4++) {
;	          for (i5=1; i5 <= 95; i5++) {
;		    A[i5+n][i4][i3][i2][i1] += 2;
;
; REQUIRES: asserts
; RUN: opt -debug -hir-ssa-deconstruction -hir-loop-interchange  < %s 2>&1 | FileCheck %s
; CHECK: Interchanged:
; CHECK-SAME:  ( 5 4 3 2 1 )  
; 

; ModuleID = 'interchange3.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [100 x [100 x [100 x [100 x [100 x float]]]]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub3(i64 %n, i64 %m) #0 {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.27, %entry
  %i1.049 = phi i64 [ 1, %entry ], [ %inc28, %for.inc.27 ]
  br label %for.cond.4.preheader

for.cond.4.preheader:                             ; preds = %for.inc.24, %for.cond.1.preheader
  %i2.048 = phi i64 [ 1, %for.cond.1.preheader ], [ %inc25, %for.inc.24 ]
  br label %for.cond.7.preheader

for.cond.7.preheader:                             ; preds = %for.inc.21, %for.cond.4.preheader
  %i3.047 = phi i64 [ 1, %for.cond.4.preheader ], [ %inc22, %for.inc.21 ]
  br label %for.cond.10.preheader

for.cond.10.preheader:                            ; preds = %for.inc.18, %for.cond.7.preheader
  %i4.046 = phi i64 [ 1, %for.cond.7.preheader ], [ %inc19, %for.inc.18 ]
  br label %for.body.12

for.body.12:                                      ; preds = %for.body.12, %for.cond.10.preheader
  %i5.045 = phi i64 [ 1, %for.cond.10.preheader ], [ %inc, %for.body.12 ]
  %add = add nsw i64 %i5.045, %n
  %arrayidx16 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x float]]]]], [100 x [100 x [100 x [100 x [100 x float]]]]]* @A, i64 0, i64 %add, i64 %i4.046, i64 %i3.047, i64 %i2.048, i64 %i1.049
  %0 = load float, float* %arrayidx16, align 4, !tbaa !1
  %add17 = fadd float %0, 2.000000e+00
  store float %add17, float* %arrayidx16, align 4, !tbaa !1
  %inc = add nuw nsw i64 %i5.045, 1
  %exitcond = icmp eq i64 %inc, 96
  br i1 %exitcond, label %for.inc.18, label %for.body.12

for.inc.18:                                       ; preds = %for.body.12
  %inc19 = add nuw nsw i64 %i4.046, 1
  %exitcond50 = icmp eq i64 %inc19, 97
  br i1 %exitcond50, label %for.inc.21, label %for.cond.10.preheader

for.inc.21:                                       ; preds = %for.inc.18
  %inc22 = add nuw nsw i64 %i3.047, 1
  %exitcond51 = icmp eq i64 %inc22, 98
  br i1 %exitcond51, label %for.inc.24, label %for.cond.7.preheader

for.inc.24:                                       ; preds = %for.inc.21
  %inc25 = add nuw nsw i64 %i2.048, 1
  %exitcond52 = icmp eq i64 %inc25, 99
  br i1 %exitcond52, label %for.inc.27, label %for.cond.4.preheader

for.inc.27:                                       ; preds = %for.inc.24
  %inc28 = add nuw nsw i64 %i1.049, 1
  %exitcond53 = icmp eq i64 %inc28, 100
  br i1 %exitcond53, label %for.end.29, label %for.cond.1.preheader

for.end.29:                                       ; preds = %for.inc.27
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1250) (llvm/branches/loopopt 1301)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
