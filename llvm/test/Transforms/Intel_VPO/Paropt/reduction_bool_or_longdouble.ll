; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s


; This test is used to check reduction with bool or operation + longdouble type.
; #include <stdio.h>
; #include <math.h>
;
; #define eps         1.0e-12
;
; int main() {
;
;   long double v3;
;   int i;
;
;   v3 = 1;
;
; #pragma omp parallel for reduction(||: v3)
;   for( i = 0; i < 3; i++ ) {
;     v3 = v3 || 1;
;   }
;
;   if ( fabsl( v3 - 1 ) > eps ) {
;     printf("FAILED\n");
;     return 3;
;   }
;     printf("PASSED\n");
;   return 0;
; }


; ModuleID = 'reduction_bool_or_longdouble.c'
source_filename = "reduction_bool_or_longdouble.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"FAILED\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"PASSED\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %v3 = alloca x86_fp80, align 16
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store x86_fp80 0xK3FFF8000000000000000, x86_fp80* %v3, align 16
  store i32 0, i32* %.omp.lb, align 4
  store i32 2, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.OR"(x86_fp80* %v3), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]

; CHECK-NOT: QUAL.OMP.REDUCTION.AND
; CHECK: %[[RED:[^,]+]] = load x86_fp80, x86_fp80* %v3.fast_red, align 16
; CHECK-NEXT: %[[ORIG:[^,]+]] = load x86_fp80, x86_fp80* %v3, align 16
; CHECK-NEXT: %[[ORIG_BOOL:[^,]+]] = fcmp une x86_fp80 %[[ORIG]], 0xK00000000000000000000
; CHECK-NEXT: %[[RED_BOOL:[^,]+]] = fcmp une x86_fp80 %[[RED]], 0xK00000000000000000000
; CHECK-NEXT: %[[BOOL_VAL:[^,]+]] = select i1 %[[ORIG_BOOL]], i1 %[[ORIG_BOOL]], i1 %[[RED_BOOL]]
; CHECK-NEXT: %[[CONV:[^,]+]] = uitofp i1 %[[BOOL_VAL]] to x86_fp80
; CHECK-NEXT: store x86_fp80 %[[CONV]], x86_fp80* %v3, align 16

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
  %5 = load x86_fp80, x86_fp80* %v3, align 16
  %tobool = fcmp une x86_fp80 %5, 0xK00000000000000000000
  br i1 %tobool, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %omp.inner.for.body
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %omp.inner.for.body
  %6 = phi i1 [ true, %omp.inner.for.body ], [ true, %lor.rhs ]
  %lor.ext = zext i1 %6 to i32
  %conv = sitofp i32 %lor.ext to x86_fp80
  store x86_fp80 %conv, x86_fp80* %v3, align 16
  br label %omp.body.continue

omp.body.continue:                                ; preds = %lor.end
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %8 = load x86_fp80, x86_fp80* %v3, align 16
  %sub = fsub x86_fp80 %8, 0xK3FFF8000000000000000
  %9 = call x86_fp80 @llvm.fabs.f80(x86_fp80 %sub)
  %cmp2 = fcmp ogt x86_fp80 %9, 0xK3FD78CBCCC096F508800
  br i1 %cmp2, label %if.then, label %if.end

if.then:                                          ; preds = %omp.loop.exit
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0))
  store i32 3, i32* %retval, align 4
  br label %return

if.end:                                           ; preds = %omp.loop.exit
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i64 0, i64 0))
  store i32 0, i32* %retval, align 4
  br label %return

return:                                           ; preds = %if.end, %if.then
  %10 = load i32, i32* %retval, align 4
  ret i32 %10
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind readnone speculatable willreturn
declare x86_fp80 @llvm.fabs.f80(x86_fp80) #2

declare dso_local i32 @printf(i8*, ...) #3

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone speculatable willreturn }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
