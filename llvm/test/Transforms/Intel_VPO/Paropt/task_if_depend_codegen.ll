; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test Src:
; void fn10();
; int Arg;
; void foo() {
;  #pragma omp task if (Arg) depend(inout : Arg)
;  fn10();
; }

; This file tests the implementation of omp task if and depend clause.
; The IR is obtained from Frontend.

; CHECK:  call void @__kmpc_omp_task_with_deps({{.*}})
; CHECK:  call void @__kmpc_omp_wait_deps({{.*}})
; CHECK:  call void @__kmpc_omp_task_begin_if0({{.*}})
; CHECK:  call void @__kmpc_omp_task_complete_if0({{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.kmp_depend_info = type { i64, i64, i8 }

@Arg = dso_local global i32 0, align 4

define dso_local void @foo() {
entry:
  %.dep.arr.addr = alloca [1 x %struct.kmp_depend_info], align 8
  %dep.counter.addr = alloca i64, align 8
  %0 = load i32, ptr @Arg, align 4
  %tobool = icmp ne i32 %0, 0
  %1 = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  %2 = getelementptr %struct.kmp_depend_info, ptr %1, i64 0
  %3 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 0
  store i64 ptrtoint (ptr @Arg to i64), ptr %3, align 8
  %4 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 1
  store i64 4, ptr %4, align 8
  %5 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 2
  store i8 3, ptr %5, align 8
  store i64 1, ptr %dep.counter.addr, align 8
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.IF"(i1 %tobool),
    "QUAL.OMP.DEPARRAY"(i32 1, ptr %1) ]

  call void (...) @fn10() #1
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TASK"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @fn10(...)
