
; Test for completely unrolling only triangluar loop with preheader and postexit.

; RUN: opt -hir-ssa-deconstruction -hir-complete-unroll -print-after=hir-complete-unroll 2>&1 < %s | FileCheck %s

; CHECK: BEGIN REGION { modified }
; CHECK: %p.promoted = {al:4}(%p)[0]
; CHECK: %conv419 = %p.promoted
; CHECK: %conv419 = %conv419  +  1.000000e+00
; CHECK: {al:4}(%A)[1] = 1
; CHECK: {al:4}(%p)[0] = %conv419
; CHECK: %conv419 = %p.promoted
; CHECK: %conv419 = %conv419  +  1.000000e+00
; CHECK: %conv419 = %conv419  +  1.000000e+00
; CHECK: %conv419 = %conv419  +  1.000000e+00
; CHECK: END REGION

; Source Code
; void func(float *p, int *A, int n) {
;  int i,j;
;  for (i=0 ; i< 4; i++) {
;    for (j=0 ; j< i; j++) {
;      A[i] = 1.0;
;      *p += 1.0;
;    }
;  }
;}



; ModuleID = 'triprepost.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @_Z4funcPfPii(float* nocapture %p, i32* nocapture %A, i32 %n) #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc5, %entry
  %indvars.iv20 = phi i64 [ 0, %entry ], [ %indvars.iv.next21, %for.inc5 ]
  %indvars.iv = phi i32 [ 0, %entry ], [ %indvars.iv.next, %for.inc5 ]
  %cmp216 = icmp sgt i64 %indvars.iv20, 0
  br i1 %cmp216, label %for.body3.lr.ph, label %for.inc5

for.body3.lr.ph:                                  ; preds = %for.cond1.preheader
  %p.promoted = load float, float* %p, align 4, !tbaa !1
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.lr.ph
  %conv419 = phi float [ %p.promoted, %for.body3.lr.ph ], [ %conv4, %for.body3 ]
  %j.017 = phi i32 [ 0, %for.body3.lr.ph ], [ %inc, %for.body3 ]
  %conv4 = fadd float %conv419, 1.000000e+00
  %inc = add nuw nsw i32 %j.017, 1
  %exitcond = icmp eq i32 %inc, %indvars.iv
  br i1 %exitcond, label %for.cond1.for.inc5_crit_edge, label %for.body3

for.cond1.for.inc5_crit_edge:                     ; preds = %for.body3
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv20
  store i32 1, i32* %arrayidx, align 4, !tbaa !5
  store float %conv4, float* %p, align 4, !tbaa !1
  br label %for.inc5

for.inc5:                                         ; preds = %for.cond1.for.inc5_crit_edge, %for.cond1.preheader
  %indvars.iv.next21 = add nuw nsw i64 %indvars.iv20, 1
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond22 = icmp eq i64 %indvars.iv.next21, 4
  br i1 %exitcond22, label %for.end7, label %for.cond1.preheader

for.end7:                                         ; preds = %for.inc5
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 3770) (llvm/branches/loopopt 6801)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
