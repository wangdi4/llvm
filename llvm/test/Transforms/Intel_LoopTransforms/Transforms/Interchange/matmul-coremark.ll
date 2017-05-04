;	for ( l=1 ; l<=loop ; l++ ) 
;		for ( k=0 ; k<25 ; k++ ) 
;			for ( i=0 ; i<25 ; i++ ) 
;				for ( j=0 ; j<n ; j++ ) 
;					px[j*25+i] = px[j*25+i] + vy[k*n+i+j] * cx[j*25+k+l];
;
; REQUIRES: asserts 
; RUN: opt -O2  -loopopt -disable-hir-complete-unroll -debug -hir-loop-interchange  < %s 2>&1 | FileCheck %s
; CHECK: Interchanged:
; CHECK-SAME:  ( 4 1 2 3 )
; TODO: Ztt recognition fails due to change in induction variable simplification phase. This needs to be looked into.
; XFAIL:*

; ModuleID = 'matmul-coremark.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@px = common global [1000 x float] zeroinitializer, align 16
@vy = common global [1000 x float] zeroinitializer, align 16
@cx = common global [1000 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @matmul(i32 %loop, i32 %n) #0 {
entry:
  %cmp59 = icmp slt i32 %loop, 1
  br i1 %cmp59, label %for.end34, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %cmp854 = icmp sgt i32 %n, 0
  %0 = sext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc32, %for.cond1.preheader.lr.ph
  %l.060 = phi i32 [ 1, %for.cond1.preheader.lr.ph ], [ %inc33, %for.inc32 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc29, %for.cond1.preheader
  %indvars.iv68 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next69, %for.inc29 ]
  %1 = mul nsw i64 %indvars.iv68, %0
  %2 = trunc i64 %indvars.iv68 to i32
  %add16 = add nuw i32 %2, %l.060
  br label %for.cond7.preheader

for.cond7.preheader:                              ; preds = %for.inc26, %for.cond4.preheader
  %indvars.iv64 = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next65, %for.inc26 ]
  br i1 %cmp854, label %for.body9.lr.ph, label %for.inc26

for.body9.lr.ph:                                  ; preds = %for.cond7.preheader
  %3 = add nsw i64 %indvars.iv64, %1
  br label %for.body9

for.body9:                                        ; preds = %for.body9, %for.body9.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body9.lr.ph ], [ %indvars.iv.next, %for.body9 ]
  %4 = mul nuw nsw i64 %indvars.iv, 25
  %5 = add nuw nsw i64 %4, %indvars.iv64
  %arrayidx = getelementptr inbounds [1000 x float], [1000 x float]* @px, i64 0, i64 %5
  %6 = load float, float* %arrayidx, align 4, !tbaa !1
  %7 = add nsw i64 %3, %indvars.iv
  %arrayidx14 = getelementptr inbounds [1000 x float], [1000 x float]* @vy, i64 0, i64 %7
  %8 = load float, float* %arrayidx14, align 4, !tbaa !1
  %9 = trunc i64 %4 to i32
  %add17 = add i32 %add16, %9
  %idxprom18 = sext i32 %add17 to i64
  %arrayidx19 = getelementptr inbounds [1000 x float], [1000 x float]* @cx, i64 0, i64 %idxprom18
  %10 = load float, float* %arrayidx19, align 4, !tbaa !1
  %mul20 = fmul float %8, %10
  %add21 = fadd float %6, %mul20
  store float %add21, float* %arrayidx, align 4, !tbaa !1
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
  %inc33 = add nuw nsw i32 %l.060, 1
  %exitcond72 = icmp eq i32 %l.060, %loop
  br i1 %exitcond72, label %for.end34, label %for.cond1.preheader

for.end34:                                        ; preds = %for.inc32, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 6818) (llvm/branches/loopopt 8655)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
