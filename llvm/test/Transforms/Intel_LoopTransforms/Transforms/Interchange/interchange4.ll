; Test for interchange with 2 loops, make sure some of the delete are okay
; 
;		
;    for (i1=1; i1 <= n; i1++) {
;        for (i2=1; i2 <= 98; i2++) {
;            for (i3=1 ; i3 <= 97; i3++) {
;                for (i4=1; i4 <= 96; i4++) {
;                    for (i5=1; i5 <= 95; i5++) {
;                        A[i5][i4][i3][i2+n][i1] =
;                            A[i5][i4][i3][i2+m][i1] + 1 ;
;     for (i2=1; i2 <= 98; i2++) {
;        for (i1=1; i1 <= n; i1++) {
;            for (i3=1 ; i3 <= 97; i3++) {
;                for (i4=1; i4 <= 96; i4++) {
;                    for (i5=1; i5 <= 95; i5++) {
;                        A[i5][i4][i3][i2+n][i1] =
;                            A[i5][i4][i3][i2+m][i1] + 1 ;
;
; REQUIRES: asserts 
; RUN: opt -O2 -loopopt -debug  -hir-loop-interchange   < %s 2>&1 | FileCheck %s
; CHECK: Interchanged:
; CHECK-SAME:  ( 2 3 4 5 1 )  
; CHECK: Interchanged:
; CHECK-SAME:  ( 1 3 4 5 2 )  
;

; ModuleID = 'interchange4.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [100 x [100 x [100 x [100 x [100 x float]]]]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub3(i64 %n, i64 %m) #0 {
entry:
  %cmp.133 = icmp slt i64 %n, 1
  br i1 %cmp.133, label %for.cond.36.preheader, label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %entry, %for.inc.33
  %i1.0134 = phi i64 [ %inc34, %for.inc.33 ], [ 1, %entry ]
  br label %for.cond.4.preheader

for.cond.36.preheader:                            ; preds = %for.inc.33, %entry
  %cmp40.126 = icmp slt i64 %n, 1
  br label %for.cond.39.preheader

for.cond.4.preheader:                             ; preds = %for.inc.30, %for.cond.1.preheader
  %i2.0132 = phi i64 [ 1, %for.cond.1.preheader ], [ %inc31, %for.inc.30 ]
  %add = add nsw i64 %i2.0132, %m
  %add18 = add nsw i64 %i2.0132, %n
  br label %for.cond.7.preheader

for.cond.7.preheader:                             ; preds = %for.inc.27, %for.cond.4.preheader
  %i3.0131 = phi i64 [ 1, %for.cond.4.preheader ], [ %inc28, %for.inc.27 ]
  br label %for.cond.10.preheader

for.cond.10.preheader:                            ; preds = %for.inc.24, %for.cond.7.preheader
  %i4.0130 = phi i64 [ 1, %for.cond.7.preheader ], [ %inc25, %for.inc.24 ]
  br label %for.body.12

for.body.12:                                      ; preds = %for.body.12, %for.cond.10.preheader
  %i5.0129 = phi i64 [ 1, %for.cond.10.preheader ], [ %inc, %for.body.12 ]
  %arrayidx16 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x float]]]]], [100 x [100 x [100 x [100 x [100 x float]]]]]* @A, i64 0, i64 %i5.0129, i64 %i4.0130, i64 %i3.0131, i64 %add, i64 %i1.0134
  %0 = load float, float* %arrayidx16, align 4, !tbaa !1
  %add17 = fadd float %0, 1.000000e+00
  %arrayidx23 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x float]]]]], [100 x [100 x [100 x [100 x [100 x float]]]]]* @A, i64 0, i64 %i5.0129, i64 %i4.0130, i64 %i3.0131, i64 %add18, i64 %i1.0134
  store float %add17, float* %arrayidx23, align 4, !tbaa !1
  %inc = add nuw nsw i64 %i5.0129, 1
  %exitcond139 = icmp eq i64 %inc, 96
  br i1 %exitcond139, label %for.inc.24, label %for.body.12

for.inc.24:                                       ; preds = %for.body.12
  %inc25 = add nuw nsw i64 %i4.0130, 1
  %exitcond140 = icmp eq i64 %inc25, 97
  br i1 %exitcond140, label %for.inc.27, label %for.cond.10.preheader

for.inc.27:                                       ; preds = %for.inc.24
  %inc28 = add nuw nsw i64 %i3.0131, 1
  %exitcond141 = icmp eq i64 %inc28, 98
  br i1 %exitcond141, label %for.inc.30, label %for.cond.7.preheader

