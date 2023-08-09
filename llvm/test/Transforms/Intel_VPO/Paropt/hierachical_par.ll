; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; #define MAX 256
;
; double A[MAX][MAX], B[MAX];
;
; int main() {
; #pragma omp target map(A, B)
;   for (int i = 0; i < MAX; i++) {
;     B[i] = i + 1;
;
;     if (i % 2 == 0) {
; #pragma omp parallel for
;       for (int k = 0; k < MAX; k++)
;         A[i][k] = B[i] + k - 1;
;     }
;   }
;
;   printf(" a = %f  b = %f\n", A[32][16], B[32]);
;   return 0;
; }

; CHECK-DAG: call spir_func void @_Z22__spirv_ControlBarrieriii(i32 2, i32 2, i32 784)
; CHECK-DAG: %{{.*}} = call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK-DAG: declare spir_func void @_Z22__spirv_ControlBarrieriii(i32, i32, i32) #[[ATTR:[0-9]+]]
; CHECK: attributes #[[ATTR]] = {{{.*}}convergent{{.*}}}

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@B = external addrspace(1) global [256 x double], align 8
@A = external addrspace(1) global [256 x [256 x double]], align 8
@.str = private unnamed_addr addrspace(1) constant [17 x i8] c" a = %f  b = %f\0A\00", align 1

; Function Attrs: convergent noinline nounwind optnone
define protected i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %k = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %k.ascast = addrspacecast ptr %k to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @B to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @B to ptr addrspace(4)), i64 2048, i64 35, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @A to ptr addrspace(4)), ptr addrspace(4) addrspacecast (ptr addrspace(1) @A to ptr addrspace(4)), i64 524288, i64 35, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]
  store i32 0, ptr addrspace(4) %i.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, ptr addrspace(4) %i.ascast, align 4
  %cmp = icmp slt i32 %1, 256
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32, ptr addrspace(4) %i.ascast, align 4
  %add = add nsw i32 %2, 1
  %conv = sitofp i32 %add to double
  %3 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds [256 x double], ptr addrspace(4) addrspacecast (ptr addrspace(1) @B to ptr addrspace(4)), i64 0, i64 %idxprom
  store double %conv, ptr addrspace(4) %arrayidx, align 8
  %4 = load i32, ptr addrspace(4) %i.ascast, align 4
  %rem = srem i32 %4, 2
  %cmp1 = icmp eq i32 %rem, 0
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 255, ptr addrspace(4) %.omp.ub.ascast, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @A to ptr addrspace(4)), double 0.000000e+00, i64 65536),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @B to ptr addrspace(4)), double 0.000000e+00, i64 256),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1) ]
  %6 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %6, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %if.then
  %7 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %8 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp3 = icmp sle i32 %7, %8
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %9, 1
  %add5 = add nsw i32 0, %mul
  store i32 %add5, ptr addrspace(4) %k.ascast, align 4
  %10 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom6 = sext i32 %10 to i64
  %arrayidx7 = getelementptr inbounds [256 x double], ptr addrspace(4) addrspacecast (ptr addrspace(1) @B to ptr addrspace(4)), i64 0, i64 %idxprom6
  %11 = load double, ptr addrspace(4) %arrayidx7, align 8
  %12 = load i32, ptr addrspace(4) %k.ascast, align 4
  %conv8 = sitofp i32 %12 to double
  %add9 = fadd fast double %11, %conv8
  %sub = fsub fast double %add9, 1.000000e+00
  %13 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom10 = sext i32 %13 to i64
  %arrayidx11 = getelementptr inbounds [256 x [256 x double]], ptr addrspace(4) addrspacecast (ptr addrspace(1) @A to ptr addrspace(4)), i64 0, i64 %idxprom10
  %14 = load i32, ptr addrspace(4) %k.ascast, align 4
  %idxprom12 = sext i32 %14 to i64
  %arrayidx13 = getelementptr inbounds [256 x double], ptr addrspace(4) %arrayidx11, i64 0, i64 %idxprom12
  store double %sub, ptr addrspace(4) %arrayidx13, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %15 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add14 = add nsw i32 %15, 1
  store i32 %add14, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %if.end

if.end:                                           ; preds = %omp.loop.exit, %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %16 = load i32, ptr addrspace(4) %i.ascast, align 4
  %inc = add nsw i32 %16, 1
  store i32 %inc, ptr addrspace(4) %i.ascast, align 4
  br label %for.cond, !llvm.loop !8

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %17 = load double, ptr addrspace(4) getelementptr inbounds ([256 x [256 x double]], ptr addrspace(4) addrspacecast (ptr addrspace(1) @A to ptr addrspace(4)), i64 0, i64 32, i64 16), align 8
  %18 = load double, ptr addrspace(4) getelementptr inbounds ([256 x double], ptr addrspace(4) addrspacecast (ptr addrspace(1) @B to ptr addrspace(4)), i64 0, i64 32), align 8
  %call = call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4) noundef addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4)), double noundef %17, double noundef %18) #3
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent
declare spir_func i32 @printf(ptr addrspace(4) noundef, ...) #2

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { convergent "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #3 = { convergent }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 53, i32 -1927664125, !"_Z4main", i32 8, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
