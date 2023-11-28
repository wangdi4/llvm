; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt-config=%S/Inputs/Intel_paropt_ndrange_heuristic.yaml -switch-to-offload -S %s | FileCheck %s
; RUN: opt -passes='require<vpo-paropt-config-analysis>,function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare)' -vpo-paropt-config=%S/Inputs/Intel_paropt_ndrange_heuristic.yaml -switch-to-offload -S %s | FileCheck %s

; Test src:
;int main(void)
;{
;  int sum = 0;
;  #pragma omp target teams distribute parallel for reduction(+:sum)
;  for (int i = 0; i < 32; i++)
;    for (int j = 0; j < 64; j++)
;      sum += j;
;  #pragma omp target teams distribute parallel for reduction(+:sum)
;  for (int i = 0; i < 32; i++)
;    for (int j = 0; j < 64; j++)
;      sum += j;
;  #pragma omp target teams distribute parallel for reduction(+:sum)
;  for (int i = 0; i < 32; i++)
;    for (int j = 0; j < 64; j++)
;      sum += j;
;  return 0;
;}

; Check that a loop satisfying ndrange tripcount heuristic conditions
; is processed differently based on heuristic enablement indicator
; passed via ParoptConfig

; CHECK-NOT: "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"
; CHECK-NOT: "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), {{.*}}, "QUAL.OMP.OFFLOAD.NDRANGE"
; CHECK: "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1), {{.*}}, "QUAL.OMP.OFFLOAD.NDRANGE"
; CHECK-NOT: "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2), {{.*}}, "QUAL.OMP.OFFLOAD.NDRANGE"

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %sum = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %.omp.lb4 = alloca i32, align 4
  %.omp.ub5 = alloca i32, align 4
  %tmp6 = alloca i32, align 4
  %.omp.iv7 = alloca i32, align 4
  %i11 = alloca i32, align 4
  %j14 = alloca i32, align 4
  %.omp.lb27 = alloca i32, align 4
  %.omp.ub28 = alloca i32, align 4
  %tmp29 = alloca i32, align 4
  %.omp.iv30 = alloca i32, align 4
  %i34 = alloca i32, align 4
  %j37 = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %sum.ascast = addrspacecast ptr %sum to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %j.ascast = addrspacecast ptr %j to ptr addrspace(4)
  %.omp.lb4.ascast = addrspacecast ptr %.omp.lb4 to ptr addrspace(4)
  %.omp.ub5.ascast = addrspacecast ptr %.omp.ub5 to ptr addrspace(4)
  %tmp6.ascast = addrspacecast ptr %tmp6 to ptr addrspace(4)
  %.omp.iv7.ascast = addrspacecast ptr %.omp.iv7 to ptr addrspace(4)
  %i11.ascast = addrspacecast ptr %i11 to ptr addrspace(4)
  %j14.ascast = addrspacecast ptr %j14 to ptr addrspace(4)
  %.omp.lb27.ascast = addrspacecast ptr %.omp.lb27 to ptr addrspace(4)
  %.omp.ub28.ascast = addrspacecast ptr %.omp.ub28 to ptr addrspace(4)
  %tmp29.ascast = addrspacecast ptr %tmp29 to ptr addrspace(4)
  %.omp.iv30.ascast = addrspacecast ptr %.omp.iv30 to ptr addrspace(4)
  %i34.ascast = addrspacecast ptr %i34 to ptr addrspace(4)
  %j37.ascast = addrspacecast ptr %j37 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 0, ptr addrspace(4) %sum.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 31, ptr addrspace(4) %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum.ascast, ptr addrspace(4) %sum.ascast, i64 4, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1) | UNKNOWN (0x8000)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1) ]

  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  store i32 0, ptr addrspace(4) %j.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %omp.inner.for.body
  %7 = load i32, ptr addrspace(4) %j.ascast, align 4
  %cmp1 = icmp slt i32 %7, 64
  br i1 %cmp1, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %8 = load i32, ptr addrspace(4) %j.ascast, align 4
  %9 = load i32, ptr addrspace(4) %sum.ascast, align 4
  %add2 = add nsw i32 %9, %8
  store i32 %add2, ptr addrspace(4) %sum.ascast, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %10 = load i32, ptr addrspace(4) %j.ascast, align 4
  %inc = add nsw i32 %10, 1
  store i32 %inc, ptr addrspace(4) %j.ascast, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add3 = add nsw i32 %11, 1
  store i32 %add3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  store i32 0, ptr addrspace(4) %.omp.lb4.ascast, align 4
  store i32 31, ptr addrspace(4) %.omp.ub5.ascast, align 4
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum.ascast, ptr addrspace(4) %sum.ascast, i64 4, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1) | UNKNOWN (0x8000)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv7.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i11.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j14.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp6.ascast, i32 0, i32 1) ]

  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv7.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i11.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j14.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp6.ascast, i32 0, i32 1) ]

  %14 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv7.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i11.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j14.ascast, i32 0, i32 1) ]

  %15 = load i32, ptr addrspace(4) %.omp.lb4.ascast, align 4
  store i32 %15, ptr addrspace(4) %.omp.iv7.ascast, align 4
  br label %omp.inner.for.cond8

