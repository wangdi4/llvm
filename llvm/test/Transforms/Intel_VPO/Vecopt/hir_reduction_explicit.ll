; RUN: opt -vplan-force-vf=8 -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -print-after=hir-vplan-vec %s 2>&1 2>&1 -disable-output | FileCheck %s
; RUN: opt -opaque-pointers -vplan-force-vf=8 -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -print-after=hir-vplan-vec %s 2>&1 2>&1 -disable-output | FileCheck %s


; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT:  %sum.red.promoted = (%sum.red)[0];
; CHECK-NEXT:  %red.local = %sum.red.promoted;
; CHECK:       %red.init = 0.000000e+00;
; CHECK:       %phi.temp = %red.init
; CHECK:       DO i1 = 0, 1023, 8   <DO_LOOP> <simd-vectorized> <novectorize> <ivdep>
; CHECK-NEXT:   %.vec = (<8 x float>*)(@arr)[0][i1];
; CHECK-NEXT:   %.vec2 = %phi.temp  +  %.vec;
; CHECK-NEXT:   %phi.temp = %.vec2
; CHECK-NEXT:  END LOOP
; CHECK:       %red.local = @llvm.vector.reduce.fadd.v8f32(%red.local,  %.vec2);
; CHECK:      END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local float @foo() local_unnamed_addr #0 {
DIR.OMP.SIMD.122:
  %sum.red = alloca float, align 4
  %index.linear.iv = alloca i32, align 4
  store float 0.000000e+00, float* %sum.red, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.122
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(float* %sum.red, float 0.000000e+00, i32 1), "QUAL.OMP.LINEAR:IV.TYPED"(i32* %index.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %sum.red.promoted = load float, float* %sum.red, align 4, !tbaa !2
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %red.local = phi float [ %sum.red.promoted, %DIR.OMP.SIMD.2 ], [ %add1, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @arr, i64 0, i64 %indvars.iv, !intel-tbaa !6
  %fval = load float, float* %arrayidx, align 4, !tbaa !6, !llvm.access.group !8
  %add1 = fadd fast float %red.local, %fval
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.223, label %omp.inner.for.body, !llvm.loop !9

DIR.OMP.END.SIMD.223:                             ; preds = %omp.inner.for.body
  %add1.lcssa = phi float [ %add1, %omp.inner.for.body ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.223
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  %ret = fadd float %add1.lcssa, 0.000000e+00
  ret float %ret
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (2021.2.0.YYYYMMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA1024_f", !3, i64 0}
!8 = distinct !{}
!9 = distinct !{!9, !10, !11}
!10 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!11 = !{!"llvm.loop.parallel_accesses", !8}
