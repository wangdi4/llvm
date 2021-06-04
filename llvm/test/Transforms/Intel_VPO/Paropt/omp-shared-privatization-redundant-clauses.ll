; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-HOST
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-HOST
;
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -switch-to-offload -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-DEVICE
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -switch-to-offload -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-DEVICE
;
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -pass-remarks-output=%t1.opt.yaml -S %s
; RUN: FileCheck %s -input-file=%t1.opt.yaml --check-prefix=CHECK-YAML
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -pass-remarks-output=%t2.opt.yaml -S %s
; RUN: FileCheck %s -input-file=%t2.opt.yaml --check-prefix=CHECK-YAML
;
; Test src:
;
; void test1(int A) {
;   int B;
; #pragma omp target
; #pragma omp parallel firstprivate(A) private(B)
;   {}
; }
;
; void test2(int N, int M) {
;   int C, D;
; #pragma omp target teams
; #pragma omp distribute firstprivate(C)
;   for (int I = 0; I < N; ++I)
; #pragma omp parallel
; #pragma omp for firstprivate(D)
;     for (int J = 0; J < M; J++) {}
; }
;
; void test3() {
;   int E, F;
; #pragma omp parallel
;   {
; #pragma omp task firstprivate(E) shared(F)
;     { (void) E; (void) F; }
;   }
; }
;
; void test4(int N) {
;   int G, H;
; #pragma omp parallel
;   {
; #pragma omp taskloop firstprivate(G) shared(H)
;     for (int i = 0; i < N; ++i)
;     { (void) G; (void) H; }
;   }
; }
;
; void test5(int N) {
;   int J;
; #pragma omp parallel
; #pragma omp for firstprivate(J) lastprivate(J)
;     for (int i = 0; i < N; ++i) {}
; }
;
; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'A.addr' is redundant
; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'A.addr' is redundant
; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'B' is redundant
; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'D' is redundant
; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'C' is redundant
; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'E' is redundant
; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'G' is redundant
; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'J' is redundant
;
; CHECK-DEVICE-NOT: remark:{{.*}} FIRSTPRIVATE clause for variable '{{.+}}' is redundant
;
; CHECK-YAML:      --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test1
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          A.addr
; CHECK-YAML-NEXT:   - String:          ''' is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test1
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          A.addr
; CHECK-YAML-NEXT:   - String:          ''' is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test1
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          B
; CHECK-YAML-NEXT:   - String:          ''' is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test2
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          D
; CHECK-YAML-NEXT:   - String:          ''' is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test2
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          C
; CHECK-YAML-NEXT:   - String:          ''' is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test3
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          E
; CHECK-YAML-NEXT:   - String:          ''' is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test4
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          G
; CHECK-YAML-NEXT:   - String:          ''' is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test5
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          J
; CHECK-YAML-NEXT:   - String:          ''' is redundant'
; CHECK-YAML-NEXT: ...

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @test1(i32 %A) #0 {
; CHECK-LABEL: @test1(
entry:
  %A.addr = alloca i32, align 4
  %B = alloca i32, align 4
  store i32 %A, i32* %A.addr, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32* %A.addr), "QUAL.OMP.FIRSTPRIVATE"(i32* %B) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %A.addr), "QUAL.OMP.PRIVATE"(i32* %B) ]
; CHECK:   call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* null)
; CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* %A.addr)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

define dso_local void @test2(i32 %N, i32 %M) #0 {
; CHECK-LABEL: @test2(
entry:
  %N.addr = alloca i32, align 4
  %M.addr = alloca i32, align 4
  %C = alloca i32, align 4
  %D = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %I = alloca i32, align 4
  %tmp5 = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv13 = alloca i32, align 4
  %.omp.lb14 = alloca i32, align 4
  %.omp.ub15 = alloca i32, align 4
  %J = alloca i32, align 4
  store i32 %N, i32* %N.addr, align 4
  store i32 %M, i32* %M.addr, align 4
  %0 = load i32, i32* %N.addr, align 4
  store i32 %0, i32* %.capture_expr.2, align 4
  %1 = load i32, i32* %.capture_expr.2, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.3, align 4
  store i32 0, i32* %.omp.lb, align 4
  %2 = load i32, i32* %.capture_expr.3, align 4
  store i32 %2, i32* %.omp.ub, align 4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1), "QUAL.OMP.FIRSTPRIVATE"(i32* %M.addr), "QUAL.OMP.FIRSTPRIVATE"(i32* %C), "QUAL.OMP.FIRSTPRIVATE"(i32* %D), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %I), "QUAL.OMP.FIRSTPRIVATE"(i32* %.capture_expr.2), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1), "QUAL.OMP.PRIVATE"(i32* %.omp.iv13), "QUAL.OMP.PRIVATE"(i32* %.omp.lb14), "QUAL.OMP.PRIVATE"(i32* %.omp.ub15), "QUAL.OMP.PRIVATE"(i32* %J), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.0), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp5) ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.SHARED"(i32* %M.addr), "QUAL.OMP.SHARED"(i32* %C), "QUAL.OMP.SHARED"(i32* %D), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.SHARED"(i32* %.omp.lb), "QUAL.OMP.SHARED"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %I), "QUAL.OMP.SHARED"(i32* %.capture_expr.2), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1), "QUAL.OMP.PRIVATE"(i32* %.omp.iv13), "QUAL.OMP.PRIVATE"(i32* %.omp.lb14), "QUAL.OMP.PRIVATE"(i32* %.omp.ub15), "QUAL.OMP.PRIVATE"(i32* %J), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.0), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp5) ]
  %5 = load i32, i32* %.capture_expr.2, align 4
  %cmp = icmp slt i32 0, %5
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end27