omp.inner.for.cond8:                              ; preds = %omp.inner.for.inc23, %omp.loop.exit
  %16 = load i32, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %17 = load i32, ptr addrspace(4) %.omp.ub5.ascast, align 4
  %cmp9 = icmp sle i32 %16, %17
  br i1 %cmp9, label %omp.inner.for.body10, label %omp.inner.for.end25

omp.inner.for.body10:                             ; preds = %omp.inner.for.cond8
  %18 = load i32, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %mul12 = mul nsw i32 %18, 1
  %add13 = add nsw i32 0, %mul12
  store i32 %add13, ptr addrspace(4) %i11.ascast, align 4
  store i32 0, ptr addrspace(4) %j14.ascast, align 4
  br label %for.cond15

for.cond15:                                       ; preds = %for.inc19, %omp.inner.for.body10
  %19 = load i32, ptr addrspace(4) %j14.ascast, align 4
  %cmp16 = icmp slt i32 %19, 64
  br i1 %cmp16, label %for.body17, label %for.end21

for.body17:                                       ; preds = %for.cond15
  %20 = load i32, ptr addrspace(4) %j14.ascast, align 4
  %21 = load i32, ptr addrspace(4) %sum.ascast, align 4
  %add18 = add nsw i32 %21, %20
  store i32 %add18, ptr addrspace(4) %sum.ascast, align 4
  br label %for.inc19

for.inc19:                                        ; preds = %for.body17
  %22 = load i32, ptr addrspace(4) %j14.ascast, align 4
  %inc20 = add nsw i32 %22, 1
  store i32 %inc20, ptr addrspace(4) %j14.ascast, align 4
  br label %for.cond15

for.end21:                                        ; preds = %for.cond15
  br label %omp.body.continue22

omp.body.continue22:                              ; preds = %for.end21
  br label %omp.inner.for.inc23

omp.inner.for.inc23:                              ; preds = %omp.body.continue22
  %23 = load i32, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %add24 = add nsw i32 %23, 1
  store i32 %add24, ptr addrspace(4) %.omp.iv7.ascast, align 4
  br label %omp.inner.for.cond8

omp.inner.for.end25:                              ; preds = %omp.inner.for.cond8
  br label %omp.loop.exit26

