; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s

; Test src:
;
; void test1(int N) {
;   int A;
; #pragma omp simd lastprivate(A)
;   for (int I = 0; I < N; ++I) {}
; }
;
; void foo(void*);
;
; void test2(int N, int M) {
;   int B, C;
; #pragma omp simd collapse(2) // lastprivate(B, C) is implicit
;   for (B = 0; B < N; ++B)
;     for (C = 0; C < M; ++C) {}
; }
;
; void test3(int N) {
;   int D;
; #pragma omp parallel firstprivate(D)
;   { foo(&D); }
; #pragma omp parallel for lastprivate(D)
;   for (int I = 0; I < N; ++I) { foo(&D); }
; }
;
; void negative_test1(int N, int M) {
;   int D;
;   for (int J = 0; J < M; ++J) {
; #pragma omp parallel firstprivate(D)
;     { foo(&D); }
; #pragma omp parallel for lastprivate(D)
;     for (int I = 0; I < N; ++I) { foo(&D); }
;   }
; }

; This test checks that shared privatization pass emits diagnostic and replaces
; lastprivate clause with private if item has no uses inside or outside the work
; region.
;
; CHECK: remark:{{.*}} LASTPRIVATE clause for variable 'A' on 'simd' construct is redundant
; CHECK: remark:{{.*}} LASTPRIVATE clause for variable 'B' on 'simd' construct is redundant
; CHECK: remark:{{.*}} LASTPRIVATE clause for variable 'C' on 'simd' construct is redundant
; CHECK: remark:{{.*}} LASTPRIVATE clause for variable 'D' on 'parallel loop' construct is redundant

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @test1(i32 noundef %N) #0 {
; CHECK-LABEL: @test1(
entry:
  %N.addr = alloca i32, align 4
  %A = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store i32 %N, ptr %N.addr, align 4
  %0 = load i32, ptr %N.addr, align 4
  store i32 %0, ptr %.capture_expr.0, align 4
  %1 = load i32, ptr %.capture_expr.0, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.1, align 4
  %2 = load i32, ptr %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %3 = load i32, ptr %.capture_expr.1, align 4
  store i32 %3, ptr %.omp.ub, align 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %A, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %I, i32 0, i32 1, i32 1) ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()
; CHECK-SAME: "QUAL.OMP.LASTPRIVATE:TYPED"(ptr null, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %A, i32 0, i32 1)

  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %5 = load i32, ptr %.omp.iv, align 4
  %6 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %5, %6
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %7, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %I, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add5 = add nsw i32 %8, 1
  store i32 %add5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond, !llvm.loop !5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @test2(i32 noundef %N, i32 noundef %M) #0 {
