; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-HOST
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-HOST
;
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -pass-remarks-output=%t1.opt.yaml -S %s
; RUN: FileCheck %s -input-file=%t1.opt.yaml --check-prefix=CHECK-YAML
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -pass-remarks-output=%t2.opt.yaml -S %s
; RUN: FileCheck %s -input-file=%t2.opt.yaml --check-prefix=CHECK-YAML

; Test src:
;
; void test3() {
;   int E, F;
; #pragma omp parallel
;   {
; #pragma omp task firstprivate(E) shared(F)
;     { (void) E; (void) F; }
;   }
; }

; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'E' on 'task' construct is redundant
; CHECK-HOST:       remark:{{.*}} SHARED clause for variable 'F' on 'task' construct is redundant
; CHECK-HOST:       remark:{{.*}} SHARED clause for variable 'E' on 'parallel' construct is redundant
; CHECK-HOST:       remark:{{.*}} SHARED clause for variable 'F' on 'parallel' construct is redundant
;
; CHECK-YAML:      --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test3
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          E
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          task
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test3
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          SHARED
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          F
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          task
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test3
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          SHARED
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          E
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          parallel
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test3
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          SHARED
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          F
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          parallel
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @test3() {
; CHECK-LABEL: @test3(
entry:
  %E = alloca i32, align 4
  %F = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %E, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %F, i32 0, i32 1) ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i32 1), "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %E, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %F, i32 0, i32 1)

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %E, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %F, i32 0, i32 1) ]

; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"()
; CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr null, i32 0, i32 1), "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %E, i32 0, i32 1), "QUAL.OMP.PRIVATE:TYPED"(ptr %F, i32 0, i32 1)

  %2 = load i32, ptr %E, align 4
  %3 = load i32, ptr %F, align 4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
