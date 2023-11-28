; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefixes=VER0,ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefixes=VER0,ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefixes=VER1,ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefixes=VER1,ALL

; // C++ source:
; #include <omp.h>
; void __attribute__((nothrow,noinline))  foo_gpu(int aaa, int *bbb, omp_interop_t interop) {
;   // printf("\n *** VARIANT FUNCTION (NOWAIT) ***\n");
; }
; #pragma omp declare variant(foo_gpu) match(construct={dispatch}, device={arch(gen)}) \
;                                      append_args(interop(targetsync))
; void __attribute__((nothrow,noinline))  foo(int aaa, int* bbb) {
;   // printf("\n *** BASE FUNCTION ***\n");
; }
; int main() {
;   int *ptr;
;   #pragma omp dispatch device(0)
;     foo(0,ptr);
;   return 0;
; }
;
; Version=0:
; This test is similar to the synchronous case of target variant dispatch, and and we get the
; interop obj by calling __tgt_create_interop_obj(device, false /*is_async*/, null /*async_obj*/)
; VER0: [[INTEROPOBJ:%[^ ]+]] = call ptr @__tgt_create_interop_obj(i64 0, i8 0, ptr null)
;
; Version=1:
; VER1: %current.task = call ptr @__kmpc_get_current_task(i32 %my.tid)
; VER1: [[INTEROPOBJ:%[^ ]+]] = call ptr @__tgt_get_interop_obj(ptr @.kmpc_loc{{.*}}, i32 1, i32 0, ptr null, i64 0, i32 %my.tid, ptr %current.task)
; ALL:  call void @_Z7foo_gpuiPiPv(i32 0, ptr %{{.*}}, ptr [[INTEROPOBJ]])
; VER1: call void @__tgt_target_sync(ptr @.kmpc_loc{{.*}}, i32 %my.tid, ptr %current.task, ptr null)
; When the interop clause is not specified, we call __tgt_target_sync instead of __tgt_interop_use_async
; VER1-NOT: __tgt_interop_use_async

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z7foo_gpuiPiPv(i32 noundef %aaa, ptr noundef %bbb, ptr noundef %interop) {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca ptr, align 8
  %interop.addr = alloca ptr, align 8
  store i32 %aaa, ptr %aaa.addr, align 4
  store ptr %bbb, ptr %bbb.addr, align 8
  store ptr %interop, ptr %interop.addr, align 8
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
  store i32 0, ptr %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0) ]

  %1 = load ptr, ptr %ptr, align 8
  call void @_Z3fooiPi(i32 noundef 0, ptr noundef %1) [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISPATCH"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #1 = { "openmp-variant"="name:_Z7foo_gpuiPiPv;construct:dispatch;arch:gen;interop:targetsync" }
