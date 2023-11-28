; REQUIRES: asserts

; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-fuse-collapse)' -vpo-paropt-loop-fuse-collapse --debug-only=vpo-paropt-loop-fuse-collapse -S %s 2>&1  | FileCheck --check-prefix=CHECK_DBG %s
; Original code:
; int main() {
;         static constexpr int N = 1024;
;         static constexpr int B = 4;
; 
;         int a[B*N], b[B*(N+1)];
; #pragma omp target teams loop map(tofrom:a[:B*N],b[:B*(N+1)])
;         for (int i = 0; i < B; i++) {
;                 int j;
; #pragma omp loop
;                 for (j = 0; j < N; j++)
;                         a[i*N+j] = j;
; #pragma omp loop
;                 for (j = 0; j < N+1; j++)
;                         b[i*(N+1)+j] = j*2;
;         }
; 
;         return 0;
; }

; The test checks that there're 2 fusion candidates picked
; when these loops are not guarded (i.e. don't have OMP lb<ub precondition
; emitted by the frontend)

; CHECK_DBG: OMP fusion candidate
; CHECK_DBG: OMP fusion candidate
; CHECK_DBG: Fusion safety checks passed

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %a = alloca [4096 x i32], align 4
  %b = alloca [4100 x i32], align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %tmp2 = alloca i32, align 4
  %.omp.iv3 = alloca i32, align 4
  %.omp.lb4 = alloca i32, align 4
  %.omp.ub5 = alloca i32, align 4
  %tmp16 = alloca i32, align 4
  %.omp.iv17 = alloca i32, align 4
  %.omp.lb18 = alloca i32, align 4
  %.omp.ub19 = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %a.ascast = addrspacecast ptr %a to ptr addrspace(4)
  %b.ascast = addrspacecast ptr %b to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %j.ascast = addrspacecast ptr %j to ptr addrspace(4)
  %tmp2.ascast = addrspacecast ptr %tmp2 to ptr addrspace(4)
  %.omp.iv3.ascast = addrspacecast ptr %.omp.iv3 to ptr addrspace(4)
  %.omp.lb4.ascast = addrspacecast ptr %.omp.lb4 to ptr addrspace(4)
  %.omp.ub5.ascast = addrspacecast ptr %.omp.ub5 to ptr addrspace(4)
  %tmp16.ascast = addrspacecast ptr %tmp16 to ptr addrspace(4)
  %.omp.iv17.ascast = addrspacecast ptr %.omp.iv17 to ptr addrspace(4)
  %.omp.lb18.ascast = addrspacecast ptr %.omp.lb18 to ptr addrspace(4)
  %.omp.ub19.ascast = addrspacecast ptr %.omp.ub19 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 3, ptr addrspace(4) %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 0
  %arrayidx1 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %a.ascast, ptr addrspace(4) %arrayidx, i64 16384, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %b.ascast, ptr addrspace(4) %arrayidx1, i64 16400, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv17.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb18.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp16.ascast, i32 0, i32 1) ]

  %array.begin47 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin48 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv17.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb18.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp16.ascast, i32 0, i32 1) ]

  %array.begin45 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin46 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv17.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb18.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp16.ascast, i32 0, i32 1) ]

  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body.lh, label %omp.inner.for.end43

omp.inner.for.body.lh:                            ; preds = %entry
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc39, %omp.inner.for.body.lh
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb4.ascast, align 4
  store i32 1023, ptr addrspace(4) %.omp.ub5.ascast, align 4
  %array.begin = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv3.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0) ]

  %8 = load i32, ptr addrspace(4) %.omp.lb4.ascast, align 4
  store i32 %8, ptr addrspace(4) %.omp.iv3.ascast, align 4
  %9 = load i32, ptr addrspace(4) %.omp.iv3.ascast, align 4
  %10 = load i32, ptr addrspace(4) %.omp.ub5.ascast, align 4
  %cmp6 = icmp sle i32 %9, %10
  br i1 %cmp6, label %omp.inner.for.body.lh7, label %omp.inner.for.end

omp.inner.for.body.lh7:                           ; preds = %omp.inner.for.body
  br label %omp.inner.for.body8

omp.inner.for.body8:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body.lh7
  %11 = load i32, ptr addrspace(4) %.omp.iv3.ascast, align 4
  %mul9 = mul nsw i32 %11, 1
  %add10 = add nsw i32 0, %mul9
  store i32 %add10, ptr addrspace(4) %j.ascast, align 4
  %12 = load i32, ptr addrspace(4) %j.ascast, align 4
  %13 = load i32, ptr addrspace(4) %i.ascast, align 4
  %mul11 = mul nsw i32 %13, 1024
  %14 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add12 = add nsw i32 %mul11, %14
  %idxprom = sext i32 %add12 to i64
  %arrayidx13 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom
  store i32 %12, ptr addrspace(4) %arrayidx13, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %15 = load i32, ptr addrspace(4) %.omp.iv3.ascast, align 4
  %add14 = add nsw i32 %15, 1
  store i32 %add14, ptr addrspace(4) %.omp.iv3.ascast, align 4
  %16 = load i32, ptr addrspace(4) %.omp.iv3.ascast, align 4
  %17 = load i32, ptr addrspace(4) %.omp.ub5.ascast, align 4
  %cmp15 = icmp sle i32 %16, %17
  br i1 %cmp15, label %omp.inner.for.body8, label %omp.inner.for.end_crit_edge

