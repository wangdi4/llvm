; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-HOST
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-HOST
;
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -switch-to-offload -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-DEVICE
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -switch-to-offload -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-DEVICE
;
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -pass-remarks-output=%t1.opt.yaml -S %s
; RUN: FileCheck %s -input-file=%t1.opt.yaml --check-prefix=CHECK-YAML
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -pass-remarks-output=%t2.opt.yaml -S %s
; RUN: FileCheck %s -input-file=%t2.opt.yaml --check-prefix=CHECK-YAML

; Test src:
;
; void test2(int N, int M, int T) {
;   int C, D;
; #pragma omp target teams thread_limit(T)
; #pragma omp distribute firstprivate(C)
;   for (int I = 0; I < N; ++I)
; #pragma omp parallel
; #pragma omp for firstprivate(D)
;     for (int J = 0; J < M; J++) {}
; }

; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'D' on 'loop' construct is redundant
; CHECK-HOST:       remark:{{.*}} SHARED clause for variable 'D' on 'parallel' construct is redundant
; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'C' on 'distribute' construct is redundant
; CHECK-HOST:       remark:{{.*}} SHARED clause for variable 'C' on 'teams' construct is redundant
; CHECK-HOST:       remark:{{.*}} SHARED clause for variable 'D' on 'teams' construct is redundant
; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'C' on 'target' construct is redundant
; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'D' on 'target' construct is redundant
;
; CHECK-DEVICE-NOT: remark:{{.*}} FIRSTPRIVATE clause for variable '{{.+}}' is redundant
;
; CHECK-YAML:      --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test2
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          D
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          loop
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test2
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          SHARED
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          D
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          parallel
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test2
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          C
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          distribute
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test2
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          SHARED
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          C
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          teams
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test2
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          SHARED
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          D
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          teams
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test2
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          C
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          target
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test2
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          D
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          target
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local void @test2(i32 noundef %N, i32 noundef %M, i32 noundef %T) {
; CHECK-LABEL: @test2(
entry:
  %N.addr = alloca i32, align 4
  %M.addr = alloca i32, align 4
  %T.addr = alloca i32, align 4
  %C = alloca i32, align 4
  %D = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.3 = alloca i32, align 4
  %.capture_expr.4 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %I = alloca i32, align 4
  %tmp5 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.omp.iv13 = alloca i32, align 4
  %.omp.lb14 = alloca i32, align 4
  %.omp.ub15 = alloca i32, align 4
  %J = alloca i32, align 4
  store i32 %N, ptr %N.addr, align 4
  store i32 %M, ptr %M.addr, align 4
  store i32 %T, ptr %T.addr, align 4
  %0 = load i32, ptr %T.addr, align 4
  store i32 %0, ptr %.capture_expr.0, align 4
  %1 = load i32, ptr %N.addr, align 4
  store i32 %1, ptr %.capture_expr.3, align 4
  %2 = load i32, ptr %.capture_expr.3, align 4
  %sub = sub nsw i32 %2, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.4, align 4
  store i32 0, ptr %.omp.lb, align 4
  %3 = load i32, ptr %.capture_expr.4, align 4
  store i32 %3, ptr %.omp.ub, align 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %M.addr, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %C, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %D, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.capture_expr.3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %J, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp5, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.capture_expr.0, i32 0, i32 1) ]
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr %.capture_expr.0, i32 0),
    "QUAL.OMP.SHARED:TYPED"(ptr %M.addr, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %C, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %D, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.capture_expr.3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %J, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp5, i32 0, i32 1) ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"()
; CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i32 1), "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %C, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %D, i32 0, i32 1)

  %6 = load i32, ptr %.capture_expr.3, align 4
  %cmp = icmp slt i32 0, %6
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end27

omp.precond.then:                                 ; preds = %entry
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %C, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %J, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp5, i32 0, i32 1) ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"()
; CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr null, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %C, i32 0, i32 1)

  %8 = load i32, ptr %.omp.lb, align 4
  store i32 %8, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc23, %omp.precond.then
  %9 = load i32, ptr %.omp.iv, align 4
  %10 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %9, %10
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end25

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %11, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %I, align 4
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %M.addr, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %D, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %J, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp5, i32 0, i32 1) ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %D, i32 0, i32 1)

  %13 = load i32, ptr %M.addr, align 4
  store i32 %13, ptr %.capture_expr.1, align 4
  %14 = load i32, ptr %.capture_expr.1, align 4
  %sub6 = sub nsw i32 %14, 0
  %sub7 = sub nsw i32 %sub6, 1
  %add8 = add nsw i32 %sub7, 1
  %div9 = sdiv i32 %add8, 1
  %sub10 = sub nsw i32 %div9, 1
  store i32 %sub10, ptr %.capture_expr.2, align 4
  %15 = load i32, ptr %.capture_expr.1, align 4
  %cmp11 = icmp slt i32 0, %15
  br i1 %cmp11, label %omp.precond.then12, label %omp.precond.end

omp.precond.then12:                               ; preds = %omp.inner.for.body
  store i32 0, ptr %.omp.lb14, align 4
  %16 = load i32, ptr %.capture_expr.2, align 4
  store i32 %16, ptr %.omp.ub15, align 4
  %17 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %D, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv13, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb14, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub15, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %J, i32 0, i32 1) ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"()
; CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr null, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %D, i32 0, i32 1)

  %18 = load i32, ptr %.omp.lb14, align 4
  store i32 %18, ptr %.omp.iv13, align 4
  br label %omp.inner.for.cond16

omp.inner.for.cond16:                             ; preds = %omp.inner.for.inc, %omp.precond.then12
  %19 = load i32, ptr %.omp.iv13, align 4
  %20 = load i32, ptr %.omp.ub15, align 4
  %cmp17 = icmp sle i32 %19, %20
  br i1 %cmp17, label %omp.inner.for.body18, label %omp.inner.for.end

omp.inner.for.body18:                             ; preds = %omp.inner.for.cond16
  %21 = load i32, ptr %.omp.iv13, align 4
  %mul19 = mul nsw i32 %21, 1
  %add20 = add nsw i32 0, %mul19
  store i32 %add20, ptr %J, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body18
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i32, ptr %.omp.iv13, align 4
  %add21 = add nsw i32 %22, 1
  store i32 %add21, ptr %.omp.iv13, align 4
  br label %omp.inner.for.cond16

omp.inner.for.end:                                ; preds = %omp.inner.for.cond16
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %17) [ "DIR.OMP.END.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %omp.inner.for.body
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.PARALLEL"() ]
  br label %omp.body.continue22

omp.body.continue22:                              ; preds = %omp.precond.end
  br label %omp.inner.for.inc23

omp.inner.for.inc23:                              ; preds = %omp.body.continue22
  %23 = load i32, ptr %.omp.iv, align 4
  %add24 = add nsw i32 %23, 1
  store i32 %add24, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end25:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit26

omp.loop.exit26:                                  ; preds = %omp.inner.for.end25
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.DISTRIBUTE"() ]
  br label %omp.precond.end27

omp.precond.end27:                                ; preds = %omp.loop.exit26, %entry
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 53, i32 -1916230714, !"_Z5test2", i32 10, i32 0, i32 0}
