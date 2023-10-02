; REQUIRES: asserts

; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-fuse-collapse)' -vpo-paropt-loop-fuse-collapse --debug-only=vpo-paropt-loop-fuse-collapse -S %s 2>&1  | FileCheck %s

; The test checks that fusion candidates collection failed due to
; the wrong number of child loops

; Original code:
; int main() {
;         static constexpr int N = 1024;
;         static constexpr int B = 4;
; 
;         int a[B*N], b[B*N], c[B*N];
;         int N0 = N;
; #pragma omp target teams loop map(tofrom:a[:B*N],b[:B*N],c[:B*N])
;         for (int i = 0; i < B; i++) {
;                 int j;
; #pragma omp loop
;                 for (j = 0; j < N0; j++)
;                         a[i*N0+j] = j;
; #pragma omp loop
;                 for (j = 0; j < N0; j++)
;                         b[i*N0+j] = j*2;
; #pragma omp loop
;                 for (j = 0; j < N0; j++)
;                         c[i*N0+j] = j*3;
;         }
; 
;         return 0;
; }

; CHECK: >2  candidates found, exiting

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64"
target device_triples = "spir64"

define protected noundef i32 @main() {
entry:
  %retval = alloca i32, align 4
  %a = alloca [4096 x i32], align 4
  %b = alloca [4096 x i32], align 4
  %c = alloca [4096 x i32], align 4
  %N0 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %tmp3 = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv8 = alloca i32, align 4
  %.omp.lb9 = alloca i32, align 4
  %.omp.ub10 = alloca i32, align 4
  %tmp20 = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i32, align 4
  %.omp.iv28 = alloca i32, align 4
  %.omp.lb29 = alloca i32, align 4
  %.omp.ub30 = alloca i32, align 4
  %tmp48 = alloca i32, align 4
  %.capture_expr.4 = alloca i32, align 4
  %.capture_expr.5 = alloca i32, align 4
  %.omp.iv56 = alloca i32, align 4
  %.omp.lb57 = alloca i32, align 4
  %.omp.ub58 = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %a.ascast = addrspacecast ptr %a to ptr addrspace(4)
  %b.ascast = addrspacecast ptr %b to ptr addrspace(4)
  %c.ascast = addrspacecast ptr %c to ptr addrspace(4)
  %N0.ascast = addrspacecast ptr %N0 to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %j.ascast = addrspacecast ptr %j to ptr addrspace(4)
  %tmp3.ascast = addrspacecast ptr %tmp3 to ptr addrspace(4)
  %.capture_expr.0.ascast = addrspacecast ptr %.capture_expr.0 to ptr addrspace(4)
  %.capture_expr.1.ascast = addrspacecast ptr %.capture_expr.1 to ptr addrspace(4)
  %.omp.iv8.ascast = addrspacecast ptr %.omp.iv8 to ptr addrspace(4)
  %.omp.lb9.ascast = addrspacecast ptr %.omp.lb9 to ptr addrspace(4)
  %.omp.ub10.ascast = addrspacecast ptr %.omp.ub10 to ptr addrspace(4)
  %tmp20.ascast = addrspacecast ptr %tmp20 to ptr addrspace(4)
  %.capture_expr.2.ascast = addrspacecast ptr %.capture_expr.2 to ptr addrspace(4)
  %.capture_expr.3.ascast = addrspacecast ptr %.capture_expr.3 to ptr addrspace(4)
  %.omp.iv28.ascast = addrspacecast ptr %.omp.iv28 to ptr addrspace(4)
  %.omp.lb29.ascast = addrspacecast ptr %.omp.lb29 to ptr addrspace(4)
  %.omp.ub30.ascast = addrspacecast ptr %.omp.ub30 to ptr addrspace(4)
  %tmp48.ascast = addrspacecast ptr %tmp48 to ptr addrspace(4)
  %.capture_expr.4.ascast = addrspacecast ptr %.capture_expr.4 to ptr addrspace(4)
  %.capture_expr.5.ascast = addrspacecast ptr %.capture_expr.5 to ptr addrspace(4)
  %.omp.iv56.ascast = addrspacecast ptr %.omp.iv56 to ptr addrspace(4)
  %.omp.lb57.ascast = addrspacecast ptr %.omp.lb57 to ptr addrspace(4)
  %.omp.ub58.ascast = addrspacecast ptr %.omp.ub58 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 1024, ptr addrspace(4) %N0.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 3, ptr addrspace(4) %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 0
  %arrayidx1 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 0
  %arrayidx2 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %c.ascast, i64 0, i64 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %a.ascast, ptr addrspace(4) %arrayidx, i64 16384, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %b.ascast, ptr addrspace(4) %arrayidx1, i64 16384, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %c.ascast, ptr addrspace(4) %arrayidx2, i64 16384, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv8.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub10.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv28.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb29.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub30.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv56.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb57.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub58.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp20.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp48.ascast, i32 0, i32 1) ]

  %array.begin84 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin85 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %array.begin86 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %c.ascast, i32 0, i32 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %c.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv8.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub10.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv28.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb29.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub30.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv56.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb57.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub58.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp20.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp48.ascast, i32 0, i32 1) ]

  %array.begin81 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin82 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %array.begin83 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %c.ascast, i32 0, i32 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %c.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv8.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub10.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv28.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb29.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub30.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.5.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv56.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb57.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub58.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp20.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp48.ascast, i32 0, i32 1) ]

  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc77, %entry
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end79

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %7 = load i32, ptr addrspace(4) %N0.ascast, align 4
  store i32 %7, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %8 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %sub = sub nsw i32 %8, 0
  %sub4 = sub nsw i32 %sub, 1
  %add5 = add nsw i32 %sub4, 1
  %div = sdiv i32 %add5, 1
  %sub6 = sub nsw i32 %div, 1
  store i32 %sub6, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  %9 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %cmp7 = icmp slt i32 0, %9
  br i1 %cmp7, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %omp.inner.for.body
  store i32 0, ptr addrspace(4) %.omp.lb9.ascast, align 4
  %10 = load i32, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  store i32 %10, ptr addrspace(4) %.omp.ub10.ascast, align 4
  %array.begin = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv8.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb9.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub10.ascast, i32 0) ]

  %12 = load i32, ptr addrspace(4) %.omp.lb9.ascast, align 4
  store i32 %12, ptr addrspace(4) %.omp.iv8.ascast, align 4
  br label %omp.inner.for.cond11

