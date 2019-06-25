; RUN: opt < %s -switch-to-offload=true -vpo-paropt -S | FileCheck %s
;
; #include <stdio.h>
;
; #define MAX 256
;
; double A[MAX][MAX], B[MAX];
;
;int main() {
;  #pragma omp target map(A, B)
;    for (int i = 0; i < MAX; i++) {
;      B[i] = i + 1;
;
;      if (i % 2 == 0) {
;        #pragma omp parallel for
;        for (int k = 0; k < MAX; k++)
;          A[i][k] = B[i] + k - 1;
;      }
;    }
;
;  printf(" a = %f  b = %f\n", A[32][16], B[32]);
;  return 0;
;}

target triple = "spir64"
target device_triples = "spir64"

@A = external dso_local global [256 x [256 x double]], align 8
@B = external dso_local global [256 x double], align 8
@.str = private unnamed_addr constant [17 x i8] c" a = %f  b = %f\0A\00", align 1
@"@tid.addr" = external global i32

; Function Attrs: noinline norecurse optnone uwtable

define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %k = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  br label %DIR.OMP.TARGET.1.split

DIR.OMP.TARGET.1.split:                           ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([256 x [256 x double]]* @A), "QUAL.OMP.MAP.TOFROM"([256 x double]* @B), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %k), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %DIR.OMP.TARGET.1.split
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %DIR.OMP.TARGET.1
  %1 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %1, 256
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32, i32* %i, align 4
  %add = add nsw i32 %2, 1
  %conv = sitofp i32 %add to double
  %3 = load i32, i32* %i, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds [256 x double], [256 x double]* @B, i64 0, i64 %idxprom
  store double %conv, double* %arrayidx, align 8
  %4 = load i32, i32* %i, align 4
  %rem = srem i32 %4, 2
  %cmp1 = icmp eq i32 %rem, 0
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  store i32 0, i32* %.omp.lb, align 4
  store volatile i32 255, i32* %.omp.ub, align 4
  br label %DIR.OMP.PARALLEL.LOOP.3.split

DIR.OMP.PARALLEL.LOOP.3.split:                    ; preds = %if.then
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %k), "QUAL.OMP.SHARED"([256 x double]* @B), "QUAL.OMP.SHARED"(i32* %i), "QUAL.OMP.SHARED"([256 x [256 x double]]* @A) ]
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:                          ; preds = %DIR.OMP.PARALLEL.LOOP.3.split
  %6 = load i32, i32* %.omp.lb, align 4
  store volatile i32 %6, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.PARALLEL.LOOP.2
  %7 = load volatile i32, i32* %.omp.iv, align 4
  %8 = load volatile i32, i32* %.omp.ub, align 4
  %cmp2 = icmp sle i32 %7, %8
  br i1 %cmp2, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load volatile i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %9, 1
  %add3 = add nsw i32 0, %mul
  store i32 %add3, i32* %k, align 4
  %10 = load i32, i32* %i, align 4
  %idxprom4 = sext i32 %10 to i64
  %arrayidx5 = getelementptr inbounds [256 x double], [256 x double]* @B, i64 0, i64 %idxprom4
  %11 = load double, double* %arrayidx5, align 8
  %12 = load i32, i32* %k, align 4
  %conv6 = sitofp i32 %12 to double
  %add7 = fadd double %11, %conv6
  %sub = fsub double %add7, 1.000000e+00
  %13 = load i32, i32* %i, align 4
  %idxprom8 = sext i32 %13 to i64
  %arrayidx9 = getelementptr inbounds [256 x [256 x double]], [256 x [256 x double]]* @A, i64 0, i64 %idxprom8
  %14 = load i32, i32* %k, align 4
  %idxprom10 = sext i32 %14 to i64
  %arrayidx11 = getelementptr inbounds [256 x double], [256 x double]* %arrayidx9, i64 0, i64 %idxprom10
  store double %sub, double* %arrayidx11, align 8
  %15 = load volatile i32, i32* %.omp.iv, align 4
  %add12 = add nsw i32 %15, 1
  store volatile i32 %add12, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %for.inc

for.inc:                                          ; preds = %for.body, %omp.loop.exit
  %16 = load i32, i32* %i, align 4
  %inc = add nsw i32 %16, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.3

DIR.OMP.END.TARGET.3:                             ; preds = %for.end
  %17 = load double, double* getelementptr inbounds ([256 x [256 x double]], [256 x [256 x double]]* @A, i64 0, i64 32, i64 16), align 8
  %18 = load double, double* getelementptr inbounds ([256 x double], [256 x double]* @B, i64 0, i64 32), align 8
  %call = call spir_func i32 (i8*, ...) @printf(i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str, i64 0, i64 0), double %17, double %18)
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local spir_func i32 @printf(i8*, ...) #2

attributes #0 = { noinline norecurse optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2055, i32 19811081, !"main", i32 8, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"cl_doubles"}
!4 = !{!"clang version 8.0.0"}

; CHECK: %{{.*}} = call i64 @_Z18work_group_barrierj(i32 0)
; CHECK: %{{.*}} = call i64 @_Z12get_local_idj(i32 0)
