; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S <%s | FileCheck %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S <%s | FileCheck %s

; Test that LOOP construct inside TEAMS is mapped according to scheme=1

; // C++ test src:
; void teams_loop() {
;   #pragma omp teams loop            // loop --> distribute parallel for
;   for (int i = 0; i < 1000; ++i) { }
; }
; CHECK-LABEL: void @_Z10teams_loopv()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"()
; 
; void teams__loop() {
;   #pragma omp teams
;   for (int i = 0; i < 1000; ++i) {
;     #pragma omp loop                // loop --> distribute parallel for
;     for (int j = 0; j < 100; ++j) { }
;   }
; }
; CHECK-LABEL: void @_Z11teams__loopv()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"()
; 
; void teams__loop__parallel_for() {
;   #pragma omp teams
;   for (int i = 0; i < 1000; ++i) {
;     #pragma omp loop                // loop --> distribute
;     for (int j = 0; j < 100; ++j) {
;       #pragma omp parallel for
;       for (int k = 0; k < 10; ++k) { }
;     }
;   }
; }
; CHECK-LABEL: void @_Z25teams__loop__parallel_forv()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
; 
; void teams__loop__loop() {
;   #pragma omp teams
;   for (int i = 0; i < 1000; ++i) {
;     #pragma omp loop                // loop --> distribute
;     for (int j = 0; j < 100; ++j) {
;       #pragma omp loop              // loop --> parallel for
;       for (int k = 0; k < 10; ++k) { }
;     }
;   }
; }
; CHECK-LABEL: void @_Z17teams__loop__loopv()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
; 
; void teams__loop__loop__loop() {
;   #pragma omp teams
;   for (int i = 0; i < 1000; ++i) {
;     #pragma omp loop                // loop --> distribute
;     for (int j = 0; j < 100; ++j) {
;       #pragma omp loop              // loop --> parallel for
;       for (int k = 0; k < 10; ++k) {
;         #pragma omp loop            // loop --> simd
;         for (int n = 0; n < 10; ++n) { }
;       }
;     }
;   }
; }
; CHECK-LABEL: void @_Z23teams__loop__loop__loopv()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()
;
; INTEL_CUSTOMIZATION
; CMPLRLLVM-50076: perf tuning to allow specific nd-range
; end INTEL_CUSTOMIZATION
; void teams_loop__loop_reduction(int aaa) {
;   #pragma omp teams loop              // loop --> distribute parallel for
;   for (int j = 0; j < 100; ++j) {
;     #pragma omp loop reduction(+:aaa) // loop --> simd
;     for (int k = 0; k < 10; ++k) { }
;   }
; }
; CHECK-LABEL: void @_Z26teams_loop__loop_reductioni(i32 %aaa)
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"()
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z10teams_loopv() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]
  store i32 0, ptr %.omp.lb, align 4
  store i32 999, ptr %.omp.ub, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %2 = load i32, ptr %.omp.lb, align 4
  store i32 %2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr %.omp.iv, align 4
  %4 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %6, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.GENERICLOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TEAMS"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

define dso_local void @_Z11teams__loopv() {
entry:
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %j = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
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
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TEAMS"() ]
  ret void
}

define dso_local void @_Z25teams__loop__parallel_forv() {
entry:
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %j = alloca i32, align 4
  %tmp2 = alloca i32, align 4
  %.omp.iv3 = alloca i32, align 4
  %.omp.lb4 = alloca i32, align 4
  %.omp.ub5 = alloca i32, align 4
  %k = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp2, i32 0, i32 1) ]
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
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp2, i32 0, i32 1) ]
  %3 = load i32, ptr %.omp.lb, align 4
  store i32 %3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc13, %for.body
  %4 = load i32, ptr %.omp.iv, align 4
  %5 = load i32, ptr %.omp.ub, align 4
  %cmp1 = icmp sle i32 %4, %5
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.end15

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %j, align 4
  store i32 0, ptr %.omp.lb4, align 4
  store i32 9, ptr %.omp.ub5, align 4
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv3, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub5, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1) ]
  %8 = load i32, ptr %.omp.lb4, align 4
  store i32 %8, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %9 = load i32, ptr %.omp.iv3, align 4
  %10 = load i32, ptr %.omp.ub5, align 4
  %cmp7 = icmp sle i32 %9, %10
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.inner.for.end

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %11 = load i32, ptr %.omp.iv3, align 4
  %mul9 = mul nsw i32 %11, 1
  %add10 = add nsw i32 0, %mul9
  store i32 %add10, ptr %k, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, ptr %.omp.iv3, align 4
  %add11 = add nsw i32 %12, 1
  store i32 %add11, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.end:                                ; preds = %omp.inner.for.cond6
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.body.continue12

