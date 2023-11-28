; REQUIRES: asserts

; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-fuse-collapse)' -vpo-paropt-loop-fuse-collapse --debug-only=vpo-paropt-loop-fuse-collapse -S %s 2>&1  | FileCheck %s

; The test checks that fusion candidates collection failed due to
; intervening collapse clause contained in the nest

; Original code:
; int main() {
;         static constexpr int N = 1024;
;         static constexpr int B = 4;
; 
;         int a[B*N], b[B*N];
;         int N0 = N;
; #pragma omp target teams loop map(tofrom:a[:B*N],b[:B*N])
;         for (int i = 0; i < B; i++) {
;                 int j;
; #pragma omp loop
;                 for (j = 0; j < N0; j++)
;                         a[i*N0+j] = j;
; #pragma omp loop collapse(2)
;                 for (j = 0; j < N0; j++)
;                     for (int k = 0; k < 3; k++)
;                         b[i*N0+j] = j*k;
;         }
; 
;         return 0;
; }

; CHECK: Intervening collapse clause found, exiting

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define protected noundef i32 @main() {
entry:
  %retval = alloca i32, align 4
  %a = alloca [4096 x i32], align 4
  %b = alloca [4096 x i32], align 4
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
  %tmp19 = alloca i32, align 4
  %tmp20 = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i64, align 8
  %.omp.uncollapsed.iv = alloca i64, align 8
  %.omp.uncollapsed.iv29 = alloca i64, align 8
  %.omp.uncollapsed.lb = alloca i64, align 8
  %.omp.uncollapsed.ub = alloca i64, align 8
  %.omp.uncollapsed.lb36 = alloca i64, align 8
  %.omp.uncollapsed.ub37 = alloca i64, align 8
  %k = alloca i32, align 4
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
  %tmp19.ascast = addrspacecast ptr %tmp19 to ptr addrspace(4)
  %tmp20.ascast = addrspacecast ptr %tmp20 to ptr addrspace(4)
  %.capture_expr.2.ascast = addrspacecast ptr %.capture_expr.2 to ptr addrspace(4)
  %.capture_expr.3.ascast = addrspacecast ptr %.capture_expr.3 to ptr addrspace(4)
  %.omp.uncollapsed.iv.ascast = addrspacecast ptr %.omp.uncollapsed.iv to ptr addrspace(4)
  %.omp.uncollapsed.iv29.ascast = addrspacecast ptr %.omp.uncollapsed.iv29 to ptr addrspace(4)
  %.omp.uncollapsed.lb.ascast = addrspacecast ptr %.omp.uncollapsed.lb to ptr addrspace(4)
  %.omp.uncollapsed.ub.ascast = addrspacecast ptr %.omp.uncollapsed.ub to ptr addrspace(4)
  %.omp.uncollapsed.lb36.ascast = addrspacecast ptr %.omp.uncollapsed.lb36 to ptr addrspace(4)
  %.omp.uncollapsed.ub37.ascast = addrspacecast ptr %.omp.uncollapsed.ub37 to ptr addrspace(4)
  %k.ascast = addrspacecast ptr %k to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 1024, ptr addrspace(4) %N0.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 3, ptr addrspace(4) %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 0
  %arrayidx1 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %a.ascast, ptr addrspace(4) %arrayidx, i64 16384, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %b.ascast, ptr addrspace(4) %arrayidx1, i64 16384, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
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
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.3.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv29.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb36.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub37.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp20.ascast, i32 0, i32 1) ]

  %array.begin67 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin68 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4096),
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
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.3.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv29.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb36.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub37.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp20.ascast, i32 0, i32 1) ]

  %array.begin65 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin66 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4096),
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
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.3.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv29.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb36.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub37.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp19.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp20.ascast, i32 0, i32 1) ]

  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc61, %entry
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end63

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
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
  br label %omp.inner.for.cond10

omp.inner.for.cond10:                             ; preds = %omp.inner.for.inc, %omp.precond.then
  %13 = load i32, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %14 = load i32, ptr addrspace(4) %.omp.ub9.ascast, align 4
  %cmp11 = icmp sle i32 %13, %14
  br i1 %cmp11, label %omp.inner.for.body12, label %omp.inner.for.end

omp.inner.for.body12:                             ; preds = %omp.inner.for.cond10
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
  br label %omp.inner.for.cond10

omp.inner.for.end:                                ; preds = %omp.inner.for.cond10
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
  %conv = sext i32 %div24 to i64
  %mul25 = mul nsw i64 %conv, 3
  %sub26 = sub nsw i64 %mul25, 1
  store i64 %sub26, ptr addrspace(4) %.capture_expr.3.ascast, align 8
  %23 = load i32, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %cmp27 = icmp slt i32 0, %23
  br i1 %cmp27, label %omp.precond.then28, label %omp.precond.end59

