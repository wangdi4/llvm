; Test for Loop Interchange - no interchange expected
; 		
;    for (i1=1; i1 <= 99; i1++) {
;        for (i2=1; i2 <=98 ; i2++) {
;            for (i3=1 ; i3 <= 97; i3++) {
;                for (i4=1; i4 <= 96; i4++) {
;                    for (i5=1; i5 <= 95; i5++) {
;                        A[i5][i4][i3][i2+n][i1+n] =
;                            A[i5][i4][i3][i2][i1] + 1 ;
;
; REQUIRES: asserts
; RUN: opt -O2  -loopopt -debug -hir-loop-interchange < %s 2>&1 | FileCheck %s
; CHECK-NOT: Interchanged

; ModuleID = 'interchange6.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [100 x [100 x [100 x [100 x [100 x float]]]]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub3(i64 %n, i64 %m) #0 {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.33, %entry
  %i1.061 = phi i64 [ 1, %entry ], [ %inc34, %for.inc.33 ]
  %add17 = add nsw i64 %i1.061, %n
  br label %for.cond.4.preheader

for.cond.4.preheader:                             ; preds = %for.inc.30, %for.cond.1.preheader
  %i2.060 = phi i64 [ 1, %for.cond.1.preheader ], [ %inc31, %for.inc.30 ]
  %add18 = add nsw i64 %i2.060, %n
  br label %for.cond.7.preheader

for.cond.7.preheader:                             ; preds = %for.inc.27, %for.cond.4.preheader
  %i3.059 = phi i64 [ 1, %for.cond.4.preheader ], [ %inc28, %for.inc.27 ]
  br label %for.cond.10.preheader

for.cond.10.preheader:                            ; preds = %for.inc.24, %for.cond.7.preheader
  %i4.058 = phi i64 [ 1, %for.cond.7.preheader ], [ %inc25, %for.inc.24 ]
  br label %for.body.12

for.body.12:                                      ; preds = %for.body.12, %for.cond.10.preheader
  %i5.057 = phi i64 [ 1, %for.cond.10.preheader ], [ %inc, %for.body.12 ]
  %arrayidx16 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x float]]]]], [100 x [100 x [100 x [100 x [100 x float]]]]]* @A, i64 0, i64 %i5.057, i64 %i4.058, i64 %i3.059, i64 %i2.060, i64 %i1.061
  %0 = load float, float* %arrayidx16, align 4, !tbaa !1
  %add = fadd float %0, 1.000000e+00
  %arrayidx23 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x float]]]]], [100 x [100 x [100 x [100 x [100 x float]]]]]* @A, i64 0, i64 %i5.057, i64 %i4.058, i64 %i3.059, i64 %add18, i64 %add17
  store float %add, float* %arrayidx23, align 4, !tbaa !1
  %inc = add nuw nsw i64 %i5.057, 1
  %exitcond = icmp eq i64 %inc, 96
  br i1 %exitcond, label %for.inc.24, label %for.body.12

for.inc.24:                                       ; preds = %for.body.12
  %inc25 = add nuw nsw i64 %i4.058, 1
  %exitcond62 = icmp eq i64 %inc25, 97
  br i1 %exitcond62, label %for.inc.27, label %for.cond.10.preheader

for.inc.27:                                       ; preds = %for.inc.24
  %inc28 = add nuw nsw i64 %i3.059, 1
  %exitcond63 = icmp eq i64 %inc28, 98
  br i1 %exitcond63, label %for.inc.30, label %for.cond.7.preheader

for.inc.30:                                       ; preds = %for.inc.27
  %inc31 = add nuw nsw i64 %i2.060, 1
  %exitcond64 = icmp eq i64 %inc31, 99
  br i1 %exitcond64, label %for.inc.33, label %for.cond.4.preheader

for.inc.33:                                       ; preds = %for.inc.30
  %inc34 = add nuw nsw i64 %i1.061, 1
  %exitcond65 = icmp eq i64 %inc34, 100
  br i1 %exitcond65, label %for.end.35, label %for.cond.1.preheader

for.end.35:                                       ; preds = %for.inc.33
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1250) (llvm/branches/loopopt 1322)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
