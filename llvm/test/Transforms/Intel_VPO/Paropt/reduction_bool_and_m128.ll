; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s


; This test is used to check reduction with bool and operation + m128 type.
; #include <stdio.h>
; #include <xmmintrin.h>
; #include <float.h>
; #include <math.h>
;
; #define eps         1.0e-12
;
; int
; main() {
;
;   __m128 v3;
;   int i;
;
;   v3 = _mm_setr_ps(FLT_MAX, FLT_MIN, FLT_MAX, FLT_MAX);
;   __m128 v4 = _mm_setr_ps(0.0, 1.0, 0.0, 1.0);
;
; #pragma omp parallel for reduction(&&: v3)
;   for( i = 0; i < 3; i++ ) {
;     v3 = v3 && v4;
;   }
;   float *test = (float*)&v3;
;   float *ref = (float*)&v4;
;   for (i=0; i<4; i++) {
;     if ( fabsl(test[i] - ref[i]) > eps ) {
;     printf("FAILED\n");
;       return 3;
;     }
;   }
;     printf("PASSED\n");
;   return 0;
; }


; ModuleID = 'reduction_bool_and_m128.cpp'
source_filename = "reduction_bool_and_m128.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"FAILED\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"PASSED\0A\00", align 1

; Function Attrs: noinline norecurse optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %v3 = alloca <4 x float>, align 16
  %i = alloca i32, align 4
  %v4 = alloca <4 x float>, align 16
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %test = alloca float*, align 8
  %ref = alloca float*, align 8
  store i32 0, i32* %retval, align 4
  %call = call <4 x float> @_ZL11_mm_setr_psffff(float 0x47EFFFFFE0000000, float 0x3810000000000000, float 0x47EFFFFFE0000000, float 0x47EFFFFFE0000000)
  store <4 x float> %call, <4 x float>* %v3, align 16
  %call1 = call <4 x float> @_ZL11_mm_setr_psffff(float 0.000000e+00, float 1.000000e+00, float 0.000000e+00, float 1.000000e+00)
  store <4 x float> %call1, <4 x float>* %v4, align 16
  store i32 0, i32* %.omp.lb, align 4
  store i32 2, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.AND"(<4 x float>* %v3), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(<4 x float>* %v4), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]


; CHECK-NOT: QUAL.OMP.REDUCTION.AND
; CHECK: %[[RED:[^,]+]] = load <4 x float>, <4 x float>* %v3.fast_red, align 16
; CHECK-NEXT: %[[ORIG:[^,]+]] = load <4 x float>, <4 x float>* %v3, align 16
; CHECK-NEXT: %[[ORIG_BOOL:[^,]+]] = fcmp une <4 x float> %[[ORIG]], zeroinitializer
; CHECK-NEXT: %[[RED_BOOL:[^,]+]] = fcmp une <4 x float> %[[RED]], zeroinitializer
; CHECK-NEXT: %[[BOOL_VAL:[^,]+]] = select <4 x i1> %[[ORIG_BOOL]], <4 x i1> %[[RED_BOOL]], <4 x i1> %[[ORIG_BOOL]]
; CHECK-NEXT: %[[CONV:[^,]+]] = uitofp <4 x i1> %[[BOOL_VAL]] to <4 x float>
; CHECK-NEXT: store <4 x float> %[[CONV]], <4 x float>* %v3, align 16


  %1 = load i32, i32* %.omp.lb, align 4
  store i32 %1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32* %.omp.iv, align 4
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %5 = load <4 x float>, <4 x float>* %v3, align 16
  %6 = load <4 x float>, <4 x float>* %v4, align 16
  %cmp2 = fcmp une <4 x float> %5, zeroinitializer
  %cmp3 = fcmp une <4 x float> %6, zeroinitializer
  %7 = and <4 x i1> %cmp2, %cmp3
  %sext = sext <4 x i1> %7 to <4 x i32>
  %8 = bitcast <4 x i32> %sext to <4 x float>
  store <4 x float> %8, <4 x float>* %v3, align 16
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, i32* %.omp.iv, align 4
  %add4 = add nsw i32 %9, 1
  store i32 %add4, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %10 = bitcast <4 x float>* %v3 to float*
  store float* %10, float** %test, align 8
  %11 = bitcast <4 x float>* %v4 to float*
  store float* %11, float** %ref, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %omp.loop.exit
  %12 = load i32, i32* %i, align 4
  %cmp5 = icmp slt i32 %12, 4
  br i1 %cmp5, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %13 = load float*, float** %test, align 8
  %14 = load i32, i32* %i, align 4
  %idxprom = sext i32 %14 to i64
  %ptridx = getelementptr inbounds float, float* %13, i64 %idxprom
  %15 = load float, float* %ptridx, align 4
  %16 = load float*, float** %ref, align 8
  %17 = load i32, i32* %i, align 4
  %idxprom6 = sext i32 %17 to i64
  %ptridx7 = getelementptr inbounds float, float* %16, i64 %idxprom6
  %18 = load float, float* %ptridx7, align 4
  %sub = fsub float %15, %18
  %conv = fpext float %sub to x86_fp80
  %19 = call x86_fp80 @llvm.fabs.f80(x86_fp80 %conv)
  %cmp8 = fcmp ogt x86_fp80 %19, 0xK3FD78CBCCC096F508800
  br i1 %cmp8, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %call9 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0))
  store i32 3, i32* %retval, align 4
  br label %return

if.end:                                           ; preds = %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %20 = load i32, i32* %i, align 4
  %inc = add nsw i32 %20, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %call10 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i64 0, i64 0))
  store i32 0, i32* %retval, align 4
  br label %return

return:                                           ; preds = %for.end, %if.then
  %21 = load i32, i32* %retval, align 4
  ret i32 %21
}

; Function Attrs: alwaysinline nounwind uwtable
define internal <4 x float> @_ZL11_mm_setr_psffff(float %__z, float %__y, float %__x, float %__w) #1 {
entry:
  %__z.addr = alloca float, align 4
  %__y.addr = alloca float, align 4
  %__x.addr = alloca float, align 4
  %__w.addr = alloca float, align 4
  %.compoundliteral = alloca <4 x float>, align 16
  store float %__z, float* %__z.addr, align 4
  store float %__y, float* %__y.addr, align 4
  store float %__x, float* %__x.addr, align 4
  store float %__w, float* %__w.addr, align 4
  %0 = load float, float* %__z.addr, align 4
  %vecinit = insertelement <4 x float> undef, float %0, i32 0
  %1 = load float, float* %__y.addr, align 4
  %vecinit1 = insertelement <4 x float> %vecinit, float %1, i32 1
  %2 = load float, float* %__x.addr, align 4
  %vecinit2 = insertelement <4 x float> %vecinit1, float %2, i32 2
  %3 = load float, float* %__w.addr, align 4
  %vecinit3 = insertelement <4 x float> %vecinit2, float %3, i32 3
  store <4 x float> %vecinit3, <4 x float>* %.compoundliteral, align 16
  %4 = load <4 x float>, <4 x float>* %.compoundliteral, align 16
  ret <4 x float> %4
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nounwind readnone speculatable willreturn
declare x86_fp80 @llvm.fabs.f80(x86_fp80) #3

declare dso_local i32 @printf(i8*, ...) #4

attributes #0 = { noinline norecurse optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="128" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { alwaysinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="128" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { nounwind readnone speculatable willreturn }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
