; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S <%s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S <%s | FileCheck %s

; Test that LOOP [BIND(...)] construct inside various constructs is mapped according to scheme=1

; //C++ test source
;
; ============================================================
;   DISTRIBUTE LOOP [BIND(...)]
; ============================================================
;
; void distribute__loop_bind_thread() {
;   #pragma omp distribute
;   for (int i = 0; i < 1000; ++i) {
;     #pragma omp loop bind(thread)             // loop --> parallel for
;     for (int j = 0; j < 100; ++j) { }
;   }
; }
; CHECK-LABEL: void @_Z28distribute__loop_bind_threadv()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
; 
; void distribute__loop_bind_parallel() {
;   #pragma omp distribute
;   for (int i = 0; i < 1000; ++i) {
;     #pragma omp loop bind(parallel)           // loop --> parallel for
;     for (int j = 0; j < 100; ++j) { }
;   }
; }
; CHECK-LABEL: void @_Z30distribute__loop_bind_parallelv()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
; 
; void distribute__loop() {
;   #pragma omp distribute
;   for (int i = 0; i < 1000; ++i) {
;     #pragma omp loop                          // loop --> parallel for
;     for (int j = 0; j < 100; ++j) { }
;   }
; }
; CHECK-LABEL: void @_Z16distribute__loopv()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
; 
;
; ============================================================
;   PARALLEL LOOP [BIND(...)]
; ============================================================
;
; void parallel__loop_bind_thread() {
;   #pragma omp parallel
;   for (int i = 0; i < 1000; ++i) {
;     #pragma omp loop bind(thread)             // loop --> simd
;     for (int j = 0; j < 100; ++j) { }
;   }
; }
; CHECK-LABEL: void @_Z26parallel__loop_bind_threadv()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()
; 
; void parallel__loop_bind_parallel() {
;   #pragma omp parallel
;   for (int i = 0; i < 1000; ++i) {
;     #pragma omp loop bind(parallel)           // loop --> for
;     for (int j = 0; j < 100; ++j) { }
;   }
; }
; CHECK-LABEL: void @_Z28parallel__loop_bind_parallelv()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.LOOP"()
; 
; void parallel__loop() {
;   #pragma omp parallel
;   for (int i = 0; i < 1000; ++i) {
;     #pragma omp loop                          // loop --> for
;     for (int j = 0; j < 100; ++j) { }
;   }
; }
; CHECK-LABEL: void @_Z14parallel__loopv()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.LOOP"()
; 
;
; ============================================================
;   (orphaned) LOOP [BIND(...)]
; ============================================================
;
; void orphaned__loop_bind_thread() {
;   #pragma omp loop bind(thread)               // loop --> simd
;   for (int j = 0; j < 100; ++j) { }
; }
; CHECK-LABEL: void @_Z26orphaned__loop_bind_threadv()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()
; 
; void orphaned__loop_bind_parallel() {
;   #pragma omp loop bind(parallel)             // loop --> for
;   for (int j = 0; j < 100; ++j) { }
; }
; CHECK-LABEL: void @_Z28orphaned__loop_bind_parallelv()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.LOOP"()
; 
; void orphaned__loop_bind_teams() {
;   #pragma omp loop bind(teams)                // loop --> distribute parallel for
;   for (int j = 0; j < 100; ++j) { }
; }
; CHECK-LABEL: void @_Z25orphaned__loop_bind_teamsv()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"()
;
; void orphaned__loop_bind_teams__parallel_for() {
;   #pragma omp loop bind(teams)                // loop --> distribute
;   for (int j = 0; j < 100; ++j) {
;     #pragma omp parallel for
;     for (int k = 0; k < 10; ++k) { }
;   }
; }
; CHECK-LABEL: void @_Z39orphaned__loop_bind_teams__parallel_forv()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z28distribute__loop_bind_threadv() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp1 = alloca i32, align 4
  %.omp.iv2 = alloca i32, align 4
  %.omp.lb3 = alloca i32, align 4
  %.omp.ub4 = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 999, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp1, i32 0, i32 1) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc12, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end14

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  store i32 0, ptr %.omp.lb3, align 4
  store i32 99, ptr %.omp.ub4, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.THREAD"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv2, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb3, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub4, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

  %6 = load i32, ptr %.omp.lb3, align 4
  store i32 %6, ptr %.omp.iv2, align 4
  br label %omp.inner.for.cond5

