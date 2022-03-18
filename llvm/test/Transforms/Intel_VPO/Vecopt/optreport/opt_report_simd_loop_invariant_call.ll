; RUN: opt %s -S -vplan-vec -intel-ir-optreport-emitter -intel-opt-report=high -disable-output 2>&1 | FileCheck %s
;
; Cheks serialization reason for llvm.invariant.start/llvm.invariant.end call
;
; CHECK: remark #15558: Call to function 'llvm.invariant.start.p0i8' was serialized due to no suitable vector variants were found.
; CHECK-NEXT: remark #15558: Call to function 'llvm.invariant.end.p0i8' was serialized due to no suitable vector variants were found.
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooPi(i32* nocapture %arr) local_unnamed_addr #0 {
DIR.OMP.SIMD.113:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.113
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %1 = bitcast i32* %i.linear.iv to i8*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %2 = call {}* @llvm.invariant.start.p0i8(i64 4, i8* nonnull %1) #2
  %3 = trunc i64 %indvars.iv to i32
  %ptridx = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  store i32 %3, i32* %ptridx, align 4
  call void @llvm.invariant.end.p0i8({}* %2, i64 1, i8* nonnull %1) #2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body, !llvm.loop !7

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  ret void
}

declare {}* @llvm.invariant.start.p0i8(i64, i8* nocapture) nounwind readonly

declare void @llvm.invariant.end.p0i8({}*, i64, i8* nocapture) nounwind

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { nounwind uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!7 = distinct !{!7, !8, !9, !10}
!8 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!9 = !{!"llvm.loop.parallel_accesses"}
!10 = !{!"llvm.loop.intel.vector.vectorlength", i64 4}