omp.precond.then:                                 ; preds = %entry
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %C), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %I), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1), "QUAL.OMP.PRIVATE"(i32* %.omp.iv13), "QUAL.OMP.PRIVATE"(i32* %.omp.lb14), "QUAL.OMP.PRIVATE"(i32* %.omp.ub15), "QUAL.OMP.PRIVATE"(i32* %J), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.0), "QUAL.OMP.PRIVATE"(i32* %tmp5) ]
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"()
; CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* null)
; CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* %C.fp)
  %7 = load i32, i32* %.omp.lb, align 4
  store i32 %7, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc23, %omp.precond.then
  %8 = load i32, i32* %.omp.iv, align 4
  %9 = load i32, i32* %.omp.ub, align 4
  %cmp3 = icmp sle i32 %8, %9
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end25

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %10, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, i32* %I, align 4
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32* %M.addr), "QUAL.OMP.SHARED"(i32* %D), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1), "QUAL.OMP.PRIVATE"(i32* %.omp.iv13), "QUAL.OMP.PRIVATE"(i32* %.omp.lb14), "QUAL.OMP.PRIVATE"(i32* %.omp.ub15), "QUAL.OMP.PRIVATE"(i32* %J), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.0), "QUAL.OMP.PRIVATE"(i32* %tmp5) ]
  %12 = load i32, i32* %M.addr, align 4
  store i32 %12, i32* %.capture_expr.0, align 4
  %13 = load i32, i32* %.capture_expr.0, align 4
  %sub6 = sub nsw i32 %13, 0
  %sub7 = sub nsw i32 %sub6, 1
  %add8 = add nsw i32 %sub7, 1
  %div9 = sdiv i32 %add8, 1
  %sub10 = sub nsw i32 %div9, 1
  store i32 %sub10, i32* %.capture_expr.1, align 4
  %14 = load i32, i32* %.capture_expr.0, align 4
  %cmp11 = icmp slt i32 0, %14
  br i1 %cmp11, label %omp.precond.then12, label %omp.precond.end

omp.precond.then12:                               ; preds = %omp.inner.for.body
  store i32 0, i32* %.omp.lb14, align 4
  %15 = load i32, i32* %.capture_expr.1, align 4
  store i32 %15, i32* %.omp.ub15, align 4
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %D), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv13), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb14), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub15), "QUAL.OMP.PRIVATE"(i32* %J) ]
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"()
; CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* null)
; CHECK-SAME:  "QUAL.OMP.PRIVATE"(i32* %D.fp)
  %17 = load i32, i32* %.omp.lb14, align 4
  store i32 %17, i32* %.omp.iv13, align 4
  br label %omp.inner.for.cond16

omp.inner.for.cond16:                             ; preds = %omp.inner.for.inc, %omp.precond.then12
  %18 = load i32, i32* %.omp.iv13, align 4
  %19 = load i32, i32* %.omp.ub15, align 4
  %cmp17 = icmp sle i32 %18, %19
  br i1 %cmp17, label %omp.inner.for.body18, label %omp.inner.for.end

omp.inner.for.body18:                             ; preds = %omp.inner.for.cond16
  %20 = load i32, i32* %.omp.iv13, align 4
  %mul19 = mul nsw i32 %20, 1
  %add20 = add nsw i32 0, %mul19
  store i32 %add20, i32* %J, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body18
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %21 = load i32, i32* %.omp.iv13, align 4
  %add21 = add nsw i32 %21, 1
  store i32 %add21, i32* %.omp.iv13, align 4
  br label %omp.inner.for.cond16

