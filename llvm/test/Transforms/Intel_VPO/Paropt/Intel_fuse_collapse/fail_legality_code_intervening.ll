; REQUIRES: asserts

; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-fuse-collapse)' -vpo-paropt-loop-fuse-collapse --debug-only=vpo-paropt-loop-fuse-collapse -S %s 2>&1  | FileCheck %s

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
;                 a[i] = 123;
; #pragma omp loop
;                 for (j = 0; j < N+1; j++)
;                         b[i*(N+1)+j] = j*2;
;         }
; 
;         return 0;
; }

; This test checks that the pass backs off because of the intervening code between the loops
; and outputs correct debug message describing it

; CHECK: Candidate pairs fusion check failed

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64"
target device_triples = "spir64"

define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %a = alloca [4096 x i32], align 4
  %b = alloca [4100 x i32], align 4
  %N0 = alloca i32, align 4
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
  %tmp18 = alloca i32, align 4
  %.omp.iv19 = alloca i32, align 4
  %.omp.lb20 = alloca i32, align 4
  %.omp.ub21 = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %a.ascast = addrspacecast ptr %a to ptr addrspace(4)
  %b.ascast = addrspacecast ptr %b to ptr addrspace(4)
  %N0.ascast = addrspacecast ptr %N0 to ptr addrspace(4)
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
  %tmp18.ascast = addrspacecast ptr %tmp18 to ptr addrspace(4)
  %.omp.iv19.ascast = addrspacecast ptr %.omp.iv19 to ptr addrspace(4)
  %.omp.lb20.ascast = addrspacecast ptr %.omp.lb20 to ptr addrspace(4)
  %.omp.ub21.ascast = addrspacecast ptr %.omp.ub21 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 1024, ptr addrspace(4) %N0.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 3, ptr addrspace(4) %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 0
  %arrayidx1 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %a.ascast, ptr addrspace(4) %arrayidx, i64 16384, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %b.ascast, ptr addrspace(4) %arrayidx1, i64 16400, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb20.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub21.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp18.ascast, i32 0, i32 1) ]

  %array.begin50 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin51 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb20.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub21.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp18.ascast, i32 0, i32 1) ]

  %array.begin48 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin49 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb20.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub21.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp18.ascast, i32 0, i32 1) ]

  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body.lh, label %omp.inner.for.end46

omp.inner.for.body.lh:                            ; preds = %entry
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc42, %omp.inner.for.body.lh
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb4.ascast, align 4
  store i32 1023, ptr addrspace(4) %.omp.ub5.ascast, align 4
  %array.begin = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
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
  %14 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %mul11 = mul nsw i32 %13, %14
  %15 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add12 = add nsw i32 %mul11, %15
  %idxprom = sext i32 %add12 to i64
  %arrayidx13 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom
  store i32 %12, ptr addrspace(4) %arrayidx13, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %16 = load i32, ptr addrspace(4) %.omp.iv3.ascast, align 4
  %add14 = add nsw i32 %16, 1
  store i32 %add14, ptr addrspace(4) %.omp.iv3.ascast, align 4
  %17 = load i32, ptr addrspace(4) %.omp.iv3.ascast, align 4
  %18 = load i32, ptr addrspace(4) %.omp.ub5.ascast, align 4
  %cmp15 = icmp sle i32 %17, %18
  br i1 %cmp15, label %omp.inner.for.body8, label %omp.inner.for.end_crit_edge

omp.inner.for.end_crit_edge:                      ; preds = %omp.inner.for.inc
  br label %omp.inner.for.end