for.inc.30:                                       ; preds = %for.inc.27
  %inc31 = add nuw nsw i64 %i2.0132, 1
  %exitcond142 = icmp eq i64 %inc31, 99
  br i1 %exitcond142, label %for.inc.33, label %for.cond.4.preheader

for.inc.33:                                       ; preds = %for.inc.30
  %inc34 = add nuw nsw i64 %i1.0134, 1
  %exitcond143 = icmp eq i64 %i1.0134, %n
  br i1 %exitcond143, label %for.cond.36.preheader, label %for.cond.1.preheader

for.cond.39.preheader:                            ; preds = %for.inc.76, %for.cond.36.preheader
  %i2.1128 = phi i64 [ 1, %for.cond.36.preheader ], [ %inc77, %for.inc.76 ]
  br i1 %cmp40.126, label %for.inc.76, label %for.cond.42.preheader.lr.ph

for.cond.42.preheader.lr.ph:                      ; preds = %for.cond.39.preheader
  %add51 = add nsw i64 %i2.1128, %m
  %add58 = add nsw i64 %i2.1128, %n
  br label %for.cond.42.preheader

for.cond.42.preheader:                            ; preds = %for.inc.73, %for.cond.42.preheader.lr.ph
  %i1.1127 = phi i64 [ 1, %for.cond.42.preheader.lr.ph ], [ %inc74, %for.inc.73 ]
  br label %for.cond.45.preheader

for.cond.45.preheader:                            ; preds = %for.inc.70, %for.cond.42.preheader
  %i3.1125 = phi i64 [ 1, %for.cond.42.preheader ], [ %inc71, %for.inc.70 ]
  br label %for.cond.48.preheader

for.cond.48.preheader:                            ; preds = %for.inc.67, %for.cond.45.preheader
  %i4.1124 = phi i64 [ 1, %for.cond.45.preheader ], [ %inc68, %for.inc.67 ]
  br label %for.body.50

for.body.50:                                      ; preds = %for.body.50, %for.cond.48.preheader
  %i5.1123 = phi i64 [ 1, %for.cond.48.preheader ], [ %inc65, %for.body.50 ]
  %arrayidx56 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x float]]]]], [100 x [100 x [100 x [100 x [100 x float]]]]]* @A, i64 0, i64 %i5.1123, i64 %i4.1124, i64 %i3.1125, i64 %add51, i64 %i1.1127
  %1 = load float, float* %arrayidx56, align 4, !tbaa !1
  %add57 = fadd float %1, 1.000000e+00
  %arrayidx63 = getelementptr inbounds [100 x [100 x [100 x [100 x [100 x float]]]]], [100 x [100 x [100 x [100 x [100 x float]]]]]* @A, i64 0, i64 %i5.1123, i64 %i4.1124, i64 %i3.1125, i64 %add58, i64 %i1.1127
  store float %add57, float* %arrayidx63, align 4, !tbaa !1
  %inc65 = add nuw nsw i64 %i5.1123, 1
  %exitcond = icmp eq i64 %inc65, 96
  br i1 %exitcond, label %for.inc.67, label %for.body.50

for.inc.67:                                       ; preds = %for.body.50
  %inc68 = add nuw nsw i64 %i4.1124, 1
  %exitcond135 = icmp eq i64 %inc68, 97
  br i1 %exitcond135, label %for.inc.70, label %for.cond.48.preheader

for.inc.70:                                       ; preds = %for.inc.67
  %inc71 = add nuw nsw i64 %i3.1125, 1
  %exitcond136 = icmp eq i64 %inc71, 98
  br i1 %exitcond136, label %for.inc.73, label %for.cond.45.preheader

for.inc.73:                                       ; preds = %for.inc.70
  %inc74 = add nuw nsw i64 %i1.1127, 1
  %exitcond137 = icmp eq i64 %i1.1127, %n
  br i1 %exitcond137, label %for.inc.76, label %for.cond.42.preheader

for.inc.76:                                       ; preds = %for.inc.73, %for.cond.39.preheader
  %inc77 = add nuw nsw i64 %i2.1128, 1
  %exitcond138 = icmp eq i64 %inc77, 99
  br i1 %exitcond138, label %for.end.78, label %for.cond.39.preheader

for.end.78:                                       ; preds = %for.inc.76
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1250) (llvm/branches/loopopt 1313)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
