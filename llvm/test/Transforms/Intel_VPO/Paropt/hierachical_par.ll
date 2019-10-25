; RUN: opt < %s -switch-to-offload -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg  -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S | FileCheck %s
; RUN: opt < %s -passes='function(loop(rotate),vpo-cfg-restructuring,vpo-paropt-prepare,simplify-cfg,loop(simplify-cfg),sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload  -S | FileCheck %s

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

; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: %{{.*}} = call i64 @_Z12get_local_idj(i32 0)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@A = external dso_local addrspace(1) global [256 x [256 x double]], align 8
@B = external dso_local addrspace(1) global [256 x double], align 8
@.str = private unnamed_addr addrspace(1) constant [17 x i8] c" a = %f  b = %f\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %0 = addrspacecast i32* %retval to i32 addrspace(4)*
  %i = alloca i32, align 4
  %1 = addrspacecast i32* %i to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %2 = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %3 = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %4 = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %5 = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %k = alloca i32, align 4
  %6 = addrspacecast i32* %k to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %0, align 4
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([256 x [256 x double]] addrspace(4)* addrspacecast ([256 x [256 x double]] addrspace(1)* @A to [256 x [256 x double]] addrspace(4)*)), "QUAL.OMP.MAP.TOFROM"([256 x double] addrspace(4)* addrspacecast ([256 x double] addrspace(1)* @B to [256 x double] addrspace(4)*)), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %1), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %4), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %5), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %6), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %2), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %3) ]
  store i32 0, i32 addrspace(4)* %1, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %8 = load i32, i32 addrspace(4)* %1, align 4
  %cmp = icmp slt i32 %8, 256
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %9 = load i32, i32 addrspace(4)* %1, align 4
  %add = add nsw i32 %9, 1
  %conv = sitofp i32 %add to double
  %10 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom = sext i32 %10 to i64
  %arrayidx = getelementptr inbounds [256 x double], [256 x double] addrspace(4)* addrspacecast ([256 x double] addrspace(1)* @B to [256 x double] addrspace(4)*), i64 0, i64 %idxprom
  store double %conv, double addrspace(4)* %arrayidx, align 8
  %11 = load i32, i32 addrspace(4)* %1, align 4
  %rem = srem i32 %11, 2
  %cmp1 = icmp eq i32 %rem, 0
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  store i32 0, i32 addrspace(4)* %4, align 4
  store i32 255, i32 addrspace(4)* %5, align 4
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %4), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %2), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %5), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %6), "QUAL.OMP.SHARED"([256 x double] addrspace(4)* addrspacecast ([256 x double] addrspace(1)* @B to [256 x double] addrspace(4)*)), "QUAL.OMP.SHARED"(i32 addrspace(4)* %1), "QUAL.OMP.SHARED"([256 x [256 x double]] addrspace(4)* addrspacecast ([256 x [256 x double]] addrspace(1)* @A to [256 x [256 x double]] addrspace(4)*)) ]
  %13 = load i32, i32 addrspace(4)* %4, align 4
  store i32 %13, i32 addrspace(4)* %2, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %if.then
  %14 = load i32, i32 addrspace(4)* %2, align 4
  %15 = load i32, i32 addrspace(4)* %5, align 4
  %cmp3 = icmp sle i32 %14, %15
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %16 = load i32, i32 addrspace(4)* %2, align 4
  %mul = mul nsw i32 %16, 1
  %add5 = add nsw i32 0, %mul
  store i32 %add5, i32 addrspace(4)* %6, align 4
  %17 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom6 = sext i32 %17 to i64
  %arrayidx7 = getelementptr inbounds [256 x double], [256 x double] addrspace(4)* addrspacecast ([256 x double] addrspace(1)* @B to [256 x double] addrspace(4)*), i64 0, i64 %idxprom6
  %18 = load double, double addrspace(4)* %arrayidx7, align 8
  %19 = load i32, i32 addrspace(4)* %6, align 4
  %conv8 = sitofp i32 %19 to double
  %add9 = fadd double %18, %conv8
  %sub = fsub double %add9, 1.000000e+00
  %20 = load i32, i32 addrspace(4)* %1, align 4
  %idxprom10 = sext i32 %20 to i64
  %arrayidx11 = getelementptr inbounds [256 x [256 x double]], [256 x [256 x double]] addrspace(4)* addrspacecast ([256 x [256 x double]] addrspace(1)* @A to [256 x [256 x double]] addrspace(4)*), i64 0, i64 %idxprom10
  %21 = load i32, i32 addrspace(4)* %6, align 4
  %idxprom12 = sext i32 %21 to i64
  %arrayidx13 = getelementptr inbounds [256 x double], [256 x double] addrspace(4)* %arrayidx11, i64 0, i64 %idxprom12
  store double %sub, double addrspace(4)* %arrayidx13, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i32, i32 addrspace(4)* %2, align 4
  %add14 = add nsw i32 %22, 1
  store i32 %add14, i32 addrspace(4)* %2, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %if.end

if.end:                                           ; preds = %omp.loop.exit, %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %23 = load i32, i32 addrspace(4)* %1, align 4
  %inc = add nsw i32 %23, 1
  store i32 %inc, i32 addrspace(4)* %1, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TARGET"() ]
  %24 = load double, double addrspace(4)* getelementptr inbounds ([256 x [256 x double]], [256 x [256 x double]] addrspace(4)* addrspacecast ([256 x [256 x double]] addrspace(1)* @A to [256 x [256 x double]] addrspace(4)*), i64 0, i64 32, i64 16), align 8
  %25 = load double, double addrspace(4)* getelementptr inbounds ([256 x double], [256 x double] addrspace(4)* addrspacecast ([256 x double] addrspace(1)* @B to [256 x double] addrspace(4)*), i64 0, i64 32), align 8
  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([17 x i8], [17 x i8] addrspace(4)* addrspacecast ([17 x i8] addrspace(1)* @.str to [17 x i8] addrspace(4)*), i64 0, i64 0), double %24, double %25)
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local spir_func i32 @printf(i8 addrspace(4)*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2052, i32 85985690, !"main", i32 8, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"cl_doubles"}
!4 = !{!"clang version 8.0.0"}