; CHECK-LABEL: @test2(
entry:
  %N.addr = alloca i32, align 4
  %M.addr = alloca i32, align 4
  %B = alloca i32, align 4
  %C = alloca i32, align 4
  %tmp = alloca i32, align 4
  %tmp1 = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i32, align 4
  %.capture_expr.4 = alloca i64, align 8
  %.omp.iv = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  store i32 %N, ptr %N.addr, align 4
  store i32 %M, ptr %M.addr, align 4
  %0 = load i32, ptr %N.addr, align 4
  store i32 %0, ptr %.capture_expr.2, align 4
  %1 = load i32, ptr %M.addr, align 4
  store i32 %1, ptr %.capture_expr.3, align 4
  %2 = load i32, ptr %.capture_expr.2, align 4
  %sub = sub nsw i32 %2, 0
  %sub2 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub2, 1
  %div = sdiv i32 %add, 1
  %conv = sext i32 %div to i64
  %3 = load i32, ptr %.capture_expr.3, align 4
  %sub3 = sub nsw i32 %3, 0
  %sub4 = sub nsw i32 %sub3, 1
  %add5 = add nsw i32 %sub4, 1
  %div6 = sdiv i32 %add5, 1
  %conv7 = sext i32 %div6 to i64
  %mul = mul nsw i64 %conv, %conv7
  %sub8 = sub nsw i64 %mul, 1
  store i64 %sub8, ptr %.capture_expr.4, align 8
  %4 = load i32, ptr %.capture_expr.2, align 4
  %cmp = icmp slt i32 0, %4
  br i1 %cmp, label %land.lhs.true, label %omp.precond.end

land.lhs.true:                                    ; preds = %entry
  %5 = load i32, ptr %.capture_expr.3, align 4
  %cmp11 = icmp slt i32 0, %5
  br i1 %cmp11, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %land.lhs.true
  %6 = load i64, ptr %.capture_expr.4, align 8
  store i64 %6, ptr %.omp.ub, align 8
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.COLLAPSE"(i32 2),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %B, i32 0, i32 1),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %C, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i64 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i64 0) ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()
; CHECK-SAME: "QUAL.OMP.LASTPRIVATE:TYPED"(ptr null, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.LASTPRIVATE:TYPED"(ptr null, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %B, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %C, i32 0, i32 1)

  store i64 0, ptr %.omp.iv, align 8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %8 = load i64, ptr %.omp.iv, align 8
  %9 = load i64, ptr %.omp.ub, align 8
  %cmp14 = icmp sle i64 %8, %9
  br i1 %cmp14, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i64, ptr %.omp.iv, align 8
  %11 = load i32, ptr %.capture_expr.3, align 4
  %sub16 = sub nsw i32 %11, 0
  %sub17 = sub nsw i32 %sub16, 1
  %add18 = add nsw i32 %sub17, 1
  %div19 = sdiv i32 %add18, 1
  %mul20 = mul nsw i32 1, %div19
  %conv21 = sext i32 %mul20 to i64
  %div22 = sdiv i64 %10, %conv21
  %mul23 = mul nsw i64 %div22, 1
  %add24 = add nsw i64 0, %mul23
  %conv25 = trunc i64 %add24 to i32
  store i32 %conv25, ptr %B, align 4
  %12 = load i64, ptr %.omp.iv, align 8
  %13 = load i64, ptr %.omp.iv, align 8
  %14 = load i32, ptr %.capture_expr.3, align 4
  %sub26 = sub nsw i32 %14, 0
  %sub27 = sub nsw i32 %sub26, 1
  %add28 = add nsw i32 %sub27, 1
  %div29 = sdiv i32 %add28, 1
  %mul30 = mul nsw i32 1, %div29
  %conv31 = sext i32 %mul30 to i64
  %div32 = sdiv i64 %13, %conv31
  %15 = load i32, ptr %.capture_expr.3, align 4
  %sub33 = sub nsw i32 %15, 0
  %sub34 = sub nsw i32 %sub33, 1
  %add35 = add nsw i32 %sub34, 1
  %div36 = sdiv i32 %add35, 1
  %mul37 = mul nsw i32 1, %div36
  %conv38 = sext i32 %mul37 to i64
  %mul39 = mul nsw i64 %div32, %conv38
  %sub40 = sub nsw i64 %12, %mul39
  %mul41 = mul nsw i64 %sub40, 1
  %add42 = add nsw i64 0, %mul41
  %conv43 = trunc i64 %add42 to i32
  store i32 %conv43, ptr %C, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %16 = load i64, ptr %.omp.iv, align 8
  %add44 = add nsw i64 %16, 1
  store i64 %add44, ptr %.omp.iv, align 8
  %17 = load i32, ptr %.capture_expr.2, align 4
  %sub45 = sub nsw i32 %17, 0
  %sub46 = sub nsw i32 %sub45, 1
  %add47 = add nsw i32 %sub46, 1
  %div48 = sdiv i32 %add47, 1
  %mul49 = mul nsw i32 %div48, 1
  %add50 = add nsw i32 0, %mul49
  store i32 %add50, ptr %B, align 4
  %18 = load i32, ptr %.capture_expr.3, align 4
  %sub51 = sub nsw i32 %18, 0
  %sub52 = sub nsw i32 %sub51, 1
  %add53 = add nsw i32 %sub52, 1
  %div54 = sdiv i32 %add53, 1
  %mul55 = mul nsw i32 %div54, 1
  %add56 = add nsw i32 0, %mul55
  store i32 %add56, ptr %C, align 4
  br label %omp.inner.for.cond, !llvm.loop !7

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %land.lhs.true, %entry
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @test3(i32 noundef %N) #0 {
; CHECK-LABEL: @test3(
entry:
  %N.addr = alloca i32, align 4
  %D = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.5 = alloca i32, align 4
  %.capture_expr.6 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store i32 %N, ptr %N.addr, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %D, i32 0, i32 1) ]
  call void @foo(ptr noundef %D) #1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %1 = load i32, ptr %N.addr, align 4
  store i32 %1, ptr %.capture_expr.5, align 4
  %2 = load i32, ptr %.capture_expr.5, align 4
  %sub = sub nsw i32 %2, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.6, align 4
  %3 = load i32, ptr %.capture_expr.5, align 4
  %cmp = icmp slt i32 0, %3
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, ptr %.omp.lb, align 4
  %4 = load i32, ptr %.capture_expr.6, align 4
  store i32 %4, ptr %.omp.ub, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %D, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1) ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
