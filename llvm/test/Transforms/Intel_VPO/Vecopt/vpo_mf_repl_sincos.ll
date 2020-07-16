; RUN: opt < %s -S -replace-with-math-library-functions -mf-x86-target -iml-trans | FileCheck %s

; Disabled until new sincos fusion implementation.
; XFAIL: *

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; TODO: Call sincos, not OCL sincos.
; TEST 1
;
; The llvm.sin and llvm.cos calls below should be combined to:

; %cos.ptr = alloca float
; ...
; ...OMP.SIMD "QUAL.OMP.PRIVATE"(float* %cos.ptr)
; ...
; %3 = call fast float @_Z6sincosfPf(float %div, float* %cos.ptr)
; %cos.val = load float, float* %cos.ptr
; %add3 = fadd fast float %3, %cos.val

; CHECK: %cos.ptr = alloca float
; CHECK: PRIVATE{{.*}}%cos.ptr
; CHECK: [[SINVAL:%[a-z0-9.]+]] = call fast float @_Z6sincosfPf
; CHECK: %cos.val = load float, float* %cos.ptr
; CHECK: fadd{{.*}}[[SINVAL]]{{.*}}%cos.val

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @_Z3fooPfS_(float* noalias nocapture readonly %f, float* noalias nocapture %r) local_unnamed_addr #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
omp.inner.for.body.lr.ph:
  %val.priv = alloca float, align 4
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LASTPRIVATE"(i32* %i.lpriv), "QUAL.OMP.PRIVATE"(float* %val.priv) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %1 = bitcast float* %val.priv to i8*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.2 ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  %arrayidx = getelementptr inbounds float, float* %f, i64 %indvars.iv
  %2 = load float, float* %arrayidx, align 4
  %div = fmul fast float %2, 0x3FD5555560000000
  %3 = call fast float @llvm.sin.f32(float %div) #2
  %4 = call fast float @llvm.cos.f32(float %div) #2
  %add3 = fadd fast float %3, %4
  %arrayidx5 = getelementptr inbounds float, float* %r, i64 %indvars.iv
  store float %add3, float* %arrayidx5, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  store i32 99, i32* %i.lpriv, align 4
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.417

DIR.OMP.END.SIMD.417:                             ; preds = %DIR.OMP.END.SIMD.3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind readnone speculatable
declare float @llvm.sin.f32(float) #3

; Function Attrs: nounwind readnone speculatable
declare float @llvm.cos.f32(float) #3

; Function Attrs: nofree nounwind
declare dso_local void @sincosf(float, float*, float*) local_unnamed_addr #0

; Function Attrs: nounwind
declare float @_Z6sincosfPf(float, float*) #2

; Function Attrs: nounwind
declare <4 x float> @_Z14sincos_ret2ptrDv4_fPS_S1_(<4 x float>, <4 x float>*, <4 x float>*) #2

; Function Attrs: nounwind
; Function Attrs: nounwind
declare <16 x float> @_Z14sincos_ret2ptrDv16_fPS_S1_(<16 x float>, <16 x float>*, <16 x float>*) #2

; Function Attrs: nounwind willreturn
declare void @llvm.masked.scatter.v4f32.v4p0f32(<4 x float>, <4 x float*>, i32 immarg, <4 x i1>) #2

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { nounwind readnone speculatable }
