; Verify that VPlan vectorizers don't vectorize libcalls without readnone
; attribute into SVML variants.

; RUN: opt < %s -vplan-vec -vplan-force-vf=2 -vplan-print-scalvec-results -vplan-vectorize-non-readonly-libcalls=false -vector-library=SVML -S | FileCheck %s --check-prefix=LLVM-IR
; Checks for LLVM-IR VPlan vectorizer
; LLVM-IR-LABEL: VPlan after ScalVec analysis:
; LLVM-IR:         [DA: Div, SVA: ( V )] float {{%vp.*}} = call float {{%vp.*}} float (float)* @logf [Serial] (SVAOpBits 0->V 1->F )

; LLVM-IR-LABEL: vector.body:
; LLVM-IR:         [[ARG_EXTRACT_1:%.*]] = extractelement <2 x float> [[ARG:%.*]], i32 1
; LLVM-IR-NEXT:    [[ARG_EXTRACT_0:%.*]] = extractelement <2 x float> [[ARG]], i32 0
; LLVM-IR-NEXT:    [[SERIAL_CALL_0:%.*]] = call float @logf(float [[ARG_EXTRACT_0]])
; LLVM-IR-NEXT:    [[CALL_INSERT_0:%.*]] = insertelement <2 x float> undef, float [[SERIAL_CALL_0]], i32 0
; LLVM-IR-NEXT:    [[SERIAL_CALL_1:%.*]] = call float @logf(float [[ARG_EXTRACT_1]])
; LLVM-IR-NEXT:    [[CALL_INSERT_1:%.*]] = insertelement <2 x float> [[CALL_INSERT_0]], float [[SERIAL_CALL_1]], i32 1

; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=2 -vplan-vectorize-non-readonly-libcalls=false -vector-library=SVML -print-after=hir-vplan-vec -disable-output  < %s 2>&1 | FileCheck %s --check-prefix=HIR
; Checks for HIR VPlan vectorizer
; HIR:           + DO i1 = 0, 1023, 2   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR-NEXT:      |   %.vec = (<2 x float>*)(%a)[i1];
; HIR-NEXT:      |   %serial.temp = undef;
; HIR-NEXT:      |   %extract.0. = extractelement %.vec,  0;
; HIR-NEXT:      |   %logf = @logf(%extract.0.);
; HIR-NEXT:      |   %serial.temp = insertelement %serial.temp,  %logf,  0;
; HIR-NEXT:      |   %extract.1. = extractelement %.vec,  1;
; HIR-NEXT:      |   %logf2 = @logf(%extract.1.);
; HIR-NEXT:      |   %serial.temp = insertelement %serial.temp,  %logf2,  1;
; HIR-NEXT:      |   (<2 x float>*)(%a)[i1] = %serial.temp;
; HIR-NEXT:      + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(float* nocapture %a) local_unnamed_addr #0 {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %omp.inner.for.body.lr.ph ]
  %ptridx = getelementptr inbounds float, float* %a, i64 %indvars.iv
  %1 = load float, float* %ptridx, align 4
  %call = call float @logf(float %1) #2
  store float %call, float* %ptridx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nofree nounwind
declare dso_local float @logf(float) local_unnamed_addr #3

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { nofree nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }

