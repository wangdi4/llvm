; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
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

; CHECK-NOT: QUAL.OMP.REDUCTION.AND
; CHECK: %[[RED:[^,]+]] = load <4 x float>, ptr %v3.fast_red, align 16
; CHECK-NEXT: %[[ORIG:[^,]+]] = load <4 x float>, ptr %v3, align 16
; CHECK-NEXT: %[[ORIG_BOOL:[^,]+]] = fcmp une <4 x float> %[[ORIG]], zeroinitializer
; CHECK-NEXT: %[[RED_BOOL:[^,]+]] = fcmp une <4 x float> %[[RED]], zeroinitializer
; CHECK-NEXT: %[[BOOL_VAL:[^,]+]] = select <4 x i1> %[[ORIG_BOOL]], <4 x i1> %[[RED_BOOL]], <4 x i1> %[[ORIG_BOOL]]
; CHECK-NEXT: %[[CONV:[^,]+]] = uitofp <4 x i1> %[[BOOL_VAL]] to <4 x float>
; CHECK-NEXT: store <4 x float> %[[CONV]], ptr %v3, align 16

; ModuleID = 'reduction_bool_and_m128.cpp'
source_filename = "reduction_bool_and_m128.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"FAILED\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"PASSED\0A\00", align 1

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() {
entry:
  %retval = alloca i32, align 4
  %v3 = alloca <4 x float>, align 16
  %i = alloca i32, align 4
  %v4 = alloca <4 x float>, align 16
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %test = alloca ptr, align 8
  %ref = alloca ptr, align 8
  store i32 0, ptr %retval, align 4
  %call = call fast noundef nofpclass(nan inf) <4 x float> @_ZL11_mm_setr_psffff(float noundef nofpclass(nan inf) 0x47EFFFFFE0000000, float noundef nofpclass(nan inf) 0x3810000000000000, float noundef nofpclass(nan inf) 0x47EFFFFFE0000000, float noundef nofpclass(nan inf) 0x47EFFFFFE0000000)
  store <4 x float> %call, ptr %v3, align 16
  %call1 = call fast noundef nofpclass(nan inf) <4 x float> @_ZL11_mm_setr_psffff(float noundef nofpclass(nan inf) 0.000000e+00, float noundef nofpclass(nan inf) 1.000000e+00, float noundef nofpclass(nan inf) 0.000000e+00, float noundef nofpclass(nan inf) 1.000000e+00)
  store <4 x float> %call1, ptr %v4, align 16
  store i32 0, ptr %.omp.lb, align 4
  store i32 2, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.AND:TYPED"(ptr %v3, <4 x float> zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %v4, <4 x float> zeroinitializer, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %5 = load <4 x float>, ptr %v3, align 16
  %6 = load <4 x float>, ptr %v4, align 16
  %cmp2 = fcmp fast une <4 x float> %5, zeroinitializer
  %cmp3 = fcmp fast une <4 x float> %6, zeroinitializer
  %7 = and <4 x i1> %cmp2, %cmp3
  %sext = sext <4 x i1> %7 to <4 x i32>
  %8 = bitcast <4 x i32> %sext to <4 x float>
  store <4 x float> %8, ptr %v3, align 16
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, ptr %.omp.iv, align 4
  %add4 = add nsw i32 %9, 1
  store i32 %add4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  store ptr %v3, ptr %test, align 8
  store ptr %v4, ptr %ref, align 8
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %omp.loop.exit
  %10 = load i32, ptr %i, align 4
  %cmp5 = icmp slt i32 %10, 4
  br i1 %cmp5, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %11 = load ptr, ptr %test, align 8
  %12 = load i32, ptr %i, align 4
  %idxprom = sext i32 %12 to i64
  %arrayidx = getelementptr inbounds float, ptr %11, i64 %idxprom
  %13 = load float, ptr %arrayidx, align 4
  %14 = load ptr, ptr %ref, align 8
  %15 = load i32, ptr %i, align 4
  %idxprom6 = sext i32 %15 to i64
  %arrayidx7 = getelementptr inbounds float, ptr %14, i64 %idxprom6
  %16 = load float, ptr %arrayidx7, align 4
  %sub = fsub fast float %13, %16
  %conv = fpext float %sub to x86_fp80
  %17 = call fast x86_fp80 @llvm.fabs.f80(x86_fp80 %conv)
  %cmp8 = fcmp fast ogt x86_fp80 %17, 0xK3FD78CBCCC096F508800
  br i1 %cmp8, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %call9 = call i32 (ptr, ...) @printf(ptr noundef @.str)
  store i32 3, ptr %retval, align 4
  br label %return

if.end:                                           ; preds = %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %18 = load i32, ptr %i, align 4
  %inc = add nsw i32 %18, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %call10 = call i32 (ptr, ...) @printf(ptr noundef @.str.1)
  store i32 0, ptr %retval, align 4
  br label %return

return:                                           ; preds = %for.end, %if.then
  %19 = load i32, ptr %retval, align 4
  ret i32 %19
}

define internal noundef nofpclass(nan inf) <4 x float> @_ZL11_mm_setr_psffff(float noundef nofpclass(nan inf) %__z, float noundef nofpclass(nan inf) %__y, float noundef nofpclass(nan inf) %__x, float noundef nofpclass(nan inf) %__w) {
entry:
  %__z.addr = alloca float, align 4
  %__y.addr = alloca float, align 4
  %__x.addr = alloca float, align 4
  %__w.addr = alloca float, align 4
  %.compoundliteral = alloca <4 x float>, align 16
  store float %__z, ptr %__z.addr, align 4
  store float %__y, ptr %__y.addr, align 4
  store float %__x, ptr %__x.addr, align 4
  store float %__w, ptr %__w.addr, align 4
  %0 = load float, ptr %__z.addr, align 4
  %vecinit = insertelement <4 x float> undef, float %0, i32 0
  %1 = load float, ptr %__y.addr, align 4
  %vecinit1 = insertelement <4 x float> %vecinit, float %1, i32 1
  %2 = load float, ptr %__x.addr, align 4
  %vecinit2 = insertelement <4 x float> %vecinit1, float %2, i32 2
  %3 = load float, ptr %__w.addr, align 4
  %vecinit3 = insertelement <4 x float> %vecinit2, float %3, i32 3
  store <4 x float> %vecinit3, ptr %.compoundliteral, align 16
  %4 = load <4 x float>, ptr %.compoundliteral, align 16
  ret <4 x float> %4
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare x86_fp80 @llvm.fabs.f80(x86_fp80)

declare dso_local i32 @printf(ptr noundef, ...)

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
