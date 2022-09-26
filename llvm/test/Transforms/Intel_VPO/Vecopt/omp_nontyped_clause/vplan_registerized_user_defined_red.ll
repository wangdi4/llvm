; Test to verify VPlan Planner bails out for fully/partially registerized
; user-defined reductions.

; #pragma omp declare reduction(+: double: omp_out += omp_in)
; double test() {
;   const int N0 = 10 ;
;   double counter = 0.;
;   #pragma omp simd reduction(+: counter)
;   for (int i0 = 0 ; i0 < N0 ; i0++ )
;   {
;     counter = counter + 1.;
;   }
;   return counter;
; }

; XFAIL: *
; RUN: opt -disable-output -vplan-vec -debug-only=LoopVectorizationPlanner  < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -passes="vplan-vec" -debug-only=LoopVectorizationPlanner  < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -hir-ssa-deconstruction -hir-temp-cleanup -hir-vplan-vec -debug-only=LoopVectorizationPlanner < %s 2>&1 | FileCheck %s
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vplan-vec" -debug-only=LoopVectorizationPlanner < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; CHECK: LVP: Registerized UDR found.


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local double @test_user_defined_reduction() local_unnamed_addr #0 {
DIR.OMP.SIMD.0:
  %linear.iv.ptr = alloca i32, align 4
  %counter.red.ptr = alloca double, align 8
  store double 0.000000e+00, double* %counter.red.ptr, align 8
  br label %omp.region.entry

omp.region.entry:                                 ; preds = %DIR.OMP.SIMD.0
  %omp.token = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.UDR"(double* %counter.red.ptr, i8* null, i8* null, void (double*, double*)* @.omp_combiner., i8* null), "QUAL.OMP.LINEAR:IV"(i32* %linear.iv.ptr, i32 1) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.region.entry
  %counter.red.promoted = load double, double* %counter.red.ptr, align 8
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %counter = phi double [ %counter.red.promoted, %DIR.OMP.SIMD.1 ], [ %counter.next, %omp.inner.for.body ]
  %iv = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %iv.next, %omp.inner.for.body ]
  store i32 %iv, i32* %linear.iv.ptr, align 4

  %counter.next = fadd fast double %counter, 1.000000e+00

  %iv.next = add nuw nsw i32 %iv, 1
  %exit.cond = icmp eq i32 %iv.next, 10
  br i1 %exit.cond, label %omp..loopexit, label %omp.inner.for.body, !llvm.loop !0

omp..loopexit:                                    ; preds = %omp.inner.for.body
  %counter.lcssa = phi double [ %counter.next, %omp.inner.for.body ]
  store double %counter.lcssa, double* %counter.red.ptr, align 8
  br label %omp.region.exit

omp.region.exit:                                  ; preds = %omp..loopexit
  call void @llvm.directive.region.exit(token %omp.token) [ "DIR.OMP.END.SIMD"() ]
  br label %func.exit

func.exit:                                        ; preds = %omp.region.exit
  ret double %counter.lcssa
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: alwaysinline mustprogress nofree norecurse nosync nounwind uwtable willreturn
define internal void @.omp_combiner.(double* noalias nocapture %arg, double* noalias nocapture readonly %arg1) #2 {
entry:
  %i = load double, double* %arg1, align 8
  %i2 = load double, double* %arg, align 8
  %add = fadd fast double %i2, %i
  store double %add, double* %arg, align 8
  ret void
}

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { alwaysinline mustprogress nofree norecurse nosync nounwind uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!0 = distinct !{!0, !1, !2}
!1 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!2 = !{!"llvm.loop.parallel_accesses", !3}
!3 = distinct !{}
