; RUN: opt < %s -S -replace-with-math-library-functions -mf-x86-target | FileCheck %s

; The call to @_Z6sincosfPf should be split to llvm.sin and llvm.cos.
;  %3 = call fast float @llvm.sin.f32(float %div)
;  %4 = call fast float @llvm.cos.f32(float %div)
;  %add3 = fadd fast float %3, %4

; CHECK-NOT: call{{.*}}Z6sincosfPf
; CHECK: [[SINVAL:%[a-z0-9.]+]] = call fast float @llvm.sin.f32(float %div)
; CHECK-NEXT: [[COSVAL:%[a-z0-9.]+]] = call fast float @llvm.cos.f32(float %div)
; CHECK: fadd fast float [[SINVAL]], [[COSVAL]]
; CHECK-NOT: call{{.*}}Z6sincosfPf

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @_Z3fooPfS_(float* noalias nocapture readonly %f, float* noalias nocapture %r) local_unnamed_addr #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
omp.inner.for.body.lr.ph:
  %cos.ptr = alloca float
  %val.priv = alloca float, align 4
  %i.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LASTPRIVATE"(i32* %i.lpriv), "QUAL.OMP.PRIVATE"(float* %val.priv), "QUAL.OMP.PRIVATE"(float* %cos.ptr) ]
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
  %3 = call fast float @_Z6sincosfPf(float %div, float* %cos.ptr)
  %cos.val = load float, float* %cos.ptr
  %add3 = fadd fast float %3, %cos.val
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

; Function Attrs: nounwind
declare float @_Z6sincosfPf(float, float*) #2

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { nounwind readnone speculatable }
