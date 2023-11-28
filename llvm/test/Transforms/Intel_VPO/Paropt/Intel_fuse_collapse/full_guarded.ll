; REQUIRES: asserts

; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-fuse-collapse)' -vpo-paropt-loop-fuse-collapse --debug-only=vpo-paropt-loop-fuse-collapse -S %s 2>&1  | FileCheck --check-prefix=CHECK_DBG %s

; Original code:
; int main() {
;         static constexpr int N = 1024;
;         static constexpr int B = 4;
; 
;         int a[B*N], b[B*(N+1)];
;         int N0 = N;
; #pragma omp target teams loop map(tofrom:a[:B*N],b[:B*(N+1)])
;         for (int i = 0; i < B; i++) {
;                 int j;
; #pragma omp loop
;                 for (j = 0; j < N0; j++)
;                         a[i*N0+j] = j;
; #pragma omp loop
;                 for (j = 0; j < N0+1; j++)
;                         b[i*(N0+1)+j] = j*2;
;         }
; 
;         return 0;
; }

; The test checks that there're 2 fusion candidates picked
; when these loops are guarded (i.e. have OMP lb<ub precondition
; emitted by the frontend)

; CHECK_DBG: OMP fusion candidate
; CHECK_DBG: OMP fusion candidate
; CHECK_DBG: Fusion safety checks passed

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
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv7 = alloca i32, align 4
  %.omp.lb8 = alloca i32, align 4
  %.omp.ub9 = alloca i32, align 4
  %tmp20 = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i32, align 4
  %.omp.iv29 = alloca i32, align 4
  %.omp.lb30 = alloca i32, align 4
  %.omp.ub31 = alloca i32, align 4
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
  %.capture_expr.0.ascast = addrspacecast ptr %.capture_expr.0 to ptr addrspace(4)
  %.capture_expr.1.ascast = addrspacecast ptr %.capture_expr.1 to ptr addrspace(4)
  %.omp.iv7.ascast = addrspacecast ptr %.omp.iv7 to ptr addrspace(4)
  %.omp.lb8.ascast = addrspacecast ptr %.omp.lb8 to ptr addrspace(4)
  %.omp.ub9.ascast = addrspacecast ptr %.omp.ub9 to ptr addrspace(4)
  %tmp20.ascast = addrspacecast ptr %tmp20 to ptr addrspace(4)
  %.capture_expr.2.ascast = addrspacecast ptr %.capture_expr.2 to ptr addrspace(4)
  %.capture_expr.3.ascast = addrspacecast ptr %.capture_expr.3 to ptr addrspace(4)
  %.omp.iv29.ascast = addrspacecast ptr %.omp.iv29 to ptr addrspace(4)
  %.omp.lb30.ascast = addrspacecast ptr %.omp.lb30 to ptr addrspace(4)
  %.omp.ub31.ascast = addrspacecast ptr %.omp.ub31 to ptr addrspace(4)
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
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv7.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb8.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv29.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb30.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub31.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp20.ascast, i32 0, i32 1) ]

  %array.begin61 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin62 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv7.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb8.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv29.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb30.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub31.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp20.ascast, i32 0, i32 1) ]

  %array.begin59 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin60 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv7.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb8.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv29.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb30.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub31.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp20.ascast, i32 0, i32 1) ]

  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body.lh, label %omp.inner.for.end57

omp.inner.for.body.lh:                            ; preds = %entry
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc53, %omp.inner.for.body.lh
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %7 = load i32, ptr addrspace(4) %N0.ascast, align 4
  store i32 %7, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %8 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %sub = sub nsw i32 %8, 0
  %sub3 = sub nsw i32 %sub, 1
  %add4 = add nsw i32 %sub3, 1
  %div = sdiv i32 %add4, 1
  %sub5 = sub nsw i32 %div, 1
  store i32 %sub5, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  %9 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %cmp6 = icmp slt i32 0, %9
  br i1 %cmp6, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %omp.inner.for.body
  store i32 0, ptr addrspace(4) %.omp.lb8.ascast, align 4
  %10 = load i32, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  store i32 %10, ptr addrspace(4) %.omp.ub9.ascast, align 4
  %array.begin = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv7.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb8.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub9.ascast, i32 0) ]

  %12 = load i32, ptr addrspace(4) %.omp.lb8.ascast, align 4
  store i32 %12, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %13 = load i32, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %14 = load i32, ptr addrspace(4) %.omp.ub9.ascast, align 4
  %cmp10 = icmp sle i32 %13, %14
  br i1 %cmp10, label %omp.inner.for.body.lh11, label %omp.inner.for.end

omp.inner.for.body.lh11:                          ; preds = %omp.precond.then
  br label %omp.inner.for.body12

omp.inner.for.body12:                             ; preds = %omp.inner.for.inc, %omp.inner.for.body.lh11
  %15 = load i32, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %mul13 = mul nsw i32 %15, 1
  %add14 = add nsw i32 0, %mul13
  store i32 %add14, ptr addrspace(4) %j.ascast, align 4
  %16 = load i32, ptr addrspace(4) %j.ascast, align 4
  %17 = load i32, ptr addrspace(4) %i.ascast, align 4
  %18 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %mul15 = mul nsw i32 %17, %18
  %19 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add16 = add nsw i32 %mul15, %19
  %idxprom = sext i32 %add16 to i64
  %arrayidx17 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom
  store i32 %16, ptr addrspace(4) %arrayidx17, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body12
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %20 = load i32, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %add18 = add nsw i32 %20, 1
  store i32 %add18, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %21 = load i32, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %22 = load i32, ptr addrspace(4) %.omp.ub9.ascast, align 4
  %cmp19 = icmp sle i32 %21, %22
  br i1 %cmp19, label %omp.inner.for.body12, label %omp.inner.for.end_crit_edge