omp.inner.for.cond5:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %7 = load i32, ptr %.omp.iv2, align 4
  %8 = load i32, ptr %.omp.ub4, align 4
  %cmp6 = icmp sle i32 %7, %8
  br i1 %cmp6, label %omp.inner.for.body7, label %omp.inner.for.end

omp.inner.for.body7:                              ; preds = %omp.inner.for.cond5
  %9 = load i32, ptr %.omp.iv2, align 4
  %mul8 = mul nsw i32 %9, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, ptr %j, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body7
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.iv2, align 4
  %add10 = add nsw i32 %10, 1
  store i32 %add10, ptr %.omp.iv2, align 4
  br label %omp.inner.for.cond5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond5
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.body.continue11

omp.body.continue11:                              ; preds = %omp.loop.exit
  br label %omp.inner.for.inc12

omp.inner.for.inc12:                              ; preds = %omp.body.continue11
  %11 = load i32, ptr %.omp.iv, align 4
  %add13 = add nsw i32 %11, 1
  store i32 %add13, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end14:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit15

omp.loop.exit15:                                  ; preds = %omp.inner.for.end14
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISTRIBUTE"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

define dso_local void @_Z30distribute__loop_bind_parallelv() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp1 = alloca i32, align 4
  %.omp.iv2 = alloca i32, align 4
  %.omp.lb3 = alloca i32, align 4
  %.omp.ub4 = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 999, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp1, i32 0, i32 1) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc12, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end14

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  store i32 0, ptr %.omp.lb3, align 4
  store i32 99, ptr %.omp.ub4, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.PARALLEL"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv2, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb3, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub4, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

  %6 = load i32, ptr %.omp.lb3, align 4
  store i32 %6, ptr %.omp.iv2, align 4
  br label %omp.inner.for.cond5

omp.inner.for.cond5:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %7 = load i32, ptr %.omp.iv2, align 4
  %8 = load i32, ptr %.omp.ub4, align 4
  %cmp6 = icmp sle i32 %7, %8
  br i1 %cmp6, label %omp.inner.for.body7, label %omp.inner.for.end

omp.inner.for.body7:                              ; preds = %omp.inner.for.cond5
  %9 = load i32, ptr %.omp.iv2, align 4
  %mul8 = mul nsw i32 %9, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, ptr %j, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body7
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.iv2, align 4
  %add10 = add nsw i32 %10, 1
  store i32 %add10, ptr %.omp.iv2, align 4
  br label %omp.inner.for.cond5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond5
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.body.continue11

omp.body.continue11:                              ; preds = %omp.loop.exit
  br label %omp.inner.for.inc12

omp.inner.for.inc12:                              ; preds = %omp.body.continue11
  %11 = load i32, ptr %.omp.iv, align 4
  %add13 = add nsw i32 %11, 1
  store i32 %add13, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end14:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit15

omp.loop.exit15:                                  ; preds = %omp.inner.for.end14
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISTRIBUTE"() ]

  ret void
}

define dso_local void @_Z16distribute__loopv() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp1 = alloca i32, align 4
  %.omp.iv2 = alloca i32, align 4
  %.omp.lb3 = alloca i32, align 4
  %.omp.ub4 = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 999, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp1, i32 0, i32 1) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc12, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end14

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  store i32 0, ptr %.omp.lb3, align 4
  store i32 99, ptr %.omp.ub4, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv2, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb3, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub4, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

  %6 = load i32, ptr %.omp.lb3, align 4
  store i32 %6, ptr %.omp.iv2, align 4
  br label %omp.inner.for.cond5

omp.inner.for.cond5:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %7 = load i32, ptr %.omp.iv2, align 4
  %8 = load i32, ptr %.omp.ub4, align 4
  %cmp6 = icmp sle i32 %7, %8
  br i1 %cmp6, label %omp.inner.for.body7, label %omp.inner.for.end

omp.inner.for.body7:                              ; preds = %omp.inner.for.cond5
  %9 = load i32, ptr %.omp.iv2, align 4
  %mul8 = mul nsw i32 %9, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, ptr %j, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body7
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.iv2, align 4
  %add10 = add nsw i32 %10, 1
  store i32 %add10, ptr %.omp.iv2, align 4
  br label %omp.inner.for.cond5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond5
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.body.continue11

omp.body.continue11:                              ; preds = %omp.loop.exit
  br label %omp.inner.for.inc12

