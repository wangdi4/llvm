; REQUIRES: asserts

; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-fuse-collapse)' -vpo-paropt-loop-fuse-collapse --debug-only=vpo-paropt-loop-fuse-collapse -S %s 2>&1  | FileCheck %s

; Original code:
; struct S {
;   int a;
;   int b;
; };
; int main() {
;         static constexpr int N = 1024;
;         static constexpr int B = 4;
;
;         S a[B*N];
;         int b[B*(N+1)];
;         int N0 = N;
; #pragma omp target teams loop map(tofrom:a[:B*N],b[:B*(N+1)])
;         for (int i = 0; i < B; i++) {
;                 int j;
; #pragma omp loop
;                 for (j = 0; j < N0; j++)
;                         a[i*N0+j] = {j,j+1};
; #pragma omp loop
;                 for (j = 0; j < N0+1; j++)
;                         b[i*(N0+1)+j] = j*2;
;         }
; 
;         return 0;
; }

; This test checks that the pass backs off because of an unsupported memory access instruction (memcpy),
; and outputs correct debug message describing it

; CHECK: Per-candidate fusion check failed

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.S = type { i32, i32 }

define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %a = alloca [4096 x %struct.S], align 4
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
  %ref.tmp = alloca %struct.S, align 4
  %tmp23 = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i32, align 4
  %.omp.iv32 = alloca i32, align 4
  %.omp.lb33 = alloca i32, align 4
  %.omp.ub34 = alloca i32, align 4
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
  %ref.tmp.ascast = addrspacecast ptr %ref.tmp to ptr addrspace(4)
  %tmp23.ascast = addrspacecast ptr %tmp23 to ptr addrspace(4)
  %.capture_expr.2.ascast = addrspacecast ptr %.capture_expr.2 to ptr addrspace(4)
  %.capture_expr.3.ascast = addrspacecast ptr %.capture_expr.3 to ptr addrspace(4)
  %.omp.iv32.ascast = addrspacecast ptr %.omp.iv32 to ptr addrspace(4)
  %.omp.lb33.ascast = addrspacecast ptr %.omp.lb33 to ptr addrspace(4)
  %.omp.ub34.ascast = addrspacecast ptr %.omp.ub34 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 1024, ptr addrspace(4) %N0.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 3, ptr addrspace(4) %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [4096 x %struct.S], ptr addrspace(4) %a.ascast, i64 0, i64 0
  %arrayidx1 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %a.ascast, ptr addrspace(4) %arrayidx, i64 32768, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
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
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv32.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb33.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub34.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ref.tmp.ascast, %struct.S zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp23.ascast, i32 0, i32 1) ]

  %array.begin64 = getelementptr inbounds [4096 x %struct.S], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin65 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, %struct.S zeroinitializer, i64 4096),
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
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv32.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb33.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub34.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ref.tmp.ascast, %struct.S zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp23.ascast, i32 0, i32 1) ]

  %array.begin62 = getelementptr inbounds [4096 x %struct.S], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %array.begin63 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, %struct.S zeroinitializer, i64 4096),
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
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv32.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.lb33.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.ub34.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp2.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ref.tmp.ascast, %struct.S zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp23.ascast, i32 0, i32 1) ]

  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body.lh, label %omp.inner.for.end60

omp.inner.for.body.lh:                            ; preds = %entry
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc56, %omp.inner.for.body.lh
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
  %array.begin = getelementptr inbounds [4096 x %struct.S], ptr addrspace(4) %a.ascast, i32 0, i32 0
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, %struct.S zeroinitializer, i64 4096),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv7.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb8.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub9.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ref.tmp.ascast, %struct.S zeroinitializer, i32 1) ]

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
  %a15 = getelementptr inbounds %struct.S, ptr addrspace(4) %ref.tmp.ascast, i32 0, i32 0
  %16 = load i32, ptr addrspace(4) %j.ascast, align 4
  store i32 %16, ptr addrspace(4) %a15, align 4
  %b16 = getelementptr inbounds %struct.S, ptr addrspace(4) %ref.tmp.ascast, i32 0, i32 1
  %17 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add17 = add nsw i32 %17, 1
  store i32 %add17, ptr addrspace(4) %b16, align 4
  %18 = load i32, ptr addrspace(4) %i.ascast, align 4
  %19 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %mul18 = mul nsw i32 %18, %19
  %20 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add19 = add nsw i32 %mul18, %20
  %idxprom = sext i32 %add19 to i64
  %arrayidx20 = getelementptr inbounds [4096 x %struct.S], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom
  call void @llvm.memcpy.p4.p4.i64(ptr addrspace(4) align 4 %arrayidx20, ptr addrspace(4) align 4 %ref.tmp.ascast, i64 8, i1 false)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body12
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %21 = load i32, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %add21 = add nsw i32 %21, 1
  store i32 %add21, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %22 = load i32, ptr addrspace(4) %.omp.iv7.ascast, align 4
  %23 = load i32, ptr addrspace(4) %.omp.ub9.ascast, align 4
  %cmp22 = icmp sle i32 %22, %23
  br i1 %cmp22, label %omp.inner.for.body12, label %omp.inner.for.end_crit_edge

omp.inner.for.end_crit_edge:                      ; preds = %omp.inner.for.inc
  br label %omp.inner.for.end

