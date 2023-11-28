; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-HOST
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-HOST
;
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -pass-remarks-output=%t1.opt.yaml -S %s
; RUN: FileCheck %s -input-file=%t1.opt.yaml --check-prefix=CHECK-YAML
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -pass-remarks-output=%t2.opt.yaml -S %s
; RUN: FileCheck %s -input-file=%t2.opt.yaml --check-prefix=CHECK-YAML

; Test src:
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

; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'G' on 'taskloop' construct is redundant
; CHECK-HOST:       remark:{{.*}} SHARED clause for variable 'H' on 'taskloop' construct is redundant
; CHECK-HOST:       remark:{{.*}} SHARED clause for variable 'G' on 'parallel' construct is redundant
; CHECK-HOST:       remark:{{.*}} SHARED clause for variable 'H' on 'parallel' construct is redundant
;
; CHECK-YAML:      --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test4
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          G
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          taskloop
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test4
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          SHARED
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          H
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          taskloop
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test4
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          SHARED
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          G
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          parallel
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test4
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          SHARED
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          H
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          parallel
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local void @test4(i32 noundef %N) {
; CHECK-LABEL: @test4(
entry:
  %N.addr = alloca i32, align 4
  %G = alloca i32, align 4
  %H = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %N, ptr %N.addr, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %N.addr, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %G, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %H, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.0, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
    "QUAL.OMP.IMPLICIT"() ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i32 1), "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %G, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %H, i32 0, i32 1)

  %2 = load i32, ptr %N.addr, align 4
  store i32 %2, ptr %.capture_expr.0, align 4
  %3 = load i32, ptr %.capture_expr.0, align 4
  %sub = sub nsw i32 %3, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.1, align 4
  %4 = load i32, ptr %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %4
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, ptr %.omp.lb, align 4
  %5 = load i32, ptr %.capture_expr.1, align 4
  store i32 %5, ptr %.omp.ub, align 4
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %G, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %H, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"()
; CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr null, i32 0, i32 1), "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %G, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %H, i32 0, i32 1)

  %7 = load i32, ptr %.omp.lb, align 4
  store i32 %7, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %8 = load i32, ptr %.omp.iv, align 4
  %9 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %8, %9
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %10, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %i, align 4
  %11 = load i32, ptr %G, align 4
  %12 = load i32, ptr %H, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %13 = load i32, ptr %.omp.iv, align 4
  %add5 = add nsw i32 %13, 1
  store i32 %add5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TASKLOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASKGROUP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}
