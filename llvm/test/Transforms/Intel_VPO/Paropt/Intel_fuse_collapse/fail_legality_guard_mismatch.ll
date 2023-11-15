; REQUIRES: asserts

; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-fuse-collapse)' -vpo-paropt-loop-fuse-collapse --debug-only=vpo-paropt-loop-fuse-collapse -S %s 2>&1  | FileCheck %s

; Original code:
; int main() {
;         static constexpr int N = 1024;
;         static constexpr int B = 4;
; 
;         int a[B*N], b[B*N];
;         int N0 = N;
; #pragma omp target teams loop map(tofrom:a[:B*N],b[:B*(N+1)])
;         for (int i = 0; i < B; i++) {
;                 int j;
; #pragma omp loop
;                 for (j = 0; j < N; j++)
;                         a[i*N+j] = j;
; #pragma omp loop
;                 for (j = 0; j < N0+1; j++)
;                         b[i*(N0+1)+j] = j*2;
;         }
; 
;         return 0;
; }

; This test checks that the pass backs off because of the mix of guarded and non-guarded loops
; to be fused and outputs correct debug message describing it

; CHECK: Candidate pairs fusion check failed

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
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
  %tmp3 = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv8 = alloca i32, align 4
  %.omp.lb9 = alloca i32, align 4
  %.omp.ub10 = alloca i32, align 4
  %tmp21 = alloca i32, align 4
  %.omp.iv22 = alloca i32, align 4
  %.omp.lb23 = alloca i32, align 4
  %.omp.ub24 = alloca i32, align 4
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
  %tmp3.ascast = addrspacecast ptr %tmp3 to ptr addrspace(4)
  %.capture_expr.0.ascast = addrspacecast ptr %.capture_expr.0 to ptr addrspace(4)
  %.capture_expr.1.ascast = addrspacecast ptr %.capture_expr.1 to ptr addrspace(4)
  %.omp.iv8.ascast = addrspacecast ptr %.omp.iv8 to ptr addrspace(4)
  %.omp.lb9.ascast = addrspacecast ptr %.omp.lb9 to ptr addrspace(4)
  %.omp.ub10.ascast = addrspacecast ptr %.omp.ub10 to ptr addrspace(4)
  %tmp21.ascast = addrspacecast ptr %tmp21 to ptr addrspace(4)
  %.omp.iv22.ascast = addrspacecast ptr %.omp.iv22 to ptr addrspace(4)
  %.omp.lb23.ascast = addrspacecast ptr %.omp.lb23 to ptr addrspace(4)
  %.omp.ub24.ascast = addrspacecast ptr %.omp.ub24 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 1024, ptr addrspace(4) %N0.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 3, ptr addrspace(4) %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 0
  %arrayidx1 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i64 0, i64 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %b.ascast, ptr addrspace(4) %arrayidx, i64 16400, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %a.ascast, ptr addrspace(4) %arrayidx1, i64 16384, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
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
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv22.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb23.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub24.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp21.ascast, i32 0, i32 1) ]

  %array.begin53 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin54 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
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
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv8.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub10.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv22.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb23.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub24.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp21.ascast, i32 0, i32 1) ]

  %array.begin51 = getelementptr inbounds [4096 x i32], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin52 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
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
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv8.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb9.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub10.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv22.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb23.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub24.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp3.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp21.ascast, i32 0, i32 1) ]

  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body.lh, label %omp.inner.for.end49

omp.inner.for.body.lh:                            ; preds = %entry
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc45, %omp.inner.for.body.lh
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %arrayidx2 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 1
  store i32 1, ptr addrspace(4) %arrayidx2, align 4
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
  %13 = load i32, ptr addrspace(4) %.omp.iv8.ascast, align 4
  %14 = load i32, ptr addrspace(4) %.omp.ub10.ascast, align 4
  %cmp11 = icmp sle i32 %13, %14
  br i1 %cmp11, label %omp.inner.for.body.lh12, label %omp.inner.for.end

omp.inner.for.body.lh12:                          ; preds = %omp.precond.then
  br label %omp.inner.for.body13

omp.inner.for.body13:                             ; preds = %omp.inner.for.inc, %omp.inner.for.body.lh12
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
  %21 = load i32, ptr addrspace(4) %.omp.iv8.ascast, align 4
  %22 = load i32, ptr addrspace(4) %.omp.ub10.ascast, align 4
  %cmp20 = icmp sle i32 %21, %22
  br i1 %cmp20, label %omp.inner.for.body13, label %omp.inner.for.end_crit_edge

