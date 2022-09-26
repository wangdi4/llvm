; Verify VPlan HIR codegen when memrefs grouped by VLS analysis are not all master VPInstructions. Check that the required shuffles are generated in vectorized code.
; Input LLVM-IR generated with command: icpx -fiopenmp -O2 -mllvm -print-module-before-loopopt

; C++ test code
; struct Point {
;     double Dims[3];
; };
;
; Point p[100];
;
; double gradient_list ()
; {
;     double ret, ret2 = 0.0;
; #pragma omp simd
;     for (int i=0;i<100;++i)
;     {
;         ret += p[i].Dims[0] + p[i].Dims[1];
;         ret2 = p[i].Dims[2];
;         if (ret2 > 1024.5) {
;             ret += ret2;
;         }
;     }
;
;     return ret * ret2;
; }

; Input HIR
; <28>    + DO i1 = 0, 99, 1   <DO_LOOP>
; <11>    |   %add6 = (@p)[0][i1].0[0]  +  (@p)[0][i1].0[1];
; <12>    |   %add7 = %ret.029  +  %add6;
; <14>    |   %3 = (@p)[0][i1].0[2];
; <16>    |   %add13 = %3  +  %add7;
; <17>    |   %ret.029 = (%3 > 1.024500e+03) ? %add13 : %add7;
; <28>    + END LOOP


; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -vplan-force-vf=2 -enable-vplan-vls-cg -hir-cg -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vplan-force-vf=2 -enable-vplan-vls-cg -S < %s 2>&1 | FileCheck %s


; TODO: Currently only a single shuffle is seen since codegen does not handle decomposed load VPInstructions. The first two implicit
; loads are vectorized into gathers. This test should be updated to check for a wide load and 3 shuffles when that feature is added (Jira : CMPLRLLVM-7542).
; With the temporary VLS analysis restriction, this loop is vectorized without any wide load or shuffle.

; XFAIL: *
; This test unexpectedly generates the right shuffles now because temp cleanup
; (which propagates loads to add instruction) is suppressed for SIMD loops.
; Before this change, we were looking for old style intel directives and did not
; consider this as a SIMD loop.
; Removing region entry/exit intrinsics also doesn't work because the loop is
; not legal to vectorize due to backward edge for %ret.029.

; CHECK: DO i1 = 0, 99, 2
; CHECK: [[Add:%.*]] = (<2 x double>*)(@p)[0][i1 + <i64 0, i64 1>].0[0]  +  (<2 x double>*)(@p)[0][i1 + <i64 0, i64 1>].0[1];
; CHECK-NOT: [[WLd:%.*]] = (<6 x double>*)(@p)[0][i1].0[0];
; CHECK-NOT: [[V3:%.*]] = shufflevector [[WLd]],  undef,  <i32 2, i32 5>;
; CHECK: END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Point = type { [3 x double] }

@p = dso_local local_unnamed_addr global [100 x %struct.Point] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local double @_Z13gradient_listv() local_unnamed_addr {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.inner.for.body.lr.ph ]
  %ret.029 = phi double [ %4, %omp.inner.for.body ], [ undef, %omp.inner.for.body.lr.ph ]
  %arrayidx1 = getelementptr inbounds [100 x %struct.Point], [100 x %struct.Point]* @p, i64 0, i64 %indvars.iv, i32 0, i64 0
  %1 = load double, double* %arrayidx1, align 8, !tbaa !2
  %arrayidx5 = getelementptr inbounds [100 x %struct.Point], [100 x %struct.Point]* @p, i64 0, i64 %indvars.iv, i32 0, i64 1
  %2 = load double, double* %arrayidx5, align 8, !tbaa !2
  %add6 = fadd double %1, %2
  %add7 = fadd double %ret.029, %add6
  %arrayidx11 = getelementptr inbounds [100 x %struct.Point], [100 x %struct.Point]* @p, i64 0, i64 %indvars.iv, i32 0, i64 2
  %3 = load double, double* %arrayidx11, align 8, !tbaa !2
  %cmp12 = fcmp ogt double %3, 1.024500e+03
  %add13 = fadd double %3, %add7
  %4 = select i1 %cmp12, double %add13, double %add7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  %.lcssa34 = phi double [ %3, %omp.inner.for.body ]
  %.lcssa = phi double [ %4, %omp.inner.for.body ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  %mul15 = fmul double %.lcssa34, %.lcssa
  ret double %mul15
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }

!2 = !{!3, !5, i64 0}
!3 = !{!"struct@_ZTS5Point", !4, i64 0}
!4 = !{!"array@_ZTSA3_d", !5, i64 0}
!5 = !{!"double", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