omp.inner.for.end:                                ; preds = %omp.inner.for.end_crit_edge, %omp.precond.then
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %omp.inner.for.body
  %24 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %add24 = add nsw i32 %24, 1
  store i32 %add24, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %25 = load i32, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %sub25 = sub nsw i32 %25, 0
  %sub26 = sub nsw i32 %sub25, 1
  %add27 = add nsw i32 %sub26, 1
  %div28 = sdiv i32 %add27, 1
  %sub29 = sub nsw i32 %div28, 1
  store i32 %sub29, ptr addrspace(4) %.capture_expr.3.ascast, align 4
  %26 = load i32, ptr addrspace(4) %.capture_expr.2.ascast, align 4
  %cmp30 = icmp slt i32 0, %26
  br i1 %cmp30, label %omp.precond.then31, label %omp.precond.end54

omp.precond.then31:                               ; preds = %omp.precond.end
  store i32 0, ptr addrspace(4) %.omp.lb33.ascast, align 4
  %27 = load i32, ptr addrspace(4) %.capture_expr.3.ascast, align 4
  store i32 %27, ptr addrspace(4) %.omp.ub34.ascast, align 4
  %array.begin53 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i32 0, i32 0
  %28 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %b.ascast, i32 0, i64 4100),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %N0.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv32.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb33.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub34.ascast, i32 0) ]

  %29 = load i32, ptr addrspace(4) %.omp.lb33.ascast, align 4
  store i32 %29, ptr addrspace(4) %.omp.iv32.ascast, align 4
  %30 = load i32, ptr addrspace(4) %.omp.iv32.ascast, align 4
  %31 = load i32, ptr addrspace(4) %.omp.ub34.ascast, align 4
  %cmp35 = icmp sle i32 %30, %31
  br i1 %cmp35, label %omp.inner.for.body.lh36, label %omp.inner.for.end51

omp.inner.for.body.lh36:                          ; preds = %omp.precond.then31
  br label %omp.inner.for.body37

omp.inner.for.body37:                             ; preds = %omp.inner.for.inc47, %omp.inner.for.body.lh36
  %32 = load i32, ptr addrspace(4) %.omp.iv32.ascast, align 4
  %mul38 = mul nsw i32 %32, 1
  %add39 = add nsw i32 0, %mul38
  store i32 %add39, ptr addrspace(4) %j.ascast, align 4
  %33 = load i32, ptr addrspace(4) %j.ascast, align 4
  %mul40 = mul nsw i32 %33, 2
  %34 = load i32, ptr addrspace(4) %i.ascast, align 4
  %35 = load i32, ptr addrspace(4) %N0.ascast, align 4
  %add41 = add nsw i32 %35, 1
  %mul42 = mul nsw i32 %34, %add41
  %36 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add43 = add nsw i32 %mul42, %36
  %idxprom44 = sext i32 %add43 to i64
  %arrayidx45 = getelementptr inbounds [4100 x i32], ptr addrspace(4) %b.ascast, i64 0, i64 %idxprom44
  store i32 %mul40, ptr addrspace(4) %arrayidx45, align 4
  br label %omp.body.continue46

omp.body.continue46:                              ; preds = %omp.inner.for.body37
  br label %omp.inner.for.inc47

omp.inner.for.inc47:                              ; preds = %omp.body.continue46
  %37 = load i32, ptr addrspace(4) %.omp.iv32.ascast, align 4
  %add48 = add nsw i32 %37, 1
  store i32 %add48, ptr addrspace(4) %.omp.iv32.ascast, align 4
  %38 = load i32, ptr addrspace(4) %.omp.iv32.ascast, align 4
  %39 = load i32, ptr addrspace(4) %.omp.ub34.ascast, align 4
  %cmp49 = icmp sle i32 %38, %39
  br i1 %cmp49, label %omp.inner.for.body37, label %omp.inner.for.end_crit_edge50

omp.inner.for.end_crit_edge50:                    ; preds = %omp.inner.for.inc47
  br label %omp.inner.for.end51

omp.inner.for.end51:                              ; preds = %omp.inner.for.end_crit_edge50, %omp.precond.then31
  br label %omp.loop.exit52

omp.loop.exit52:                                  ; preds = %omp.inner.for.end51
  call void @llvm.directive.region.exit(token %28) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.precond.end54

omp.precond.end54:                                ; preds = %omp.loop.exit52, %omp.precond.end
  br label %omp.body.continue55

omp.body.continue55:                              ; preds = %omp.precond.end54
  br label %omp.inner.for.inc56

omp.inner.for.inc56:                              ; preds = %omp.body.continue55
  %40 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add57 = add nsw i32 %40, 1
  store i32 %add57, ptr addrspace(4) %.omp.iv.ascast, align 4
  %41 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %42 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp58 = icmp sle i32 %41, %42
  br i1 %cmp58, label %omp.inner.for.body, label %omp.inner.for.end_crit_edge59

omp.inner.for.end_crit_edge59:                    ; preds = %omp.inner.for.inc56
  br label %omp.inner.for.end60

omp.inner.for.end60:                              ; preds = %omp.inner.for.end_crit_edge59, %entry
  br label %omp.loop.exit61

omp.loop.exit61:                                  ; preds = %omp.inner.for.end60
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.memcpy.p4.p4.i64(ptr addrspace(4) noalias nocapture writeonly, ptr addrspace(4) noalias nocapture readonly, i64, i1 immarg)

attributes #0 = { "contains-openmp-target"="true" }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2050, i32 49129116, !"_Z4main", i32 15, i32 0, i32 0, i32 0}