omp.loop.exit26:                                  ; preds = %omp.inner.for.end25
  call void @llvm.directive.region.exit(token %14) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.TARGET"() ]

  store i32 0, ptr addrspace(4) %.omp.lb27.ascast, align 4
  store i32 31, ptr addrspace(4) %.omp.ub28.ascast, align 4
  %24 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum.ascast, ptr addrspace(4) %sum.ascast, i64 4, i64 33315, ptr null, ptr null), ; MAP type: 33315 = 0x8223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1) | UNKNOWN (0x8000)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv30.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb27.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub28.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i34.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j37.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp29.ascast, i32 0, i32 1) ]

  %25 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv30.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb27.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub28.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i34.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j37.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp29.ascast, i32 0, i32 1) ]

  %26 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv30.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb27.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub28.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i34.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j37.ascast, i32 0, i32 1) ]

  %27 = load i32, ptr addrspace(4) %.omp.lb27.ascast, align 4
  store i32 %27, ptr addrspace(4) %.omp.iv30.ascast, align 4
  br label %omp.inner.for.cond31

omp.inner.for.cond31:                             ; preds = %omp.inner.for.inc46, %omp.loop.exit26
  %28 = load i32, ptr addrspace(4) %.omp.iv30.ascast, align 4
  %29 = load i32, ptr addrspace(4) %.omp.ub28.ascast, align 4
  %cmp32 = icmp sle i32 %28, %29
  br i1 %cmp32, label %omp.inner.for.body33, label %omp.inner.for.end48

omp.inner.for.body33:                             ; preds = %omp.inner.for.cond31
  %30 = load i32, ptr addrspace(4) %.omp.iv30.ascast, align 4
  %mul35 = mul nsw i32 %30, 1
  %add36 = add nsw i32 0, %mul35
  store i32 %add36, ptr addrspace(4) %i34.ascast, align 4
  store i32 0, ptr addrspace(4) %j37.ascast, align 4
  br label %for.cond38

for.cond38:                                       ; preds = %for.inc42, %omp.inner.for.body33
  %31 = load i32, ptr addrspace(4) %j37.ascast, align 4
  %cmp39 = icmp slt i32 %31, 64
  br i1 %cmp39, label %for.body40, label %for.end44

for.body40:                                       ; preds = %for.cond38
  %32 = load i32, ptr addrspace(4) %j37.ascast, align 4
  %33 = load i32, ptr addrspace(4) %sum.ascast, align 4
  %add41 = add nsw i32 %33, %32
  store i32 %add41, ptr addrspace(4) %sum.ascast, align 4
  br label %for.inc42

for.inc42:                                        ; preds = %for.body40
  %34 = load i32, ptr addrspace(4) %j37.ascast, align 4
  %inc43 = add nsw i32 %34, 1
  store i32 %inc43, ptr addrspace(4) %j37.ascast, align 4
  br label %for.cond38

for.end44:                                        ; preds = %for.cond38
  br label %omp.body.continue45

omp.body.continue45:                              ; preds = %for.end44
  br label %omp.inner.for.inc46

omp.inner.for.inc46:                              ; preds = %omp.body.continue45
  %35 = load i32, ptr addrspace(4) %.omp.iv30.ascast, align 4
  %add47 = add nsw i32 %35, 1
  store i32 %add47, ptr addrspace(4) %.omp.iv30.ascast, align 4
  br label %omp.inner.for.cond31

omp.inner.for.end48:                              ; preds = %omp.inner.for.cond31
  br label %omp.loop.exit49

omp.loop.exit49:                                  ; preds = %omp.inner.for.end48
  call void @llvm.directive.region.exit(token %26) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %25) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %24) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "contains-openmp-target"="true" }

!omp_offload.info = !{!0, !1, !2}

!0 = !{i32 0, i32 2050, i32 49102707, !"_Z4main", i32 4, i32 0, i32 0, i32 0}
!1 = !{i32 0, i32 2050, i32 49102707, !"_Z4main", i32 8, i32 0, i32 1, i32 0}
!2 = !{i32 0, i32 2050, i32 49102707, !"_Z4main", i32 12, i32 0, i32 2, i32 0}
