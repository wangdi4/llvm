; Test that we can vectorize float, float version of mandelbrot kernel.
; RUN: opt -vplan-enable-soa=false -S %s -vplan-vec -vector-library=SVML 2>&1 | FileCheck %s
; RUN: opt -vplan-enable-soa=false -S %s -passes="vplan-vec" -vector-library=SVML 2>&1 | FileCheck %s
;
; The run lines below used to crash before CMPLRLLVM-22366 was fixed.
; RUN: opt -vplan-enable-soa=false -S %s -vplan-vec -vector-library=SVML -vplan-enable-all-zero-bypass-loops=0 -disable-output
; RUN: opt -vplan-enable-soa=false -S %s -passes="vplan-vec" -vector-library=SVML -vplan-enable-all-zero-bypass-loops=0 -disable-output

; The run lines below used to crash w/o the fix to enable DA recalculation in clones
; RUN: opt -vplan-enable-soa=false -S %s -vplan-vec -vector-library=SVML -vplan-enable-non-masked-vectorized-remainder 2>&1 | FileCheck %s
; RUN: opt -vplan-enable-soa=false -S %s -passes="vplan-vec" -vector-library=SVML -vplan-enable-non-masked-vectorized-remainder 2>&1 | FileCheck %s
;
; CHECK: call svml_cc <16 x float> @__svml_sqrtf16(

; ModuleID = 'mod.cpp'
source_filename = "mod.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@count = dso_local local_unnamed_addr global [3000 x [3000 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse uwtable
define dso_local i32 @main() local_unnamed_addr #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  br label %omp.inner.for.body.lr.ph

for.cond.cleanup:                                 ; preds = %DIR.OMP.END.SIMD.3
  call void @_Z3foov()
  ret i32 0

omp.inner.for.body.lr.ph:                         ; preds = %DIR.OMP.END.SIMD.3, %entry
  %indvars.iv39 = phi i64 [ 0, %entry ], [ %indvars.iv.next40, %DIR.OMP.END.SIMD.3 ]
  %0 = trunc i64 %indvars.iv39 to i32
  %conv = sitofp i32 %0 to float
  %mul = fmul float %conv, 0x3F5063B3E0000000
  %sub = fsub float 0x3FFCCCCCC0000000, %mul
  %in_vals_tmp_imagine.priv = alloca float, align 4
  %in_vals_tmp_real.priv = alloca float, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %in_vals_tmp_imagine.priv.priv = alloca float
  %in_vals_tmp_real.priv.priv = alloca float
  br label %DIR.OMP.SIMD.142

DIR.OMP.SIMD.142:                                 ; preds = %DIR.OMP.SIMD.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.PRIVATE"(float* %in_vals_tmp_real.priv.priv), "QUAL.OMP.PRIVATE"(float* %in_vals_tmp_imagine.priv.priv) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.142
  %2 = bitcast float* %in_vals_tmp_real.priv.priv to i8*
  %3 = bitcast float* %in_vals_tmp_imagine.priv.priv to i8*
  %mul1.i = fmul float %sub, %sub
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %_ZL6mandelffj.exit, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ %indvars.iv.next, %_ZL6mandelffj.exit ], [ 0, %DIR.OMP.SIMD.2 ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #2
  %4 = trunc i64 %indvars.iv to i32
  %conv3 = sitofp i32 %4 to float
  %mul4 = fmul float %conv3, 0x3F5063B3E0000000
  %add5 = fadd float %mul4, -2.000000e+00
  store float %add5, float* %in_vals_tmp_real.priv.priv, align 4, !tbaa !2
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3) #2
  store float %sub, float* %in_vals_tmp_imagine.priv.priv, align 4, !tbaa !2
  %mul.i = fmul float %add5, %add5
  %add.i = fadd float %mul1.i, %mul.i
  %call.i = call float @sqrtf(float %add.i) #2
  %cmp.i33 = fcmp olt float %call.i, 2.000000e+00
  br i1 %cmp.i33, label %while.body.i.preheader, label %_ZL6mandelffj.exit

while.body.i.preheader:                           ; preds = %omp.inner.for.body
  br label %while.body.i

while.body.i:                                     ; preds = %while.body.i.preheader, %while.body.i
  %z_imagine.0.i37 = phi float [ %add8.i, %while.body.i ], [ %sub, %while.body.i.preheader ]
  %z_real.0.i36 = phi float [ %add5.i, %while.body.i ], [ %add5, %while.body.i.preheader ]
  %count.0.i35 = phi i32 [ %inc.i, %while.body.i ], [ 1, %while.body.i.preheader ]
  %mul3.i = fmul float %z_real.0.i36, %z_real.0.i36
  %mul4.i = fmul float %z_imagine.0.i37, %z_imagine.0.i37
  %sub.i = fsub float %mul3.i, %mul4.i
  %add5.i = fadd float %add5, %sub.i
  %mul6.i = fmul float %add5.i, 2.000000e+00
  %mul7.i = fmul float %z_imagine.0.i37, %mul6.i
  %add8.i = fadd float %sub, %mul7.i
  %mul9.i = fmul float %add5.i, %add5.i
  %mul10.i = fmul float %add8.i, %add8.i
  %add11.i = fadd float %mul9.i, %mul10.i
  %call12.i = call float @sqrtf(float %add11.i) #2
  %inc.i = add nuw nsw i32 %count.0.i35, 1
  %cmp.i = fcmp olt float %call12.i, 2.000000e+00
  %cmp2.i = icmp ult i32 %inc.i, 3000
  %or.cond.i = and i1 %cmp2.i, %cmp.i
  br i1 %or.cond.i, label %while.body.i, label %_ZL6mandelffj.exit.loopexit

_ZL6mandelffj.exit.loopexit:                      ; preds = %while.body.i
  %inc.i.lcssa = phi i32 [ %inc.i, %while.body.i ]
  br label %_ZL6mandelffj.exit

_ZL6mandelffj.exit:                               ; preds = %_ZL6mandelffj.exit.loopexit, %omp.inner.for.body
  %count.0.i.lcssa = phi i32 [ 1, %omp.inner.for.body ], [ %inc.i.lcssa, %_ZL6mandelffj.exit.loopexit ]
  %arrayidx8 = getelementptr inbounds [3000 x [3000 x i32]], [3000 x [3000 x i32]]* @count, i64 0, i64 %indvars.iv39, i64 %indvars.iv, !intel-tbaa !6
  store i32 %count.0.i.lcssa, i32* %arrayidx8, align 4, !tbaa !6
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %3) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 3000
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %_ZL6mandelffj.exit
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  %indvars.iv.next40 = add nuw nsw i64 %indvars.iv39, 1
  %exitcond41 = icmp eq i64 %indvars.iv.next40, 3000
  br i1 %exitcond41, label %for.cond.cleanup, label %omp.inner.for.body.lr.ph
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

declare dso_local void @_Z3foov() local_unnamed_addr #3

; Function Attrs: nounwind
declare dso_local float @sqrtf(float) local_unnamed_addr #4

attributes #0 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind readnone }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 073c4ff27c752ef4458b2ccda1ade381622937e4) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 1bb4bf87d07135f8ca12532427c838c9dcac65da)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !9, i64 0}
!7 = !{!"array@_ZTSA3000_A3000_j", !8, i64 0}
!8 = !{!"array@_ZTSA3000_j", !9, i64 0}
!9 = !{!"int", !4, i64 0}
