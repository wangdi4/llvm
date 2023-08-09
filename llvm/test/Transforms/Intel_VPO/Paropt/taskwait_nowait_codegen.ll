; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
; #include <omp.h>
; #include <stdio.h>
;
; int main() {
;  int Result[10];
;  int a;
;  #pragma omp taskwait depend(in:a) nowait
;  // printf("Result = %d .... \n", Result[0]);
;}

;This test checks that we are parsing the nowait clause on taskwait depend as a task depend and that we are deleting the fence instruction
; CHECK: %{{.*}} = tail call i32 @__kmpc_global_thread_num(ptr @{{.*}})
; CHECK: [[TASK_ALLOC:%.+]]  = call ptr @__kmpc_omp_task_alloc(ptr @{{.*}}, i32 %{{.*}}, i32 1, i64 72, i64 0, ptr @{{.*}})
; CHECK: call void @__kmpc_omp_task_with_deps(ptr @{{.*}}, i32 %{{.*}}, ptr [[TASK_ALLOC]], i32 1, ptr %.dep.arr.addr, i32 0, ptr null)
; CHECK-NOT: fence acq_rel

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
%struct.kmp_depend_info = type { i64, i64, i8 }

@.str = private unnamed_addr constant [19 x i8] c"Result = %d .... \0A\00", align 1

define dso_local i32 @main() {
entry:
  %Result = alloca [10 x i32], align 16
  %a = alloca i32, align 4
  %.dep.arr.addr = alloca [1 x %struct.kmp_depend_info], align 8
  %dep.counter.addr = alloca i64, align 8
  %0 = ptrtoint ptr %a to i64
  store i64 %0, ptr %.dep.arr.addr, align 8
  %1 = getelementptr inbounds %struct.kmp_depend_info, ptr %.dep.arr.addr, i32 0, i32 1
  store i64 4, ptr %1, align 8
  %2 = getelementptr inbounds %struct.kmp_depend_info, ptr %.dep.arr.addr, i32 0, i32 2
  store i8 1, ptr %2, align 8
  store i64 1, ptr %dep.counter.addr, align 8
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"(),
    "QUAL.OMP.NOWAIT"(),
    "QUAL.OMP.DEPARRAY"(i32 1, ptr %.dep.arr.addr) ]

  fence acq_rel
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASKWAIT"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