omp.inner.for.inc12:                              ; preds = %omp.body.continue11
  %11 = load i32, ptr %.omp.iv, align 4
  %add13 = add nsw i32 %11, 1
  store i32 %add13, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end14:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit15

omp.loop.exit15:                                  ; preds = %omp.inner.for.end14
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISTRIBUTE"() ]

  ret void
}

define dso_local void @_Z26parallel__loop_bind_threadv() {
entry:
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %j = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %1, 1000
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.THREAD"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

  %3 = load i32, ptr %.omp.lb, align 4
  store i32 %3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.body
  %4 = load i32, ptr %.omp.iv, align 4
  %5 = load i32, ptr %.omp.ub, align 4
  %cmp1 = icmp sle i32 %4, %5
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %j, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %7, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %for.inc

for.inc:                                          ; preds = %omp.loop.exit
  %8 = load i32, ptr %i, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

define dso_local void @_Z28parallel__loop_bind_parallelv() {
entry:
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %j = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %1, 1000
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.PARALLEL"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

  %3 = load i32, ptr %.omp.lb, align 4
  store i32 %3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.body
  %4 = load i32, ptr %.omp.iv, align 4
  %5 = load i32, ptr %.omp.ub, align 4
  %cmp1 = icmp sle i32 %4, %5
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %j, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %7, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %for.inc

for.inc:                                          ; preds = %omp.loop.exit
  %8 = load i32, ptr %i, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

define dso_local void @_Z14parallel__loopv() {
entry:
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %j = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %1, 1000
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

  %3 = load i32, ptr %.omp.lb, align 4
  store i32 %3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.body
  %4 = load i32, ptr %.omp.iv, align 4
  %5 = load i32, ptr %.omp.ub, align 4
  %cmp1 = icmp sle i32 %4, %5
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %j, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %7, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %for.inc

for.inc:                                          ; preds = %omp.loop.exit
  %8 = load i32, ptr %i, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

define dso_local void @_Z26orphaned__loop_bind_threadv() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.THREAD"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %j, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %5, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.GENERICLOOP"() ]

  ret void
}

define dso_local void @_Z28orphaned__loop_bind_parallelv() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.PARALLEL"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %j, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %5, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.GENERICLOOP"() ]

  ret void
}

define dso_local void @_Z25orphaned__loop_bind_teamsv() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.TEAMS"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %j, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %5, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.GENERICLOOP"() ]

  ret void
}

define dso_local void @_Z39orphaned__loop_bind_teams__parallel_forv() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %j = alloca i32, align 4
  %tmp1 = alloca i32, align 4
  %.omp.iv2 = alloca i32, align 4
  %.omp.lb3 = alloca i32, align 4
  %.omp.ub4 = alloca i32, align 4
  %k = alloca i32, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.TEAMS"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp1, i32 0, i32 1) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc12, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end14

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %j, align 4
  store i32 0, ptr %.omp.lb3, align 4
  store i32 9, ptr %.omp.ub4, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv2, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb3, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub4, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1) ]

  %6 = load i32, ptr %.omp.lb3, align 4
  store i32 %6, ptr %.omp.iv2, align 4
  br label %omp.inner.for.cond5

omp.inner.for.cond5:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %7 = load i32, ptr %.omp.iv2, align 4
  %8 = load i32, ptr %.omp.ub4, align 4
  %cmp6 = icmp sle i32 %7, %8
  br i1 %cmp6, label %omp.inner.for.body7, label %omp.inner.for.end

omp.inner.for.body7:                              ; preds = %omp.inner.for.cond5
  %9 = load i32, ptr %.omp.iv2, align 4
  %mul8 = mul nsw i32 %9, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, ptr %k, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body7
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.iv2, align 4
  %add10 = add nsw i32 %10, 1
  store i32 %add10, ptr %.omp.iv2, align 4
  br label %omp.inner.for.cond5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond5
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  br label %omp.body.continue11

omp.body.continue11:                              ; preds = %omp.loop.exit
  br label %omp.inner.for.inc12

omp.inner.for.inc12:                              ; preds = %omp.body.continue11
  %11 = load i32, ptr %.omp.iv, align 4
  %add13 = add nsw i32 %11, 1
  store i32 %add13, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end14:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit15

omp.loop.exit15:                                  ; preds = %omp.inner.for.end14
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.GENERICLOOP"() ]

  ret void
}
