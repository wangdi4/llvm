; REQUIRES: asserts

; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-fuse-collapse)' -vpo-paropt-loop-fuse-collapse --debug-only=vpo-paropt-loop-fuse-collapse -S %s 2>&1  | FileCheck --check-prefix=CHECK_DBG %s

; The test checks that there're 2 fusion candidates picked
; when these loops are guarded (i.e. have OMP lb<ub precondition
; emitted by the frontend)

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

; CHECK_DBG: OMP fusion candidate
; CHECK_DBG: OMP fusion candidate

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "spir64"
target device_triples = "spir64"

define protected noundef i32 @main() {
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
  %tmp4 = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv9 = alloca i32, align 4
  %.omp.lb10 = alloca i32, align 4
  %.omp.ub11 = alloca i32, align 4
  %tmp21 = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i32, align 4
  %.omp.iv30 = alloca i32, align 4
  %.omp.lb31 = alloca i32, align 4
  %.omp.ub32 = alloca i32, align 4
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
  %tmp4.ascast = addrspacecast ptr %tmp4 to ptr addrspace(4)
  %.capture_expr.0.ascast = addrspacecast ptr %.capture_expr.0 to ptr addrspace(4)
  %.capture_expr.1.ascast = addrspacecast ptr %.capture_expr.1 to ptr addrspace(4)
  %.omp.iv9.ascast = addrspacecast ptr %.omp.iv9 to ptr addrspace(4)
  %.omp.lb10.ascast = addrspacecast ptr %.omp.lb10 to ptr addrspace(4)
  %.omp.ub11.ascast = addrspacecast ptr %.omp.ub11 to ptr addrspace(4)
  %tmp21.ascast = addrspacecast ptr %tmp21 to ptr addrspace(4)
  %.capture_expr.2.ascast = addrspacecast ptr %.capture_expr.2 to ptr addrspace(4)
  %.capture_expr.3.ascast = addrspacecast ptr %.capture_expr.3 to ptr addrspace(4)
  %.omp.iv30.ascast = addrspacecast ptr %.omp.iv30 to ptr addrspace(4)
  %.omp.lb31.ascast = addrspacecast ptr %.omp.lb31 to ptr addrspace(4)
  %.omp.ub32.ascast = addrspacecast ptr %.omp.ub32 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 1024, ptr addrspace(4) %N0.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 3, ptr addrspace(4) %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 0
  %arrayidx1 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 0
  %0 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %add = add nsw i32 %0, 1
  %mul = mul nsw i32 4, %add
  %conv = sext i32 %mul to i64
  %1 = mul nuw i64 %conv, 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %a.ascast, ptr addrspace(4) %arrayidx, i64 16384, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %b.ascast, ptr addrspace(4) %arrayidx1, i64 %1, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb10.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub11.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv30.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb31.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub32.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp21.ascast, i32 0, i32 1) ]

  %array.begin58 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin59 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb10.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub11.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv30.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb31.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub32.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp21.ascast, i32 0, i32 1) ]

  %array.begin56 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin57 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb10.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub11.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv30.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb31.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub32.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp4.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp21.ascast, i32 0, i32 1) ]

  %5 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %5, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc52, %entry
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %7 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end54

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul2 = mul nsw i32 %8, 1
  %add3 = add nsw i32 0, %mul2
  store i32 %add3, ptr addrspace(4) %i.ascast, align 4
  %9 = load i32, ptr addrspace(4) %N0.ascast, align 4
  store i32 %9, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %10 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %sub = sub nsw i32 %10, 0
  %sub5 = sub nsw i32 %sub, 1
  %add6 = add nsw i32 %sub5, 1
  %div = sdiv i32 %add6, 1
  %sub7 = sub nsw i32 %div, 1
  store i32 %sub7, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  %11 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %cmp8 = icmp slt i32 0, %11
  br i1 %cmp8, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %omp.inner.for.body
  store i32 0, ptr addrspace(4) %.omp.lb10.ascast, align 4
  %12 = load i32, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  store i32 %12, ptr addrspace(4) %.omp.ub11.ascast, align 4
  %array.begin = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv9.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb10.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub11.ascast, i32 0) ]

  %14 = load i32, ptr addrspace(4) %.omp.lb10.ascast, align 4
  store i32 %14, ptr addrspace(4) %.omp.iv9.ascast, align 4
  br label %omp.inner.for.cond12

omp.inner.for.cond12:                             ; preds = %omp.inner.for.inc, %omp.precond.then
  %15 = load i32, ptr addrspace(4) %.omp.iv9.ascast, align 4
  %16 = load i32, ptr addrspace(4) %.omp.ub11.ascast, align 4
  %cmp13 = icmp sle i32 %15, %16
  br i1 %cmp13, label %omp.inner.for.body14, label %omp.inner.for.end