omp.inner.for.cond11:                             ; preds = %omp.inner.for.inc, %omp.precond.then
  %13 = load i32, ptr addrspace(4) %.omp.iv8.ascast, align 4
  %14 = load i32, ptr addrspace(4) %.omp.ub10.ascast, align 4
  %cmp12 = icmp sle i32 %13, %14
  br i1 %cmp12, label %omp.inner.for.body13, label %omp.inner.for.end

omp.inner.for.body13:                             ; preds = %omp.inner.for.cond11
  %15 = load i32, ptr addrspace(4) %.omp.iv8.ascast, align 4
  %mul14 = mul nsw i32 %15, 1
  %add15 = add nsw i32 0, %mul14
  store i32 %add15, ptr addrspace(4) %j.ascast, align 4
  %16 = load i32, ptr addrspace(4) %j.ascast, align 4
  %17 = load i32, ptr addrspace(4) %i.ascast, align 4
  %18 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %mul16 = mul nsw i32 %17, %18
  %19 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add17 = add nsw i32 %mul16, %19
  %idxprom = sext i32 %add17 to i64
  %arrayidx18 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom
  store i32 %16, ptr addrspace(4) %arrayidx18, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body13
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %20 = load i32, ptr addrspace(4) %.omp.iv8.ascast, align 4
  %add19 = add nsw i32 %20, 1
  store i32 %add19, ptr addrspace(4) %.omp.iv8.ascast, align 4
  br label %omp.inner.for.cond11