omp.inner.for.end_crit_edge:                      ; preds = %omp.inner.for.inc
  br label %omp.inner.for.end

omp.inner.for.end:                                ; preds = %omp.inner.for.end_crit_edge, %omp.precond.then
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %omp.inner.for.body
  store i32 0, ptr addrspace(4) %.omp.lb23.ascast, align 4
  store i32 1024, ptr addrspace(4) %.omp.ub24.ascast, align 4
  %array.begin43 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %23 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv22.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb23.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub24.ascast, i32 0) ]

  %24 = load i32, ptr addrspace(4) %.omp.lb23.ascast, align 4
  store i32 %24, ptr addrspace(4) %.omp.iv22.ascast, align 4
  %25 = load i32, ptr addrspace(4) %.omp.iv22.ascast, align 4
  %26 = load i32, ptr addrspace(4) %.omp.ub24.ascast, align 4
  %cmp25 = icmp sle i32 %25, %26
  br i1 %cmp25, label %omp.inner.for.body.lh26, label %omp.inner.for.end41

omp.inner.for.body.lh26:                          ; preds = %omp.precond.end
  br label %omp.inner.for.body27

omp.inner.for.body27:                             ; preds = %omp.inner.for.inc37, %omp.inner.for.body.lh26
  %27 = load i32, ptr addrspace(4) %.omp.iv22.ascast, align 4
  %mul28 = mul nsw i32 %27, 1
  %add29 = add nsw i32 0, %mul28
  store i32 %add29, ptr addrspace(4) %j.ascast, align 4
  %28 = load i32, ptr addrspace(4) %j.ascast, align 4
  %mul30 = mul nsw i32 %28, 2
  %29 = load i32, ptr addrspace(4) %i.ascast, align 4
  %30 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %add31 = add nsw i32 %30, 1
  %mul32 = mul nsw i32 %29, %add31
  %31 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add33 = add nsw i32 %mul32, %31
  %idxprom34 = sext i32 %add33 to i64
  %arrayidx35 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 %idxprom34
  store i32 %mul30, ptr addrspace(4) %arrayidx35, align 4
  br label %omp.body.continue36

omp.body.continue36:                              ; preds = %omp.inner.for.body27
  br label %omp.inner.for.inc37

omp.inner.for.inc37:                              ; preds = %omp.body.continue36
  %32 = load i32, ptr addrspace(4) %.omp.iv22.ascast, align 4
  %add38 = add nsw i32 %32, 1
  store i32 %add38, ptr addrspace(4) %.omp.iv22.ascast, align 4
  %33 = load i32, ptr addrspace(4) %.omp.iv22.ascast, align 4
  %34 = load i32, ptr addrspace(4) %.omp.ub24.ascast, align 4
  %cmp39 = icmp sle i32 %33, %34
  br i1 %cmp39, label %omp.inner.for.body27, label %omp.inner.for.end_crit_edge40

omp.inner.for.end_crit_edge40:                    ; preds = %omp.inner.for.inc37
  br label %omp.inner.for.end41

omp.inner.for.end41:                              ; preds = %omp.inner.for.end_crit_edge40, %omp.precond.end
  br label %omp.loop.exit42

omp.loop.exit42:                                  ; preds = %omp.inner.for.end41
  call void @llvm.directive.region.exit(token %23) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.body.continue44

omp.body.continue44:                              ; preds = %omp.loop.exit42
  br label %omp.inner.for.inc45

omp.inner.for.inc45:                              ; preds = %omp.body.continue44
  %35 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add46 = add nsw i32 %35, 1
  store i32 %add46, ptr addrspace(4) %.omp.iv.ascast, align 4
  %36 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %37 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp47 = icmp sle i32 %36, %37
  br i1 %cmp47, label %omp.inner.for.body, label %omp.inner.for.end_crit_edge48

omp.inner.for.end_crit_edge48:                    ; preds = %omp.inner.for.inc45
  br label %omp.inner.for.end49

omp.inner.for.end49:                              ; preds = %omp.inner.for.end_crit_edge48, %entry
  br label %omp.loop.exit50

omp.loop.exit50:                                  ; preds = %omp.inner.for.end49
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
