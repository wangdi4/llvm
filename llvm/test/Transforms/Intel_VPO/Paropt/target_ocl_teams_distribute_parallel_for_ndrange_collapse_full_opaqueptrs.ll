; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck -check-prefixes=CHECK %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck -check-prefixes=CHECK %s
; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck -check-prefixes=CHECK-POSTCOLLAPSE %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s | FileCheck -check-prefixes=CHECK-POSTCOLLAPSE %s
; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -loop-rotate -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck -check-prefixes=CHECK %s
; RUN: opt -switch-to-offload -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck -check-prefixes=CHECK %s
; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -loop-rotate -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck -check-prefixes=CHECK-POSTCOLLAPSE %s
; RUN: opt -switch-to-offload -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s | FileCheck -check-prefixes=CHECK-POSTCOLLAPSE %s

; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck -check-prefixes=CHECK %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare)' -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck -check-prefixes=CHECK %s
; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck -check-prefixes=CHECK-POSTCOLLAPSE-NONSPEC %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -vpo-paropt-gpu-execution-scheme=0 -S %s | FileCheck -check-prefixes=CHECK-POSTCOLLAPSE-NONSPEC %s

; Original code:
;
; int main(void)
; {
;   int sum = 0;
;   #pragma omp target teams distribute parallel for reduction(+:sum) collapse(3)
;   for (int i = 0; i < 32; i++)
;     for (int j = 0; j < 64; j++)
;       for (int k = 0; k < 128; k++)
;         sum += k;
;   return 0;
; }

; Check that KNOWN_NDRANGE clause has the complete collapse flag set
; and is removed later, but OFFLOAD_NDRANGE clause is kept whatever
; gpu-execution-scheme is used.
; CHECK:      "DIR.OMP.TARGET"()
; CHECK-SAME: "QUAL.OMP.OFFLOAD.NDRANGE"({{.*}})
; CHECK-NOT:  "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"({{.*}})
; CHECK-POSTCOLLAPSE:      "DIR.OMP.TARGET"()
; CHECK-POSTCOLLAPSE-SAME: "QUAL.OMP.OFFLOAD.NDRANGE"({{.*}})
; CHECK-POSTCOLLAPSE:      "DIR.OMP.DISTRIBUTE.PARLOOP"()
; CHECK-POSTCOLLAPSE-SAME: "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"(i1 true)
; CHECK-POSTCOLLAPSE-NONSPEC:       "DIR.OMP.TARGET"()
; CHECK-POSTCOLLAPSE-NONSPEC-SAME:  "QUAL.OMP.OFFLOAD.NDRANGE"
; CHECK-POSTCOLLAPSE-NONSPEC-NOT:   "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define protected noundef i32 @main() {
entry:
  %retval = alloca i32, align 4
  %sum = alloca i32, align 4
  %.omp.uncollapsed.lb = alloca i32, align 4
  %.omp.uncollapsed.ub = alloca i32, align 4
  %.omp.uncollapsed.lb1 = alloca i32, align 4
  %.omp.uncollapsed.ub2 = alloca i32, align 4
  %.omp.uncollapsed.lb3 = alloca i32, align 4
  %.omp.uncollapsed.ub4 = alloca i32, align 4
  %tmp = alloca i32, align 4
  %tmp5 = alloca i32, align 4
  %tmp6 = alloca i32, align 4
  %.omp.uncollapsed.iv = alloca i32, align 4
  %.omp.uncollapsed.iv7 = alloca i32, align 4
  %.omp.uncollapsed.iv8 = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %sum.ascast = addrspacecast ptr %sum to ptr addrspace(4)
  %.omp.uncollapsed.lb.ascast = addrspacecast ptr %.omp.uncollapsed.lb to ptr addrspace(4)
  %.omp.uncollapsed.ub.ascast = addrspacecast ptr %.omp.uncollapsed.ub to ptr addrspace(4)
  %.omp.uncollapsed.lb1.ascast = addrspacecast ptr %.omp.uncollapsed.lb1 to ptr addrspace(4)
  %.omp.uncollapsed.ub2.ascast = addrspacecast ptr %.omp.uncollapsed.ub2 to ptr addrspace(4)
  %.omp.uncollapsed.lb3.ascast = addrspacecast ptr %.omp.uncollapsed.lb3 to ptr addrspace(4)
  %.omp.uncollapsed.ub4.ascast = addrspacecast ptr %.omp.uncollapsed.ub4 to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %tmp5.ascast = addrspacecast ptr %tmp5 to ptr addrspace(4)
  %tmp6.ascast = addrspacecast ptr %tmp6 to ptr addrspace(4)
  %.omp.uncollapsed.iv.ascast = addrspacecast ptr %.omp.uncollapsed.iv to ptr addrspace(4)
  %.omp.uncollapsed.iv7.ascast = addrspacecast ptr %.omp.uncollapsed.iv7 to ptr addrspace(4)
  %.omp.uncollapsed.iv8.ascast = addrspacecast ptr %.omp.uncollapsed.iv8 to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %j.ascast = addrspacecast ptr %j to ptr addrspace(4)
  %k.ascast = addrspacecast ptr %k to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 0, ptr addrspace(4) %sum.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 4
  store i32 31, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.uncollapsed.lb1.ascast, align 4
  store i32 63, ptr addrspace(4) %.omp.uncollapsed.ub2.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.uncollapsed.lb3.ascast, align 4
  store i32 127, ptr addrspace(4) %.omp.uncollapsed.ub4.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum.ascast, ptr addrspace(4) %sum.ascast, i64 4, i64 547, ptr null, ptr null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv7.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb1.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv8.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb3.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp6.ascast, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv7.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb1.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv8.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb3.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp6.ascast, i32 0, i32 1) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.COLLAPSE"(i32 3),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i32 0, ptr addrspace(4) %.omp.uncollapsed.iv7.ascast, i32 0, ptr addrspace(4) %.omp.uncollapsed.iv8.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i32 0, ptr addrspace(4) %.omp.uncollapsed.ub2.ascast, i32 0, ptr addrspace(4) %.omp.uncollapsed.ub4.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1) ]

  %3 = load i32, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc24, %entry
  %4 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end26

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %6 = load i32, ptr addrspace(4) %.omp.uncollapsed.lb1.ascast, align 4
  store i32 %6, ptr addrspace(4) %.omp.uncollapsed.iv7.ascast, align 4
  br label %omp.uncollapsed.loop.cond9