omp.inner.for.end_crit_edge:                      ; preds = %omp.inner.for.inc
  br label %omp.inner.for.end

omp.inner.for.end:                                ; preds = %omp.inner.for.end_crit_edge, %omp.inner.for.body
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.GENERICLOOP"() ]

  store i32 0, ptr addrspace(4) %.omp.lb18.ascast, align 4
  store i32 1024, ptr addrspace(4) %.omp.ub19.ascast, align 4
  %array.begin37 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %18 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv17.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb18.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub19.ascast, i32 0) ]

  %19 = load i32, ptr addrspace(4) %.omp.lb18.ascast, align 4
  store i32 %19, ptr addrspace(4) %.omp.iv17.ascast, align 4
  %20 = load i32, ptr addrspace(4) %.omp.iv17.ascast, align 4
  %21 = load i32, ptr addrspace(4) %.omp.ub19.ascast, align 4
  %cmp20 = icmp sle i32 %20, %21
  br i1 %cmp20, label %omp.inner.for.body.lh21, label %omp.inner.for.end35

omp.inner.for.body.lh21:                          ; preds = %omp.loop.exit
  br label %omp.inner.for.body22

omp.inner.for.body22:                             ; preds = %omp.inner.for.inc31, %omp.inner.for.body.lh21
  %22 = load i32, ptr addrspace(4) %.omp.iv17.ascast, align 4
  %mul23 = mul nsw i32 %22, 1
  %add24 = add nsw i32 0, %mul23
  store i32 %add24, ptr addrspace(4) %j.ascast, align 4
  %23 = load i32, ptr addrspace(4) %j.ascast, align 4
  %mul25 = mul nsw i32 %23, 2
  %24 = load i32, ptr addrspace(4) %i.ascast, align 4
  %mul26 = mul nsw i32 %24, 1025
  %25 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add27 = add nsw i32 %mul26, %25
  %idxprom28 = sext i32 %add27 to i64
  %arrayidx29 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 %idxprom28
  store i32 %mul25, ptr addrspace(4) %arrayidx29, align 4
  br label %omp.body.continue30

omp.body.continue30:                              ; preds = %omp.inner.for.body22
  br label %omp.inner.for.inc31

omp.inner.for.inc31:                              ; preds = %omp.body.continue30
  %26 = load i32, ptr addrspace(4) %.omp.iv17.ascast, align 4
  %add32 = add nsw i32 %26, 1
  store i32 %add32, ptr addrspace(4) %.omp.iv17.ascast, align 4
  %27 = load i32, ptr addrspace(4) %.omp.iv17.ascast, align 4
  %28 = load i32, ptr addrspace(4) %.omp.ub19.ascast, align 4
  %cmp33 = icmp sle i32 %27, %28
  br i1 %cmp33, label %omp.inner.for.body22, label %omp.inner.for.end_crit_edge34

omp.inner.for.end_crit_edge34:                    ; preds = %omp.inner.for.inc31
  br label %omp.inner.for.end35

omp.inner.for.end35:                              ; preds = %omp.inner.for.end_crit_edge34, %omp.loop.exit
  br label %omp.loop.exit36

omp.loop.exit36:                                  ; preds = %omp.inner.for.end35
  call void @llvm.directive.region.exit(token %18) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.body.continue38

omp.body.continue38:                              ; preds = %omp.loop.exit36
  br label %omp.inner.for.inc39

omp.inner.for.inc39:                              ; preds = %omp.body.continue38
  %29 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add40 = add nsw i32 %29, 1
  store i32 %add40, ptr addrspace(4) %.omp.iv.ascast, align 4
  %30 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %31 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp41 = icmp sle i32 %30, %31
  br i1 %cmp41, label %omp.inner.for.body, label %omp.inner.for.end_crit_edge42

omp.inner.for.end_crit_edge42:                    ; preds = %omp.inner.for.inc39
  br label %omp.inner.for.end43

omp.inner.for.end43:                              ; preds = %omp.inner.for.end_crit_edge42, %entry
  br label %omp.loop.exit44

omp.loop.exit44:                                  ; preds = %omp.inner.for.end43
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "contains-openmp-target"="true" }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2050, i32 49127144, !"_Z4main", i32 14, i32 0, i32 0, i32 0}

