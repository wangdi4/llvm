; RUN: opt -passes='default<O3>' -paropt=31 -S %s | FileCheck %s

; Test src:
;
; void bar(float * A, int N) {
; #pragma omp parallel for
;   for (int i = 0; i < N; ++i)
;     A[i] = i;
; #pragma omp parallel for
;   for (int i = 0; i < N; ++i)
;     A[i] += i;
; }

; Check that shared A and B are passed to the outlined function by value. That
; depends on A's and B's references to be privatized inside the outlined parallel
; region by Paropt and then promoted to values by the argument promotion pass.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local void @bar(ptr noundef %A, i32 noundef %N) #0 {
entry:
  %A.addr = alloca ptr, align 8
  %N.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.4 = alloca i32, align 4
  %.capture_expr.5 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp6 = alloca i32, align 4
  %.capture_expr.6 = alloca i32, align 4
  %.capture_expr.7 = alloca i32, align 4
  %.omp.iv15 = alloca i32, align 4
  %.omp.lb16 = alloca i32, align 4
  %.omp.ub17 = alloca i32, align 4
  %i22 = alloca i32, align 4
  store ptr %A, ptr %A.addr, align 8
  store i32 %N, ptr %N.addr, align 4
  %0 = load i32, ptr %N.addr, align 4
  store i32 %0, ptr %.capture_expr.4, align 4
  %1 = load i32, ptr %.capture_expr.4, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.5, align 4
  %2 = load i32, ptr %.capture_expr.4, align 4
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, ptr %.omp.lb, align 4
  %3 = load i32, ptr %.capture_expr.5, align 4
  store i32 %3, ptr %.omp.ub, align 4

; CHECK: call {{.*}} @__kmpc_fork_call({{.*}}, i32 3, ptr nonnull [[OUTLINED_BAR1:@.+]], ptr %A,

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %A.addr, ptr null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %5 = load i32, ptr %.omp.lb, align 4
  store i32 %5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %6 = load i32, ptr %.omp.iv, align 4
  %7 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %6, %7
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %8, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %i, align 4
  %9 = load i32, ptr %i, align 4
  %conv = sitofp i32 %9 to float
  %10 = load ptr, ptr %A.addr, align 8
  %11 = load i32, ptr %i, align 4
  %idxprom = sext i32 %11 to i64
  %arrayidx = getelementptr inbounds float, ptr %10, i64 %idxprom
  store float %conv, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, ptr %.omp.iv, align 4
  %add5 = add nsw i32 %12, 1
  store i32 %add5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %13 = load i32, ptr %N.addr, align 4
  store i32 %13, ptr %.capture_expr.6, align 4
  %14 = load i32, ptr %.capture_expr.6, align 4
  %sub7 = sub nsw i32 %14, 0
  %sub8 = sub nsw i32 %sub7, 1
  %add9 = add nsw i32 %sub8, 1
  %div10 = sdiv i32 %add9, 1
  %sub11 = sub nsw i32 %div10, 1
  store i32 %sub11, ptr %.capture_expr.7, align 4
  %15 = load i32, ptr %.capture_expr.6, align 4
  %cmp12 = icmp slt i32 0, %15
  br i1 %cmp12, label %omp.precond.then14, label %omp.precond.end34

omp.precond.then14:                               ; preds = %omp.precond.end
  store i32 0, ptr %.omp.lb16, align 4
  %16 = load i32, ptr %.capture_expr.7, align 4
  store i32 %16, ptr %.omp.ub17, align 4

; CHECK: call {{.*}} @__kmpc_fork_call({{.*}}, i32 3, ptr nonnull [[OUTLINED_BAR2:@.+]], ptr %A,

  %17 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %A.addr, ptr null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv15, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb16, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub17, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i22, i32 0, i32 1) ]
  %18 = load i32, ptr %.omp.lb16, align 4
  store i32 %18, ptr %.omp.iv15, align 4
  br label %omp.inner.for.cond18

omp.inner.for.cond18:                             ; preds = %omp.inner.for.inc30, %omp.precond.then14
  %19 = load i32, ptr %.omp.iv15, align 4
  %20 = load i32, ptr %.omp.ub17, align 4
  %cmp19 = icmp sle i32 %19, %20
  br i1 %cmp19, label %omp.inner.for.body21, label %omp.inner.for.end32

omp.inner.for.body21:                             ; preds = %omp.inner.for.cond18
  %21 = load i32, ptr %.omp.iv15, align 4
  %mul23 = mul nsw i32 %21, 1
  %add24 = add nsw i32 0, %mul23
  store i32 %add24, ptr %i22, align 4
  %22 = load i32, ptr %i22, align 4
  %conv25 = sitofp i32 %22 to float
  %23 = load ptr, ptr %A.addr, align 8
  %24 = load i32, ptr %i22, align 4
  %idxprom26 = sext i32 %24 to i64
  %arrayidx27 = getelementptr inbounds float, ptr %23, i64 %idxprom26
  %25 = load float, ptr %arrayidx27, align 4
  %add28 = fadd fast float %25, %conv25
  store float %add28, ptr %arrayidx27, align 4
  br label %omp.body.continue29

omp.body.continue29:                              ; preds = %omp.inner.for.body21
  br label %omp.inner.for.inc30

omp.inner.for.inc30:                              ; preds = %omp.body.continue29
  %26 = load i32, ptr %.omp.iv15, align 4
  %add31 = add nsw i32 %26, 1
  store i32 %add31, ptr %.omp.iv15, align 4
  br label %omp.inner.for.cond18

omp.inner.for.end32:                              ; preds = %omp.inner.for.cond18
  br label %omp.loop.exit33

omp.loop.exit33:                                  ; preds = %omp.inner.for.end32
  call void @llvm.directive.region.exit(token %17) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end34

omp.precond.end34:                                ; preds = %omp.loop.exit33, %omp.precond.end
  ret void
}

; CHECK: define internal void [[OUTLINED_BAR1]](ptr nocapture readonly %tid, ptr nocapture readnone %bid, ptr nocapture writeonly [[ABASE:%.+]], i64 %{{.+}}, i64 %{{.+}}) #{{[0-9]+}} {
; CHECK: omp.inner.for.body{{.*}}:
; CHECK:   [[AADDR:%.+]] = getelementptr inbounds float, ptr [[ABASE]], i64 [[IV:%.+]]
; CHECK:   store float %{{.+}}, ptr [[AADDR]]
; CHECK: }

; CHECK: define internal void [[OUTLINED_BAR2]](ptr nocapture readonly %tid, ptr nocapture readnone %bid, ptr nocapture [[ABASE:%.+]], i64 %{{.+}}, i64 %{{.+}}) #{{[0-9]+}} {
; CHECK: omp.inner.for.body{{.*}}:
; CHECK:   [[AADDR:%.+]] = getelementptr inbounds float, ptr [[ABASE]], i64 [[IV:%.+]]
; CHECK:   %{{.+}} = load float, ptr [[AADDR]]
; CHECK:   store float %{{.+}}, ptr [[AADDR]]
; CHECK: }
