; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefixes=VER0,ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefixes=VER0,ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefixes=VER1,ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefixes=VER1,ALL

; // C++ source:
; // #include <stdio.h>
; void __attribute__((nothrow,noinline))  foo_gpu(int aaa, int* bbb) {
;   // printf("\n *** VARIANT FUNCTION ***\n");
; }
; #pragma omp declare variant(foo_gpu) match(construct={dispatch}, device={arch(gen)})
; void __attribute__((nothrow,noinline))  foo(int aaa, int* bbb) {
;   // printf("\n *** BASE FUNCTION ***\n");
; }
; int main() {
;   int *ptr;
;   int ccc, ddd;
;   #pragma omp dispatch device(0) depend(in:ccc) depend(out:ddd)
;     foo(0,ptr);
;   return 0;
; }
;
; VER0: call ptr @__kmpc_omp_task_alloc(ptr @.kmpc_loc{{.*}}, i32 %{{.*}}, i32 0, i64 0, i64 0, ptr null)
; VER1-NOT:  call ptr @__kmpc_omp_task_alloc
; ALL:  call void @__kmpc_omp_wait_deps(ptr @.kmpc_loc{{.*}}, i32 %{{.*}}, i32 2, ptr %{{.*}}, i32 0, ptr null)
; VER0: call void @__kmpc_omp_task_begin_if0(ptr @.kmpc_loc{{.*}}, i32 %{{.*}}, ptr %{{.*}})
; ALL:  call void @_Z7foo_gpuiPi(i32 0, ptr %{{.*}})
; VER0: call void @__kmpc_omp_task_complete_if0(ptr @.kmpc_loc{{.*}}, i32 %{{.*}}, ptr %{{.*}})
; VER1: %current.task = call ptr @__kmpc_get_current_task(i32 %my.tid)
; VER1: call void @__tgt_target_sync(ptr @.kmpc_loc{{.*}}, i32 %my.tid, ptr %current.task, ptr null)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.kmp_depend_info = type { i64, i64, i8 }

define dso_local void @_Z7foo_gpuiPi(i32 noundef %aaa, ptr noundef %bbb) {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca ptr, align 8
  store i32 %aaa, ptr %aaa.addr, align 4
  store ptr %bbb, ptr %bbb.addr, align 8
  ret void
}

define dso_local void @_Z3fooiPi(i32 noundef %aaa, ptr noundef %bbb) #1 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca ptr, align 8
  store i32 %aaa, ptr %aaa.addr, align 4
  store ptr %bbb, ptr %bbb.addr, align 8
  ret void
}

define dso_local noundef i32 @main() {
entry:
  %retval = alloca i32, align 4
  %ptr = alloca ptr, align 8
  %ccc = alloca i32, align 4
  %ddd = alloca i32, align 4
  %.dep.arr.addr = alloca [2 x %struct.kmp_depend_info], align 8
  %dep.counter.addr = alloca i64, align 8
  store i32 0, ptr %retval, align 4
  %0 = getelementptr inbounds [2 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  %1 = ptrtoint ptr %ccc to i64
  %2 = getelementptr %struct.kmp_depend_info, ptr %0, i64 0
  %3 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 0
  store i64 %1, ptr %3, align 8
  %4 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 1
  store i64 4, ptr %4, align 8
  %5 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 2
  store i8 1, ptr %5, align 8
  %6 = ptrtoint ptr %ddd to i64
  %7 = getelementptr %struct.kmp_depend_info, ptr %0, i64 1
  %8 = getelementptr inbounds %struct.kmp_depend_info, ptr %7, i32 0, i32 0
  store i64 %6, ptr %8, align 8
  %9 = getelementptr inbounds %struct.kmp_depend_info, ptr %7, i32 0, i32 1
  store i64 4, ptr %9, align 8
  %10 = getelementptr inbounds %struct.kmp_depend_info, ptr %7, i32 0, i32 2
  store i8 3, ptr %10, align 8
  store i64 2, ptr %dep.counter.addr, align 8
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.IF"(i32 0),
    "QUAL.OMP.IMPLICIT"(),
    "QUAL.OMP.DEPARRAY"(i32 2, ptr %0),
    "QUAL.OMP.SHARED:TYPED"(ptr %ptr, ptr null, i32 1) ]

  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0) ]

  %13 = load ptr, ptr %ptr, align 8
  call void @_Z3fooiPi(i32 noundef 0, ptr noundef %13) [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.DISPATCH"() ]

  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.TASK"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #1 = { "openmp-variant"="name:_Z7foo_gpuiPi;construct:dispatch;arch:gen" }