omp.inner.for.body14:                             ; preds = %omp.inner.for.cond12
  %17 = load i32, ptr addrspace(4) %.omp.iv9.ascast, align 4
  %mul15 = mul nsw i32 %17, 1
  %add16 = add nsw i32 0, %mul15
  store i32 %add16, ptr addrspace(4) %j.ascast, align 4
  %18 = load i32, ptr addrspace(4) %j.ascast, align 4
  %19 = load i32, ptr addrspace(4) %i.ascast, align 4
  %20 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %mul17 = mul nsw i32 %19, %20
  %21 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add18 = add nsw i32 %mul17, %21
  %idxprom = sext i32 %add18 to i64
  %arrayidx19 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom
  store i32 %18, ptr addrspace(4) %arrayidx19, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body14
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i32, ptr addrspace(4) %.omp.iv9.ascast, align 4
  %add20 = add nsw i32 %22, 1
  store i32 %add20, ptr addrspace(4) %.omp.iv9.ascast, align 4
  br label %omp.inner.for.cond12

omp.inner.for.end:                                ; preds = %omp.inner.for.cond12
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %omp.inner.for.body
  %23 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %add22 = add nsw i32 %23, 1
  store i32 %add22, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %24 = load i32, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %sub23 = sub nsw i32 %24, 0
  %sub24 = sub nsw i32 %sub23, 1
  %add25 = add nsw i32 %sub24, 1
  %div26 = sdiv i32 %add25, 1
  %sub27 = sub nsw i32 %div26, 1
  store i32 %sub27, ptr addrspace(4) %.capture_expr.3.ascast, align 4
  %25 = load i32, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %cmp28 = icmp slt i32 0, %25
  br i1 %cmp28, label %omp.precond.then29, label %omp.precond.end50

omp.precond.then29:                               ; preds = %omp.precond.end
  store i32 0, ptr addrspace(4) %.omp.lb31.ascast, align 4
  %26 = load i32, ptr addrspace(4) %.capture_expr.3.ascast, align 4
  store i32 %26, ptr addrspace(4) %.omp.ub32.ascast, align 4
  %array.begin49 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %27 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv30.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb31.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub32.ascast, i32 0) ]

  %28 = load i32, ptr addrspace(4) %.omp.lb31.ascast, align 4
  store i32 %28, ptr addrspace(4) %.omp.iv30.ascast, align 4
  br label %omp.inner.for.cond33

omp.inner.for.cond33:                             ; preds = %omp.inner.for.inc45, %omp.precond.then29
  %29 = load i32, ptr addrspace(4) %.omp.iv30.ascast, align 4
  %30 = load i32, ptr addrspace(4) %.omp.ub32.ascast, align 4
  %cmp34 = icmp sle i32 %29, %30
  br i1 %cmp34, label %omp.inner.for.body35, label %omp.inner.for.end47

omp.inner.for.body35:                             ; preds = %omp.inner.for.cond33
  %31 = load i32, ptr addrspace(4) %.omp.iv30.ascast, align 4
  %mul36 = mul nsw i32 %31, 1
  %add37 = add nsw i32 0, %mul36
  store i32 %add37, ptr addrspace(4) %j.ascast, align 4
  %32 = load i32, ptr addrspace(4) %j.ascast, align 4
  %mul38 = mul nsw i32 %32, 2
  %33 = load i32, ptr addrspace(4) %i.ascast, align 4
  %34 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %add39 = add nsw i32 %34, 1
  %mul40 = mul nsw i32 %33, %add39
  %35 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add41 = add nsw i32 %mul40, %35
  %idxprom42 = sext i32 %add41 to i64
  %arrayidx43 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 %idxprom42
  store i32 %mul38, ptr addrspace(4) %arrayidx43, align 4
  br label %omp.body.continue44

omp.body.continue44:                              ; preds = %omp.inner.for.body35
  br label %omp.inner.for.inc45

omp.inner.for.inc45:                              ; preds = %omp.body.continue44
  %36 = load i32, ptr addrspace(4) %.omp.iv30.ascast, align 4
  %add46 = add nsw i32 %36, 1
  store i32 %add46, ptr addrspace(4) %.omp.iv30.ascast, align 4
  br label %omp.inner.for.cond33

omp.inner.for.end47:                              ; preds = %omp.inner.for.cond33
  br label %omp.loop.exit48

omp.loop.exit48:                                  ; preds = %omp.inner.for.end47
  call void @llvm.directive.region.exit(token %27) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.precond.end50

omp.precond.end50:                                ; preds = %omp.loop.exit48, %omp.precond.end
  br label %omp.body.continue51

omp.body.continue51:                              ; preds = %omp.precond.end50
  br label %omp.inner.for.inc52

omp.inner.for.inc52:                              ; preds = %omp.body.continue51
  %37 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add53 = add nsw i32 %37, 1
  store i32 %add53, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end54:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit55

omp.loop.exit55:                                  ; preds = %omp.inner.for.end54
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2050, i32 49058423, !"_Z4main", i32 7, i32 0, i32 0, i32 0}
