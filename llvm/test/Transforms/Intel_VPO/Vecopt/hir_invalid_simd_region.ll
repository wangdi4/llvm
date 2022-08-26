; Test that VPlanDriverHIR exits early from vectorization for an invalid HIR SIMD region.

; C code:
; void foo(int *a, int n) {
;     int i;
; #pragma omp simd
;     for (i = 0; i < 100; ++i) {
;         if (n!=20) {
;             a[i] = i;
;         }
;     }
; }

; Input HIR:
; NOTE: The test explicitly does not run HIR Vec Directive insertion pass to avoid auto-vectorization directives inside the
; HLIf node before the loop.

; <0>     BEGIN REGION { }
; <2>           %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null) ]
; <4>           if (%n != 20)
; <4>           {
; <25>             + DO i1 = 0, 99, 1   <DO_LOOP>
; <13>             |   (%a)[i1] = i1;
; <25>             + END LOOP
; <4>           }
; <23>          @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; <24>          ret ;
; <0>     END REGION


; NOTE: In debug builds we throw an assertion for invalid HIR SIMD region, hence this test is expected to run only in prod/release
; builds.
; REQUIRES: !asserts

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vplan-vec -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vplan-vec" -S < %s 2>&1 | FileCheck %s


; Check that loop was not vectorized
; CHECK-LABEL: omp.inner.for.body:
; CHECK-NOT: x i32>


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32* nocapture %a, i32 %n) local_unnamed_addr {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  %cmp1 = icmp eq i32 %n, 20
  br i1 %cmp1, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body.preheader

omp.inner.for.body.preheader:                     ; preds = %omp.inner.for.body.lr.ph
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body.preheader, %omp.inner.for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.inner.for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2.loopexit, label %omp.inner.for.body

DIR.OMP.END.SIMD.2.loopexit:                      ; preds = %omp.inner.for.body
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.2.loopexit, %omp.inner.for.body.lr.ph
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }


!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
