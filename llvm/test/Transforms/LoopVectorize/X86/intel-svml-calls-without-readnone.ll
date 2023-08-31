; Verify that LV does not vectorize libcalls without readnone attribute into
; SVML variants.

; RUN: opt -passes="inject-tli-mappings,loop-vectorize" -vector-library=SVML -vectorize-non-readonly-libcalls=false -force-vector-width=2 -force-vector-interleave=1 -mattr=avx -S < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr nocapture %a) local_unnamed_addr #0 {
; CHECK-LABEL: @foo(
; CHECK:       vector.body:
; CHECK:         [[TMP4:%.*]] = extractelement <2 x float> [[WIDE_LOAD:%.*]], i32 0
; CHECK-NEXT:    [[TMP5:%.*]] = call float @logf(float [[TMP4]]) #2
; CHECK-NEXT:    [[TMP6:%.*]] = extractelement <2 x float> [[WIDE_LOAD]], i32 1
; CHECK-NEXT:    [[TMP7:%.*]] = call float @logf(float [[TMP6]]) #2
; CHECK-NEXT:    [[TMP8:%.*]] = insertelement <2 x float> poison, float [[TMP5]], i32 0
; CHECK-NEXT:    [[TMP9:%.*]] = insertelement <2 x float> [[TMP8]], float [[TMP7]], i32 1
;
omp.inner.for.body.lr.ph:
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.inner.for.body.lr.ph ]
  %ptridx = getelementptr inbounds float, ptr %a, i64 %indvars.iv
  %0 = load float, ptr %ptridx, align 4
  %call = call float @logf(float %0) #2
  store float %call, ptr %ptridx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  ret void
}

; Function Attrs: nofree nounwind
declare dso_local float @logf(float) local_unnamed_addr #3

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }

