;    for (i=0; i < n; i++) 
;        for (j=0; j <= i ; j++) 
;            A[j][i] = A[j][i] + 1; 
;
; REQUIRES: asserts
; RUN: opt -hir-ssa-deconstruction -debug -hir-loop-interchange < %s 2>&1 | FileCheck %s
; CHECK-NOT:  Interchanged

; ModuleID = 'triangular.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [1000 x [1000 x float]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @sub3(i64 %n) #0 {
entry:
  %cmp20 = icmp sgt i64 %n, 0
  br i1 %cmp20, label %for.body3.preheader, label %for.end9

for.body3.preheader:                              ; preds = %entry, %for.inc7
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc7 ], [ 1, %entry ]
  %i.021 = phi i64 [ %inc8, %for.inc7 ], [ 0, %entry ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.preheader
  %j.019 = phi i64 [ %inc, %for.body3 ], [ 0, %for.body3.preheader ]
  %arrayidx4 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @A, i64 0, i64 %j.019, i64 %i.021
  %0 = load float, float* %arrayidx4, align 4, !tbaa !1
  %add = fadd float %0, 1.000000e+00
  store float %add, float* %arrayidx4, align 4, !tbaa !1
  %inc = add nuw nsw i64 %j.019, 1
  %exitcond = icmp eq i64 %inc, %indvars.iv
  br i1 %exitcond, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.body3
  %inc8 = add nuw nsw i64 %i.021, 1
  %indvars.iv.next = add nuw i64 %indvars.iv, 1
  %exitcond23 = icmp eq i64 %inc8, %n
  br i1 %exitcond23, label %for.end9, label %for.body3.preheader

for.end9:                                         ; preds = %for.inc7, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 6818) (llvm/branches/loopopt 9722)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
