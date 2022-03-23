; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; void foo() {
;   int x = -1;
; #pragma omp target teams distribute map(from: x) lastprivate(x) collapse(2)
;   for (int i = 0; i < 100; ++i)
;     for (int j = 0; j <= 100; ++j)
;       x = i * j;
; }

; CHECK-DAG: [[L0_IS_LAST_AI:%loop0.is.last]] = alloca i32
; CHECK-DAG: [[L1_IS_LAST_AI:%loop1.is.last]] = alloca i32
; CHECK-DAG: store i32 0, i32* [[L0_IS_LAST_AI]]
; CHECK-DAG: store i32 0, i32* [[L1_IS_LAST_AI]]

; CHECK: load i32, i32* %loop0.lower.bnd
; CHECK: load i32, i32* %loop0.upper.bnd
; CHECK: [[L0_LB:%[a-zA-Z._0-9]+]] = load i32, i32* %loop0.lower.bnd
; CHECK: [[L0_UB:%[a-zA-Z._0-9]+]] = load i32, i32* %loop0.upper.bnd
; CHECK: [[L0_ZTT:%[a-zA-Z._0-9]+]] = icmp sle i32 [[L0_LB]], [[L0_UB]]
; CHECK: [[L0_LI_CHECK:%[a-zA-Z._0-9]+]] = icmp eq i32 [[L0_UB]], [[L0_ORIG_UB:%[a-zA-Z._0-9]+]]
; CHECK: [[L0_LI_PRED:%[a-zA-Z._0-9]+]] = and i1 [[L0_ZTT]], [[L0_LI_CHECK]]
; CHECK: [[L0_LI_VAL:%[a-zA-Z._0-9]+]] = zext i1 [[L0_LI_PRED]] to i32
; CHECK: store i32 [[L0_LI_VAL]], i32* [[L0_IS_LAST_AI]]

; CHECK: load i32, i32* %loop1.lower.bnd
; CHECK: load i32, i32* %loop1.upper.bnd
; CHECK: [[L1_LB:%[a-zA-Z._0-9]+]] = load i32, i32* %loop1.lower.bnd
; CHECK: [[L1_UB:%[a-zA-Z._0-9]+]] = load i32, i32* %loop1.upper.bnd
; CHECK: [[L1_ZTT:%[a-zA-Z._0-9]+]] = icmp sle i32 [[L1_LB]], [[L1_UB]]
; CHECK: [[L1_LI_CHECK:%[a-zA-Z._0-9]+]] = icmp eq i32 [[L1_UB]], [[L1_ORIG_UB:%[a-zA-Z._0-9]+]]
; CHECK: [[L1_LI_PRED:%[a-zA-Z._0-9]+]] = and i1 [[L1_ZTT]], [[L1_LI_CHECK]]
; CHECK: [[L1_LI_VAL:%[a-zA-Z._0-9]+]] = zext i1 [[L1_LI_PRED]] to i32
; CHECK: store i32 [[L1_LI_VAL]], i32* [[L1_IS_LAST_AI]]

; CHECK: [[TMP1:%[a-zA-Z._0-9]+]] = load i32, i32* [[L1_IS_LAST_AI]]
; CHECK: [[TMP2:%[a-zA-Z._0-9]+]] = load i32, i32* [[L0_IS_LAST_AI]]
; CHECK: [[AND:%[a-zA-Z._0-9]+]] = and i32 [[TMP1]], [[TMP2]]
; CHECK: [[CMP:%[a-zA-Z._0-9]+]] = icmp ne i32 [[AND]], 0
; CHECK: br i1 [[CMP]], label %[[LAST_THEN:[a-zA-Z._0-9]+]], label %[[LAST_DONE:[a-zA-Z._0-9]+]]

