; Test to check vectorization of sincos functions is working with masks in LLVM-IR vector CG.

; void unit_strided(float* input, float* b, float* vsin, float* vcos) {
; #pragma omp simd
;   for (int i = 0; i < N; i++) {
;     if (b[i] > 3)
;       sincosf(input[i], &vsin[i], &vcos[i]);
;   }
; }

; RUN: opt -vector-library=SVML -VPlanDriver -verify -S -vplan-force-vf=4 %s | FileCheck -DVL=4 --check-prefixes=CHECK,CHECK-128 %s
; RUN: opt -vector-library=SVML -VPlanDriver -verify -S -vplan-force-vf=16 %s | FileCheck -DVL=16 --check-prefixes=CHECK,CHECK-512 %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@N = common dso_local local_unnamed_addr global i32 0, align 4

; CHECK-LABEL: @unit_strided(
; CHECK-128: [[RESULT:%.*]] = call svml_cc { <[[VL]] x float>, <[[VL]] x float> } @__svml_sincosf4_mask(<[[VL]] x float> {{.*}}, <[[VL]] x i32> {{.*}})
; CHECK-512: [[RESULT:%.*]] = call svml_cc { <[[VL]] x float>, <[[VL]] x float> } @__svml_sincosf16_mask({ <[[VL]] x float>, <[[VL]] x float> } undef, <[[VL]] x i1> {{.*}}, <[[VL]] x float> {{.*}})
; CHECK: [[RESULT_SIN:%.*]] = extractvalue { <[[VL]] x float>, <[[VL]] x float> } [[RESULT]], 0
; CHECK: [[RESULT_COS:%.*]] = extractvalue { <[[VL]] x float>, <[[VL]] x float> } [[RESULT]], 1
; CHECK: call void @llvm.masked.store.v[[VL]]f32.p0v[[VL]]f32(<[[VL]] x float> [[RESULT_SIN]], <[[VL]] x float>* {{.*}}, i32 4, <[[VL]] x i1> {{.*}})
; CHECK: call void @llvm.masked.store.v[[VL]]f32.p0v[[VL]]f32(<[[VL]] x float> [[RESULT_COS]], <[[VL]] x float>* {{.*}}, i32 4, <[[VL]] x i1> {{.*}})
define dso_local void @unit_strided(float* nocapture readonly %input, float* nocapture readonly %b, float* %vsin, float* %vcos) local_unnamed_addr #0 {
entry:
  %0 = load i32, i32* @N, align 4, !tbaa !2
  %cmp = icmp sgt i32 %0, 0
  br i1 %cmp, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %wide.trip.count = sext i32 %0 to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.body.continue, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.body.continue ]
  %arrayidx = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %2 = load float, float* %arrayidx, align 4, !tbaa !6
  %cmp6 = fcmp ogt float %2, 3.000000e+00
  br i1 %cmp6, label %if.then, label %omp.body.continue

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx8 = getelementptr inbounds float, float* %input, i64 %indvars.iv
  %3 = load float, float* %arrayidx8, align 4, !tbaa !6
  %arrayidx10 = getelementptr inbounds float, float* %vsin, i64 %indvars.iv
  %arrayidx12 = getelementptr inbounds float, float* %vcos, i64 %indvars.iv
  tail call void @sincosf(float %3, float* nonnull %arrayidx10, float* nonnull %arrayidx12) #2
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

; void non_unit_strided(float* input, float* b, float* vsin, float* vcos) {
; #pragma omp simd
;   for (int i = 0; i < N; i++) {
;     if (b[i] > 3)
;       sincosf(input[i], &vsin[i * 2], &vcos[i * 3]);
;   }
; }

; CHECK-LABEL: @non_unit_strided(
; CHECK-128: [[RESULT:%.*]] = call svml_cc { <[[VL]] x float>, <[[VL]] x float> } @__svml_sincosf4_mask(<[[VL]] x float> {{.*}}, <[[VL]] x i32> {{.*}})
; CHECK-512: [[RESULT:%.*]] = call svml_cc { <[[VL]] x float>, <[[VL]] x float> } @__svml_sincosf16_mask({ <[[VL]] x float>, <[[VL]] x float> } undef, <[[VL]] x i1> {{.*}}, <[[VL]] x float> {{.*}})
; CHECK: [[RESULT_SIN:%.*]] = extractvalue { <[[VL]] x float>, <[[VL]] x float> } [[RESULT]], 0
; CHECK: [[RESULT_COS:%.*]] = extractvalue { <[[VL]] x float>, <[[VL]] x float> } [[RESULT]], 1
; CHECK: call void @llvm.masked.scatter.v[[VL]]f32.v[[VL]]p0f32(<[[VL]] x float> [[RESULT_SIN]], <[[VL]] x float*> {{.*}}, i32 4, <[[VL]] x i1> {{.*}})
; CHECK: call void @llvm.masked.scatter.v[[VL]]f32.v[[VL]]p0f32(<[[VL]] x float> [[RESULT_COS]], <[[VL]] x float*> {{.*}}, i32 4, <[[VL]] x i1> {{.*}})
define dso_local void @non_unit_strided(float* nocapture readonly %input, float* nocapture readonly %b, float* %vsin, float* %vcos) local_unnamed_addr #0 {
entry:
  %0 = load i32, i32* @N, align 4, !tbaa !2
  %cmp = icmp sgt i32 %0, 0
  br i1 %cmp, label %DIR.OMP.SIMD.2, label %omp.precond.end

DIR.OMP.SIMD.2:                                   ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.2
  %wide.trip.count = sext i32 %0 to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.body.continue, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.body.continue ]
  %arrayidx = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %2 = load float, float* %arrayidx, align 4, !tbaa !6
  %cmp6 = fcmp fast ogt float %2, 3.000000e+00
  br i1 %cmp6, label %if.then, label %omp.body.continue

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx8 = getelementptr inbounds float, float* %input, i64 %indvars.iv
  %3 = load float, float* %arrayidx8, align 4, !tbaa !6
  %4 = shl nuw nsw i64 %indvars.iv, 1
  %arrayidx11 = getelementptr inbounds float, float* %vsin, i64 %4
  %5 = mul nuw nsw i64 %indvars.iv, 3
  %arrayidx14 = getelementptr inbounds float, float* %vcos, i64 %5
  tail call void @sincosf(float %3, float* %arrayidx11, float* %arrayidx14) #1
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
declare dso_local void @sincosf(float, float*, float*) local_unnamed_addr #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

attributes #0 = { nofree noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "prefer-vector-width"="256" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nofree nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "prefer-vector-width"="256" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"DPC++ Compiler 2021.1 (YYYY.8.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"float", !4, i64 0}
