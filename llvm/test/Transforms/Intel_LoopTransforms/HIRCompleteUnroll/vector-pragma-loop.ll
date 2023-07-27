; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s --check-prefix=PREVEC
; RUN: opt -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll,print<hir>" -disable-output 2>&1 < %s | FileCheck %s --check-prefix=POSTVEC

; Verify that the vector pragma disables pre-vec complete unroll but not post-vec complete unroll.

; Src-
; double foo(double *x, double *y)
; {
;   #pragma unroll(2)
;   for (int i = 0; i < 4; ++i) {
;     x[i] = y[i] + 1.0;
;   }
;   return x[0];
; }


; PREVEC: Skipping complete unroll due to presence of vector pragma!

; POSTVEC-NOT: DO i1


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local double @foo(ptr nocapture %x, ptr nocapture readonly %y) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %0 = load double, ptr %x, align 8, !tbaa !2
  ret double %0

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds double, ptr %y, i64 %indvars.iv
  %1 = load double, ptr %arrayidx, align 8, !tbaa !2
  %add = fadd double %1, 1.000000e+00
  %arrayidx2 = getelementptr inbounds double, ptr %x, i64 %indvars.iv
  store double %add, ptr %arrayidx2, align 8, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %for.cond.cleanup, label %for.body, !llvm.loop !6
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c95d67b22c5ea6ea67afdc54154ea9648f91208c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 327c30da279626b3187f2e7799fb42b59492c966)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.vectorize.enable", i32 1}