omp.inner.for.end_crit_edge:                      ; preds = %omp.inner.for.inc
  br label %omp.inner.for.end

omp.inner.for.end:                                ; preds = %omp.inner.for.end_crit_edge, %omp.precond.then
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %omp.inner.for.body
  %23 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %add21 = add nsw i32 %23, 1
  store i32 %add21, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %24 = load i32, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %sub22 = sub nsw i32 %24, 0
  %sub23 = sub nsw i32 %sub22, 1
  %add24 = add nsw i32 %sub23, 1
  %div25 = sdiv i32 %add24, 1
  %sub26 = sub nsw i32 %div25, 1
  store i32 %sub26, ptr addrspace(4) %.capture_expr.3.ascast, align 4
  %25 = load i32, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %cmp27 = icmp slt i32 0, %25
  br i1 %cmp27, label %omp.precond.then28, label %omp.precond.end51

omp.precond.then28:                               ; preds = %omp.precond.end
  store i32 0, ptr addrspace(4) %.omp.lb30.ascast, align 4
  %26 = load i32, ptr addrspace(4) %.capture_expr.3.ascast, align 4
  store i32 %26, ptr addrspace(4) %.omp.ub31.ascast, align 4
  %array.begin50 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %27 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv29.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb30.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub31.ascast, i32 0) ]

  %28 = load i32, ptr addrspace(4) %.omp.lb30.ascast, align 4
  store i32 %28, ptr addrspace(4) %.omp.iv29.ascast, align 4
  %29 = load i32, ptr addrspace(4) %.omp.iv29.ascast, align 4
  %30 = load i32, ptr addrspace(4) %.omp.ub31.ascast, align 4
  %cmp32 = icmp sle i32 %29, %30
  br i1 %cmp32, label %omp.inner.for.body.lh33, label %omp.inner.for.end48

omp.inner.for.body.lh33:                          ; preds = %omp.precond.then28
  br label %omp.inner.for.body34

omp.inner.for.body34:                             ; preds = %omp.inner.for.inc44, %omp.inner.for.body.lh33
  %31 = load i32, ptr addrspace(4) %.omp.iv29.ascast, align 4
  %mul35 = mul nsw i32 %31, 1
  %add36 = add nsw i32 0, %mul35
  store i32 %add36, ptr addrspace(4) %j.ascast, align 4
  %32 = load i32, ptr addrspace(4) %j.ascast, align 4
  %mul37 = mul nsw i32 %32, 2
  %33 = load i32, ptr addrspace(4) %i.ascast, align 4
  %34 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %add38 = add nsw i32 %34, 1
  %mul39 = mul nsw i32 %33, %add38
  %35 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add40 = add nsw i32 %mul39, %35
  %idxprom41 = sext i32 %add40 to i64
  %arrayidx42 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 %idxprom41
  store i32 %mul37, ptr addrspace(4) %arrayidx42, align 4
  br label %omp.body.continue43

omp.body.continue43:                              ; preds = %omp.inner.for.body34
  br label %omp.inner.for.inc44

omp.inner.for.inc44:                              ; preds = %omp.body.continue43
  %36 = load i32, ptr addrspace(4) %.omp.iv29.ascast, align 4
  %add45 = add nsw i32 %36, 1
  store i32 %add45, ptr addrspace(4) %.omp.iv29.ascast, align 4
  %37 = load i32, ptr addrspace(4) %.omp.iv29.ascast, align 4
  %38 = load i32, ptr addrspace(4) %.omp.ub31.ascast, align 4
  %cmp46 = icmp sle i32 %37, %38
  br i1 %cmp46, label %omp.inner.for.body34, label %omp.inner.for.end_crit_edge47

omp.inner.for.end_crit_edge47:                    ; preds = %omp.inner.for.inc44
  br label %omp.inner.for.end48

omp.inner.for.end48:                              ; preds = %omp.inner.for.end_crit_edge47, %omp.precond.then28
  br label %omp.loop.exit49

omp.loop.exit49:                                  ; preds = %omp.inner.for.end48
  call void @llvm.directive.region.exit(token %27) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.precond.end51

omp.precond.end51:                                ; preds = %omp.loop.exit49, %omp.precond.end
  br label %omp.body.continue52

omp.body.continue52:                              ; preds = %omp.precond.end51
  br label %omp.inner.for.inc53

omp.inner.for.inc53:                              ; preds = %omp.body.continue52
  %39 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add54 = add nsw i32 %39, 1
  store i32 %add54, ptr addrspace(4) %.omp.iv.ascast, align 4
  %40 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %41 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp55 = icmp sle i32 %40, %41
  br i1 %cmp55, label %omp.inner.for.body, label %omp.inner.for.end_crit_edge56

omp.inner.for.end_crit_edge56:                    ; preds = %omp.inner.for.inc53
  br label %omp.inner.for.end57

omp.inner.for.end57:                              ; preds = %omp.inner.for.end_crit_edge56, %entry
  br label %omp.loop.exit58

omp.loop.exit58:                                  ; preds = %omp.inner.for.end57
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "contains-openmp-target"="true" }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2050, i32 49129119, !"_Z4main", i32 14, i32 0, i32 0, i32 0}

