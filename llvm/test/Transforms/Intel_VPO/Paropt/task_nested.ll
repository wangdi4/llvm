; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; The purpose of this test is to make sure that the compiler doesn't
; comp-fail while handling nested tasks.
;
; Test src:
; double x;
;
; void foo() {
; #pragma omp task
;   {
; #pragma omp task
;     {
;       x++;
;     }
;   }
; }
;
; Check there are two calls to kmps_omp_task_alloc, one for each task.
; CHECK: call ptr @__kmpc_omp_task_alloc
; CHECK: call ptr @__kmpc_omp_task_alloc

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = common dso_local global double 0.000000e+00, align 8

define dso_local void @foo() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
     "QUAL.OMP.SHARED"(ptr @x) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
     "QUAL.OMP.SHARED"(ptr @x) ]

  %2 = load double, ptr @x, align 8
  %inc = fadd double %2, 1.000000e+00
  store double %inc, ptr @x, align 8
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASK"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
