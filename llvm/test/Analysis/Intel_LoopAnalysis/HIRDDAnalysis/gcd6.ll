;   gcd test: walking all iv & blob coeffs
; 			A[8 *n * i1 + 2 * i2 + 2 *m +3] =
;   		A[8 *n * i1 + 2 * i2 + 2 *m +2 + 2*n] + 2;
; RUN: opt < %s -hir-ssa-deconstruction | opt -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 < %s | FileCheck %s

; CHECK-NOT: ANTI
; ModuleID = 'dd10.c'
source_filename = "dd10.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(float* nocapture %A, i32 %n, i32 %m, i32 %mm) local_unnamed_addr #0 {
entry:
  %cmp40 = icmp sgt i32 %mm, 0
  br i1 %cmp40, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %mul = shl i32 %n, 3
  %mul7 = shl nsw i32 %m, 1
  %mul10 = shl i32 %n, 1
  %add9 = add i32 %mul10, 2
  %wide.trip.count = zext i32 %mm to i64
  br label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup3
  %i1.041 = phi i32 [ 0, %for.cond1.preheader.lr.ph ], [ %inc23, %for.cond.cleanup3 ]
  %mul5 = mul nsw i32 %mul, %i1.041
  %add = add i32 %mul5, %mul7
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %inc23 = add nuw nsw i32 %i1.041, 1
  %exitcond43 = icmp eq i32 %inc23, %mm
  br i1 %exitcond43, label %for.cond.cleanup.loopexit, label %for.body4.lr.ph

for.body4:                                        ; preds = %for.body4, %for.body4.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.body4 ]
  %indvars.iv.tr = trunc i64 %indvars.iv to i32
  %0 = shl i32 %indvars.iv.tr, 1
  %add8 = add i32 %add, %0
  %add11 = add i32 %add9, %add8
  %idxprom = sext i32 %add11 to i64
  %arrayidx = getelementptr inbounds float, float* %A, i64 %idxprom
  %1 = load float, float* %arrayidx, align 4, !tbaa !1
  %add12 = fadd float %1, 2.000000e+00
  %add19 = add nsw i32 %add8, 3
  %idxprom20 = sext i32 %add19 to i64
  %arrayidx21 = getelementptr inbounds float, float* %A, i64 %idxprom20
  store float %add12, float* %arrayidx21, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20789) (llvm/branches/loopopt 20847)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
