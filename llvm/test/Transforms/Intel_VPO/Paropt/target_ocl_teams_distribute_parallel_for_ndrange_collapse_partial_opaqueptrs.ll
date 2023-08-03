; RUN: opt -opaque-pointers=1 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck -check-prefixes=CHECK %s
; RUN: opt -opaque-pointers=1 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck -check-prefixes=CHECK %s
; RUN: opt -opaque-pointers=1 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck -check-prefixes=CHECK-POSTCOLLAPSE %s
; RUN: opt -opaque-pointers=1 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s | FileCheck -check-prefixes=CHECK-POSTCOLLAPSE %s

; RUN: opt -opaque-pointers=1 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck -check-prefixes=CHECK %s
; RUN: opt -opaque-pointers=1 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare)' -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck -check-prefixes=CHECK %s
; RUN: opt -opaque-pointers=1 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck -check-prefixes=CHECK-POSTCOLLAPSE-NONSPEC %s
; RUN: opt -opaque-pointers=1 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck -check-prefixes=CHECK-POSTCOLLAPSE-NONSPEC %s

; Original code:
;
; int main(void)
; {
;   int sum = 0;
;   #pragma omp target teams distribute parallel for reduction(+:sum) collapse(2)
;   for (int i = 0; i < 32; i++)
;     for (int j = 0; j < 64; j++)
;       for (int k = 0; k < 128; k++)
;         sum += k;
;   return 0;
; }

; Check that KNOWN_NDRANGE clause has the complete collapse flag unset
; and is removed later, and OFFLOAD_NDRANGE clause is removed too.
; Also check that it's never added when specific nd-range is disabled
; explicitly.
; CHECK-NOT: "QUAL.OMP.OFFLOAD.NDRANGE"({{.*}})
; CHECK-NOT: "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"({{.*}})
; CHECK-POSTCOLLAPSE:      "DIR.OMP.TARGET"()
; CHECK-POSTCOLLAPSE-SAME: "QUAL.OMP.OFFLOAD.NDRANGE"({{.*}})
; CHECK-POSTCOLLAPSE:      "DIR.OMP.DISTRIBUTE.PARLOOP"()
; CHECK-POSTCOLLAPSE-SAME: "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"(i1 false)
; CHECK-POSTCOLLAPSE-NONSPEC-NOT: "QUAL.OMP.OFFLOAD.NDRANGE"
; CHECK-POSTCOLLAPSE-NONSPEC-NOT: "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %sum = alloca i32, align 4
  %.omp.uncollapsed.lb = alloca i32, align 4
  %.omp.uncollapsed.ub = alloca i32, align 4
  %.omp.uncollapsed.lb1 = alloca i32, align 4
  %.omp.uncollapsed.ub2 = alloca i32, align 4
  %tmp = alloca i32, align 4
  %tmp3 = alloca i32, align 4
  %.omp.uncollapsed.iv = alloca i32, align 4
  %.omp.uncollapsed.iv4 = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %sum.ascast = addrspacecast ptr %sum to ptr addrspace(4)
  %.omp.uncollapsed.lb.ascast = addrspacecast ptr %.omp.uncollapsed.lb to ptr addrspace(4)
  %.omp.uncollapsed.ub.ascast = addrspacecast ptr %.omp.uncollapsed.ub to ptr addrspace(4)
  %.omp.uncollapsed.lb1.ascast = addrspacecast ptr %.omp.uncollapsed.lb1 to ptr addrspace(4)
  %.omp.uncollapsed.ub2.ascast = addrspacecast ptr %.omp.uncollapsed.ub2 to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %tmp3.ascast = addrspacecast ptr %tmp3 to ptr addrspace(4)
  %.omp.uncollapsed.iv.ascast = addrspacecast ptr %.omp.uncollapsed.iv to ptr addrspace(4)
  %.omp.uncollapsed.iv4.ascast = addrspacecast ptr %.omp.uncollapsed.iv4 to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %j.ascast = addrspacecast ptr %j to ptr addrspace(4)
  %k.ascast = addrspacecast ptr %k to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 0, ptr addrspace(4) %sum.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 4
  store i32 31, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.uncollapsed.lb1.ascast, align 4
  store i32 63, ptr addrspace(4) %.omp.uncollapsed.ub2.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum.ascast, ptr addrspace(4) %sum.ascast, i64 4, i64 547, ptr null, ptr null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb1.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp3.ascast, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb1.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp3.ascast, i32 0, i32 1) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, ptr addrspace(4) %.omp.uncollapsed.ub2.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1) ]

  %3 = load i32, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc13, %entry
  %4 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end15

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %6 = load i32, ptr addrspace(4) %.omp.uncollapsed.lb1.ascast, align 4
  store i32 %6, ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.cond5:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %7 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, align 4
  %8 = load i32, ptr addrspace(4) %.omp.uncollapsed.ub2.ascast, align 4
  %cmp6 = icmp sle i32 %7, %8
  br i1 %cmp6, label %omp.uncollapsed.loop.body7, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body7:                       ; preds = %omp.uncollapsed.loop.cond5
  %9 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %10 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, align 4
  %mul8 = mul nsw i32 %10, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, ptr addrspace(4) %j.ascast, align 4
  store i32 0, ptr addrspace(4) %k.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %omp.uncollapsed.loop.body7
  %11 = load i32, ptr addrspace(4) %k.ascast, align 4
  %cmp10 = icmp slt i32 %11, 128
  br i1 %cmp10, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %12 = load i32, ptr addrspace(4) %k.ascast, align 4
  %13 = load i32, ptr addrspace(4) %sum.ascast, align 4
  %add11 = add nsw i32 %13, %12
  store i32 %add11, ptr addrspace(4) %sum.ascast, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %14 = load i32, ptr addrspace(4) %k.ascast, align 4
  %inc = add nsw i32 %14, 1
  store i32 %inc, ptr addrspace(4) %k.ascast, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue
  %15 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, align 4
  %add12 = add nsw i32 %15, 1
  store i32 %add12, ptr addrspace(4) %.omp.uncollapsed.iv4.ascast, align 4
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond5
  br label %omp.uncollapsed.loop.inc13

omp.uncollapsed.loop.inc13:                       ; preds = %omp.uncollapsed.loop.end
  %16 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %add14 = add nsw i32 %16, 1
  store i32 %add14, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end15:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2050, i32 49102707, !"_Z4main", i32 4, i32 0, i32 0, i32 0}
