; RUN: opt -mtriple=x86_64 -passes=vplan-vec -vplan-force-vf=8 -vplan-print-after-call-vec-decisions -vplan-enable-non-masked-vectorized-remainder=0 -disable-output< %s | FileCheck %s
; RUN: opt -mtriple=x86_64 -passes=vplan-vec -vplan-force-vf=8 -vplan-enable-non-masked-vectorized-remainder=0 -S < %s | FileCheck %s -check-prefix=VPLANCG
; RUN: opt -mtriple=x86_64 -passes='hir-ssa-deconstruction,hir-vplan-vec,hir-cg' -vplan-force-vf=8 -S < %s | FileCheck %s -check-prefix=HIRCG

@ARRAY_SIZE = external dso_local local_unnamed_addr constant i32, align 4

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

declare nofpclass(nan inf) double @Interpolate(double noundef nofpclass(nan inf), ptr noundef) local_unnamed_addr #1

; Function Attrs: noinline nounwind uwtable
define dso_local void @run_test.a(ptr noundef %vals, ptr nocapture noundef readonly %src, ptr nocapture noundef writeonly %dst) local_unnamed_addr #2 {
; CHECK-LABEL: VPlan after CallVecDecisions analysis for merged CFG:
; CHECK-NEXT:  VPlan IR for: run_test.a:omp.inner.for.body.#1
; AVX512 vector variant calls
; CHECK:     [DA: Div] double [[VP0:%.*]] = call double [[VP1:%.*]] ptr %vals _ZGVZN8vu_Interpolate [x 1]
; CHECK:     [DA: Div] double [[VP2:%.*]] = call double [[VP3:%.*]] ptr %vals _ZGVZM8vu_Interpolate [x 1]
;
; VPLANCG:  define dso_local void @run_test.a(ptr noundef [[VALS0:%.*]], ptr nocapture noundef readonly [[SRC0:%.*]], ptr nocapture noundef writeonly [[DST0:%.*]]) local_unnamed_addr #2 {
; VPLANCG:  [[TMP4:%.*]] = call fast x86_regcallcc nofpclass(nan inf) <8 x double> @_ZGVZN8vu_Interpolate(<8 x double> noundef nofpclass(nan inf) [[WIDE_LOAD0:%.*]], ptr noundef [[VALS0]]) #0
; VPLANCG:  [[TMP14:%.*]] = call fast x86_regcallcc nofpclass(nan inf) <8 x double> @_ZGVZM8vu_Interpolate(<8 x double> noundef nofpclass(nan inf) [[WIDE_MASKED_LOAD0:%.*]], ptr noundef [[VALS0]], i32 [[MASKCAST0:%.*]]) #0
; VPLANCG:  [[CALLRET0:%.*]] = call fast nofpclass(nan inf) double @Interpolate(double noundef nofpclass(nan inf) [[TMP22:%.*]], ptr noundef [[VALS0]]) #0
;
; HIRCG:    define dso_local void @run_test.a(ptr noundef [[VALS0:%.*]], ptr nocapture noundef readonly [[SRC0:%.*]], ptr nocapture noundef writeonly [[DST0:%.*]]) local_unnamed_addr #2 {
; HIRCG:    [[CALLRET0:%.*]] = call fast nofpclass(nan inf) double @Interpolate(double noundef nofpclass(nan inf) [[TMP2:%.*]], ptr noundef [[VALS0]]) #0
; HIRCG:    [[TMP25:%.*]] = call fast x86_regcallcc nofpclass(nan inf) <8 x double> @_ZGVZN8vu_Interpolate(<8 x double> noundef nofpclass(nan inf) [[T29_0:%.*]], ptr noundef [[VALS0]]) #0
; HIRCG:    [[TMP14:%.*]] = call fast x86_regcallcc nofpclass(nan inf) <8 x double> @_ZGVZM8vu_Interpolate(<8 x double> noundef nofpclass(nan inf) [[T43_0:%.*]], ptr noundef [[VALS0]], i32 [[T45_0:%.*]]) #0
;
entry:
  %i.linear.iv = alloca i32, align 4
  %0 = load i32, ptr @ARRAY_SIZE, align 4
  %cmp = icmp sgt i32 %0, 0
  br i1 %cmp, label %DIR.OMP.SIMD.1, label %omp.precond.end

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.117

DIR.OMP.SIMD.117:                                 ; preds = %DIR.OMP.SIMD.1
  %wide.trip.count = zext i32 %0 to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.117
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.117 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds double, ptr %src, i64 %indvars.iv
  %2 = load double, ptr %arrayidx, align 8
  %callret = call fast nofpclass(nan inf) double @Interpolate(double noundef nofpclass(nan inf) %2, ptr noundef %vals) #0
  %arrayidx6 = getelementptr inbounds double, ptr %dst, i64 %indvars.iv
  store double %callret, ptr %arrayidx6, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge: ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge, %entry
  ret void
}

attributes #0 = { nounwind }
attributes #1 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+ssse3,+x87" "unsafe-fp-math"="true" "vector-variants"="_ZGVyN4vu_Interpolate,_ZGVyM4vu_Interpolate,_ZGVYN4vu_Interpolate,_ZGVYM4vu_Interpolate,_ZGVZN8vu_Interpolate,_ZGVZM8vu_Interpolate,_ZGVxN2vu_Interpolate,_ZGVxM2vu_Interpolate" }
attributes #2 = { noinline nounwind uwtable "advanced-optim"="true" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "tune-cpu"="skylake-avx512" "prefer-vector-width"="512" "unsafe-fp-math"="true" }
