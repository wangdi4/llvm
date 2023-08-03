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
; CHECK: %cos.val = load float, ptr %cos.ptr
; CHECK: fadd{{.*}}[[SINVAL]]{{.*}}%cos.val

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @_Z3fooPfS_(ptr noalias nocapture readonly %f, ptr noalias nocapture %r) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
omp.inner.for.body.lr.ph:
  %val.priv = alloca float, align 4
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
%0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %i.lpriv, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %val.priv, float 0.000000e+00, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.2 ]
  %arrayidx = getelementptr inbounds float, ptr %f, i64 %indvars.iv
  %1 = load float, ptr %arrayidx, align 4
  %div = fmul fast float %1, 0x3FD5555560000000
  %2 = call fast float @llvm.sin.f32(float %div) #2
  %3 = call fast float @llvm.cos.f32(float %div) #2
  %add3 = fadd fast float %2, %3
  %arrayidx5 = getelementptr inbounds float, ptr %r, i64 %indvars.iv
  store float %add3, ptr %arrayidx5, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  store i32 99, ptr %i.lpriv, align 4
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

declare dso_local i32 @__gxx_personality_v0(...)

declare float @llvm.sin.f32(float) #3
declare float @llvm.cos.f32(float) #3
declare float @_Z6sincosfPf(float, ptr) #2

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { nounwind readnone speculatable }
