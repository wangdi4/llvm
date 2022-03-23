; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; #include <stdio.h>
; int main() {
;   int indexK = 0;
;   int x = 10;
;
; #pragma omp parallel num_threads(4)
;   {
; #pragma omp simd linear(x)
;     for (indexK = 0; indexK < 2; indexK++) {
;       printf("%d\n", indexK);
;       x++;
;     }
;   }
;   printf("%d %d\n", indexK, x);
; }
; ModuleID = 'par_simd_linea_linearivr.cpp'
source_filename = "par_simd_linear_lineariv.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@.str.1 = private unnamed_addr constant [7 x i8] c"%d %d\0A\00", align 1

; Function Attrs: noinline norecurse optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %indexK = alloca i32, align 4
; Check for allocation of the private copies of %x and %indexK inside the outlined
; function for PARALLEL.
; CHECK: define internal void @main.DIR.OMP.PARALLEL.{{.*}}
; CHECK: %x.linear = alloca i32, align 4
; CHECK: %indexK.linear.iv = alloca i32, align 4

  %x = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 0, i32* %indexK, align 4
  store i32 10, i32* %x, align 4

; Check that the local copy of %x is initialized before the directive.
; CHECK: [[INIT_VAL:%[^ ]+]] = load i32, i32* %x
; CHECK: store i32 [[INIT_VAL]], i32* %x.linear

; Check that the private copies of %x and %indexK is used on the SIMD directive.
; CHECK: {{[^ ]+}} = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR"(i32* %x.linear, i32 1), {{.*}} "QUAL.OMP.LINEAR:IV"(i32* %indexK.linear.iv, i32 1) ]

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.NUM_THREADS"(i32 4), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.SHARED"(i32* %x), "QUAL.OMP.SHARED"(i32* %indexK), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  store i32 1, i32* %.omp.ub, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR"(i32* %x, i32 1), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.LINEAR:IV"(i32* %indexK, i32 1) ]
  store i32 0, i32* %.omp.iv, align 4
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
  store i32 %add, i32* %indexK, align 4
  %5 = load i32, i32* %indexK, align 4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %5) #1
  %6 = load i32, i32* %x, align 4
  %inc = add nsw i32 %6, 1
  store i32 %inc, i32* %x, align 4
; Check that %x is replaced by its private copy inside the region.
; CHECK:  store i32 %inc, i32* %x.linear, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, i32* %.omp.iv, align 4
  %8 = load i32, i32* %indexK, align 4
  %add2 = add nsw i32 %8, 1
  store i32 %add2, i32* %indexK, align 4
; Check that %indexK is replaced by its private copy inside the region.
; CHECK: store i32 %add2, i32* %indexK.linear.iv, align 4
  br label %omp.inner.for.cond

; Check for the copy-out of the private copies of %x and %indexK back to the original, after the loop.
; CHECK: [[X_FINAL_VAL:%[^ ]+]] = load i32, i32* %x.linear
; CHECK: store i32 [[X_FINAL_VAL]], i32* %x
; CHECK: [[IK_FINAL_VAL:%[^ ]+]] = load i32, i32* %indexK.linear.iv
; CHECK: store i32 [[IK_FINAL_VAL]], i32* %indexK

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %9 = load i32, i32* %indexK, align 4
  %10 = load i32, i32* %x, align 4
  %call3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str.1, i64 0, i64 0), i32 %9, i32 %10)
  %11 = load i32, i32* %retval, align 4
  ret i32 %11
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline norecurse optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 11.0.0"}