omp.body.continue12:                              ; preds = %omp.loop.exit
  br label %omp.inner.for.inc13

omp.inner.for.inc13:                              ; preds = %omp.body.continue12
  %13 = load i32, ptr %.omp.iv, align 4
  %add14 = add nsw i32 %13, 1
  store i32 %add14, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end15:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit16

omp.loop.exit16:                                  ; preds = %omp.inner.for.end15
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]
  br label %for.inc

for.inc:                                          ; preds = %omp.loop.exit16
  %14 = load i32, ptr %i, align 4
  %inc = add nsw i32 %14, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TEAMS"() ]
  ret void
}

define dso_local void @_Z17teams__loop__loopv() {
entry:
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %j = alloca i32, align 4
  %tmp2 = alloca i32, align 4
  %.omp.iv3 = alloca i32, align 4
  %.omp.lb4 = alloca i32, align 4
  %.omp.ub5 = alloca i32, align 4
  %k = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp2, i32 0, i32 1) ]
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
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp2, i32 0, i32 1) ]
  %3 = load i32, ptr %.omp.lb, align 4
  store i32 %3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc13, %for.body
  %4 = load i32, ptr %.omp.iv, align 4
  %5 = load i32, ptr %.omp.ub, align 4
  %cmp1 = icmp sle i32 %4, %5
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.end15

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %j, align 4
  store i32 0, ptr %.omp.lb4, align 4
  store i32 9, ptr %.omp.ub5, align 4
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv3, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub5, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1) ]
  %8 = load i32, ptr %.omp.lb4, align 4
  store i32 %8, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %9 = load i32, ptr %.omp.iv3, align 4
  %10 = load i32, ptr %.omp.ub5, align 4
  %cmp7 = icmp sle i32 %9, %10
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.inner.for.end

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %11 = load i32, ptr %.omp.iv3, align 4
  %mul9 = mul nsw i32 %11, 1
  %add10 = add nsw i32 0, %mul9
  store i32 %add10, ptr %k, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, ptr %.omp.iv3, align 4
  %add11 = add nsw i32 %12, 1
  store i32 %add11, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.end:                                ; preds = %omp.inner.for.cond6
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.GENERICLOOP"() ]
  br label %omp.body.continue12

omp.body.continue12:                              ; preds = %omp.loop.exit
  br label %omp.inner.for.inc13

omp.inner.for.inc13:                              ; preds = %omp.body.continue12
  %13 = load i32, ptr %.omp.iv, align 4
  %add14 = add nsw i32 %13, 1
  store i32 %add14, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end15:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit16

omp.loop.exit16:                                  ; preds = %omp.inner.for.end15
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]
  br label %for.inc

for.inc:                                          ; preds = %omp.loop.exit16
  %14 = load i32, ptr %i, align 4
  %inc = add nsw i32 %14, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TEAMS"() ]
  ret void
}

define dso_local void @_Z23teams__loop__loop__loopv() {
entry:
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %j = alloca i32, align 4
  %tmp2 = alloca i32, align 4
  %.omp.iv3 = alloca i32, align 4
  %.omp.lb4 = alloca i32, align 4
  %.omp.ub5 = alloca i32, align 4
  %k = alloca i32, align 4
  %tmp11 = alloca i32, align 4
  %.omp.iv12 = alloca i32, align 4
  %.omp.lb13 = alloca i32, align 4
  %.omp.ub14 = alloca i32, align 4
  %n = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv12, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %n, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp11, i32 0, i32 1) ]
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
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv12, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %n, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp11, i32 0, i32 1) ]
  %3 = load i32, ptr %.omp.lb, align 4
  store i32 %3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc27, %for.body
  %4 = load i32, ptr %.omp.iv, align 4
  %5 = load i32, ptr %.omp.ub, align 4
  %cmp1 = icmp sle i32 %4, %5
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.end29

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %j, align 4
  store i32 0, ptr %.omp.lb4, align 4
  store i32 9, ptr %.omp.ub5, align 4
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv3, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub5, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv12, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %n, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp11, i32 0, i32 1) ]
  %8 = load i32, ptr %.omp.lb4, align 4
  store i32 %8, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.inc22, %omp.inner.for.body
  %9 = load i32, ptr %.omp.iv3, align 4
  %10 = load i32, ptr %.omp.ub5, align 4
  %cmp7 = icmp sle i32 %9, %10
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.inner.for.end24

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %11 = load i32, ptr %.omp.iv3, align 4
  %mul9 = mul nsw i32 %11, 1
  %add10 = add nsw i32 0, %mul9
  store i32 %add10, ptr %k, align 4
  store i32 0, ptr %.omp.lb13, align 4
  store i32 9, ptr %.omp.ub14, align 4
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv12, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb13, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub14, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %n, i32 0, i32 1) ]
  %13 = load i32, ptr %.omp.lb13, align 4
  store i32 %13, ptr %.omp.iv12, align 4
  br label %omp.inner.for.cond15

