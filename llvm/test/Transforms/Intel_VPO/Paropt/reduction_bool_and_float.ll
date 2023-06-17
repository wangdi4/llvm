; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This test is used to check reduction with bool and operation + float type.
; #include <stdio.h>
; #include <math.h>
;
; #define eps         1.0e-12
;
; int
; main() {
;
;    float v3;
;    int i;
;
;    v3 = 1;
;
;    #pragma omp parallel for reduction(&&: v3)
;       for( i = 0; i < 3; i++ ) {
;          v3 = v3 && 1;
;       }
;    if ( fabsl( v3 - 1 ) > eps ) {
;     printf("FAILED\n");
;      return 3;
;    }
;     printf("PASSED\n");
;    return 0;
; }

; CHECK-NOT: QUAL.OMP.REDUCTION.AND
; CHECK: %[[RED:[^,]+]] = load float, ptr %v3.fast_red, align 4
; CHECK-NEXT: %[[ORIG:[^,]+]] = load float, ptr %v3, align 4
; CHECK-NEXT: %[[ORIG_BOOL:[^,]+]] = fcmp une float %[[ORIG]], 0.000000e+00
; CHECK-NEXT: %[[RED_BOOL:[^,]+]] = fcmp une float %[[RED]], 0.000000e+00
; CHECK-NEXT: %[[BOOL_VAL:[^,]+]] = select i1 %[[ORIG_BOOL]], i1 %[[RED_BOOL]], i1 %[[ORIG_BOOL]]
; CHECK-NEXT: %[[CONV:[^,]+]] = uitofp i1 %[[BOOL_VAL]] to float
; CHECK-NEXT: store float %[[CONV]], ptr %v3, align 4

; ModuleID = 'reduction_bool_and_float.c'
source_filename = "reduction_bool_and_float.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"FAILED\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"PASSED\0A\00", align 1

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %v3 = alloca float, align 4
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store float 1.000000e+00, ptr %v3, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 2, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.AND:TYPED"(ptr %v3, float 0.000000e+00, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
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
  %5 = load float, ptr %v3, align 4
  %tobool = fcmp fast une float %5, 0.000000e+00
  br i1 %tobool, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %omp.inner.for.body
  br label %land.end

land.end:                                         ; preds = %land.rhs, %omp.inner.for.body
  %6 = phi i1 [ false, %omp.inner.for.body ], [ true, %land.rhs ]
  %land.ext = zext i1 %6 to i32
  %conv = sitofp i32 %land.ext to float
  store float %conv, ptr %v3, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %land.end
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  %8 = load float, ptr %v3, align 4
  %sub = fsub fast float %8, 1.000000e+00
  %conv2 = fpext float %sub to x86_fp80
  %9 = call fast x86_fp80 @llvm.fabs.f80(x86_fp80 %conv2)
  %cmp3 = fcmp fast ogt x86_fp80 %9, 0xK3FD78CBCCC096F508800
  br i1 %cmp3, label %if.then, label %if.end

if.then:                                          ; preds = %omp.loop.exit
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str)
  store i32 3, ptr %retval, align 4
  br label %return

if.end:                                           ; preds = %omp.loop.exit
  %call5 = call i32 (ptr, ...) @printf(ptr noundef @.str.1)
  store i32 0, ptr %retval, align 4
  br label %return

return:                                           ; preds = %if.end, %if.then
  %10 = load i32, ptr %retval, align 4
  ret i32 %10
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