omp.precond.then28:                               ; preds = %omp.precond.end
  store i64 0, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 8
  %24 = load i32, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %sub30 = sub nsw i32 %24, 0
  %sub31 = sub nsw i32 %sub30, 1
  %add32 = add nsw i32 %sub31, 1
  %div33 = sdiv i32 %add32, 1
  %conv34 = sext i32 %div33 to i64
  %sub35 = sub nsw i64 %conv34, 1
  store i64 %sub35, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 8
  store i64 0, ptr addrspace(4) %.omp.uncollapsed.lb36.ascast, align 8
  store i64 2, ptr addrspace(4) %.omp.uncollapsed.ub37.ascast, align 8
  %array.begin58 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %25 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.uncollapsed.iv.ascast, i64 0, ptr addrspace(4) %.omp.uncollapsed.iv29.ascast, i64 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb.ascast, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.uncollapsed.ub.ascast, i64 0, ptr addrspace(4) %.omp.uncollapsed.ub37.ascast, i64 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.uncollapsed.lb36.ascast, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1) ]

  %26 = load i64, ptr addrspace(4) %.omp.uncollapsed.lb.ascast, align 8
  store i64 %26, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 8
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc55, %omp.precond.then28
  %27 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 8
  %28 = load i64, ptr addrspace(4) %.omp.uncollapsed.ub.ascast, align 8
  %cmp38 = icmp sle i64 %27, %28
  br i1 %cmp38, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end57

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %29 = load i64, ptr addrspace(4) %.omp.uncollapsed.lb36.ascast, align 8
  store i64 %29, ptr addrspace(4) %.omp.uncollapsed.iv29.ascast, align 8
  br label %omp.uncollapsed.loop.cond39

omp.uncollapsed.loop.cond39:                      ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %30 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv29.ascast, align 8
  %31 = load i64, ptr addrspace(4) %.omp.uncollapsed.ub37.ascast, align 8
  %cmp40 = icmp sle i64 %30, %31
  br i1 %cmp40, label %omp.uncollapsed.loop.body41, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body41:                      ; preds = %omp.uncollapsed.loop.cond39
  %32 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 8
  %mul42 = mul nsw i64 %32, 1
  %add43 = add nsw i64 0, %mul42
  %conv44 = trunc i64 %add43 to i32
  store i32 %conv44, ptr addrspace(4) %j.ascast, align 4
  %33 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv29.ascast, align 8
  %mul45 = mul nsw i64 %33, 1
  %add46 = add nsw i64 0, %mul45
  %conv47 = trunc i64 %add46 to i32
  store i32 %conv47, ptr addrspace(4) %k.ascast, align 4
  %34 = load i32, ptr addrspace(4) %k.ascast, align 4
  %35 = load i32, ptr addrspace(4) %j.ascast, align 4
  %mul48 = mul nsw i32 %34, %35
  %36 = load i32, ptr addrspace(4) %i.ascast, align 4
  %37 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %mul49 = mul nsw i32 %36, %37
  %38 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add50 = add nsw i32 %mul49, %38
  %idxprom51 = sext i32 %add50 to i64
  %arrayidx52 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 %idxprom51
  store i32 %mul48, ptr addrspace(4) %arrayidx52, align 4
  br label %omp.body.continue53

omp.body.continue53:                              ; preds = %omp.uncollapsed.loop.body41
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue53
  %39 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv29.ascast, align 8
  %add54 = add nsw i64 %39, 1
  store i64 %add54, ptr addrspace(4) %.omp.uncollapsed.iv29.ascast, align 8
  br label %omp.uncollapsed.loop.cond39

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond39
  br label %omp.uncollapsed.loop.inc55

omp.uncollapsed.loop.inc55:                       ; preds = %omp.uncollapsed.loop.end
  %40 = load i64, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 8
  %add56 = add nsw i64 %40, 1
  store i64 %add56, ptr addrspace(4) %.omp.uncollapsed.iv.ascast, align 8
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end57:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %25) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.precond.end59

omp.precond.end59:                                ; preds = %omp.uncollapsed.loop.end57, %omp.precond.end
  br label %omp.body.continue60

omp.body.continue60:                              ; preds = %omp.precond.end59
  br label %omp.inner.for.inc61

omp.inner.for.inc61:                              ; preds = %omp.body.continue60
  %41 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add62 = add nsw i32 %41, 1
  store i32 %add62, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end63:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit64

omp.loop.exit64:                                  ; preds = %omp.inner.for.end63
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token) 

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2050, i32 49060403, !"_Z4main", i32 7, i32 0, i32 0, i32 0}

