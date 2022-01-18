; Test to verify that VPlan vectorizer bails out for AND reduction
; REQUIRES: asserts
; RUN: opt -vplan-vec -vplan-force-vf=2 -S -debug-only=vplan-vec -debug-only=vpo-ir-loop-vectorize-legality -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="vplan-vec" -vplan-force-vf=2 -S -debug-only=vplan-vec -debug-only=vpo-ir-loop-vectorize-legality -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=2 -debug-only=HIRLegality -debug-only=vplan-vec -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIR
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -debug-only=HIRLegality -debug-only=vplan-vec -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIR

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


; CHECK: VPlan LLVM-IR Driver for Function: foo
; CHECK: A reduction of this operation is not supported
; CHECK: VD: Not vectorizing: Cannot prove legality.

; HIR: VPlan HIR Driver for Function: foo
; HIR: A reduction of this operation is not supported
; HIR: VD: Not vectorizing: Cannot prove legality.
; HIR: Function: foo

; Function Attrs: nounwind uwtable
define dso_local void @foo(float** nocapture readonly %_C) local_unnamed_addr #0 {
DIR.OMP.SIMD.0:
  %i.linear.iv.ptr = alloca i32, align 4
  %r.red.ptr = alloca i8, align 4
  store i8 1, i8* %r.red.ptr, align 1
  br label %omp.simd.region.entry

omp.simd.region.entry:                            ; preds = %DIR.OMP.SIMD.0
  %i = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.AND"(i8* %r.red.ptr), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv.ptr, i32 1) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.simd.region.entry
  %c.ptr = load float*, float** %_C, align 8
  %i1 = bitcast i32* %i.linear.iv.ptr to i8*
  %r.red.promoted = load i8, i8* %r.red.ptr, align 1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %iv = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %iv.next, %omp.inner.for.body ]
  %red.local = phi i8 [ %r.red.promoted, %DIR.OMP.SIMD.1 ], [ %and, %omp.inner.for.body ]
  store i32 %iv, i32* %i.linear.iv.ptr, align 4
  %arrayidx = getelementptr inbounds float, float* %c.ptr, i32 %iv
  %fval = load float, float* %arrayidx, align 4
  %fcmp = fcmp fast oeq float %fval, 1.000000e+00
  %conv = zext i1 %fcmp to i8
  %and = and i8 %red.local, %conv
  %iv.next = add nuw nsw i32 %iv, 1
  %exitcond = icmp eq i32 %iv.next, 10
  br i1 %exitcond, label %omp.simd.loop.exit, label %omp.inner.for.body, !llvm.loop !0

omp.simd.loop.exit:                               ; preds = %omp.inner.for.body
  %and.lcssa = phi i8 [ %and, %omp.inner.for.body ]
  store i8 %and.lcssa, i8* %r.red.ptr, align 1
  br label %omp.simd.region.exit

omp.simd.region.exit:                             ; preds = %omp.simd.loop.exit
  call void @llvm.directive.region.exit(token %i) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD

DIR.OMP.END.SIMD:                                 ; preds = %omp.simd.region.exit
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!0 = distinct !{!0, !1, !2}
!1 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!2 = !{!"llvm.loop.parallel_accesses", !3}
!3 = distinct !{}
