
; This IR is generated from the following C code:
; void foo(float* input, float* b, float* a) {
; #pragma omp simd
;   for (int i = 0; i < N; i++) {
;     if (b[i] > 3)
;       a[i] = sinf(input[i]);
;   }
; }

; RUN: opt -vector-library=SVML -passes=vplan-vec -S -vplan-force-vf=4 %s | FileCheck -DVL=4 --check-prefixes=CHECK,FLOAT-LT-512,DOUBLE-LT-512 %s
; RUN: opt -vector-library=SVML -passes=vplan-vec -S -vplan-force-vf=8 %s | FileCheck -DVL=8 --check-prefixes=CHECK,FLOAT-LT-512,DOUBLE-512 %s
; RUN: opt -vector-library=SVML -passes=vplan-vec -S -vplan-force-vf=16 %s | FileCheck -DVL=16 --check-prefixes=CHECK,FLOAT-512,DOUBLE-512 %s
; RUN: opt -vector-library=SVML -passes=vplan-vec -S -vplan-force-vf=32 %s | FileCheck -DVL=32 --check-prefixes=CHECK,FLOAT-512,DOUBLE-512 %s

; CHECK-LABEL: test_sinf
; FLOAT-LT-512:   [[MASK_EXT:%.*]] = sext <[[VL]] x i1> [[MASK:%.*]] to <[[VL]] x i32>
; FLOAT-LT-512:   [[RESULT:%.*]] = call afn svml_cc <[[VL]] x float> @__svml_sinf[[VL]]_mask(<[[VL]] x float> {{.*}}, <[[VL]] x i32> [[MASK_EXT:%.*]])
; FLOAT-512:      [[RESULT:%.*]] = call afn svml_cc <[[VL]] x float> @__svml_sinf[[VL]]_mask(<[[VL]] x float> undef, <[[VL]] x i1> [[MASK:%.*]], <[[VL]] x float> {{.*}})
; CHECK:          call void @llvm.masked.store.v[[VL]]f32.p0(<[[VL]] x float> [[RESULT]], ptr {{.*}}, i32 4, <[[VL]] x i1> [[MASK]])

; CHECK-LABEL: test_sin
; DOUBLE-LT-512:  [[MASK_EXT:%.*]] = sext <[[VL]] x i1> [[MASK:%.*]] to <[[VL]] x i64>
; DOUBLE-LT-512:  [[RESULT:%.*]] = call afn svml_cc <[[VL]] x double> @__svml_sin[[VL]]_mask(<[[VL]] x double> {{.*}}, <[[VL]] x i64> [[MASK_EXT:%.*]])
; DOUBLE-512:     [[RESULT:%.*]] = call afn svml_cc <[[VL]] x double> @__svml_sin[[VL]]_mask(<[[VL]] x double> undef, <[[VL]] x i1> [[MASK:%.*]], <[[VL]] x double> {{.*}})
; CHECK:          call void @llvm.masked.store.v[[VL]]f64.p0(<[[VL]] x double> [[RESULT]], ptr {{.*}}, i32 8, <[[VL]] x i1> [[MASK]])

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@N = common dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: nounwind uwtable
define dso_local void @test_sinf(ptr nocapture readonly %input, ptr nocapture readonly %b, ptr %a) local_unnamed_addr #0 {
entry:
  %0 = load i32, ptr @N, align 4, !tbaa !2
  %cmp = icmp sgt i32 %0, 0
  br i1 %cmp, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %wide.trip.count = sext i32 %0 to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.body.continue, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.body.continue ]
  %arrayidx = getelementptr inbounds float, ptr %b, i64 %indvars.iv
  %2 = load float, ptr %arrayidx, align 4, !tbaa !6
  %cmp6 = fcmp ogt float %2, 3.000000e+00
  br i1 %cmp6, label %if.then, label %omp.body.continue

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx8 = getelementptr inbounds float, ptr %input, i64 %indvars.iv
  %3 = load float, ptr %arrayidx8, align 4, !tbaa !6
  %arrayidx10 = getelementptr inbounds float, ptr %a, i64 %indvars.iv
  %call = tail call afn float @sinf(float %3) #2
  store float %call, ptr %arrayidx10, align 4, !tbaa !6
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

DIR.OMP.END.SIMD.3:                               ; preds = %omp.body.continue
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @test_sin(ptr nocapture readonly %input, ptr nocapture readonly %b, ptr %a) local_unnamed_addr #0 {
entry:
  %0 = load i32, ptr @N, align 4, !tbaa !2
  %cmp = icmp sgt i32 %0, 0
  br i1 %cmp, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %wide.trip.count = sext i32 %0 to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.body.continue, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.body.continue ]
  %arrayidx = getelementptr inbounds double, ptr %b, i64 %indvars.iv
  %2 = load double, ptr %arrayidx, align 8
  %cmp6 = fcmp ogt double %2, 3.000000e+00
  br i1 %cmp6, label %if.then, label %omp.body.continue

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx8 = getelementptr inbounds double, ptr %input, i64 %indvars.iv
  %3 = load double, ptr %arrayidx8, align 8
  %arrayidx10 = getelementptr inbounds double, ptr %a, i64 %indvars.iv
  %call = tail call afn double @sin(double %3) #2
  store double %call, ptr %arrayidx10, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %DIR.OMP.END.SIMD.3, label %omp.inner.for.body

DIR.OMP.END.SIMD.3:                               ; preds = %omp.body.continue
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret void
}

; Function Attrs: nofree nounwind
declare dso_local float @sinf(float) local_unnamed_addr #1

declare dso_local double @sin(double) local_unnamed_addr #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

attributes #0 = { nofree noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "prefer-vector-width"="256" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nofree nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "prefer-vector-width"="256" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"DPC++ Compiler 2021.1 (YYYY.8.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"float", !4, i64 0}
