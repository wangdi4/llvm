; REQUIRES:asserts
; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-blocking,print<hir>" -hir-loop-blocking-skip-anti-pattern-check=false -hir-loop-blocking-no-delinear=false -debug-only=hir-loop-blocking -aa-pipeline="basic-aa" 2>&1 < %s -disable-output | FileCheck %s
; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-blocking,print<hir>" -hir-loop-blocking-skip-anti-pattern-check=false -hir-loop-blocking-no-delinear=true -debug-only=hir-loop-blocking -aa-pipeline="basic-aa" 2>&1 < %s -disable-output | FileCheck %s

; Check the blocking doesn't happen because it is trivially clear that blocking doesn't help.
; Notice that this is true whether deliearization is done or not (-hir-loop-blocking-no-delinear).

; - Innerloop is already unit-strided.
; - Outerloop's blocking will be just stripmining without blocking of the innerloop.
; Delinearization is disabled. With delinearization, num_dimensions >= num_depth hinders loop blocking.
; But with or without blocking, this pattern is not a good candidate for loop blocking.
;
; On the contrary, notice that this code is very close to the representative example in K&R book's chapter for loop blocking.
; The code there was described as shown below:
;
;   DO I = 1, M
;     DO J = 1, N
;       D(J) = D(J) + B(J, I)
;
; However, in practice, the code showed performance degradation (e.g. coremark-pro/linear_alg-mid-100x100-sp@opt_base_glm)

;         BEGIN REGION { }
;               + DO i1 = 0, %N + -1, 1   <DO_LOOP>
;               |   + DO i2 = 0, %N + -1, 1   <DO_LOOP>
;               |   |   %1 = (%B)[i2];
;               |   |   %mul5 = (%A)[%N * i1 + i2]  *  %1;
;               |   |   %add7 = %1  +  %mul5;
;               |   |   (%B)[i2] = %add7;
;               |   + END LOOP
;               + END LOOP
;         END REGION

; CHECK: Trivial anti-pattern

; CHECK: Function: matrix_mul_vect
; CHECK-NOT: DO i3
; CHECK-NOT: DO i4

; Without filtering as an anti-pattern, with disabling delinearization + enabling loop depth check, loop blocking currently considers it as profitable.
; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-blocking,print<hir>"  -hir-loop-blocking-skip-anti-pattern-check=true -hir-loop-blocking-no-delinear=true -disable-hir-loop-blocking-loop-depth-check=false -aa-pipeline="basic-aa" 2>&1 < %s -disable-output | FileCheck %s --check-prefix=CHECK-BLOCKED

; Without filtering as an anti-pattern, with enabling delinearization + disabling loop depth check, loop blocking currently considers it as profitable.
; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-blocking,print<hir>"  -hir-loop-blocking-skip-anti-pattern-check=true -hir-loop-blocking-no-delinear=false -disable-hir-loop-blocking-loop-depth-check=true -aa-pipeline="basic-aa" 2>&1 < %s -disable-output | FileCheck %s --check-prefix=CHECK-BLOCKED

; CHECK-BLOCKED: Function: matrix_mul_vect
; CHECK-BLOCKED:  BEGIN REGION { modified }
; CHECK-BLOCKED:        + DO i1 = 0, (%N + -1)/u64, 1   <DO_LOOP>
; CHECK-BLOCKED:        |   %min = (-64 * i1 + %N + -1 <= 63) ? -64 * i1 + %N + -1 : 63;
;                       |
; CHECK-BLOCKED:        |   + DO i2 = 0, (%N + -1)/u64, 1   <DO_LOOP>
; CHECK-BLOCKED:        |   |   %min2 = (-64 * i2 + %N + -1 <= 63) ? -64 * i2 + %N + -1 : 63;
;                       |   |
; CHECK-BLOCKED:        |   |   + DO i3 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK-BLOCKED:        |   |   |   + DO i4 = 0, %min2, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK-BLOCKED:        |   |   |   |   %1 = (%B)[64 * i2 + i4];
; CHECK-BLOCKED:        |   |   |   |   %mul5 = (%A)[64 * %N * i1 + 64 * i2 + %N * i3 + i4]  *  %1;
; CHECK-BLOCKED:        |   |   |   |   %add7 = %1  +  %mul5;
; CHECK-BLOCKED:        |   |   |   |   (%B)[64 * i2 + i4] = %add7;
; CHECK-BLOCKED:        |   |   |   + END LOOP
; CHECK-BLOCKED:        |   |   + END LOOP
; CHECK-BLOCKED:        |   + END LOOP
; CHECK-BLOCKED:        + END LOOP
; CHECK-BLOCKED:  END REGION

;Module Before HIR
; ModuleID = 'anti-pattern.c'
source_filename = "anti-pattern.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @matrix_mul_vect(i64 %N, ptr noalias nocapture readonly %A, ptr noalias nocapture %B) local_unnamed_addr #0 {
entry:
  %cmp24 = icmp sgt i64 %N, 0
  br i1 %cmp24, label %for.body3.lr.ph.preheader, label %for.end10

for.body3.lr.ph.preheader:                        ; preds = %entry
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.body3.lr.ph.preheader, %for.inc8
  %i.025 = phi i64 [ %inc9, %for.inc8 ], [ 0, %for.body3.lr.ph.preheader ]
  %mul = mul nsw i64 %i.025, %N
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.body3
  %j.023 = phi i64 [ 0, %for.body3.lr.ph ], [ %inc, %for.body3 ]
  %add = add nsw i64 %j.023, %mul
  %ptridx = getelementptr inbounds float, ptr %A, i64 %add
  %0 = load float, ptr %ptridx, align 4, !tbaa !2
  %ptridx4 = getelementptr inbounds float, ptr %B, i64 %j.023
  %1 = load float, ptr %ptridx4, align 4, !tbaa !2
  %mul5 = fmul float %0, %1
  %add7 = fadd float %1, %mul5
  store float %add7, ptr %ptridx4, align 4, !tbaa !2
  %inc = add nuw nsw i64 %j.023, 1
  %exitcond = icmp eq i64 %inc, %N
  br i1 %exitcond, label %for.inc8, label %for.body3

for.inc8:                                         ; preds = %for.body3
  %inc9 = add nuw nsw i64 %i.025, 1
  %exitcond27 = icmp eq i64 %inc9, %N
  br i1 %exitcond27, label %for.end10.loopexit, label %for.body3.lr.ph

for.end10.loopexit:                               ; preds = %for.inc8
  br label %for.end10

for.end10:                                        ; preds = %for.end10.loopexit, %entry
  ret void
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
