; RUN: opt -enable-new-pm=0 -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src
;
; void foo(float *A, int NX, int NY) {
; #pragma omp parallel for
;   for (int Y = 0; Y < NY; ++Y)
; #pragma omp simd nontemporal(A)
;     for (int X = 0; X < NX; ++X)
;       A[Y * NX + X] = X * 2.5f;
; }

; Check that we can parse the nontemporal clause and translate it into
; !nontemporal metadata on a store to 'A'.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(ptr noundef %A, i32 noundef %NX, i32 noundef %NY) #0 {
entry:
  %A.addr = alloca ptr, align 8
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
  store ptr %A, ptr %A.addr, align 8
  store i32 %NX, ptr %NX.addr, align 4
  store i32 %NY, ptr %NY.addr, align 4
  %0 = load i32, ptr %NY.addr, align 4
  store i32 %0, ptr %.capture_expr.2, align 4
  %1 = load i32, ptr %.capture_expr.2, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.3, align 4
  %2 = load i32, ptr %.capture_expr.2, align 4
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end29

omp.precond.then:                                 ; preds = %entry
  store i32 0, ptr %.omp.lb, align 4
  %3 = load i32, ptr %.capture_expr.3, align 4
  store i32 %3, ptr %.omp.ub, align 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %A.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %NX.addr, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %Y, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %X, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.0, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp5, i32 0, i32 1) ]
  %5 = load i32, ptr %.omp.lb, align 4
  store i32 %5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc25, %omp.precond.then
  %6 = load i32, ptr %.omp.iv, align 4
  %7 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %6, %7
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end27

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %8, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %Y, align 4
  %9 = load i32, ptr %NX.addr, align 4
  store i32 %9, ptr %.capture_expr.0, align 4
  %10 = load i32, ptr %.capture_expr.0, align 4
  %sub6 = sub nsw i32 %10, 0
  %sub7 = sub nsw i32 %sub6, 1
  %add8 = add nsw i32 %sub7, 1
  %div9 = sdiv i32 %add8, 1
  %sub10 = sub nsw i32 %div9, 1
  store i32 %sub10, ptr %.capture_expr.1, align 4
  %11 = load i32, ptr %.capture_expr.0, align 4
  %cmp11 = icmp slt i32 0, %11
  br i1 %cmp11, label %omp.precond.then12, label %omp.precond.end

omp.precond.then12:                               ; preds = %omp.inner.for.body
  %12 = load i32, ptr %.capture_expr.1, align 4
  store i32 %12, ptr %.omp.ub14, align 4
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(ptr %A.addr),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv13, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub14, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %X, i32 0, i32 1, i32 1) ]
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NONTEMPORAL:PTR_TO_PTR"(ptr null),
  store i32 0, ptr %.omp.iv13, align 4
  br label %omp.inner.for.cond15

omp.inner.for.cond15:                             ; preds = %omp.inner.for.inc, %omp.precond.then12
  %14 = load i32, ptr %.omp.iv13, align 4
  %15 = load i32, ptr %.omp.ub14, align 4
  %cmp16 = icmp sle i32 %14, %15
  br i1 %cmp16, label %omp.inner.for.body17, label %omp.inner.for.end

omp.inner.for.body17:                             ; preds = %omp.inner.for.cond15
  %16 = load i32, ptr %.omp.iv13, align 4
  %mul18 = mul nsw i32 %16, 1
  %add19 = add nsw i32 0, %mul18
  store i32 %add19, ptr %X, align 4
  %17 = load i32, ptr %X, align 4
  %conv = sitofp i32 %17 to float
  %mul20 = fmul fast float %conv, 2.500000e+00
  %18 = load ptr, ptr %A.addr, align 8
  %19 = load i32, ptr %Y, align 4
  %20 = load i32, ptr %NX.addr, align 4
  %mul21 = mul nsw i32 %19, %20
  %21 = load i32, ptr %X, align 4
  %add22 = add nsw i32 %mul21, %21
  %idxprom = sext i32 %add22 to i64
  %arrayidx = getelementptr inbounds float, ptr %18, i64 %idxprom
  store float %mul20, ptr %arrayidx, align 4
; CHECK: store float %mul20, ptr %arrayidx, align 4, {{.*}}!nontemporal ![[NTMD:[0-9]+]]
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body17
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i32, ptr %.omp.iv13, align 4
  %add23 = add nsw i32 %22, 1
  store i32 %add23, ptr %.omp.iv13, align 4
  br label %omp.inner.for.cond15, !llvm.loop !5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond15
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %omp.inner.for.body
  br label %omp.body.continue24

omp.body.continue24:                              ; preds = %omp.precond.end
  br label %omp.inner.for.inc25

omp.inner.for.inc25:                              ; preds = %omp.body.continue24
  %23 = load i32, ptr %.omp.iv, align 4
  %add26 = add nsw i32 %23, 1
  store i32 %add26, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end27:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit28

omp.loop.exit28:                                  ; preds = %omp.inner.for.end27
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end29

omp.precond.end29:                                ; preds = %omp.loop.exit28, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.vectorize.enable", i1 true}

; CHECK: ![[NTMD]] = !{i32 1}