omp.inner.for.cond15:                             ; preds = %omp.inner.for.inc, %omp.inner.for.body8
  %14 = load i32, ptr %.omp.iv12, align 4
  %15 = load i32, ptr %.omp.ub14, align 4
  %cmp16 = icmp sle i32 %14, %15
  br i1 %cmp16, label %omp.inner.for.body17, label %omp.inner.for.end

omp.inner.for.body17:                             ; preds = %omp.inner.for.cond15
  %16 = load i32, ptr %.omp.iv12, align 4
  %mul18 = mul nsw i32 %16, 1
  %add19 = add nsw i32 0, %mul18
  store i32 %add19, ptr %n, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body17
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %17 = load i32, ptr %.omp.iv12, align 4
  %add20 = add nsw i32 %17, 1
  store i32 %add20, ptr %.omp.iv12, align 4
  br label %omp.inner.for.cond15

omp.inner.for.end:                                ; preds = %omp.inner.for.cond15
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.GENERICLOOP"() ]
  br label %omp.body.continue21

omp.body.continue21:                              ; preds = %omp.loop.exit
  br label %omp.inner.for.inc22

omp.inner.for.inc22:                              ; preds = %omp.body.continue21
  %18 = load i32, ptr %.omp.iv3, align 4
  %add23 = add nsw i32 %18, 1
  store i32 %add23, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.end24:                              ; preds = %omp.inner.for.cond6
  br label %omp.loop.exit25

omp.loop.exit25:                                  ; preds = %omp.inner.for.end24
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.GENERICLOOP"() ]
  br label %omp.body.continue26

omp.body.continue26:                              ; preds = %omp.loop.exit25
  br label %omp.inner.for.inc27

omp.inner.for.inc27:                              ; preds = %omp.body.continue26
  %19 = load i32, ptr %.omp.iv, align 4
  %add28 = add nsw i32 %19, 1
  store i32 %add28, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end29:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit30

omp.loop.exit30:                                  ; preds = %omp.inner.for.end29
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.GENERICLOOP"() ]
  br label %for.inc

for.inc:                                          ; preds = %omp.loop.exit30
  %20 = load i32, ptr %i, align 4
  %inc = add nsw i32 %20, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TEAMS"() ]
  ret void
}

define dso_local void @_Z26teams_loop__loop_reductioni(i32 %aaa) {
entry:
  %aaa.addr = alloca i32, align 4
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
  store i32 %aaa, ptr %aaa.addr, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %aaa.addr, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp1, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %aaa.addr, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp1, i32 0, i32 1) ]

  %2 = load i32, ptr %.omp.lb, align 4
  store i32 %2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc12, %entry
  %3 = load i32, ptr %.omp.iv, align 4
  %4 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end14

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %j, align 4
  store i32 0, ptr %.omp.lb3, align 4
  store i32 9, ptr %.omp.ub4, align 4
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %aaa.addr, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv2, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb3, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub4, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1) ]

  %7 = load i32, ptr %.omp.lb3, align 4
  store i32 %7, ptr %.omp.iv2, align 4
  br label %omp.inner.for.cond5

omp.inner.for.cond5:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %8 = load i32, ptr %.omp.iv2, align 4
  %9 = load i32, ptr %.omp.ub4, align 4
  %cmp6 = icmp sle i32 %8, %9
  br i1 %cmp6, label %omp.inner.for.body7, label %omp.inner.for.end

omp.inner.for.body7:                              ; preds = %omp.inner.for.cond5
  %10 = load i32, ptr %.omp.iv2, align 4
  %mul8 = mul nsw i32 %10, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, ptr %k, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body7
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, ptr %.omp.iv2, align 4
  %add10 = add nsw i32 %11, 1
  store i32 %add10, ptr %.omp.iv2, align 4
  br label %omp.inner.for.cond5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond5
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.GENERICLOOP"() ]

  br label %omp.body.continue11

omp.body.continue11:                              ; preds = %omp.loop.exit
  br label %omp.inner.for.inc12

omp.inner.for.inc12:                              ; preds = %omp.body.continue11
  %12 = load i32, ptr %.omp.iv, align 4
  %add13 = add nsw i32 %12, 1
  store i32 %add13, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end14:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit15

omp.loop.exit15:                                  ; preds = %omp.inner.for.end14
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TEAMS"() ]

  ret void
}
