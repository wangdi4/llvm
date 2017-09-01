
;	for ( l=1 ; l<=loop ; l++ ) {
;		for ( k=0 ; k<25 ; k++ ) {
;			for ( i=0 ; i<25 ; i++ ) {
;				for ( j=0 ; j<n ; j++ ) {
;					px[j*25+i] = px[j*25+i -n +10] + vy[k*n+i] * cx[j*25+k+l];
; Cannot be interchanged even with RTDD: has flow dep (* * * *) for px
; REQUIRES: asserts
; RUN: opt -O2 -loopopt -debug  -hir-loop-interchange  < %s 2>&1 | FileCheck %s
; CHECK-NOT: Interchanged:
; ModuleID = 'matmul9.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @matmul(float* nocapture %px, float* nocapture readonly %vy, float* nocapture readonly %cx, i32 %loop, i32 %n) #0 {
entry:
  %cmp60 = icmp slt i32 %loop, 1
  br i1 %cmp60, label %for.end34, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp855 = icmp sgt i32 %n, 0
  %sub = sub i32 10, %n
  %0 = sext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc32, %for.cond1.preheader.lr.ph
  %l.061 = phi i32 [ 1, %for.cond1.preheader.lr.ph ], [ %inc33, %for.inc32 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc29, %for.cond1.preheader
  %indvars.iv68 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next69, %for.inc29 ]
  %1 = mul nsw i64 %indvars.iv68, %0
  %2 = trunc i64 %indvars.iv68 to i32
  %add16 = add nuw i32 %2, %l.061
  br label %for.cond7.preheader

for.cond7.preheader:                              ; preds = %for.inc26, %for.cond4.preheader
  %indvars.iv64 = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next65, %for.inc26 ]
  br i1 %cmp855, label %for.body9.lr.ph, label %for.inc26

for.body9.lr.ph:                                  ; preds = %for.cond7.preheader
  %3 = add nsw i64 %indvars.iv64, %1
  %arrayidx14 = getelementptr inbounds float, float* %vy, i64 %3
  br label %for.body9

for.body9:                                        ; preds = %for.body9, %for.body9.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body9.lr.ph ], [ %indvars.iv.next, %for.body9 ]
  %4 = mul nuw nsw i64 %indvars.iv, 25
  %5 = add nuw nsw i64 %4, %indvars.iv64
  %6 = trunc i64 %5 to i32
  %add10 = add i32 %sub, %6
  %idxprom = sext i32 %add10 to i64
  %arrayidx = getelementptr inbounds float, float* %px, i64 %idxprom
  %7 = load float, float* %arrayidx, align 4, !tbaa !1
  %8 = load float, float* %arrayidx14, align 4, !tbaa !1
  %9 = trunc i64 %4 to i32
  %add17 = add i32 %add16, %9
  %idxprom18 = sext i32 %add17 to i64
  %arrayidx19 = getelementptr inbounds float, float* %cx, i64 %idxprom18
  %10 = load float, float* %arrayidx19, align 4, !tbaa !1
  %mul20 = fmul float %8, %10
  %add21 = fadd float %7, %mul20
  %arrayidx25 = getelementptr inbounds float, float* %px, i64 %5
  store float %add21, float* %arrayidx25, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.inc26, label %for.body9

for.inc26:                                        ; preds = %for.body9, %for.cond7.preheader
  %indvars.iv.next65 = add nuw nsw i64 %indvars.iv64, 1
  %exitcond67 = icmp eq i64 %indvars.iv.next65, 25
  br i1 %exitcond67, label %for.inc29, label %for.cond7.preheader

for.inc29:                                        ; preds = %for.inc26
  %indvars.iv.next69 = add nuw nsw i64 %indvars.iv68, 1
  %exitcond71 = icmp eq i64 %indvars.iv.next69, 25
  br i1 %exitcond71, label %for.inc32, label %for.cond4.preheader

for.inc32:                                        ; preds = %for.inc29
  %inc33 = add nuw nsw i32 %l.061, 1
  %exitcond72 = icmp eq i32 %l.061, %loop
  br i1 %exitcond72, label %for.end34, label %for.cond1.preheader

for.end34:                                        ; preds = %for.inc32, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 3770) (llvm/branches/loopopt 6798)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