omp.inner.for.end:                                ; preds = %omp.inner.for.cond16
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %omp.inner.for.body
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.PARALLEL"() ]
  br label %omp.body.continue22

omp.body.continue22:                              ; preds = %omp.precond.end
  br label %omp.inner.for.inc23

omp.inner.for.inc23:                              ; preds = %omp.body.continue22
  %22 = load i32, i32* %.omp.iv, align 4
  %add24 = add nsw i32 %22, 1
  store i32 %add24, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end25:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit26

omp.loop.exit26:                                  ; preds = %omp.inner.for.end25
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.DISTRIBUTE"() ]
  br label %omp.precond.end27

omp.precond.end27:                                ; preds = %omp.loop.exit26, %entry
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

define dso_local void @test3() #0 {
; CHECK-LABEL: @test3(
entry:
  %E = alloca i32, align 4
  %F = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32* %E), "QUAL.OMP.SHARED"(i32* %F) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %E), "QUAL.OMP.SHARED"(i32* %F) ]
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"()
; CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* null)
; CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* %E.fp)
  %2 = load i32, i32* %E, align 4
  %3 = load i32, i32* %F, align 4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

define dso_local void @test4(i32 %N) #0 {
; CHECK-LABEL: @test4(
entry:
  %N.addr = alloca i32, align 4
  %G = alloca i32, align 4
  %H = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.4 = alloca i32, align 4
  %.capture_expr.5 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %N, i32* %N.addr, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32* %N.addr), "QUAL.OMP.SHARED"(i32* %G), "QUAL.OMP.SHARED"(i32* %H), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.5), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.4), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  %1 = load i32, i32* %N.addr, align 4
  store i32 %1, i32* %.capture_expr.4, align 4
  %2 = load i32, i32* %.capture_expr.4, align 4
  %sub = sub nsw i32 %2, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.5, align 4
  %3 = load i32, i32* %.capture_expr.4, align 4
  %cmp = icmp slt i32 0, %3
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, i32* %.omp.lb, align 4
  %4 = load i32, i32* %.capture_expr.5, align 4
  store i32 %4, i32* %.omp.ub, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %G), "QUAL.OMP.SHARED"(i32* %H), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"()
; CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* null)
; CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* %G.fp)
  %6 = load i32, i32* %.omp.lb, align 4
  store i32 %6, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %7 = load i32, i32* %.omp.iv, align 4
  %8 = load i32, i32* %.omp.ub, align 4
  %cmp3 = icmp sle i32 %7, %8
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %9, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, i32* %i, align 4
  %10 = load i32, i32* %G, align 4
  %11 = load i32, i32* %H, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, i32* %.omp.iv, align 4
  %add5 = add nsw i32 %12, 1
  store i32 %add5, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TASKLOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

define dso_local void @test5(i32 %N) #0 {
; CHECK-LABEL: @test5(
entry:
  %N.addr = alloca i32, align 4
  %J = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.6 = alloca i32, align 4
  %.capture_expr.7 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %N, i32* %N.addr, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32* %N.addr), "QUAL.OMP.SHARED"(i32* %J), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.7), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.6), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  %1 = load i32, i32* %N.addr, align 4
  store i32 %1, i32* %.capture_expr.6, align 4
  %2 = load i32, i32* %.capture_expr.6, align 4
  %sub = sub nsw i32 %2, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.7, align 4
  %3 = load i32, i32* %.capture_expr.6, align 4
  %cmp = icmp slt i32 0, %3
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, i32* %.omp.lb, align 4
  %4 = load i32, i32* %.capture_expr.7, align 4
  store i32 %4, i32* %.omp.ub, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %J), "QUAL.OMP.LASTPRIVATE"(i32* %J), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"()
; CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* null), "QUAL.OMP.LASTPRIVATE"(i32* null)
; CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* %J.fp)
  %6 = load i32, i32* %.omp.lb, align 4
  store i32 %6, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %7 = load i32, i32* %.omp.iv, align 4
  %8 = load i32, i32* %.omp.ub, align 4
  %cmp3 = icmp sle i32 %7, %8
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %9, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, i32* %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, i32* %.omp.iv, align 4
  %add5 = add nsw i32 %10, 1
  store i32 %add5, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

attributes #0 = { "may-have-openmp-directive"="true" }

!omp_offload.info = !{!0, !1}

!0 = !{i32 0, i32 52, i32 -694943886, !"_Z5test2", i32 10, i32 1, i32 0}
!1 = !{i32 0, i32 52, i32 -694943886, !"_Z5test1", i32 3, i32 0, i32 0}