omp.inner.for.end:                                ; preds = %omp.inner.for.cond11
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %omp.inner.for.body
  %21 = load i32, ptr addrspace(4) %N0.ascast, align 4
  store i32 %21, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %22 = load i32, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %sub21 = sub nsw i32 %22, 0
  %sub22 = sub nsw i32 %sub21, 1
  %add23 = add nsw i32 %sub22, 1
  %div24 = sdiv i32 %add23, 1
  %sub25 = sub nsw i32 %div24, 1
  store i32 %sub25, ptr addrspace(4) %.capture_expr.3.ascast, align 4
  %23 = load i32, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %cmp26 = icmp slt i32 0, %23
  br i1 %cmp26, label %omp.precond.then27, label %omp.precond.end47

omp.precond.then27:                               ; preds = %omp.precond.end
  store i32 0, ptr addrspace(4) %.omp.lb29.ascast, align 4
  %24 = load i32, ptr addrspace(4) %.capture_expr.3.ascast, align 4
  store i32 %24, ptr addrspace(4) %.omp.ub30.ascast, align 4
  %array.begin46 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %25 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv28.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb29.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub30.ascast, i32 0) ]

  %26 = load i32, ptr addrspace(4) %.omp.lb29.ascast, align 4
  store i32 %26, ptr addrspace(4) %.omp.iv28.ascast, align 4
  br label %omp.inner.for.cond31

omp.inner.for.cond31:                             ; preds = %omp.inner.for.inc42, %omp.precond.then27
  %27 = load i32, ptr addrspace(4) %.omp.iv28.ascast, align 4
  %28 = load i32, ptr addrspace(4) %.omp.ub30.ascast, align 4
  %cmp32 = icmp sle i32 %27, %28
  br i1 %cmp32, label %omp.inner.for.body33, label %omp.inner.for.end44

omp.inner.for.body33:                             ; preds = %omp.inner.for.cond31
  %29 = load i32, ptr addrspace(4) %.omp.iv28.ascast, align 4
  %mul34 = mul nsw i32 %29, 1
  %add35 = add nsw i32 0, %mul34
  store i32 %add35, ptr addrspace(4) %j.ascast, align 4
  %30 = load i32, ptr addrspace(4) %j.ascast, align 4
  %mul36 = mul nsw i32 %30, 2
  %31 = load i32, ptr addrspace(4) %i.ascast, align 4
  %32 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %mul37 = mul nsw i32 %31, %32
  %33 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add38 = add nsw i32 %mul37, %33
  %idxprom39 = sext i32 %add38 to i64
  %arrayidx40 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 %idxprom39
  store i32 %mul36, ptr addrspace(4) %arrayidx40, align 4
  br label %omp.body.continue41

omp.body.continue41:                              ; preds = %omp.inner.for.body33
  br label %omp.inner.for.inc42

omp.inner.for.inc42:                              ; preds = %omp.body.continue41
  %34 = load i32, ptr addrspace(4) %.omp.iv28.ascast, align 4
  %add43 = add nsw i32 %34, 1
  store i32 %add43, ptr addrspace(4) %.omp.iv28.ascast, align 4
  br label %omp.inner.for.cond31

omp.inner.for.end44:                              ; preds = %omp.inner.for.cond31
  br label %omp.loop.exit45

omp.loop.exit45:                                  ; preds = %omp.inner.for.end44
  call void @llvm.directive.region.exit(token %25) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.precond.end47