omp.uncollapsed.loop.cond9:                       ; preds = %omp.uncollapsed.loop.inc21, %omp.uncollapsed.loop.body
  %7 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv7.ascast, align 4
  %8 = load i32, ptr addrspace(4) %.omp.uncollapsed.ub2.ascast, align 4
  %cmp10 = icmp sle i32 %7, %8
  br i1 %cmp10, label %omp.uncollapsed.loop.body11, label %omp.uncollapsed.loop.end23

omp.uncollapsed.loop.body11:                      ; preds = %omp.uncollapsed.loop.cond9
  %9 = load i32, ptr addrspace(4) %.omp.uncollapsed.lb3.ascast, align 4
  store i32 %9, ptr addrspace(4) %.omp.uncollapsed.iv8.ascast, align 4
  br label %omp.uncollapsed.loop.cond12

omp.uncollapsed.loop.cond12:                      ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body11
  %10 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv8.ascast, align 4
  %11 = load i32, ptr addrspace(4) %.omp.uncollapsed.ub4.ascast, align 4
  %cmp13 = icmp sle i32 %10, %11
  br i1 %cmp13, label %omp.uncollapsed.loop.body14, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body14:                      ; preds = %omp.uncollapsed.loop.cond12
  %12 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %mul = mul nsw i32 %12, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %13 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv7.ascast, align 4
  %mul15 = mul nsw i32 %13, 1
  %add16 = add nsw i32 0, %mul15
  store i32 %add16, ptr addrspace(4) %j.ascast, align 4
  %14 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv8.ascast, align 4
  %mul17 = mul nsw i32 %14, 1
  %add18 = add nsw i32 0, %mul17
  store i32 %add18, ptr addrspace(4) %k.ascast, align 4
  %15 = load i32, ptr addrspace(4) %k.ascast, align 4
  %16 = load i32, ptr addrspace(4) %sum.ascast, align 4
  %add19 = add nsw i32 %16, %15
  store i32 %add19, ptr addrspace(4) %sum.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.uncollapsed.loop.body14
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue
  %17 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv8.ascast, align 4
  %add20 = add nsw i32 %17, 1
  store i32 %add20, ptr addrspace(4) %.omp.uncollapsed.iv8.ascast, align 4
  br label %omp.uncollapsed.loop.cond12

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond12
  br label %omp.uncollapsed.loop.inc21

omp.uncollapsed.loop.inc21:                       ; preds = %omp.uncollapsed.loop.end
  %18 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv7.ascast, align 4
  %add22 = add nsw i32 %18, 1
  store i32 %add22, ptr addrspace(4) %.omp.uncollapsed.iv7.ascast, align 4
  br label %omp.uncollapsed.loop.cond9

omp.uncollapsed.loop.end23:                       ; preds = %omp.uncollapsed.loop.cond9
  br label %omp.uncollapsed.loop.inc24

omp.uncollapsed.loop.inc24:                       ; preds = %omp.uncollapsed.loop.end23
  %19 = load i32, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  %add25 = add nsw i32 %19, 1
  store i32 %add25, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end26:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2050, i32 49101565, !"_Z4main", i32 4, i32 0, i32 0, i32 0}