omp.inner.for.end:                                ; preds = %omp.inner.for.end_crit_edge, %omp.inner.for.body
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.GENERICLOOP"() ]

  %19 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom16 = sext i32 %19 to i64
  %arrayidx17 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom16
  store i32 123, ptr addrspace(4) %arrayidx17, align 4
  store i32 0, ptr addrspace(4) %.omp.lb20.ascast, align 4
  store i32 1024, ptr addrspace(4) %.omp.ub21.ascast, align 4
  %array.begin40 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %20 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv19.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb20.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub21.ascast, i32 0) ]

  %21 = load i32, ptr addrspace(4) %.omp.lb20.ascast, align 4
  store i32 %21, ptr addrspace(4) %.omp.iv19.ascast, align 4
  %22 = load i32, ptr addrspace(4) %.omp.iv19.ascast, align 4
  %23 = load i32, ptr addrspace(4) %.omp.ub21.ascast, align 4
  %cmp22 = icmp sle i32 %22, %23
  br i1 %cmp22, label %omp.inner.for.body.lh23, label %omp.inner.for.end38

omp.inner.for.body.lh23:                          ; preds = %omp.loop.exit
  br label %omp.inner.for.body24

omp.inner.for.body24:                             ; preds = %omp.inner.for.inc34, %omp.inner.for.body.lh23
  %24 = load i32, ptr addrspace(4) %.omp.iv19.ascast, align 4
  %mul25 = mul nsw i32 %24, 1
  %add26 = add nsw i32 0, %mul25
  store i32 %add26, ptr addrspace(4) %j.ascast, align 4
  %25 = load i32, ptr addrspace(4) %j.ascast, align 4
  %mul27 = mul nsw i32 %25, 2
  %26 = load i32, ptr addrspace(4) %i.ascast, align 4
  %27 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %add28 = add nsw i32 %27, 1
  %mul29 = mul nsw i32 %26, %add28
  %28 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add30 = add nsw i32 %mul29, %28
  %idxprom31 = sext i32 %add30 to i64
  %arrayidx32 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 %idxprom31
  store i32 %mul27, ptr addrspace(4) %arrayidx32, align 4
  br label %omp.body.continue33

omp.body.continue33:                              ; preds = %omp.inner.for.body24
  br label %omp.inner.for.inc34

omp.inner.for.inc34:                              ; preds = %omp.body.continue33
  %29 = load i32, ptr addrspace(4) %.omp.iv19.ascast, align 4
  %add35 = add nsw i32 %29, 1
  store i32 %add35, ptr addrspace(4) %.omp.iv19.ascast, align 4
  %30 = load i32, ptr addrspace(4) %.omp.iv19.ascast, align 4
  %31 = load i32, ptr addrspace(4) %.omp.ub21.ascast, align 4
  %cmp36 = icmp sle i32 %30, %31
  br i1 %cmp36, label %omp.inner.for.body24, label %omp.inner.for.end_crit_edge37

omp.inner.for.end_crit_edge37:                    ; preds = %omp.inner.for.inc34
  br label %omp.inner.for.end38

omp.inner.for.end38:                              ; preds = %omp.inner.for.end_crit_edge37, %omp.loop.exit
  br label %omp.loop.exit39

omp.loop.exit39:                                  ; preds = %omp.inner.for.end38
  call void @llvm.directive.region.exit(token %20) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.body.continue41

omp.body.continue41:                              ; preds = %omp.loop.exit39
  br label %omp.inner.for.inc42

omp.inner.for.inc42:                              ; preds = %omp.body.continue41
  %32 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add43 = add nsw i32 %32, 1
  store i32 %add43, ptr addrspace(4) %.omp.iv.ascast, align 4
  %33 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %34 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp44 = icmp sle i32 %33, %34
  br i1 %cmp44, label %omp.inner.for.body, label %omp.inner.for.end_crit_edge45

omp.inner.for.end_crit_edge45:                    ; preds = %omp.inner.for.inc42
  br label %omp.inner.for.end46

omp.inner.for.end46:                              ; preds = %omp.inner.for.end_crit_edge45, %entry
  br label %omp.loop.exit47

omp.loop.exit47:                                  ; preds = %omp.inner.for.end46
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "contains-openmp-target"="true" }


!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2050, i32 49129117, !"_Z4main", i32 9, i32 0, i32 0, i32 0}