omp.precond.end47:                                ; preds = %omp.loop.exit45, %omp.precond.end
  %35 = load i32, ptr addrspace(4) %N0.ascast, align 4
  store i32 %35, ptr addrspace(4) %.capture_expr.4.ascast, align 4
  %36 = load i32, ptr addrspace(4) %.capture_expr.4.ascast, align 4
  %sub49 = sub nsw i32 %36, 0
  %sub50 = sub nsw i32 %sub49, 1
  %add51 = add nsw i32 %sub50, 1
  %div52 = sdiv i32 %add51, 1
  %sub53 = sub nsw i32 %div52, 1
  store i32 %sub53, ptr addrspace(4) %.capture_expr.5.ascast, align 4
  %37 = load i32, ptr addrspace(4) %.capture_expr.4.ascast, align 4
  %cmp54 = icmp slt i32 0, %37
  br i1 %cmp54, label %omp.precond.then55, label %omp.precond.end75

omp.precond.then55:                               ; preds = %omp.precond.end47
  store i32 0, ptr addrspace(4) %.omp.lb57.ascast, align 4
  %38 = load i32, ptr addrspace(4) %.capture_expr.5.ascast, align 4
  store i32 %38, ptr addrspace(4) %.omp.ub58.ascast, align 4
  %array.begin74 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %c.ascast, i32 0, i32 0
  %39 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %c.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv56.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb57.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub58.ascast, i32 0) ]

  %40 = load i32, ptr addrspace(4) %.omp.lb57.ascast, align 4
  store i32 %40, ptr addrspace(4) %.omp.iv56.ascast, align 4
  br label %omp.inner.for.cond59

omp.inner.for.cond59:                             ; preds = %omp.inner.for.inc70, %omp.precond.then55
  %41 = load i32, ptr addrspace(4) %.omp.iv56.ascast, align 4
  %42 = load i32, ptr addrspace(4) %.omp.ub58.ascast, align 4
  %cmp60 = icmp sle i32 %41, %42
  br i1 %cmp60, label %omp.inner.for.body61, label %omp.inner.for.end72

omp.inner.for.body61:                             ; preds = %omp.inner.for.cond59
  %43 = load i32, ptr addrspace(4) %.omp.iv56.ascast, align 4
  %mul62 = mul nsw i32 %43, 1
  %add63 = add nsw i32 0, %mul62
  store i32 %add63, ptr addrspace(4) %j.ascast, align 4
  %44 = load i32, ptr addrspace(4) %j.ascast, align 4
  %mul64 = mul nsw i32 %44, 3
  %45 = load i32, ptr addrspace(4) %i.ascast, align 4
  %46 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %mul65 = mul nsw i32 %45, %46
  %47 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add66 = add nsw i32 %mul65, %47
  %idxprom67 = sext i32 %add66 to i64
  %arrayidx68 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %c.ascast, i64 0, i64 %idxprom67
  store i32 %mul64, ptr addrspace(4) %arrayidx68, align 4
  br label %omp.body.continue69

omp.body.continue69:                              ; preds = %omp.inner.for.body61
  br label %omp.inner.for.inc70

omp.inner.for.inc70:                              ; preds = %omp.body.continue69
  %48 = load i32, ptr addrspace(4) %.omp.iv56.ascast, align 4
  %add71 = add nsw i32 %48, 1
  store i32 %add71, ptr addrspace(4) %.omp.iv56.ascast, align 4
  br label %omp.inner.for.cond59

omp.inner.for.end72:                              ; preds = %omp.inner.for.cond59
  br label %omp.loop.exit73

omp.loop.exit73:                                  ; preds = %omp.inner.for.end72
  call void @llvm.directive.region.exit(token %39) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.precond.end75

omp.precond.end75:                                ; preds = %omp.loop.exit73, %omp.precond.end47
  br label %omp.body.continue76

omp.body.continue76:                              ; preds = %omp.precond.end75
  br label %omp.inner.for.inc77

omp.inner.for.inc77:                              ; preds = %omp.body.continue76
  %49 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add78 = add nsw i32 %49, 1
  store i32 %add78, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end79:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit80

omp.loop.exit80:                                  ; preds = %omp.inner.for.end79
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}


declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2050, i32 49060396, !"_Z4main", i32 7, i32 0, i32 0, i32 0}
