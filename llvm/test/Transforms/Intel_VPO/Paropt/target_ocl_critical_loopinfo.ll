; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-spirv-target-has-eu-fusion=true -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-paropt-spirv-target-has-eu-fusion=true -vpo-paropt-atomic-free-reduction=false -S %s | FileCheck %s

; Original code:
; extern void bar(void);
;
; void foo() {
;   double s = 0.0;
; #pragma omp target teams distribute reduction(+: s)
;   for (int j = 0; j < 100; ++j) {
; #pragma omp parallel for reduction(+: s)
;     for (int k = 0; k < 100; ++k) {
; #pragma omp critical
;       {
;         for (int i = 0; i < 10; ++i)
;           bar();
;       }
;       s += 1;
;     }
;   }
; }

; Verify that 'omp critical' lowering keeps the containing it
; parallel loop in valid state so that the loop rotation is possible.
; Invalid lowering causes an assertion in Paropt, so we just check
; that the critical calls were generated:
; CHECK: call{{.*}}__kmpc_critical
; CHECK: call{{.*}}__kmpc_critical
; CHECK: call{{.*}}__kmpc_end_critical
; CHECK: call{{.*}}__kmpc_end_critical

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone
define hidden spir_func void @foo() #0 {
entry:
  %s = alloca double, align 8
  %s.ascast = addrspacecast double* %s to double addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %j = alloca i32, align 4
  %j.ascast = addrspacecast i32* %j to i32 addrspace(4)*
  %tmp1 = alloca i32, align 4
  %tmp1.ascast = addrspacecast i32* %tmp1 to i32 addrspace(4)*
  %.omp.iv2 = alloca i32, align 4
  %.omp.iv2.ascast = addrspacecast i32* %.omp.iv2 to i32 addrspace(4)*
  %.omp.lb3 = alloca i32, align 4
  %.omp.lb3.ascast = addrspacecast i32* %.omp.lb3 to i32 addrspace(4)*
  %.omp.ub4 = alloca i32, align 4
  %.omp.ub4.ascast = addrspacecast i32* %.omp.ub4 to i32 addrspace(4)*
  %k = alloca i32, align 4
  %k.ascast = addrspacecast i32* %k to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  store double 0.000000e+00, double addrspace(4)* %s.ascast, align 8
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 99, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %s.ascast, double addrspace(4)* %s.ascast, i64 8, i64 547), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %k.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp1.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.REDUCTION.ADD"(double addrspace(4)* %s.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %k.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp1.ascast) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.lb3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.ub4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %k.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp1.ascast) ]
  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc14, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %5 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end16

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %j.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.lb3.ascast, align 4
  store i32 99, i32 addrspace(4)* %.omp.ub4.ascast, align 4
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(double addrspace(4)* %s.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv2.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb3.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %k.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ]
  %8 = load i32, i32 addrspace(4)* %.omp.lb3.ascast, align 4
  store i32 %8, i32 addrspace(4)* %.omp.iv2.ascast, align 4
  br label %omp.inner.for.cond5

omp.inner.for.cond5:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %9 = load i32, i32 addrspace(4)* %.omp.iv2.ascast, align 4
  %10 = load i32, i32 addrspace(4)* %.omp.ub4.ascast, align 4
  %cmp6 = icmp sle i32 %9, %10
  br i1 %cmp6, label %omp.inner.for.body7, label %omp.inner.for.end

omp.inner.for.body7:                              ; preds = %omp.inner.for.cond5
  %11 = load i32, i32 addrspace(4)* %.omp.iv2.ascast, align 4
  %mul8 = mul nsw i32 %11, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, i32 addrspace(4)* %k.ascast, align 4
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"() ]
  fence acquire
  store i32 0, i32 addrspace(4)* %i.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %omp.inner.for.body7
  %13 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %cmp10 = icmp slt i32 %13, 10
  br i1 %cmp10, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  call spir_func void @bar() #1
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %14 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %inc = add nsw i32 %14, 1
  store i32 %inc, i32 addrspace(4)* %i.ascast, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  fence release
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.CRITICAL"() ]
  %15 = load double, double addrspace(4)* %s.ascast, align 8
  %add11 = fadd fast double %15, 1.000000e+00
  store double %add11, double addrspace(4)* %s.ascast, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %16 = load i32, i32 addrspace(4)* %.omp.iv2.ascast, align 4
  %add12 = add nsw i32 %16, 1
  store i32 %add12, i32 addrspace(4)* %.omp.iv2.ascast, align 4
  br label %omp.inner.for.cond5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond5
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.body.continue13

omp.body.continue13:                              ; preds = %omp.loop.exit
  br label %omp.inner.for.inc14

omp.inner.for.inc14:                              ; preds = %omp.body.continue13
  %17 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add15 = add nsw i32 %17, 1
  store i32 %add15, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end16:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit17

omp.loop.exit17:                                  ; preds = %omp.inner.for.end16
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare spir_func void @bar() #2

attributes #0 = { noinline nounwind optnone "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!4}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!5}

!0 = !{i32 0, i32 2054, i32 1857328, !"_Z3foo", i32 7, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{}
!4 = !{!"cl_doubles"}
!5 = !{!"clang version 10.0.0"}
