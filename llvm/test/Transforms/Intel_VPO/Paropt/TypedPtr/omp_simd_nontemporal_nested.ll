; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
;void foo(float *A, int NX, int NY) {
;#pragma omp parallel for
;  for (int Y = 0; Y < NY; ++Y)
;#pragma omp simd nontemporal(A)
;    for (int X = 0; X < NX; ++X)
;      A[Y*NX+X] = X*2.5f;
;}
;
; Check that we can parse the nontemporal clause and translate it into
; !nontemporal metadata on a store to 'A'.
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(float* %A, i32 %NX, i32 %NY) {
entry:
  %A.addr = alloca float*, align 8
  %NX.addr = alloca i32, align 4
  %NY.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %Y = alloca i32, align 4
  %tmp5 = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv13 = alloca i32, align 4
  %.omp.ub14 = alloca i32, align 4
  %X = alloca i32, align 4
  store float* %A, float** %A.addr, align 8
  store i32 %NX, i32* %NX.addr, align 4
  store i32 %NY, i32* %NY.addr, align 4
  %0 = load i32, i32* %NY.addr, align 4
  store i32 %0, i32* %.capture_expr.2, align 4
  %1 = load i32, i32* %.capture_expr.2, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.3, align 4
  %2 = load i32, i32* %.capture_expr.2, align 4
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end29

omp.precond.then:                                 ; preds = %entry
  store i32 0, i32* %.omp.lb, align 4
  %3 = load i32, i32* %.capture_expr.3, align 4
  store i32 %3, i32* %.omp.ub, align 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"(float** %A.addr), "QUAL.OMP.SHARED"(i32* %NX.addr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %Y), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1), "QUAL.OMP.PRIVATE"(i32* %.omp.iv13), "QUAL.OMP.PRIVATE"(i32* %.omp.ub14), "QUAL.OMP.PRIVATE"(i32* %X), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.0), "QUAL.OMP.PRIVATE"(i32* %tmp5) ]
  %5 = load i32, i32* %.omp.lb, align 4
  store i32 %5, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc25, %omp.precond.then
  %6 = load i32, i32* %.omp.iv, align 4
  %7 = load i32, i32* %.omp.ub, align 4
  %cmp3 = icmp sle i32 %6, %7
  br i1 %cmp3, label %omp.inner.for.body, label %omp.loop.exit28

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %8, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, i32* %Y, align 4
  %9 = load i32, i32* %NX.addr, align 4
  store i32 %9, i32* %.capture_expr.0, align 4
  %10 = load i32, i32* %.capture_expr.0, align 4
  %sub6 = sub nsw i32 %10, 0
  %sub7 = sub nsw i32 %sub6, 1
  %add8 = add nsw i32 %sub7, 1
  %div9 = sdiv i32 %add8, 1
  %sub10 = sub nsw i32 %div9, 1
  store i32 %sub10, i32* %.capture_expr.1, align 4
  %11 = load i32, i32* %.capture_expr.0, align 4
  %cmp11 = icmp slt i32 0, %11
  br i1 %cmp11, label %omp.precond.then12, label %omp.inner.for.inc25

omp.precond.then12:                               ; preds = %omp.inner.for.body
  %12 = load i32, i32* %.capture_expr.1, align 4
  store i32 %12, i32* %.omp.ub14, align 4
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(float** %A.addr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv13), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub14), "QUAL.OMP.LINEAR:IV"(i32* %X, i32 1) ]
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(float** null),
  store i32 0, i32* %.omp.iv13, align 4
  br label %omp.inner.for.cond15

omp.inner.for.cond15:                             ; preds = %omp.inner.for.body17, %omp.precond.then12
  %14 = load i32, i32* %.omp.iv13, align 4
  %15 = load i32, i32* %.omp.ub14, align 4
  %cmp16 = icmp sle i32 %14, %15
  br i1 %cmp16, label %omp.inner.for.body17, label %omp.loop.exit

omp.inner.for.body17:                             ; preds = %omp.inner.for.cond15
  %16 = load i32, i32* %.omp.iv13, align 4
  %mul18 = mul nsw i32 %16, 1
  %add19 = add nsw i32 0, %mul18
  store i32 %add19, i32* %X, align 4
  %17 = load i32, i32* %X, align 4
  %conv = sitofp i32 %17 to float
  %mul20 = fmul fast float %conv, 2.500000e+00
  %18 = load float*, float** %A.addr, align 8
  %19 = load i32, i32* %Y, align 4
  %20 = load i32, i32* %NX.addr, align 4
  %mul21 = mul nsw i32 %19, %20
  %21 = load i32, i32* %X, align 4
  %add22 = add nsw i32 %mul21, %21
  %idxprom = sext i32 %add22 to i64
  %ptridx = getelementptr inbounds float, float* %18, i64 %idxprom
  store float %mul20, float* %ptridx, align 4
; CHECK: store float %mul20, float* %ptridx, align 4, {{.*}}!nontemporal ![[NTMD:[0-9]+]]
  %22 = load i32, i32* %.omp.iv13, align 4
  %add23 = add nsw i32 %22, 1
  store i32 %add23, i32* %.omp.iv13, align 4
  br label %omp.inner.for.cond15

omp.loop.exit:                                    ; preds = %omp.inner.for.cond15
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.inner.for.inc25

omp.inner.for.inc25:                              ; preds = %omp.loop.exit, %omp.inner.for.body
  %23 = load i32, i32* %.omp.iv, align 4
  %add26 = add nsw i32 %23, 1
  store i32 %add26, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.loop.exit28:                                  ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end29

omp.precond.end29:                                ; preds = %omp.loop.exit28, %entry
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

; CHECK: ![[NTMD]] = !{i32 1}