; CHECK-SAME: "QUAL.OMP.LASTPRIVATE:TYPED"(ptr null, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %D, i32 0, i32 1)

  %6 = load i32, ptr %.omp.lb, align 4
  store i32 %6, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %7 = load i32, ptr %.omp.iv, align 4
  %8 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %7, %8
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %9, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %I, align 4
  call void @foo(ptr noundef %D) #1
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.iv, align 4
  %add5 = add nsw i32 %10, 1
  store i32 %add5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

declare dso_local void @foo(ptr noundef) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @negative_test1(i32 noundef %N, i32 noundef %M) #0 {
; CHECK-LABEL: @negative_test1(
entry:
  %N.addr = alloca i32, align 4
  %M.addr = alloca i32, align 4
  %D = alloca i32, align 4
  %J = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.7 = alloca i32, align 4
  %.capture_expr.8 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store i32 %N, ptr %N.addr, align 4
  store i32 %M, ptr %M.addr, align 4
  store i32 0, ptr %J, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %J, align 4
  %1 = load i32, ptr %M.addr, align 4
  %cmp = icmp slt i32 %0, %1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %D, i32 0, i32 1) ]
  call void @foo(ptr noundef %D) #1
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL"() ]
  %3 = load i32, ptr %N.addr, align 4
  store i32 %3, ptr %.capture_expr.7, align 4
  %4 = load i32, ptr %.capture_expr.7, align 4
  %sub = sub nsw i32 %4, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.8, align 4
  %5 = load i32, ptr %.capture_expr.7, align 4
  %cmp3 = icmp slt i32 0, %5
  br i1 %cmp3, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %for.body
  store i32 0, ptr %.omp.lb, align 4
  %6 = load i32, ptr %.capture_expr.8, align 4
  store i32 %6, ptr %.omp.ub, align 4
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %D, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1) ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
; CHECK-SAME: "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %D, i32 0, i32 1)

  %8 = load i32, ptr %.omp.lb, align 4
  store i32 %8, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %9 = load i32, ptr %.omp.iv, align 4
  %10 = load i32, ptr %.omp.ub, align 4
  %cmp4 = icmp sle i32 %9, %10
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %11, 1
  %add5 = add nsw i32 0, %mul
  store i32 %add5, ptr %I, align 4
  call void @foo(ptr noundef %D) #1
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, ptr %.omp.iv, align 4
  %add6 = add nsw i32 %12, 1
  store i32 %add6, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %for.body
  br label %for.inc

for.inc:                                          ; preds = %omp.precond.end
  %13 = load i32, ptr %J, align 4
  %inc = add nsw i32 %13, 1
  store i32 %inc, ptr %J, align 4
  br label %for.cond, !llvm.loop !8

for.end:                                          ; preds = %for.cond
  ret void
}

attributes #0 = { noinline nounwind uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.vectorize.enable", i1 true}
!7 = distinct !{!7, !6}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