; CHECK: [[LAST_THEN]]:
; CHECK: [[LAST_VAL:%[a-zA-Z._0-9]+]] = load i32, i32* [[LPRIV:%x.ascast.lpriv]]
; CHECK: store i32 [[LAST_VAL]], i32 addrspace(1)* [[VAR:%x.ascast]]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define hidden spir_func void @foo() #0 {
entry:
  %x = alloca i32, align 4
  %x.ascast = addrspacecast i32* %x to i32 addrspace(4)*
  %.omp.uncollapsed.lb = alloca i32, align 4
  %.omp.uncollapsed.lb.ascast = addrspacecast i32* %.omp.uncollapsed.lb to i32 addrspace(4)*
  %.omp.uncollapsed.ub = alloca i32, align 4
  %.omp.uncollapsed.ub.ascast = addrspacecast i32* %.omp.uncollapsed.ub to i32 addrspace(4)*
  %.omp.uncollapsed.lb1 = alloca i32, align 4
  %.omp.uncollapsed.lb1.ascast = addrspacecast i32* %.omp.uncollapsed.lb1 to i32 addrspace(4)*
  %.omp.uncollapsed.ub2 = alloca i32, align 4
  %.omp.uncollapsed.ub2.ascast = addrspacecast i32* %.omp.uncollapsed.ub2 to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %tmp3 = alloca i32, align 4
  %tmp3.ascast = addrspacecast i32* %tmp3 to i32 addrspace(4)*
  %.omp.uncollapsed.iv = alloca i32, align 4
  %.omp.uncollapsed.iv.ascast = addrspacecast i32* %.omp.uncollapsed.iv to i32 addrspace(4)*
  %.omp.uncollapsed.iv4 = alloca i32, align 4
  %.omp.uncollapsed.iv4.ascast = addrspacecast i32* %.omp.uncollapsed.iv4 to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %j = alloca i32, align 4
  %j.ascast = addrspacecast i32* %j to i32 addrspace(4)*
  store i32 -1, i32 addrspace(4)* %x.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.uncollapsed.lb.ascast, align 4
  store i32 99, i32 addrspace(4)* %.omp.uncollapsed.ub.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.uncollapsed.lb1.ascast, align 4
  store i32 100, i32 addrspace(4)* %.omp.uncollapsed.ub2.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.FROM"(i32 addrspace(4)* %x.ascast, i32 addrspace(4)* %x.ascast, i64 4, i64 34), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.ub.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb1.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.ub2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp3.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %x.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.uncollapsed.ub.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.uncollapsed.lb1.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.uncollapsed.ub2.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp3.ascast) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(), "QUAL.OMP.LASTPRIVATE"(i32 addrspace(4)* %x.ascast), "QUAL.OMP.COLLAPSE"(i32 2), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.uncollapsed.ub.ascast, i32 addrspace(4)* %.omp.uncollapsed.ub2.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb1.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast) ]
  %3 = load i32, i32 addrspace(4)* %.omp.uncollapsed.lb.ascast, align 4
  store i32 %3, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc12, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  %5 = load i32, i32 addrspace(4)* %.omp.uncollapsed.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end14

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %6 = load i32, i32 addrspace(4)* %.omp.uncollapsed.lb1.ascast, align 4
  store i32 %6, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.cond5:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %7 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast, align 4
  %8 = load i32, i32 addrspace(4)* %.omp.uncollapsed.ub2.ascast, align 4
  %cmp6 = icmp sle i32 %7, %8
  br i1 %cmp6, label %omp.uncollapsed.loop.body7, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body7:                       ; preds = %omp.uncollapsed.loop.cond5
  %9 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %10 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast, align 4
  %mul8 = mul nsw i32 %10, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, i32 addrspace(4)* %j.ascast, align 4
  %11 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %12 = load i32, i32 addrspace(4)* %j.ascast, align 4
  %mul10 = mul nsw i32 %11, %12
  store i32 %mul10, i32 addrspace(4)* %x.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.uncollapsed.loop.body7
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue
  %13 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast, align 4
  %add11 = add nsw i32 %13, 1
  store i32 %add11, i32 addrspace(4)* %.omp.uncollapsed.iv4.ascast, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond5
  br label %omp.uncollapsed.loop.inc12

omp.uncollapsed.loop.inc12:                       ; preds = %omp.uncollapsed.loop.end
  %14 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  %add13 = add nsw i32 %14, 1
  store i32 %add13, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end14:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 2054, i32 1839208, !"foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 9.0.0"}
